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
                esphome::text_sensor::TextSensor* weatherConditionSensor = nullptr;

                uint8_t lastHour = 255;
                uint8_t lastMinute = 255;
                bool lastTemperatureKnown = false;
                float lastTemperature = 0;
                std::string lastCondition = "";

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

                bool hasWeatherCondition(){
                    return this->weatherConditionSensor != nullptr && this->weatherConditionSensor->has_state();
                }

                bool conditionChanged(){
                    if(!this->hasWeatherCondition()){
                        return !this->lastCondition.empty();
                    }
                    return this->weatherConditionSensor->state != this->lastCondition;
                }

                // Weather-condition icons are drawn from primitives (fillCircle/fillArc/
                // fillTriangle/fillRect), the same techniques already used for the analog
                // clock face and hands, rather than pre-rendered bitmaps.
                void drawSun(LovyanGFX* gfx, int cx, int cy, int r, uint16_t color){
                    gfx->fillCircle(cx, cy, r, color);
                    int rayInner = r + 3;
                    int rayOuter = r + r / 2 + 3;
                    for(int angle = 0; angle < 360; angle += 45){
                        gfx->fillArc(cx, cy, rayOuter, rayInner, angle - 8, angle + 8, color);
                    }
                }

                void drawMoon(LovyanGFX* gfx, int cx, int cy, int r, uint16_t color){
                    gfx->fillCircle(cx, cy, r, color);
                    gfx->fillCircle(cx + r / 2, cy - (int)(r * 0.35), (int)(r * 0.85), BLACK);
                }

                void drawCloud(LovyanGFX* gfx, int cx, int cy, int r, uint16_t color){
                    int baseW = (int)(r * 1.8);
                    int baseH = (int)(r * 0.55);
                    gfx->fillRect(cx - baseW / 2, cy, baseW, baseH, color);
                    gfx->fillCircle(cx - r / 2, cy, (int)(r * 0.55), color);
                    gfx->fillCircle(cx + (int)(r * 0.1), cy - (int)(r * 0.25), (int)(r * 0.7), color);
                    gfx->fillCircle(cx + r, cy, (int)(r * 0.5), color);
                }

                void drawDrops(LovyanGFX* gfx, int cx, int cy, int r, int count, uint16_t color){
                    int spacing = (int)(r * 0.7);
                    int startX = cx - (spacing * (count - 1)) / 2;
                    for(int i = 0; i < count; i++){
                        int x = startX + i * spacing;
                        gfx->drawLine(x, cy, x - 4, cy + (int)(r * 0.6), color);
                        gfx->drawLine(x + 1, cy, x - 3, cy + (int)(r * 0.6), color);
                    }
                }

                void drawFlakes(LovyanGFX* gfx, int cx, int cy, int r, int count, uint16_t color){
                    int spacing = (int)(r * 0.7);
                    int startX = cx - (spacing * (count - 1)) / 2;
                    for(int i = 0; i < count; i++){
                        gfx->fillCircle(startX + i * spacing, cy + (int)(r * 0.35), 2, color);
                    }
                }

                void drawBolt(LovyanGFX* gfx, int cx, int cy, int r, uint16_t color){
                    int x0 = cx + 2, y0 = cy;
                    int x1 = cx - 6, y1 = cy + (int)(r * 0.55);
                    int x2 = cx + 3, y2 = cy + (int)(r * 0.55);
                    int x3 = cx - 4, y3 = cy + r;
                    gfx->fillTriangle(x0, y0, x1, y1, x2, y2, color);
                    gfx->fillTriangle(x1, y1, x2, y2, x3, y3, color);
                }

                void drawFog(LovyanGFX* gfx, int cx, int cy, int r, uint16_t color){
                    int w = (int)(r * 1.8);
                    int gap = (int)(r * 0.4);
                    for(int i = -1; i <= 1; i++){
                        int lineW = (i == 0) ? w : (int)(w * 0.7);
                        gfx->fillRect(cx - lineW / 2, cy + i * gap, lineW, 3, color);
                    }
                }

                void drawWind(LovyanGFX* gfx, int cx, int cy, int r, uint16_t color){
                    int gap = (int)(r * 0.45);
                    int lengths[3] = {(int)(r * 1.6), (int)(r * 1.9), (int)(r * 1.3)};
                    for(int i = 0; i < 3; i++){
                        int y = cy - gap + i * gap;
                        gfx->fillRect(cx - lengths[i] / 2, y, lengths[i], 3, color);
                    }
                }

                void drawWeatherIcon(LovyanGFX* gfx, const std::string& condition, int cx, int cy, int r){
                    uint16_t sunColor   = M5Dial.Display.color565(255, 214, 0);
                    uint16_t cloudColor = M5DialDisplay::THEME_TEXT_MUTED;
                    uint16_t rainColor  = M5Dial.Display.color565(90, 160, 255);
                    uint16_t boltColor  = M5Dial.Display.color565(255, 196, 0);
                    uint16_t moonColor  = M5Dial.Display.color565(210, 210, 225);

                    if(condition == "sunny"){
                        drawSun(gfx, cx, cy, r, sunColor);
                    } else if(condition == "clear-night"){
                        drawMoon(gfx, cx, cy, r, moonColor);
                    } else if(condition == "partlycloudy"){
                        drawSun(gfx, cx - r / 3, cy - r / 3, (int)(r * 0.55), sunColor);
                        drawCloud(gfx, cx + r / 5, cy + r / 6, (int)(r * 0.8), cloudColor);
                    } else if(condition == "pouring"){
                        drawCloud(gfx, cx, cy - r / 6, r, cloudColor);
                        drawDrops(gfx, cx, cy + (int)(r * 0.4), r, 4, rainColor);
                    } else if(condition == "rainy" || condition == "hail"){
                        drawCloud(gfx, cx, cy - r / 6, r, cloudColor);
                        drawDrops(gfx, cx, cy + (int)(r * 0.4), r, 3, rainColor);
                    } else if(condition == "lightning" || condition == "lightning-rainy"){
                        drawCloud(gfx, cx, cy - r / 6, r, cloudColor);
                        drawBolt(gfx, cx, cy + (int)(r * 0.3), r, boltColor);
                    } else if(condition == "snowy" || condition == "snowy-rainy"){
                        drawCloud(gfx, cx, cy - r / 6, r, cloudColor);
                        drawFlakes(gfx, cx, cy + (int)(r * 0.4), r, 3, WHITE);
                    } else if(condition == "fog"){
                        drawFog(gfx, cx, cy, r, cloudColor);
                    } else if(condition == "windy" || condition == "windy-variant"){
                        drawWind(gfx, cx, cy, r, cloudColor);
                    } else {
                        // cloudy, exceptional, or an unrecognized condition string
                        drawCloud(gfx, cx, cy, r, cloudColor);
                    }
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

                    bool conditionKnown = this->hasWeatherCondition();
                    if(conditionKnown){
                        this->drawWeatherIcon(gfx, this->weatherConditionSensor->state, centerX, centerY - 65, 22);
                        this->lastCondition = this->weatherConditionSensor->state;
                    } else {
                        this->lastCondition = "";
                    }

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

                    if(init || timeChanged || this->temperatureChanged() || this->conditionChanged()){
                        this->draw(display);
                    }
                }

                void setLocalTime(esphome::time::RealTimeClock* time_comp) {
                    this->localTime = time_comp;
                }

                void setOutdoorTemperatureSensor(esphome::sensor::Sensor* sensor) {
                    this->outdoorTemperatureSensor = sensor;
                }

                void setWeatherConditionSensor(esphome::text_sensor::TextSensor* sensor) {
                    this->weatherConditionSensor = sensor;
                }
        };

    }
}
