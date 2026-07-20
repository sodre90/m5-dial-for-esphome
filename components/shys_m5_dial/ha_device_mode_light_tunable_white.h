#pragma once
#include <math.h>

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeLightTunableWhite: public esphome::shys_m5_dial::HaDeviceMode {
            protected:
                void sendValueToHomeAssistant(int value) override {
                    haApi.turnLightOnWhite(this->device.getEntityId(), value);
                }

                uint32_t colorTemperatureToRGB888(int kelvin){
                    int temp = kelvin / 100;
                    int red, green, blue;

                    if( temp <= 66 ){ 
                        red = 255; 
                        green = temp;
                        green = 99.4708025861 * log(green) - 161.1195681661;
                        
                        if( temp <= 19){
                            blue = 0;
                        } else {
                            blue = temp-10;
                            blue = 138.5177312231 * log(blue) - 305.0447927307;
                        }
                    } else {
                        red = temp - 60;
                        red = 329.698727446 * pow(red, -0.1332047592);
                        
                        green = temp - 60;
                        green = 288.1221695283 * pow(green, -0.0755148492 );

                        blue = 255;
                    }

                    return M5Dial.Display.color888(clamp(red,   0, 255), clamp(green, 0, 255), clamp(blue,  0, 255));
                }

                uint16_t colorTemperatureToRGB(int kelvin){
                    int temp = kelvin / 100;
                    int red, green, blue;

                    if( temp <= 66 ){ 
                        red = 255; 
                        green = temp;
                        green = 99.4708025861 * log(green) - 161.1195681661;
                        
                        if( temp <= 19){
                            blue = 0;
                        } else {
                            blue = temp-10;
                            blue = 138.5177312231 * log(blue) - 305.0447927307;
                        }
                    } else {
                        red = temp - 60;
                        red = 329.698727446 * pow(red, -0.1332047592);
                        
                        green = temp - 60;
                        green = 288.1221695283 * pow(green, -0.0755148492 );

                        blue = 255;
                    }

                    return M5Dial.Display.color565(clamp(red,   0, 255), clamp(green, 0, 255), clamp(blue,  0, 255));
                }


                int clamp(int x, int min, int max ) {
                    if(x<min){ return min; }
                    if(x>max){ return max; }
                    return x;
                }


                void refreshWhiteMenu(M5DialDisplay& display){
                    LovyanGFX* gfx = display.getGfx();

                    int currentValue = getValue();

                    int height = gfx->height();
                    int width  = gfx->width();

                    uint16_t base565 = colorTemperatureToRGB(currentValue);
                    uint16_t textColor = M5DialDisplay::getContrastColor(base565);

                    gfx->setTextColor(textColor);
                    gfx->setTextDatum(middle_center);

                    gfx->startWrite();                    // Secure SPI bus
                    display.drawLayeredButton(width/2, height/2, 70, base565);

                    display.setFontsize(1);
                    gfx->drawString(String(currentValue),
                                    width / 2,
                                    height / 2 - 20);

                    display.setFontsize(1);
                    gfx->drawString(this->device.getName().c_str(),
                                    width / 2,
                                    height / 2 + 20);
                    gfx->drawString("White",
                                    width / 2,
                                    height / 2 + 50);  

                    float temp = map(currentValue, this->minValue, this->maxValue, 360, 0);
                    // draw a slightly thicker pointer arc for current temperature
                    for(int off=-3; off<=3; ++off){
                        display.drawColorCircleLine(temp+off, 40, 69, textColor);
                    }
                    gfx->endWrite();                      // Release SPI bus
                }

                void showWhiteMenu(M5DialDisplay& display){
                    LovyanGFX* gfx = display.getGfx();

                    int currentValue = getValue();

                    int height = gfx->height();
                    int width  = gfx->width();

                    gfx->startWrite();                      // Secure SPI bus

                    display.clear(BLACK);

                    for (int i=0; i<360; i++){
                        float tmp = map(i, 0, 360, this->minValue, this->maxValue);
                        display.drawColorCircleLine(360-i, 70.0, 130.0, colorTemperatureToRGB888(tmp));
                    }

                    gfx->endWrite();                      // Release SPI bus

                    refreshWhiteMenu(display);
                }


            public:
                HaDeviceModeLightTunableWhite(HaDevice& device) : HaDeviceMode(device){
                    this->value = 2000;
                    this->minValue = 2000;
                    this->maxValue = 6500;
                    this->endlessRotaryValue = false;
                }


                void refreshDisplay(M5DialDisplay& display, bool init) override {
                    ESP_LOGD("DISPLAY", "refresh Display: Farbwahl-Modus");
                    if(init){
                        showWhiteMenu(display);
                    } else {
                        refreshWhiteMenu(display);
                    }
                }


                void registerHAListener() override {
                    subscribeHaNumericState(optional<std::string>("color_temp_kelvin"), "Kelvin", [this](float val) {
                        this->setReceivedValue(round(val));
                    });
                }
                
                bool onTouch(M5DialDisplay& display, uint16_t x, uint16_t y) override {
                    uint16_t degree = display.getDegByCoord(x, y);
                    float tmp = map(degree, 0, 360, this->minValue, this->maxValue);
                    setValue(tmp);
                    ESP_LOGD("TOUCH", "Neuen Weiß-Wert auf %d gesetzt", tmp);
                    
                    return true;                }

                bool onRotary(M5DialDisplay& display, const char * direction) override {
                    return defaultOnRotary(display, direction);
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