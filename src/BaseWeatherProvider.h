#pragma once
#include "Arduino.h"
#include "ArduinoJson.h"
#include <vector>
#include <string>

enum MeasurementValue
{
    MV_Temperature = 0,
    MV_TemperatureMin = 1,
    MV_TemperatureMax = 2,
    MV_TemperatureMorning = 3,
    MV_TemperatureDay = 4,
    MV_TemperatureEvening = 5,
    MV_TemperatureNight = 6,
    MV_TemperatureFeelsLike = 7,
    MV_TemperatureFeelsLikeMin = 8,
    MV_TemperatureFeelsLikeMax = 9,
    MV_TemperatureFeelsLikeMorning = 10,
    MV_TemperatureFeelsLikeDay = 11,
    MV_TemperatureFeelsLikeEvening = 12,
    MV_TemperatureFeelsLikeNight = 13,
    MV_Preasure = 14,
    MV_Humidity = 15,
    MV_DewPoint = 16,
    MV_WindSpeed = 17,
    MV_WindGust = 18,
    MV_WindDirection = 19,
    MV_Rain = 20,
    MV_Snow = 21,
    MV_PrecipitationProbability = 22,
    MV_Clouds = 23,
    MV_Visibility = 24,
    MV_Uvi = 25
};

union MeasurementValueData
{
    float temperature_C;
    float humidity_percent;
    uint16_t pressure_hPa;
    float windSpeed_Km_h;
    float windGust_Km_h;
    uint16_t windDirection_deg;
    uint8_t probabilityOfPrecipitation_percent;
    float rain_mm;
    float snow_mm;
    float uvi_unitOne;
    uint8_t cloudsCover_percent;
};

class MeasurementValueRequest
{
    public:
        MeasurementValue request;
        MeasurementValueData data = {0};
        
};

class BaseWeatherProvider
{
private:
    bool _requestFinished = false;
  
protected:
    void startHttpRequest(const char* httpVerb, std::string url, std::string body, std::string headers);    
    virtual void httpRequestFinished(std::vector<MeasurementValueRequest>& requestedValues, std::string response);

    void startParseJson(std::string json);
    virtual void parseJsonFinished(std::vector<MeasurementValueRequest>& requestedValues, JsonDocument& json);
   
    void setFinished()
    {
        _requestFinished = true;
    }

public:
    virtual void startRequest(std::vector<MeasurementValueRequest>& requestedValues) = 0;
    bool requestFinished()
    {
        return _requestFinished;
    }
};

class SampleProvdiver : public BaseWeatherProvider
{
    public:
        void startRequest(std::vector<MeasurementValueRequest>& requestedValues) override
        {
            startHttpRequest("GET", "http://example.com", "", "");
        }
        
        void httpRequestFinished(std::vector<MeasurementValueRequest>& requestedValues, std::string response)
        {
            startParseJson(response);
        }

        void parseJsonFinished(std::vector<MeasurementValueRequest>& requestedValues, JsonDocument& json)
        {
            for(MeasurementValueRequest& value : requestedValues)
            {
                switch (value.request)
                {
                    case MV_Temperature:
                        value.data.temperature_C = json["temperature"];
                        break;
                    case MV_Humidity:
                        value.data.humidity_percent = json["humidity"];
                        break;
                }
            }
        }

};
