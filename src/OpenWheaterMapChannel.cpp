#include "OpenWheaterMapChannel.h"

#define OpenWheaterMapUrl "http://api.openweathermap.org/data/3.0/onecall?units=metric&lang=de&exclude=minutely,hourly"

OpenWheaterMapChannel::OpenWheaterMapChannel(uint8_t index)
    : BaseWheaterChannel(index)
{
}

const std::string OpenWheaterMapChannel::name()
{
    return "OpenWheaterMap";
}

int16_t OpenWheaterMapChannel::fillWheater(CurrentWheatherData& currentWheater, ForecastWheatherData& todayWheater, ForecastWheatherData& tomorrowWheater)
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
    currentWheater.temperature = current["temp"];                  // 22.34
    currentWheater.temperatureFeelsLike = current["feels_like"];   // 21.95
    currentWheater.humidity = current["humidity"];                 // 69
    currentWheater.pressure = current["pressure"];                 // 1006
    currentWheater.windSpeed = 3.6 * (float)current["wind_speed"]; // 69
    currentWheater.windGust = 3.6 * (float)current["wind_gust"];   // 69
    currentWheater.windDirection = current["wind_deg"];            // 70
    JsonObject rainObject = current["rain"];
    currentWheater.rain = rainObject ? (float) rainObject["1h"] : (float)0; // 2.5
    JsonObject snowObject = current["snow"];
    currentWheater.snow = snowObject ? (float) snowObject["1h"] : (float)0; // 2.5
    currentWheater.uvi = current["uvi"];                                 // 6.29
    currentWheater.clouds = current["clouds"];                           // 40

    JsonArray daily = doc["daily"];
    JsonObject today = daily[0];
    fillForecast(today, todayWheater);
    JsonObject tomorrow = daily[1];
    fillForecast(tomorrow, tomorrowWheater);
    return httpStatus;
}

void OpenWheaterMapChannel::fillForecast(JsonObject& json, ForecastWheatherData& wheater)
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
