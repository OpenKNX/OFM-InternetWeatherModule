#pragma once
#include "BaseWheaterChannel.h"


class OpenWheaterMapChannel : public BaseWheaterChannel
{
  private:
    void fillForecast(JsonObject& json, ForecastDayWheatherData& wheater);
    void fillForecast(JsonObject& json, CurrentWheatherData& wheater);
    void fillForecast(JsonObject& json, ForecastHourWheatherData& wheater);
  protected:
    int16_t fillWheater(CurrentWheatherData& currentWheater, ForecastDayWheatherData& todayWheater, ForecastDayWheatherData& tomorrowWheater, ForecastHourWheatherData& hour1Wheater, ForecastHourWheatherData& hour2Wheater) override;
  
  public:
    OpenWheaterMapChannel(uint8_t index);
    const std::string name() override;
 };
