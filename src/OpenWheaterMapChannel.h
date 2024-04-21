#pragma once
#include "BaseWheaterChannel.h"


class OpenWheaterMapChannel : public BaseWheaterChannel
{
  private:
    void fillForecast(JsonObject& json, ForecastWheatherData& wheater);
  protected:
    int16_t fillWheater(CurrentWheatherData& currentWheater, ForecastWheatherData& todayWheater, ForecastWheatherData& tomorrowWheater) override;
  
  public:
    OpenWheaterMapChannel(uint8_t index);
    const std::string name() override;
 };
