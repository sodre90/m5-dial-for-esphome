#pragma once

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeMediaPlayerPlay: public esphome::shys_m5_dial::HaDeviceMode {
            protected:
                std::string player_state = "";

                std::string media_title = "";
                std::string media_artist = "";
                std::string media_album_name = "";

                uint16_t media_duration = 0;
                uint16_t media_position = 0;

                unsigned long lastRefresh = 0;

                void sendValueToHomeAssistant(int value) override {
                    haApi.setMediaPlayerVolume(this->device.getEntityId(), value);
                }

                float getMediaPositionPct(){
                    if(strcmp(this->player_state.c_str(), "playing")==0
                          && this->media_duration > 0
                          && this->media_position <= this->media_duration){
                        return (float)this->media_position / this->media_duration * 100.0f;
                    }

                    return 0;
                }

                void drawControlButton(M5DialDisplay& display, uint16_t cx, uint16_t cy, uint16_t color){
                    display.drawLayeredButton(cx, cy, 30, color);
                }

                void showPlayMenu(M5DialDisplay& display){
                    drawMenuFrame(display, display.getBackgroundColor(),
                                  [this, &display](LovyanGFX* gfx, uint16_t width, uint16_t height){

                        uint16_t cx = width / 2;
                        uint16_t cy = height / 2;

                        uint16_t accent = display.getAccentColor();

                        // Round Volume Bar
                        gfx->fillArc(cx, cy, 115, 100,
                                     150,
                                     getValue()==0?150:(((float)240 / 100) * getValue()) + 150,
                                     accent);

                        gfx->fillArc(cx, cy, 115, 100,
                                     getValue()==0?150:(((float)240 / 100) * getValue()) + 150,
                                     390,
                                     M5DialDisplay::THEME_TRACK);

                        // Percent
                        display.setFontsize(1.7);
                        gfx->drawString((String(getValue()) + "%").c_str(),
                                        cx,
                                        cy - 70);

                        // Player state
                        gfx->setTextColor(M5DialDisplay::THEME_TEXT_MUTED);
                        display.setFontsize(1);
                        gfx->drawString(this->player_state.c_str(),
                                        cx,
                                        cy - 40);

                        drawControlButton(display, cx - 80, cy, M5DialDisplay::THEME_SURFACE);
                        drawControlButton(display, cx, cy, accent);
                        drawControlButton(display, cx + 80, cy, M5DialDisplay::THEME_SURFACE);

                        uint16_t sideGlyphColor   = M5DialDisplay::getContrastColor(M5DialDisplay::THEME_SURFACE);
                        uint16_t centerGlyphColor = M5DialDisplay::getContrastColor(accent);

                        if(strcmp(this->player_state.c_str(), "playing") == 0){
                            // Pause glyph
                            gfx->fillRect(cx-10, cy-12, 7, 24, centerGlyphColor);
                            gfx->fillRect(cx+3, cy-12, 7, 24, centerGlyphColor);
                        } else {
                            // Play glyph
                            gfx->fillTriangle(cx-8, cy-12, cx-8, cy+12, cx+12, cy, centerGlyphColor);
                        }

                        // FWD glyph
                        gfx->fillTriangle(cx+80-11, cy-9, cx+80-11, cy+9, cx+80+1, cy, sideGlyphColor);
                        gfx->fillTriangle(cx+80-1, cy-9, cx+80-1, cy+9, cx+80+11, cy, sideGlyphColor);

                        // PREV glyph
                        gfx->fillTriangle(cx-80+11, cy-9, cx-80+11, cy+9, cx-80-1, cy, sideGlyphColor);
                        gfx->fillTriangle(cx-80+1, cy-9, cx-80+1, cy+9, cx-80-11, cy, sideGlyphColor);

                        // Position Bar
                        gfx->fillRect(cx-50, cy+40, 100, 5, M5DialDisplay::THEME_TRACK);
                        gfx->fillRect(cx-50, cy+40, getMediaPositionPct(), 5, accent);

                        // Media-Artist/Title
                        display.setFontsize(.7);
                        bool displayTitle = ((millis() / 5000) % 2 == 1);
                        gfx->drawString(displayTitle ? this->media_title.c_str() : this->media_artist.c_str(),
                                        cx,
                                        cy + 65);

                        // Device Name
                        display.setFontsize(1);
                        gfx->drawString(this->device.getName().c_str(),
                                        cx,
                                        cy + 90);
                    });
                }

            public:
                HaDeviceModeMediaPlayerPlay(HaDevice& device) : HaDeviceMode(device){}

                void refreshDisplay(M5DialDisplay& display, bool init) override {
                    this->showPlayMenu(display);
                    ESP_LOGD("DISPLAY", "Play-Modus");

                    this->displayRefreshNeeded = false;
                }

                void registerHAListener() override {
                    subscribeHaNumericState(optional<std::string>("volume_level"), "Volume", [this](float val) {
                        this->setReceivedValue(val*100);
                    });

                    subscribeHaState(optional<std::string>(), [this](const std::string &state) {
                        this->player_state = state;
                        this->displayRefreshNeeded = true;
                        ESP_LOGI("HA_API", "Got State %s for %s", state.c_str(), this->device.getEntityId().c_str());
                    });

                    subscribeHaState(optional<std::string>("media_title"), [this](const std::string &state) {
                        this->media_title = state;
                        this->displayRefreshNeeded = true;
                        ESP_LOGI("HA_API", "Got Title %s for %s", state.c_str(), this->device.getEntityId().c_str());
                    });

                    subscribeHaState(optional<std::string>("media_artist"), [this](const std::string &state) {
                        this->media_artist = state;
                        this->displayRefreshNeeded = true;
                        ESP_LOGI("HA_API", "Got Artist %s for %s", state.c_str(), this->device.getEntityId().c_str());
                    });

                    subscribeHaState(optional<std::string>("media_album_name"), [this](const std::string &state) {
                        this->media_album_name = state;
                        this->displayRefreshNeeded = true;
                        ESP_LOGI("HA_API", "Got Album %s for %s", state.c_str(), this->device.getEntityId().c_str());
                    });

                    subscribeHaNumericState(optional<std::string>("media_duration"), "Media-Duration", [this](float val) {
                        this->media_duration = (int)val;
                    });

                    subscribeHaNumericState(optional<std::string>("media_position"), "Media-Position", [this](float val) {
                        this->media_position = (int)val;
                        this->displayRefreshNeeded = true;
                    });
                }

                bool onTouch(M5DialDisplay& display, uint16_t x, uint16_t y) override {
                    ESP_LOGI("TOUCH", "Touch %i / %i for %s", x, y, this->device.getEntityId().c_str());

                    LovyanGFX* gfx = display.getGfx();
                    uint16_t height = gfx->height();
                    uint16_t width  = gfx->width();

                    uint16_t minHeight = height/2-30;
                    uint16_t maxHeight = height/2+30;

                    uint16_t minPrevX = width/2-110;
                    uint16_t maxPrevX = width/2-50;

                    uint16_t minPlayX = width/2-30;
                    uint16_t maxPlayX = width/2+30;

                    uint16_t minNextX = width/2+50;
                    uint16_t maxNextX = width/2+110;

                    if(y > minHeight && y < maxHeight){
                        if(x>minPrevX && x<maxPrevX){
                            M5Dial.Speaker.tone(5000, 20);
                            haApi.setPreviousTrackOnMediaPlayer(this->device.getEntityId());
                            return true;

                        } else if(x>minPlayX && x<maxPlayX){
                            M5Dial.Speaker.tone(5000, 20);
                            haApi.playPauseMediaPlayer(this->device.getEntityId());
                            return true;

                        } else if(x>minNextX && x<maxNextX){
                            M5Dial.Speaker.tone(5000, 20);
                            haApi.setNextTrackOnMediaPlayer(this->device.getEntityId());
                            return true;
                        }
                    }

                    return false;
                }

                bool onRotary(M5DialDisplay& display, const char * direction) override {
                    this->defaultOnRotary(display, direction);
                    return true;
                }

                bool onButton(M5DialDisplay& display, const char * clickType) override {
                    if (strcmp(clickType, BUTTON_SHORT)==0){
                        haApi.playPauseMediaPlayer(this->device.getEntityId());
                        return true;
                    }
                    return false;
                }

                void onLoop() override {
                    if(strcmp(this->player_state.c_str(), "playing")==0){
                        bool timebasedRefresh = (this->lastRefresh + 2000) < esphome::millis();
                        if(timebasedRefresh){
                            this->lastRefresh = esphome::millis();

                            haApi.updateEntity(this->device.getEntityId().c_str());
                        }
                    }
                }

        };
    }
}
