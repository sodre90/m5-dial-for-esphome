#pragma once
#include "ha_device.h"
#include "ha_device_mode_scene_select.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceScenes: public esphome::shys_m5_dial::HaDevice {
            protected:
                HaDeviceModeSceneSelect*   modeSceneSelect   = new HaDeviceModeSceneSelect(*this);

            public:
                HaDeviceScenes(const std::string& entity_id, const std::string& name, const std::string& modes) : HaDevice(entity_id, name, modes) {}

                void init() override {
                    ESP_LOGD("HA_DEVICE", "Init Scenes: %s", this->getEntityId().c_str());

                    modeSceneSelect->loadScenes(this->getModeConfig());
                    this->addMode(modeSceneSelect);
                }

        };

    }
}
