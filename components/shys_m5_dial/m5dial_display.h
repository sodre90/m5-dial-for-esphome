#pragma once
#include "M5Dial.h"
#include "esphome.h"
#include "default_font_16px.h"
#include "screensaver.h"

#define FF_DEFAULT &fonts::FreeSans12pt7b


/**
 * M5Dial Display
 *--------------------------
 * Display driver: GC9A01
 * Resolution: 240x240
 * Touch driver: FT3267
 */
namespace esphome
{
    namespace shys_m5_dial
    {
        class M5DialDisplay {
            public:
                static const uint16_t THEME_BG         = 0x0841;  // #080808
                static const uint16_t THEME_SURFACE    = 0x2104;  // #202020
                static const uint16_t THEME_TRACK      = 0x3186;  // #303030
                static const uint16_t THEME_TEXT_MUTED = 0x9CD3;  // #989898
                static const uint16_t THEME_GOOD       = 0x4508;  // #40A040
                static const uint16_t THEME_DANGER     = 0xE1C6;  // #E03830

            protected:
                uint16_t backgroundColor = THEME_BG;
                uint16_t accentColor = 0xFD80;                    // #FFB300 amber

                LovyanGFX* gfx = &M5Dial.Display;
                M5Canvas* canvas = nullptr;

                int timeToScreenOff = 30000;
                unsigned long lastEvent = 0;
                uint16_t lastMode = -1;

                std::string fontName = "default";  //"FreeMono12pt7b";
                float fontFactor = 1;

                int displayRotation = 2;

                Screensaver* screensaver = nullptr;
                bool screensaverRunning = false;

                std::function<void(bool)> display_refresh_action;

            public:
                M5DialDisplay() {
                }

                void init(){
                    M5Dial.Display.setRotation(displayRotation);
                    this->initCanvas();
                }

                // Full-frame canvas removes visible redraw flicker; frames are
                // composed off-screen and pushed at once by commit().
                void initCanvas(){
                    canvas = new M5Canvas(&M5Dial.Display);
                    canvas->setColorDepth(16);
                    if(canvas->createSprite(M5Dial.Display.width(), M5Dial.Display.height()) == nullptr){
                        canvas->setColorDepth(8);
                        if(canvas->createSprite(M5Dial.Display.width(), M5Dial.Display.height()) == nullptr){
                            delete canvas;
                            canvas = nullptr;
                            ESP_LOGW("DISPLAY", "Canvas allocation failed, drawing directly to panel");
                            return;
                        }
                        ESP_LOGI("DISPLAY", "Using 8-bit canvas");
                    } else {
                        ESP_LOGI("DISPLAY", "Using 16-bit canvas");
                    }
                    gfx = canvas;
                }

                void commit(){
                    if(canvas != nullptr){
                        canvas->pushSprite(0, 0);
                    }
                }

                void on_display_refresh(std::function<void(bool)> callback){
                    ESP_LOGD("DEVICE", "register on_swipe Callback");
                    this->display_refresh_action = callback;
                }

                void setTimeToScreenOff(int value){
                    this->timeToScreenOff = value;
                }

                void setRotation(int value){
                    this->displayRotation = value;
                }

                void resetLastEventTimer(){
                    lastEvent = esphome::millis();
                }

                uint16_t getHeight(){
                    return M5Dial.Display.height();
                }
                uint16_t getWidth(){
                    return M5Dial.Display.width();
                }

                LovyanGFX* getGfx() {
                    return gfx;
                }

                void setFontName(std::string name){
                    this->fontName = name;
                }

                void setFontFactor(float factor){
                    this->fontFactor = factor;
                }

                bool isDisplayOn(){
                    return M5Dial.Display.getBrightness() > 0;
                }

                void setBackgroundColor(uint16_t color){
                    this->backgroundColor = color;
                }

                uint16_t getBackgroundColor(){
                    return this->backgroundColor;
                }

                void setAccentColor(uint16_t color){
                    this->accentColor = color;
                }

                uint16_t getAccentColor(){
                    return this->accentColor;
                }

                void setScreensaver(Screensaver* saver){
                    this->screensaver = saver;
                }

                bool isScreensaverActive(){
                    return this->screensaver != nullptr;
                }
                
                bool isScreensaverRunning(){
                    return screensaverRunning;
                }
                
                void resetScreensaverRunning(){
                    screensaverRunning = false;
                }

                void validateTimeout(){
                    if (esphome::millis() - lastEvent > timeToScreenOff ) {
                        if(this->isScreensaverActive()){
                            bool forceRefresh = !screensaverRunning;
                            screensaver->show(*this, forceRefresh);
                            this->commit();

                            screensaverRunning = true;
                        } else {
                            if(M5Dial.Display.getBrightness()>0){
                                M5Dial.Display.setBrightness(0);
                                ESP_LOGI("DISPLAY", "Sleep after %d ms", timeToScreenOff);
                            }
                        } 
                    } else {
                        if(screensaverRunning){
                            this->resetScreensaverRunning();
                            this->display_refresh_action(true);
                        }

                        if ( M5Dial.Display.getBrightness()<=0 ) {
                            M5Dial.Display.setBrightness(100);
                            ESP_LOGI("DISPLAY", "Display on");
                        }
                    }
                }


                void showOffline(){
                    uint16_t height = this->getHeight();
                    uint16_t width  = this->getWidth();

                    gfx->setTextColor(THEME_TEXT_MUTED);
                    gfx->setTextDatum(middle_center);

                    this->setFontByName(this->fontName);

                    gfx->startWrite();                      // Secure SPI bus
                    this->clear(THEME_BG);

                    this->setFontsize(2);
                    gfx->drawString("OFFLINE",
                                    width / 2,
                                    height / 2);

                    gfx->endWrite();                      // Release SPI bus
                    this->commit();
                    this->resetScreensaverRunning();
                }

                void showDisconnected(){
                    uint16_t height = this->getHeight();
                    uint16_t width  = this->getWidth();

                    gfx->setTextColor(this->accentColor);
                    gfx->setTextDatum(middle_center);

                    this->setFontByName(this->fontName);

                    gfx->startWrite();                      // Secure SPI bus
                    this->clear(THEME_BG);

                    this->setFontsize(1);
                    gfx->drawString("DISCONNECTED",
                                    width / 2,
                                    height / 2);

                    gfx->endWrite();                      // Release SPI bus
                    this->commit();
                    this->resetScreensaverRunning();
                }

                void showUnknown(){
                    uint16_t height = this->getHeight();
                    uint16_t width  = this->getWidth();

                    gfx->setTextColor(THEME_DANGER);
                    gfx->setTextDatum(middle_center);

                    this->setFontByName(this->fontName);

                    gfx->startWrite();                      // Secure SPI bus

                    this->clear(THEME_BG);

                    this->setFontsize(2);

                    gfx->drawString("UNKNOWN",
                                    width / 2,
                                    height / 2);

                    gfx->endWrite();                      // Release SPI bus
                    this->commit();
                    this->resetScreensaverRunning();
                }

                static uint16_t blendColor(uint16_t c1, uint16_t c2, float t){
                    uint8_t r1 = ((c1 >> 11) & 0x1F) << 3;
                    uint8_t g1 = ((c1 >> 5) & 0x3F) << 2;
                    uint8_t b1 = (c1 & 0x1F) << 3;

                    uint8_t r2 = ((c2 >> 11) & 0x1F) << 3;
                    uint8_t g2 = ((c2 >> 5) & 0x3F) << 2;
                    uint8_t b2 = (c2 & 0x1F) << 3;

                    uint8_t r = r1 + (int)((r2 - r1) * t);
                    uint8_t g = g1 + (int)((g2 - g1) * t);
                    uint8_t b = b1 + (int)((b2 - b1) * t);

                    return M5Dial.Display.color565(r,g,b);
                }

                static float getLuminance(uint16_t color565){
                    uint8_t r = ((color565 >> 11) & 0x1F) << 3;
                    uint8_t g = ((color565 >> 5) & 0x3F) << 2;
                    uint8_t b = (color565 & 0x1F) << 3;
                    return 0.2126f*r + 0.7152f*g + 0.0722f*b;
                }

                static uint16_t getContrastColor(uint16_t background565){
                    return getLuminance(background565) > 140.0f ? BLACK : WHITE;
                }

                void drawLayeredButton(uint16_t cx, uint16_t cy, uint16_t r, uint16_t base){
                    uint16_t ringOuter = r * 5 / 7;
                    gfx->fillCircle(cx, cy, r, base);
                    gfx->fillCircle(cx, cy, ringOuter, WHITE);
                    gfx->fillCircle(cx, cy, ringOuter > 2 ? ringOuter - 2 : 0, base);
                }

                float getDegByCoord(uint16_t x, uint16_t y){
                    float mx = M5Dial.Display.width()/2;
                    float my = M5Dial.Display.height()/2;

                    float angle = atan2(y - my, x - mx) * 180.0 / M_PI;
                    //angle = 360 - fmod((angle + 360.0 - 90), 360.0);
                    angle = fmod((angle + 360.0 - 90), 360.0);
                    return angle;
                }

                float getRadiusFromCoord(float touchX, float touchY) {
                    float dx = touchX - (getWidth() / 2.0f);
                    float dy = touchY - (getHeight() / 2.0f);
                    float radius = sqrt(dx * dx + dy * dy);

                    return radius;
                }

                coord getColorCoord(float radius, float degree){
                    coord result;
                    result.x = radius * sin(degree*M_PI/180) + (gfx->width()/2);
                    result.y = radius * cos(degree*M_PI/180) + (gfx->height()/2);
                    return result;
                }

                void drawColorCircleLine(float degree, float r1, float r2, uint32_t color) {
                    uint16_t step = 1;
                    coord c1 = getColorCoord(r1, degree);
                    coord c2 = getColorCoord(r2, degree-step);
                    coord c3 = getColorCoord(r2, degree+step);

                    gfx->fillTriangle(c1.x, c1.y, c2.x, c2.y, c3.x, c3.y, color);

                    c1 = getColorCoord(r1, degree);
                    c2 = getColorCoord(r1, degree-step-step);
                    c3 = getColorCoord(r2, degree-step);
                    gfx->fillTriangle(c1.x, c1.y, c2.x, c2.y, c3.x, c3.y, color);
                }

                void setFontsize(float size) {
                    getGfx()->setTextSize(size * this->fontFactor);
                }

                int getRowHeight(float fontSize){
                    return (int)(this->fontFactor * fontSize);
                }

                void applyConfiguredFont(){
                    this->setFontByName(this->fontName);
                }

                void setFontByName(const std::string& name) {
                    if (strcmp(name.c_str(), "default")==0) {
                        this->setFontName("default");
                        getGfx()->setFont(FF_DEFAULT);
                    } else if (FONT_MAP.find(name) != FONT_MAP.end()) {
                        this->setFontName(name);
                        getGfx()->setFont(FONT_MAP[this->fontName]);
                    } else {
                        this->setFontName("default");
                        ESP_LOGE("DISPLAY", "Font '%s' not found, using default font: 'default'", name.c_str());
                        getGfx()->setFont(FF_DEFAULT);
                    }
                }

                void drawBitmap(const uint8_t* bmp, int size, uint8_t x, uint8_t y, uint8_t width, uint8_t height){
                    gfx->drawJpg(bmp, size, x, y, width, height, 0, 0);
                }

                void drawBitmapTransparent(const uint16_t* bmp, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint32_t transparentColor){
                    gfx->pushImage(x, y, width, height, bmp, transparentColor);
                }

                void clear(uint16_t bgColor){
                    gfx->fillRect(0, 0, getWidth(), getHeight(), bgColor);
                }

                void clear(){
                    this->clear(this->backgroundColor);
                }
        };
    }
}