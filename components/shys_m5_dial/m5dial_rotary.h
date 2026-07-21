#pragma once
#include "M5Dial.h"
#include "driver/pulse_cnt.h"

namespace esphome
{
    namespace shys_m5_dial
    {
        class M5DialRotary {
            protected:
                std::function<void(void)> rotary_left_action;
                std::function<void(void)> rotary_right_action;
                std::function<void(void)> short_button_press_action;
                std::function<void(void)> long_button_press_action;

                int longPressMs = 1500;

                long oldPosition = 0;
                bool longPress = false;

                pcnt_unit_handle_t pcntUnit = nullptr;

                static const int PCNT_HIGH_LIMIT = 1000;
                static const int PCNT_LOW_LIMIT = -1000;

                long readPosition(){
                    int count = 0;
                    pcnt_unit_get_count(this->pcntUnit, &count);
                    return count;
                }

            public:
               /**
                * M5Dial's vendored Encoder library decodes quadrature in software
                * from GPIO interrupts and has no glitch filtering; under fast
                * rotation it can latch a sustained wrong direction (see git log
                * for the diagnosis). This drives the same two GPIOs
                * (DIAL_ENCODER_PIN_A/B) through the ESP32's PCNT hardware
                * quadrature decoder instead, which glitch-filters in silicon.
                * M5Dial's own encoder must stay disabled (enableEncoder=false in
                * ShysM5Dial::initDevice) so it doesn't also attach interrupts to
                * these pins.
                */
                void begin(){
                    pcnt_unit_config_t unitConfig = {};
                    unitConfig.low_limit = PCNT_LOW_LIMIT;
                    unitConfig.high_limit = PCNT_HIGH_LIMIT;
                    unitConfig.flags.accum_count = true;
                    ESP_ERROR_CHECK(pcnt_new_unit(&unitConfig, &this->pcntUnit));

                    pcnt_glitch_filter_config_t filterConfig = {};
                    filterConfig.max_glitch_ns = 1000;
                    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(this->pcntUnit, &filterConfig));

                    pcnt_chan_config_t chanAConfig = {};
                    chanAConfig.edge_gpio_num = DIAL_ENCODER_PIN_A;
                    chanAConfig.level_gpio_num = DIAL_ENCODER_PIN_B;
                    pcnt_channel_handle_t chanA = nullptr;
                    ESP_ERROR_CHECK(pcnt_new_channel(this->pcntUnit, &chanAConfig, &chanA));

                    pcnt_chan_config_t chanBConfig = {};
                    chanBConfig.edge_gpio_num = DIAL_ENCODER_PIN_B;
                    chanBConfig.level_gpio_num = DIAL_ENCODER_PIN_A;
                    pcnt_channel_handle_t chanB = nullptr;
                    ESP_ERROR_CHECK(pcnt_new_channel(this->pcntUnit, &chanBConfig, &chanB));

                    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chanA, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
                    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chanA, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
                    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chanB, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
                    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chanB, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

                    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(this->pcntUnit, PCNT_HIGH_LIMIT));
                    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(this->pcntUnit, PCNT_LOW_LIMIT));

                    ESP_ERROR_CHECK(pcnt_unit_enable(this->pcntUnit));
                    ESP_ERROR_CHECK(pcnt_unit_clear_count(this->pcntUnit));
                    ESP_ERROR_CHECK(pcnt_unit_start(this->pcntUnit));

                    this->oldPosition = this->readPosition();
                }

                void on_rotary_right(std::function<void(void)> callback){
                    ESP_LOGD("DEVICE", "register on_rotary_right Callback");
                    this->rotary_right_action = callback;
                }

                void on_rotary_left(std::function<void(void)> callback){
                    ESP_LOGD("DEVICE", "register on_rotary_left Callback");
                    this->rotary_left_action = callback;
                }

                void on_short_button_press(std::function<void(void)> callback){
                    ESP_LOGD("DEVICE", "register on_short_button_press Callback");
                    this->short_button_press_action = callback;
                }

                void on_long_button_press(std::function<void(void)> callback){
                    ESP_LOGD("DEVICE", "register on_long_button_press Callback");
                    this->long_button_press_action = callback;
                }


               /**
                * 
                */
                void setLongPressDuration(int value){
                    longPressMs = value;
                }

               /**
                * 
                */
                void handleRotary(){
                    long newPosition = this->readPosition();
                    if (newPosition == this->oldPosition) {
                        return;
                    }

                    if(newPosition > this->oldPosition){
                        ESP_LOGI("DEVICE", "Rotary left");
                        this->rotary_left_action();
                    } else {
                        ESP_LOGI("DEVICE", "Rotary right");
                        this->rotary_right_action();
                    }

                    this->oldPosition = newPosition;
                }

               /**
                * 
                */
                bool handleButtonPress(){
                    bool is_event = false;

                    if (M5Dial.BtnA.wasPressed()) {
                        longPress = false;
                        is_event = true;
                    }

                    if (M5Dial.BtnA.pressedFor(longPressMs)) {
                        M5Dial.Speaker.tone(4000, 200);
                        longPress = true;
                        is_event = true;
                    }

                    if (M5Dial.BtnA.wasReleased()) {
                        if(longPress){
                            this->long_button_press_action();
                            ESP_LOGI("DEVICE", "Long press");
                        } else {
                            this->short_button_press_action();
                            ESP_LOGI("DEVICE", "Short press");
                        }
                        is_event = true;
                    }

                    return is_event;
                }

        };
    }
}
