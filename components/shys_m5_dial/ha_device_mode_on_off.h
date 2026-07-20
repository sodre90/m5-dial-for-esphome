#pragma once
#include "ha_device_mode.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeOnOff: public esphome::shys_m5_dial::HaDeviceMode {
            protected:
                virtual void turnOn() = 0;
                virtual void turnOff() = 0;
                virtual void toggle() = 0;

                void sendValueToHomeAssistant(int value) override {
                    if(getValue()<=0){
                        turnOff();
                    } else {
                        turnOn();
                    }
                }

                void drawStateRing(M5DialDisplay& display, uint16_t ringColor){
                    for(int i=0;i<360;i+=2){
                        display.drawColorCircleLine(i, 130, 134, ringColor);
                    }
                }

                void showOnOffMenu(M5DialDisplay& display){
                    bool isOn = getValue() > 0;
                    uint16_t accent = display.getAccentColor();

                    drawMenuFrame(display, M5DialDisplay::THEME_BG, [this, &display, accent, isOn](LovyanGFX* gfx, uint16_t width, uint16_t height){
                        drawStateRing(display, isOn ? accent : M5DialDisplay::THEME_TRACK);

                        gfx->setTextColor(isOn ? accent : M5DialDisplay::THEME_TEXT_MUTED);
                        display.setFontsize(3);
                        gfx->drawString(isOn?"on":"off",
                                        width / 2,
                                        height / 2 - 30);

                        gfx->setTextColor(M5DialDisplay::THEME_TEXT_MUTED);
                        display.setFontsize(1);
                        gfx->drawString(this->device.getName().c_str(),
                                        width / 2,
                                        height / 2 + 20);
                        gfx->drawString("On/Off",
                                        width / 2,
                                        height / 2 + 50);
                    });
                }

            public:
                HaDeviceModeOnOff(HaDevice& device) : HaDeviceMode(device){}

                void refreshDisplay(M5DialDisplay& display, bool init) override {
                    this->showOnOffMenu(display);
                    ESP_LOGD("DISPLAY", "An/Aus-Modus");
                }

                void registerHAListener() override {
                    subscribeHaState(optional<std::string>(), [this](const std::string &state) {
                        this->setReceivedValue(strcmp("on", state.c_str())==0 ? 1 : 0);
                        ESP_LOGI("HA_API", "Got value %s for %s", state.c_str(), this->device.getEntityId().c_str());
                    });
                }

                bool onTouch(M5DialDisplay& display, uint16_t x, uint16_t y) override {
                    toggle();
                    return true;
                }

                bool onRotary(M5DialDisplay& display, const char * direction) override {
                    if(strcmp(direction, ROTARY_LEFT)==0){
                        turnOff();
                    } else if(strcmp(direction, ROTARY_RIGHT)==0){
                        turnOn();
                    }

                    return true;
                }

                bool onButton(M5DialDisplay& display, const char * clickType) override {
                    if (strcmp(clickType, BUTTON_SHORT)==0){
                        toggle();
                        return true;
                    }
                    return false;
                }
        };
    }
}
