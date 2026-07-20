#pragma once
#include "esphome.h"
#include "esphome/components/api/api_pb2.h"

#include <string>
#include <utility>
#include <vector>

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaApi {
            protected:
                using ServiceData = std::vector<std::pair<const char*, std::string>>;

                bool startsWith(const char *pre, const char *str){
                    return strncmp(pre, str, strlen(pre)) == 0;
                }

                void fillData(FixedVector<esphome::api::HomeassistantServiceMap>& target, const ServiceData& data){
                    target.init(data.size());
                    for (const auto& entry : data){
                        auto &kv = target.emplace_back();
                        kv.key = esphome::StringRef(entry.first);
                        kv.value = esphome::StringRef(entry.second);
                    }
                }

                void callService(const char* service, const ServiceData& data, const ServiceData& dataTemplate = {}){
                    esphome::api::HomeassistantActionRequest request;
                    request.service = esphome::StringRef(service);

                    fillData(request.data, data);
                    fillData(request.data_template, dataTemplate);

                    api::global_api_server->send_homeassistant_action(request);
                    ESP_LOGD("HA_API", "call %s for %s", service, data.empty() ? "-" : data.front().second.c_str());
                }

                void callEntityService(const char* service, const std::string& entity, ServiceData data = {}){
                    data.insert(data.begin(), {"entity_id", entity});
                    callService(service, data);
                }

            public:
                void updateEntity(const std::string& entity){
                    callEntityService("homeassistant.update_entity", entity);
                }


                void turnLightOn(const std::string& entity, int brightness = -1, int colorValue = -1){
                    ServiceData data = {{"entity_id", entity}};
                    if(brightness >= 0){
                        data.push_back({"brightness_pct", to_string(brightness)});
                    }

                    ServiceData dataTemplate;
                    if(colorValue >= 0){
                        // hs_color needs a real list, only expressible via a data_template
                        char colorTemplate[32];
                        snprintf(colorTemplate, sizeof(colorTemplate), "{{(%d,100)|list}}", colorValue);
                        dataTemplate.push_back({"hs_color", colorTemplate});
                    }

                    callService("light.turn_on", data, dataTemplate);
                }

                void turnLightOnWhite(const std::string& entity, int kelvin = -1){
                    ServiceData data;
                    if(kelvin >= 0){
                        data.push_back({"kelvin", to_string(kelvin)});
                    }
                    callEntityService("light.turn_on", entity, data);
                }

                void turnLightOff(const std::string& entity){
                    callEntityService("light.turn_off", entity);
                }

                void toggleLight(const std::string& entity){
                    callEntityService("light.toggle", entity);
                }


                void turnClimateOn(const std::string& entity){
                    callEntityService("climate.turn_on", entity);
                }

                void turnClimateOff(const std::string& entity){
                    callEntityService("climate.turn_off", entity);
                }

                void setClimateTemperature(const std::string& entity, int value){
                    callEntityService("climate.set_temperature", entity, {{"temperature", to_string(value)}});
                }

                void setClimateFanMode(const std::string& entity, const std::string& mode){
                    callEntityService("climate.set_fan_mode", entity, {{"fan_mode", mode}});
                }


                void setCoverPosition(const std::string& entity, int value){
                    callEntityService("cover.set_cover_position", entity, {{"position", to_string(value)}});
                }


                void turnSwitchOn(const std::string& entity){
                    callEntityService("switch.turn_on", entity);
                }

                void turnSwitchOff(const std::string& entity){
                    callEntityService("switch.turn_off", entity);
                }

                void toggleSwitch(const std::string& entity){
                    callEntityService("switch.toggle", entity);
                }


                void turnFanOn(const std::string& entity){
                    callEntityService("fan.turn_on", entity);
                }

                void turnFanOff(const std::string& entity){
                    callEntityService("fan.turn_off", entity);
                }

                void toggleFan(const std::string& entity){
                    callEntityService("fan.toggle", entity);
                }

                void setFanDirection(const std::string& entity, const char* direction){
                    callEntityService("fan.set_direction", entity, {{"direction", direction}});
                }

                void setFanSpeed(const std::string& entity, int value){
                    callEntityService("fan.set_percentage", entity, {{"percentage", to_string(value)}});
                }


                void setMediaPlayerVolume(const std::string& entity, int value){
                    callEntityService("media_player.volume_set", entity, {{"volume_level", to_string((float)value/100)}});
                }

                void stopMediaPlayer(const std::string& entity){
                    callEntityService("media_player.media_stop", entity);
                }

                void setNextTrackOnMediaPlayer(const std::string& entity){
                    callEntityService("media_player.media_next_track", entity);
                }

                void setPreviousTrackOnMediaPlayer(const std::string& entity){
                    callEntityService("media_player.media_previous_track", entity);
                }

                void playPauseMediaPlayer(const std::string& entity){
                    callEntityService("media_player.media_play_pause", entity);
                }

                void playMediaOnMediaPlayer(const std::string& entity, const std::string& media_content_id, const std::string& media_content_type){
                    callEntityService("media_player.play_media", entity, {
                        {"media_content_id", media_content_id},
                        {"media_content_type", media_content_type}
                    });
                }


                void openLock(const std::string& entity){
                    callEntityService("lock.open", entity);
                }

                void lockLock(const std::string& entity){
                    callEntityService("lock.lock", entity);
                }

                void unlockLock(const std::string& entity){
                    callEntityService("lock.unlock", entity);
                }


                void setNumberValue(const std::string& entity, int value){
                    const char* service = this->startsWith("input_number.", entity.c_str()) ?
                                          "input_number.set_value" : "number.set_value";
                    callEntityService(service, entity, {{"value", to_string(value)}});
                }


                void activateScene(const std::string& entity){
                    const char* service = this->startsWith("script.", entity.c_str()) ?
                                          "script.turn_on" : "scene.turn_on";
                    callEntityService(service, entity);
                }


                void timerStart(const std::string& entity){
                    callEntityService("timer.start", entity);
                }

                void timerStart(const std::string& entity, int duration){
                    callEntityService("timer.start", entity, {{"duration", to_string(duration)}});
                }

                void timerPause(const std::string& entity){
                    callEntityService("timer.pause", entity);
                }

                void timerCancel(const std::string& entity){
                    callEntityService("timer.cancel", entity);
                }

                void timerFinish(const std::string& entity){
                    callEntityService("timer.finish", entity);
                }

                void timerChange(const std::string& entity, int duration){
                    callEntityService("timer.change", entity, {{"duration", to_string(duration)}});
                }
        };
    }
}
