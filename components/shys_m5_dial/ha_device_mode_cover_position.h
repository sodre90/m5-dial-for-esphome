#pragma once
#include "ha_device_mode_percentage.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeCoverPosition: public esphome::shys_m5_dial::HaDeviceModePercentage {
            protected:
                void sendValueToHomeAssistant(int value) override {
                    haApi.setCoverPosition(this->device.getEntityId(), value);
                }

            public:
                HaDeviceModeCoverPosition(HaDevice& device) : HaDeviceModePercentage(device){
                    this->setLabel("Position");
                    this->setIcon(COVER_CLOSED_IMG, 4900);
                }

                void registerHAListener() override {
                    subscribeHaNumericState(optional<std::string>("current_position"), "Position", [this](float val) {
                        this->setReceivedValue(val);
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