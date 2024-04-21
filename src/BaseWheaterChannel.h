#pragma once
#include "OpenKNX.h"
#ifdef WLAN_WifiSSID
#include "WLANModule.h"
#else
#include "NetworkModule.h"
#endif
#include "ArduinoJson.h"
#include "HTTPClient.h"


struct CurrentWheatherData
{
    float temperature = 0;
    float temperatureFeelsLike = 0;
    float humidity = 0;
    uint16_t pressure = 0;
    float windSpeed = 0;
    float windGust = 0;
    uint8_t windDirection = 0;
    float rain = 0;
    float snow = 0;
    float uvi = 0;
    uint8_t clouds = 0;
};

struct ForecastHourWheatherData
{
    float temperature = 0;
    float temperatureFeelsLike = 0;
    float humidity = 0;
    uint16_t pressure = 0;
    float windSpeed = 0;
    float windGust = 0;
    uint8_t windDirection = 0;
    uint8_t probabilityOfPrecipitation = 0;
    float rain = 0;
    float snow = 0;
    float uvi = 0;
    uint8_t clouds = 0;
};


struct ForecastDayWheatherData
{
    float temperatureMin = 0;
    float temperatureMax = 0;
    float temperatureMorning = 0;
    float temperatureDay = 0;
    float temperatureEvening = 0;
    float temperatureNight = 0;

    float temperatureFeelsLikeMorning = 0;
    float temperatureFeelsLikeDay = 0;
    float temperatureFeelsLikeEvening = 0;
    float temperatureFeelsLikeNight = 0;

    float humidity = 0;
    uint16_t pressure = 0;
    float windSpeed = 0;
    float windGust = 0;
    uint8_t windDirection = 0;
    uint8_t probabilityOfPrecipitation = 0;
    float rain = 0;
    float snow = 0;
    float uvi = 0;
    uint8_t clouds = 0;
};

struct ForecastDayWheatherDataWithDescription : ForecastDayWheatherData
{
    char description[15] = {0};
};


class BaseWheaterChannel : public OpenKNX::Channel
{
  private:
    unsigned long _lastApiCall = 0;
    unsigned long _updateIntervalInMs = 0;
    void buildDescription(char* description, float rain, float snow, uint8_t clouds, const char* prefix);
    void updateSwitchableKos();
    void copyGroupObject(GroupObject& koTarget, bool select, GroupObject& ko1, GroupObject& ko2);
  protected:
    BaseWheaterChannel(uint8_t index);
    virtual int16_t fillWheater(CurrentWheatherData& currentWheater, ForecastDayWheatherData& todayWheater, ForecastDayWheatherData& tomorrowWheater, ForecastHourWheatherData& hour1Wheater, ForecastHourWheatherData& hour2Wheater) = 0;
    void setValueCompare(GroupObject& groupObject, const KNXValue& value, const Dpt& type);
 public:
    void loop() override;
    void setup() override;
    void fetchData();
    void processInputKo(GroupObject &ko) override;
    virtual bool processCommand(const std::string cmd, bool diagnoseKo);

 };