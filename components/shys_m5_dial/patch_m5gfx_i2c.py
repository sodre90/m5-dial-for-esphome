"""PlatformIO pre-build patch: keep M5GFX@0.2.6 off the legacy I2C driver.

M5GFX must stay pinned to 0.2.6 (0.2.7+ needs esp_lcd headers ESPHome's
Arduino build doesn't expose - see the lib_deps comment in shys-m5-dial.yaml).
0.2.6's only call into the legacy `driver/i2c.h` API is a single i2c_set_pin()
helper. Arduino-esp32's own Wire library, on the ESP-IDF version this project
currently builds against, uses the new i2c_master ("driver_ng") driver. ESP-IDF
aborts at boot if both drivers are linked into the same firmware (see
components/driver/i2c/i2c.c's check_i2c_driver_conflict, a __attribute__
((constructor)) function). This script rewrites that one call site to do the
equivalent GPIO-matrix pin setup directly, so the legacy driver is never
linked in at all.
"""

import os

Import("env")

PATCH_MARKER = "// shys_m5_dial: i2c_master pin-setup patch"

OLD_CALLS = [
    "i2c_set_pin((i2c_port_t)i2c_port, sda_io, scl_io, gpio_pullup_t::GPIO_PULLUP_ENABLE, gpio_pullup_t::GPIO_PULLUP_ENABLE, I2C_MODE_MASTER);",
    "i2c_set_pin((i2c_port_t)i2c_port, i2c_context[i2c_port].pin_sda, i2c_context[i2c_port].pin_scl, gpio_pullup_t::GPIO_PULLUP_ENABLE, gpio_pullup_t::GPIO_PULLUP_ENABLE, I2C_MODE_MASTER);",
]

NEW_CALLS = [
    "shys_m5_dial_i2c_set_pin(i2c_port, sda_io, scl_io);",
    "shys_m5_dial_i2c_set_pin(i2c_port, i2c_context[i2c_port].pin_sda, i2c_context[i2c_port].pin_scl);",
]

HELPER_FUNCTION = f"""{PATCH_MARKER}
#if __has_include(<driver/i2c_master.h>)
#include <esp_rom_gpio.h>
#include <soc/i2c_periph.h>
static void shys_m5_dial_i2c_set_pin(int i2c_num, gpio_num_t pin_sda, gpio_num_t pin_scl)
{{
  if ((int8_t)pin_sda >= 0) {{
    gpio_set_level(pin_sda, true);
    gpio_iomux_out(pin_sda, PIN_FUNC_GPIO, false);
    gpio_set_direction(pin_sda, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(pin_sda, GPIO_PULLUP_ONLY);
    esp_rom_gpio_connect_out_signal(pin_sda, i2c_periph_signal[i2c_num].sda_out_sig, 0, 0);
    esp_rom_gpio_connect_in_signal(pin_sda, i2c_periph_signal[i2c_num].sda_in_sig, 0);
  }}
  if ((int8_t)pin_scl >= 0) {{
    gpio_set_level(pin_scl, true);
    gpio_iomux_out(pin_scl, PIN_FUNC_GPIO, false);
    gpio_set_direction(pin_scl, GPIO_MODE_INPUT_OUTPUT_OD);
    esp_rom_gpio_connect_out_signal(pin_scl, i2c_periph_signal[i2c_num].scl_out_sig, 0, 0);
    esp_rom_gpio_connect_in_signal(pin_scl, i2c_periph_signal[i2c_num].scl_in_sig, 0);
    gpio_set_pull_mode(pin_scl, GPIO_PULLUP_ONLY);
  }}
}}
#else
static void shys_m5_dial_i2c_set_pin(int i2c_num, gpio_num_t pin_sda, gpio_num_t pin_scl)
{{
  i2c_set_pin((i2c_port_t)i2c_num, pin_sda, pin_scl, gpio_pullup_t::GPIO_PULLUP_ENABLE, gpio_pullup_t::GPIO_PULLUP_ENABLE, I2C_MODE_MASTER);
}}
#endif
"""

INSERT_AFTER = "static i2c_dev_t* getDev(int num)"


def find_common_cpp():
    libdeps_dir = env.subst("$PROJECT_LIBDEPS_DIR")
    pioenv = env["PIOENV"]
    path = os.path.join(
        libdeps_dir, pioenv, "M5GFX@0.2.6", "src", "lgfx", "v1",
        "platforms", "esp32", "common.cpp",
    )
    return path


def patch_file(path):
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()

    if PATCH_MARKER in content:
        return  # already patched

    if INSERT_AFTER not in content:
        raise RuntimeError(
            f"shys_m5_dial i2c patch: anchor '{INSERT_AFTER}' not found in {path}. "
            "M5GFX@0.2.6 source may have changed; update patch_m5gfx_i2c.py."
        )

    anchor_idx = content.index(INSERT_AFTER)
    insert_idx = content.index("\n", anchor_idx) + 1
    # Skip past the getDev function body to insert after its closing brace.
    brace_depth = 0
    i = content.index("{", insert_idx)
    brace_depth = 1
    i += 1
    while brace_depth > 0:
        if content[i] == "{":
            brace_depth += 1
        elif content[i] == "}":
            brace_depth -= 1
        i += 1
    insert_idx = i + 1

    content = content[:insert_idx] + "\n" + HELPER_FUNCTION + content[insert_idx:]

    for old_call, new_call in zip(OLD_CALLS, NEW_CALLS):
        if old_call not in content:
            raise RuntimeError(
                f"shys_m5_dial i2c patch: expected call site not found in {path}:\n{old_call}\n"
                "M5GFX@0.2.6 source may have changed; update patch_m5gfx_i2c.py."
            )
        content = content.replace(old_call, new_call)

    with open(path, "w", encoding="utf-8") as f:
        f.write(content)

    print(f"shys_m5_dial: patched {path} to avoid legacy I2C driver conflict")


common_cpp = find_common_cpp()
if os.path.exists(common_cpp):
    patch_file(common_cpp)
else:
    print(f"shys_m5_dial: M5GFX common.cpp not found yet at {common_cpp}, skipping i2c patch")
