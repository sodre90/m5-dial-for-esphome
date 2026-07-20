#pragma once

// Definitions of HaDeviceMode members that need the complete HaDevice type.
// Included at the end of ha_device.h - do not include directly.

namespace esphome
{
    namespace shys_m5_dial
    {
        inline void HaDeviceMode::subscribeHaState(optional<std::string> attribute,
                                                   std::function<void(const std::string&)> handler,
                                                   bool skipWhenModified){
            api::global_api_server->subscribe_home_assistant_state(
                        this->device.getEntityId(),
                        std::move(attribute),
                        std::function<void(const std::string &)>(
                            [this, skipWhenModified, handler = std::move(handler)](const std::string &state) {
                if(skipWhenModified && this->isValueModified()){
                    return;
                }
                handler(state);
            }));
        }

        inline void HaDeviceMode::subscribeHaNumericState(optional<std::string> attribute,
                                                          const char* valueName,
                                                          std::function<void(float)> apply){
            std::string label = valueName;
            subscribeHaState(std::move(attribute),
                             [this, label, apply = std::move(apply)](const std::string &state) {
                auto val = parse_number<float>(state);
                if (!val.has_value()) {
                    apply(0);
                    ESP_LOGD("HA_API", "No %s value in %s for %s", label.c_str(), state.c_str(), this->device.getEntityId().c_str());
                } else {
                    apply(val.value());
                    ESP_LOGI("HA_API", "Got %s value %f for %s", label.c_str(), val.value(), this->device.getEntityId().c_str());
                }
            });
        }

        inline void HaDeviceMode::drawSelectionMenu(M5DialDisplay& display,
                                                    const std::vector<std::string>& items,
                                                    const char* footerLabel){
            drawMenuFrame(display, display.getBackgroundColor(),
                          [this, &display, &items, footerLabel](LovyanGFX* gfx, uint16_t width, uint16_t height){
                gfx->setTextColor(M5DialDisplay::THEME_TEXT_MUTED);
                display.setFontsize(1);
                gfx->drawString(this->device.getName().c_str(),
                                width / 2,
                                height / 2 - 80);

                if(items.empty()){
                    gfx->drawString("-", width / 2, height / 2);
                    return;
                }

                int current = this->getValue();
                if(current > (int)items.size() - 1){
                    current = items.size() - 1;
                }

                if(current > 0){
                    display.setFontsize(.7);
                    gfx->drawString(items[current-1].c_str(),
                                    width / 2,
                                    height / 2 - 40);
                }

                gfx->setTextColor(display.getAccentColor());
                display.setFontsize(1.5);
                gfx->drawString(items[current].c_str(),
                                width / 2,
                                height / 2);
                gfx->setTextColor(M5DialDisplay::THEME_TEXT_MUTED);

                if(current < (int)items.size() - 1){
                    display.setFontsize(.7);
                    gfx->drawString(items[current+1].c_str(),
                                    width / 2,
                                    height / 2 + 40);
                }

                if(footerLabel != nullptr){
                    display.setFontsize(1);
                    gfx->drawString(footerLabel,
                                    width / 2,
                                    height / 2 + 80);
                }
            });
        }
    }
}
