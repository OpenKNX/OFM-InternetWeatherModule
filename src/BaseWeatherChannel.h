#pragma once
#include "OpenKNX.h"

#include "ArduinoJson.h"
#include "HTTPClient.h"


// simple compile-time-checks for ko calculation
#if ((IW_KoCHTomorrowDescription - IW_KoCHTodayDescription) != (IW_KoCHTomorrowClouds - IW_KoCHTodayClouds))
    #error "KO offset for tomorrow is NOT constant!"
#endif
#if ((IW_KoCHForecastDescription - IW_KoCHTodayDescription) != (IW_KoCHForecastClouds - IW_KoCHTodayClouds))
    #error "KO offset for switchable forecast is NOT constant!"
#endif
#if ((IW_KoCHTomorrowDescription - IW_KoCHTomorrowClouds) != (IW_KoCHForecastDescription - IW_KoCHForecastClouds))
    #error "different structure of KO-groups"
#endif

// ko numbers relative to today forecast
#define IW_KoOffset_Tomorrow (IW_KoCHTomorrowDescription - IW_KoCHTodayDescription)
#define IW_KoOffset_Forecast (IW_KoCHForecastDescription - IW_KoCHTodayDescription)


struct CurrentWheatherData
{
    float temperature = 0;
    float temperatureFeelsLike = 0;
    float humidity = 0;
    uint16_t pressure = 0;
    float windSpeed = 0;
    float windGust = 0;
    uint16_t windDirection = 0;
    float rain = 0;
    float snow_mm = 0;
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
    uint16_t windDirection = 0;
    uint8_t probabilityOfPrecipitation = 0;
    float rain = 0;
    float snow_mm = 0;
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
    uint16_t windDirection = 0;
    uint8_t probabilityOfPrecipitation = 0;
    float rain = 0;
    float snow_mm = 0;
    float uvi = 0;
    uint8_t clouds = 0;
};

struct ForecastDayWheatherDataWithDescription : ForecastDayWheatherData
{
    char description[15] = {0};
};


class BaseWeatherChannel : public OpenKNX::Channel
{
  private:
    unsigned long _lastApiCall = 0;
    unsigned long _updateIntervalInMs = 0;
    bool _available = false;
    // TODO rename with prefix _
    ForecastDayWheatherDataWithDescription today = ForecastDayWheatherDataWithDescription();
    ForecastDayWheatherDataWithDescription tomorrow = ForecastDayWheatherDataWithDescription();
    void buildDescription(char* description, float rain, float snow, uint8_t clouds, const char* prefix);
    void updateUviKo(GroupObject& groupObject, float uviFloatValue);
    void updateDayForecastKo(ForecastDayWheatherDataWithDescription today, int koOffset);
    void fetchData();

  protected:
    BaseWeatherChannel(uint8_t index);
    virtual int16_t fillWeather(CurrentWheatherData& currentWeather, ForecastDayWheatherData& todayWeather, ForecastDayWheatherData& tomorrowWeather, ForecastHourWheatherData& hour1Weather, ForecastHourWheatherData& hour2Weather) = 0;
    void setValueCompare(GroupObject& groupObject, const KNXValue& value, const Dpt& type);
    void setValueCompare(uint goNumber, const KNXValue& value, const Dpt& type);

  public:
    void loop() override;
    void setup() override;
    void processInputKo(GroupObject& ko) override;
    virtual bool processCommand(const std::string cmd, bool diagnoseKo);
};