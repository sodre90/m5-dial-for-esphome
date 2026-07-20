#pragma once
#include "ha_device_mode_percentage.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeClimateTemperature: public esphome::shys_m5_dial::HaDeviceModePercentage {
            protected:
                std::string hvac_mode = "none";

                std::string getHvacMode(){
                    return this->hvac_mode;
                }

                void setHvacMode(const std::string& newMode){
                    this->hvac_mode = newMode;
                }

                void sendValueToHomeAssistant(int value) override {
                    haApi.setClimateTemperature(this->device.getEntityId(), value);
                }

            public:
                HaDeviceModeClimateTemperature(HaDevice& device) : HaDeviceModePercentage(device){
                    this->maxValue = 40;
                    this->setLabel("Temperature");
                    this->setIcon(HEATER_ON_IMG, 4900);
                    this->setUnit("°");
                }

                void refreshDisplay(M5DialDisplay& display, bool init) override {
                    this->showPercentageMenu(display);
                    ESP_LOGD("DISPLAY", "Temperature-Modus");
                }

                void registerHAListener() override {
                    subscribeHaState(optional<std::string>(), [this](const std::string &state) {
                        this->setHvacMode(state);
                        ESP_LOGI("HA_API", "Got Mode %s for %s", state.c_str(), this->device.getEntityId().c_str());
                    });

                    subscribeHaNumericState(optional<std::string>("temperature"), "Temperature", [this](float val) {
                        this->setReceivedValue(int(val));
                    });
                }


                bool onTouch(M5DialDisplay& display, uint16_t x, uint16_t y) override {
                    return defaultOnTouch(display, x, y);        
                }

                bool onRotary(M5DialDisplay& display, const char * direction) override {
                    return defaultOnRotary(display, direction);
                }

                bool onButton(M5DialDisplay& display, const char * clickType) override {
                    if (strcmp(clickType, BUTTON_SHORT)==0){
                        if(strcmp(this->getHvacMode().c_str(), "off")==0){
                            haApi.turnClimateOn(this->device.getEntityId());
                        } else {
                            haApi.turnClimateOff(this->device.getEntityId());
                        }
                        
                        return true;
                    } 
                    return false;
                }

        };
    }
}