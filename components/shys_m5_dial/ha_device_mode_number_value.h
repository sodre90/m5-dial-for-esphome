#pragma once
#include "ha_device_mode_percentage.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeNumberValue: public esphome::shys_m5_dial::HaDeviceModePercentage {
            protected:
                void sendValueToHomeAssistant(int value) override {
                    haApi.setNumberValue(this->device.getEntityId(), value);
                }

            public:
                HaDeviceModeNumberValue(HaDevice& device) : HaDeviceModePercentage(device){
                    this->setLabel("Value");
                    this->setUnit("");
                    this->setIcon(COUNTER_IMG, 4900);
                }

                void registerHAListener() override {
                    subscribeHaNumericState(optional<std::string>(), "Number", [this](float val) {
                        this->setReceivedValue(val);
                    });

                    subscribeHaNumericState(optional<std::string>("min"), "min", [this](float val) {
                        this->setMinValue(val);
                    });

                    subscribeHaNumericState(optional<std::string>("max"), "max", [this](float val) {
                        this->setMaxValue(val);
                    });
                }

                bool onButton(M5DialDisplay& display, const char * clickType) override {
                    if (strcmp(clickType, BUTTON_SHORT)==0){
                        if(this->getValue() > 50){
                            this->setValue(0);
                        } else {
                            this->setValue(100);
                        }
                        
                        return true;
                    } 
                    return false;
                }
        };
    }
}