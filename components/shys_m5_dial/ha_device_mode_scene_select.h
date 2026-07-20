#pragma once
#include "ha_device_mode.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeSceneSelect: public esphome::shys_m5_dial::HaDeviceMode {
            protected:
                struct SceneEntry {
                    std::string entity_id;
                    std::string name;
                    uint16_t color;
                };

                static const uint16_t buttonRadius = 26;
                static const uint16_t ringRadius = 78;

                std::vector<SceneEntry> scenes = {};

                void sendValueToHomeAssistant(int value) override {
                    // scenes are activated by touch or button press only
                }

                uint16_t getButtonCenterX(uint16_t cx, int index){
                    return cx + ringRadius * cos(getButtonAngle(index));
                }

                uint16_t getButtonCenterY(uint16_t cy, int index){
                    return cy + ringRadius * sin(getButtonAngle(index));
                }

                float getButtonAngle(int index){
                    return (2.0f * M_PI * index / this->scenes.size()) - (M_PI / 2.0f);
                }

                void activateScene(int index){
                    M5Dial.Speaker.tone(5000, 20);
                    haApi.activateScene(this->scenes[index].entity_id);
                }

                void showSceneMenu(M5DialDisplay& display){
                    drawMenuFrame(display, display.getBackgroundColor(),
                                  [this, &display](LovyanGFX* gfx, uint16_t width, uint16_t height){
                        uint16_t cx = width / 2;
                        uint16_t cy = height / 2;

                        if(this->scenes.empty()){
                            gfx->setTextColor(M5DialDisplay::THEME_TEXT_MUTED);
                            display.setFontsize(1);
                            gfx->drawString("No scenes", cx, cy);
                            return;
                        }

                        int selected = this->getValue();

                        for(int i = 0; i < (int)this->scenes.size(); i++){
                            const SceneEntry& scene = this->scenes[i];
                            uint16_t x = getButtonCenterX(cx, i);
                            uint16_t y = getButtonCenterY(cy, i);
                            uint16_t radius = (i == selected) ? buttonRadius + 4 : buttonRadius;

                            display.drawLayeredButton(x, y, radius, scene.color);
                            if(i == selected){
                                gfx->drawCircle(x, y, radius + 3, display.getAccentColor());
                            }

                            display.setFontsize(1);
                            gfx->setTextColor(M5DialDisplay::getContrastColor(scene.color));
                            gfx->drawString(scene.name.substr(0, 1).c_str(), x, y);
                        }

                        gfx->setTextColor(M5DialDisplay::getContrastColor(display.getBackgroundColor()));
                        display.setFontsize(1);
                        gfx->drawString(this->scenes[selected].name.c_str(), cx, cy);
                    });
                }

            public:
                HaDeviceModeSceneSelect(HaDevice& device) : HaDeviceMode(device){
                    this->setMaxValue(0);
                }

                void refreshDisplay(M5DialDisplay& display, bool init) override {
                    this->showSceneMenu(display);
                    ESP_LOGD("DISPLAY", "Scene-Modus");

                    this->displayRefreshNeeded = false;
                }

                void registerHAListener() override {
                }

                bool onRotary(M5DialDisplay& display, const char * direction) override {
                    if(this->scenes.empty()){
                        return true;
                    }

                    int count = this->scenes.size();
                    if (strcmp(direction, ROTARY_LEFT)==0){
                        this->setReceivedValue((this->getValue() + count - 1) % count);
                    } else if (strcmp(direction, ROTARY_RIGHT)==0){
                        this->setReceivedValue((this->getValue() + 1) % count);
                    }

                    this->displayRefreshNeeded = true;
                    return true;
                }

                bool onButton(M5DialDisplay& display, const char * clickType) override {
                    if (strcmp(clickType, BUTTON_SHORT)==0 && !this->scenes.empty()){
                        this->activateScene(this->getValue());
                        return true;
                    }

                    return false;
                }

                bool onTouch(M5DialDisplay& display, uint16_t x, uint16_t y) override {
                    uint16_t cx = display.getWidth() / 2;
                    uint16_t cy = display.getHeight() / 2;

                    for(int i = 0; i < (int)this->scenes.size(); i++){
                        float dx = (float)x - getButtonCenterX(cx, i);
                        float dy = (float)y - getButtonCenterY(cy, i);

                        if(sqrt(dx*dx + dy*dy) <= buttonRadius + 6){
                            this->setReceivedValue(i);
                            this->displayRefreshNeeded = true;
                            this->activateScene(i);
                            return true;
                        }
                    }

                    return false;
                }

               /**
                *
                */
                void loadScenes(JsonObject modeConfig){
                    static const uint16_t palette[] = {RED, ORANGE, YELLOW, DARKGREEN, CYAN, BLUE, MAGENTA, OLIVE};
                    static const int paletteSize = sizeof(palette) / sizeof(palette[0]);

                    this->scenes = {};

                    if (modeConfig["scenes"].is<JsonArray>()) {
                        JsonArray sceneList = modeConfig["scenes"].as<JsonArray>();

                        for(JsonObject sceneConfig : sceneList){
                            SceneEntry scene;
                            scene.entity_id = sceneConfig["entity"].as<std::string>();
                            scene.name = sceneConfig["name"].as<std::string>();
                            scene.color = palette[this->scenes.size() % paletteSize];

                            ESP_LOGD("SCENE", "Add Scene: %s (%s)", scene.name.c_str(), scene.entity_id.c_str());
                            this->scenes.push_back(std::move(scene));
                        }
                        this->setMaxValue(this->scenes.empty() ? 0 : this->scenes.size()-1);
                    }
                }

        };
    }
}
