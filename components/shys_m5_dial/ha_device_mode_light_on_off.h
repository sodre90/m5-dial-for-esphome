#pragma once
#include "ha_device_mode_on_off.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeLightOnOff: public esphome::shys_m5_dial::HaDeviceModeOnOff {
            protected:
                void turnOn() override {
                    haApi.turnLightOn(this->device.getEntityId());
                }

                void turnOff() override {
                    haApi.turnLightOff(this->device.getEntityId());
                }

                void toggle() override {
                    haApi.toggleLight(this->device.getEntityId());
                }

            public:
                HaDeviceModeLightOnOff(HaDevice& device) : HaDeviceModeOnOff(device){}
        };
    }
}
