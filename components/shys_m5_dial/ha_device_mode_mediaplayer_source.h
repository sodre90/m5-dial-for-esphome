#pragma once

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeMediaPlayerSource: public esphome::shys_m5_dial::HaDeviceMode {
            protected:
                struct MediaContent {
                    std::string name;
                    std::string content_id;
                    std::string content_type;
                };

                std::vector<MediaContent> mediaContents = {};

                void sendValueToHomeAssistant(int value) override {
                    // not choose Playlist on change value.
                    // selection activated by button pressed
                }

                void showSourceSelection(M5DialDisplay& display){
                    std::vector<std::string> names;
                    for(const MediaContent& content : this->mediaContents){
                        names.push_back(content.name);
                    }

                    drawSelectionMenu(display, names, nullptr);
                }

            public:
                HaDeviceModeMediaPlayerSource(HaDevice& device) : HaDeviceMode(device){
                    this->setMaxValue(0);
                }

                void refreshDisplay(M5DialDisplay& display, bool init) override {
                    this->showSourceSelection(display);
                    ESP_LOGD("DISPLAY", "Source-Modus");

                    this->displayRefreshNeeded = false;
                }

                void registerHAListener() override {
                }

                bool onRotary(M5DialDisplay& display, const char * direction) override {
                    this->defaultOnRotary(display, direction);
                    return true;
                }

                bool onButton(M5DialDisplay& display, const char * clickType) override {
                    if (strcmp(clickType, BUTTON_SHORT)==0 && !this->mediaContents.empty()){
                        const MediaContent& content = this->mediaContents[this->getValue()];

                        haApi.playMediaOnMediaPlayer(this->device.getEntityId(), content.content_id, content.content_type);

                        delay(300);
                        haApi.updateEntity(this->device.getEntityId().c_str());

                        return true;
                    }

                    return false;
                }

               /**
                *
                */
                void loadMediaContents(JsonObject modeConfig){
                    this->mediaContents = {};

                    if (modeConfig["source_mode"].is<JsonObject>()) {
                        JsonObject sourceModeConfig = modeConfig["source_mode"];

                        if (sourceModeConfig["sources"].is<JsonArray>()) {
                            JsonArray sources = sourceModeConfig["sources"].as<JsonArray>();

                            for(JsonObject source : sources){
                                MediaContent content;
                                content.name = source["name"].as<std::string>();
                                content.content_id = source["content_id"].as<std::string>();
                                content.content_type = source["content_type"].as<std::string>();

                                ESP_LOGD("MEDIAPLAYER", "Add Source: %s on %s", content.name.c_str(), this->device.getEntityId().c_str());
                                this->mediaContents.push_back(std::move(content));
                            }
                            this->setMaxValue(this->mediaContents.empty() ? 0 : this->mediaContents.size()-1);
                        }
                    }
                }

        };
    }
}
