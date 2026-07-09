#pragma once
#include "BaseWeatherChannel.h"

class OpenMeteoChannel : public BaseWeatherChannel
{
  private:
    float avg(JsonArray& arr, int begin, int n);
    void fillForecast(JsonObject& json, JsonObject& jsonHourly, int vi, ForecastDayWeatherData& wheater);
    void fillForecast(JsonObject& json, CurrentWeatherData& wheater);
    void fillForecast(JsonObject& json, int vi, ForecastHourWeatherData& wheater);

  protected:
    int16_t fillWeather(CurrentWeatherData& currentWeather, ForecastDayWeatherData& todayWeather, ForecastDayWeatherData& tomorrowWeather, ForecastHourWeatherData& hour1Weather, ForecastHourWeatherData& hour2Weather) override;

  public:
    OpenMeteoChannel(uint8_t index);
    const std::string name() override;
};
