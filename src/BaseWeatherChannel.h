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


struct CurrentWeatherData
{
    float temperature_C = 0;
    float temperatureFeelsLike_C = 0;
    float humidity_percent = 0;
    uint16_t pressure_hPa = 0;
    float windSpeed_Km_h = 0;
    float windGust_Km_h = 0;
    uint16_t windDirection_deg = 0;
    float rain_mm = 0;
    float snow_mm = 0;
    float uvi_unitOne = 0;
    uint8_t cloudsCover_percent = 0;
};

struct ForecastHourWeatherData
{
    float temperature_C = 0;
    float temperatureFeelsLike_C = 0;
    float humidity_percent = 0;
    uint16_t pressure_hPa = 0;
    float windSpeed_Km_h = 0;
    float windGust_Km_h = 0;
    uint16_t windDirection_deg = 0;
    uint8_t probabilityOfPrecipitation_percent = 0;
    float rain_mm = 0;
    float snow_mm = 0;
    float uvi_unitOne = 0;
    uint8_t cloudsCover_percent = 0;
};


struct ForecastDayWeatherData
{
    float temperatureMin_C = 0;
    float temperatureMax_C = 0;
    float temperatureMorning_C = 0;
    float temperatureDay_C = 0;
    float temperatureEvening_C = 0;
    float temperatureNight_C = 0;

    float temperatureFeelsLikeMorning_C = 0;
    float temperatureFeelsLikeDay_C = 0;
    float temperatureFeelsLikeEvening_C = 0;
    float temperatureFeelsLikeNight_C = 0;

    float humidity_percent = 0;
    uint16_t pressure_hPa = 0;
    float windSpeed_Km_h = 0;
    float windGust_Km_h = 0;
    uint16_t windDirection_deg = 0;
    uint8_t probabilityOfPrecipitation_percent = 0;
    float rain_mm = 0;
    float snow_mm = 0;
    float uvi_unitOne = 0;
    uint8_t cloudsCover_percent = 0;
};

struct ForecastDayWeatherDataWithDescription : ForecastDayWeatherData
{
    char description[15] = {0};
};


class BaseWeatherChannel : public OpenKNX::Channel
{
  private:
    unsigned long _lastApiCall = 0;
    unsigned long _updateIntervalInMs = 0;
    bool _available = false;
    ForecastDayWeatherDataWithDescription _today = ForecastDayWeatherDataWithDescription();
    ForecastDayWeatherDataWithDescription _tomorrow = ForecastDayWeatherDataWithDescription();
    void buildDescription(char* description, float rain, float snow, uint8_t clouds, const char* prefix);
    void updateDayForecastKo(ForecastDayWeatherDataWithDescription& day, int koOffset);
    void fetchData();

  protected:
    BaseWeatherChannel(uint8_t index);
    virtual int16_t fillWeather(CurrentWeatherData& currentWeather, ForecastDayWeatherData& todayWeather, ForecastDayWeatherData& tomorrowWeather, ForecastHourWeatherData& hour1Weather, ForecastHourWeatherData& hour2Weather) = 0;
    void setValueCompare(GroupObject& groupObject, const KNXValue& value, const Dpt& type);
    void setValueCompare(uint goNumber, const KNXValue& value, const Dpt& type);

  public:
    void loop() override;
    void setup() override;
    void processInputKo(GroupObject& ko) override;
    virtual bool processCommand(const std::string cmd, bool diagnoseKo);
};