#pragma once

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeLightOnOff: public esphome::shys_m5_dial::HaDeviceMode {
            protected:
                void sendValueToHomeAssistant(int value) override {
                    if(getValue()<=0){
                        haApi.turnLightOff(this->device.getEntityId());
                    } else {
                        haApi.turnLightOn(this->device.getEntityId());
                    }
                }

                uint16_t blendColor(uint16_t c1, uint16_t c2, float t){
                    // blend two RGB565 colors
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

                void drawVignetteBackground(M5DialDisplay& display, uint16_t base){
                    // simple 4-step radial vignette from edge (darker) to center (lighter)
                    uint16_t w = display.getWidth();
                    uint16_t h = display.getHeight();
                    uint16_t cx = w/2, cy = h/2;

                    // darker target color towards edge
                    uint16_t darker = blendColor(base, BLACK, 0.35f);
                    uint16_t mid    = blendColor(base, BLACK, 0.18f);
                    uint16_t light  = base;
                    uint16_t highlight = blendColor(base, WHITE, 0.08f);

                    // draw from large to small to simulate vignette
                    M5Dial.Display.fillCircle(cx, cy, 120, darker);
                    M5Dial.Display.fillCircle(cx, cy, 100, mid);
                    M5Dial.Display.fillCircle(cx, cy, 78, light);
                    M5Dial.Display.fillCircle(cx, cy, 56, highlight);
                }

                void drawStateRing(M5DialDisplay& display, bool is_on){
                    // thin ring around the wheel to indicate state
                    uint16_t ring_color = is_on ? M5Dial.Display.color565(255,255,255) : M5Dial.Display.color565(255,80,80);
                    for(int i=0;i<360;i+=2){
                        display.drawColorCircleLine(i, 130, 134, ring_color);
                    }
                }

                void showOnOffMenu(M5DialDisplay& display){
                    LovyanGFX* gfx = display.getGfx();
                    
                    uint16_t currentValue = getValue();

                    uint16_t height = gfx->height();
                    uint16_t width  = gfx->width();

                    gfx->setTextColor(MAROON);
                    gfx->setTextDatum(middle_center);

                    gfx->startWrite();                      // Secure SPI bus

                    uint16_t base = currentValue>0?YELLOW:RED;
                    display.clear(currentValue>0?YELLOW:RED);

                    // vignette and ring
                    drawVignetteBackground(display, base);
                    drawStateRing(display, currentValue>0);

                    display.setFontsize(3);
                    gfx->drawString(currentValue>0?"on":"off",
                                    width / 2,
                                    height / 2 - 30);                        
                    
                    display.setFontsize(1);
                    gfx->drawString(this->device.getName().c_str(),
                                    width / 2,
                                    height / 2 + 20);
                    gfx->drawString("On/Off",
                                    width / 2,
                                    height / 2 + 50);  

                    gfx->endWrite();                      // Release SPI bus
                }

            public:
                HaDeviceModeLightOnOff(HaDevice& device) : HaDeviceMode(device){}

                void refreshDisplay(M5DialDisplay& display, bool init) override {
                    this->showOnOffMenu(display);
                    ESP_LOGD("DISPLAY", "An/Aus-Modus");
                }

                void registerHAListener() override {
                    api::global_api_server->subscribe_home_assistant_state(
                                this->device.getEntityId().c_str(),
                                optional<std::string>(), 
                                [this](const std::string &state) {
                                    
                        if(this->isValueModified()){
                            return;
                        }

                        int newState = strcmp("on", state.c_str())==0?1:0;

                        this->setReceivedValue(newState);
                        ESP_LOGI("HA_API", "Got value %s for %s", state.c_str(), this->device.getEntityId().c_str());
                    });
                }

                bool onTouch(M5DialDisplay& display, uint16_t x, uint16_t y) override {
                    haApi.toggleLight(this->device.getEntityId());
                    return true;
                }

                bool onRotary(M5DialDisplay& display, const char * direction) override {
                    if(strcmp(direction, ROTARY_LEFT)==0){
                        haApi.turnLightOff(this->device.getEntityId());
                    } else if(strcmp(direction, ROTARY_RIGHT)==0){
                        haApi.turnLightOn(this->device.getEntityId());
                    }

                    return true;
                }

                bool onButton(M5DialDisplay& display, const char * clickType) override {
                    if (strcmp(clickType, BUTTON_SHORT)==0){
                        haApi.toggleLight(this->device.getEntityId());
                        return true;
                    } 
                    return false;
                }

        };
    }
}