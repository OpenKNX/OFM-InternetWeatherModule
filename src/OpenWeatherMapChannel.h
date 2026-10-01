#pragma once
#include "BaseWeatherChannel.h"


class OpenWeatherMapChannel : public BaseWeatherChannel
{
  private:
    void fillForecast(JsonObject& json, ForecastDayWeatherData& wheater);
    void fillForecast(JsonObject& json, CurrentWeatherData& wheater);
    void fillForecast(JsonObject& json, ForecastHourWeatherData& wheater);
  protected:
    bool sendWeatherRequest() override;
    void fillWeather(const std::string& responseBody, CurrentWeatherData& currentWeather, ForecastDayWeatherData& todayWeather, ForecastDayWeatherData& tomorrowWeather, ForecastHourWeatherData& hour1Weather, ForecastHourWeatherData& hour2Weather) override;

  public:
    OpenWeatherMapChannel(uint8_t index);
    const std::string name() override;
 };
