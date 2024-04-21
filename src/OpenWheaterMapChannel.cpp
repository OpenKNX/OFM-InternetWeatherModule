#include "OpenWheaterMapChannel.h"
#ifdef ARDUINO_ARCH_RP2040
#define OpenWheaterMapUrl "http://api.openweathermap.org/data/3.0/onecall?units=metric&lang=de&exclude=minutely,alerts"
#else
#define OpenWheaterMapUrl "https://api.openweathermap.org/data/3.0/onecall?units=metric&lang=de&exclude=minutely,alerts"
#endif

OpenWheaterMapChannel::OpenWheaterMapChannel(uint8_t index)
    : BaseWheaterChannel(index)
{
}

const std::string OpenWheaterMapChannel::name()
{
    return "OpenWheaterMap";
}

int16_t OpenWheaterMapChannel::fillWheater(CurrentWheatherData& currentWheater, ForecastDayWheatherData& todayWheater, ForecastDayWheatherData& tomorrowWheater, ForecastHourWheatherData& hour1Wheater, ForecastHourWheatherData& hour2Wheater)
{
    String url = OpenWheaterMapUrl;
    url += "&appid=";
    url += (const char*)ParamIW_APIKey;
    url += "&lat=";
    url += ParamIW_CHWheaterLocationType == 0 ? ParamBASE_Latitude : ParamIW_CHLatitude;
    url += "&lon=";
    url += ParamIW_CHWheaterLocationType == 0 ? ParamBASE_Longitude : ParamIW_CHLongitude;
    logDebugP("Call: %s", url.c_str());
    HTTPClient http;
    http.begin(url);

    // Send HTTP GET request
    auto httpStatus = http.GET();
    if (httpStatus != 200)
        return httpStatus;

    JsonDocument doc;
    deserializeJson(doc, http.getString());

    JsonObject current = doc["current"];
    fillForecast(current, currentWheater);
  
    JsonArray daily = doc["daily"];
    JsonObject today = daily[0];
    fillForecast(today, todayWheater);
    JsonObject tomorrow = daily[1];
    fillForecast(tomorrow, tomorrowWheater);

    JsonArray hourly = doc["hourly"];
    JsonObject hour1 = hourly[1];
    fillForecast(hour1, hour1Wheater);
    JsonObject hour2 = hourly[2];
    fillForecast(hour2, hour2Wheater);

    return httpStatus;
}

void OpenWheaterMapChannel::fillForecast(JsonObject& json, CurrentWheatherData& wheater)
{
    wheater.temperature = json["temp"];                  // 22.34
    wheater.temperatureFeelsLike = json["feels_like"];   // 21.95
    wheater.humidity = json["humidity"];                 // 69
    wheater.pressure = json["pressure"];                 // 1006
    wheater.windSpeed = 3.6 * (float)json["wind_speed"]; // 69
    wheater.windGust = 3.6 * (float)json["wind_gust"];   // 69
    wheater.windDirection = json["wind_deg"];            // 70
    JsonObject rainObject = json["rain"];
    wheater.rain = rainObject ? (float) rainObject["1h"] : (float)0; // 2.5
    JsonObject snowObject = json["snow"];
    wheater.snow = snowObject ? (float) snowObject["1h"] : (float)0; // 2.5
    wheater.uvi = json["uvi"];                                 // 6.29
    wheater.clouds = json["clouds"];                           // 40
}

void OpenWheaterMapChannel::fillForecast(JsonObject& json, ForecastHourWheatherData& wheater)
{
    fillForecast(json, (CurrentWheatherData&) wheater);
    wheater.probabilityOfPrecipitation = json["pop"];    // 70
}

void OpenWheaterMapChannel::fillForecast(JsonObject& json, ForecastDayWheatherData& wheater)
{
    JsonObject tempObject = json["temp"];
    wheater.temperatureDay = tempObject["day"];     // 21.95
    wheater.temperatureNight = tempObject["night"]; // 21.95
    wheater.temperatureEvening = tempObject["eve"]; // 21.95
    wheater.temperatureMorning = tempObject["morn"]; // 21.95
    wheater.temperatureMin = tempObject["min"];      // 21.95
    wheater.temperatureMax = tempObject["max"];      // 21.95
    tempObject = json["feels_like"];
    wheater.temperatureFeelsLikeDay = tempObject["day"];     // 21.95
    wheater.temperatureFeelsLikeNight = tempObject["night"]; // 21.95
    wheater.temperatureFeelsLikeEvening = tempObject["eve"]; // 21.95
    wheater.temperatureFeelsLikeMorning = tempObject["morn"]; // 21.95
    wheater.humidity = json["humidity"];                 // 69
    wheater.pressure = json["pressure"];                 // 1006
    wheater.windSpeed = 3.6 * (float)json["wind_speed"]; // 69
    wheater.windGust = 3.6 * (float)json["wind_gust"];   // 69
    wheater.windDirection = json["wind_deg"];            // 70
    wheater.rain = json["rain"];                         // 2.5
    wheater.snow = json["snow"];                         // 2.5
    wheater.probabilityOfPrecipitation = json["pop"];    // 70
    wheater.uvi = json["uvi"];                           // 6.29
    wheater.clouds = json["clouds"];                     // 40
}
