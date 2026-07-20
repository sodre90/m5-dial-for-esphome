#pragma once
#include "ha_device_mode_on_off.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeSwitchOnOff: public esphome::shys_m5_dial::HaDeviceModeOnOff {
            protected:
                void turnOn() override {
                    haApi.turnSwitchOn(this->device.getEntityId());
                }

                void turnOff() override {
                    haApi.turnSwitchOff(this->device.getEntityId());
                }

                void toggle() override {
                    haApi.toggleSwitch(this->device.getEntityId());
                }

            public:
                HaDeviceModeSwitchOnOff(HaDevice& device) : HaDeviceModeOnOff(device){}
        };
    }
}
