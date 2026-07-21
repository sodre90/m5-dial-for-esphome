#pragma once
#include "screensaver.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class ScreensaverDigitalClock: public esphome::shys_m5_dial::Screensaver {
            protected:
                esphome::time::RealTimeClock* localTime = nullptr;
                esphome::sensor::Sensor* outdoorTemperatureSensor = nullptr;

                uint8_t lastHour = 255;
                uint8_t lastMinute = 255;
                bool lastTemperatureKnown = false;
                float lastTemperature = 0;

                bool hasOutdoorTemperature(){
                    return this->outdoorTemperatureSensor != nullptr && this->outdoorTemperatureSensor->has_state();
                }

                bool temperatureChanged(){
                    bool known = this->hasOutdoorTemperature();
                    if(known != this->lastTemperatureKnown){
                        return true;
                    }
                    if(!known){
                        return false;
                    }
                    return fabs(this->outdoorTemperatureSensor->state - this->lastTemperature) >= 0.1f;
                }

                void draw(M5DialDisplay& display){
                    auto now = this->localTime->now();
                    int centerX = display.getWidth() / 2;
                    int centerY = display.getHeight() / 2;

                    char timeStr[6];
                    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", now.hour, now.minute);

                    LovyanGFX* gfx = display.getGfx();

                    gfx->startWrite();
                    display.clear(BLACK);
                    display.applyConfiguredFont();
                    gfx->setTextDatum(middle_center);

                    gfx->setTextColor(WHITE);
                    display.setFontsize(2);
                    gfx->drawString(timeStr, centerX, centerY - 15);

                    bool temperatureKnown = this->hasOutdoorTemperature();
                    if(temperatureKnown){
                        char tempStr[16];
                        snprintf(tempStr, sizeof(tempStr), "%.1f C", this->outdoorTemperatureSensor->state);

                        gfx->setTextColor(M5DialDisplay::THEME_TEXT_MUTED);
                        display.setFontsize(1);
                        gfx->drawString(tempStr, centerX, centerY + 35);

                        this->lastTemperature = this->outdoorTemperatureSensor->state;
                    }
                    this->lastTemperatureKnown = temperatureKnown;

                    gfx->endWrite();
                    display.commit();

                    this->lastHour = now.hour;
                    this->lastMinute = now.minute;
                }

            public:
                void show(M5DialDisplay& display, bool init) override {
                    auto now = this->localTime->now();
                    bool timeChanged = now.hour != this->lastHour || now.minute != this->lastMinute;

                    if(init || timeChanged || this->temperatureChanged()){
                        this->draw(display);
                    }
                }

                void setLocalTime(esphome::time::RealTimeClock* time_comp) {
                    this->localTime = time_comp;
                }

                void setOutdoorTemperatureSensor(esphome::sensor::Sensor* sensor) {
                    this->outdoorTemperatureSensor = sensor;
                }
        };

    }
}
