#include "OpenMeteoChannel.h"
#define OpenMeteoUrl "https://api.openweathermap.org/data/3.0/onecall?units=metric&lang=de&exclude=minutely,alerts"

OpenMeteoChannel::OpenMeteoChannel(uint8_t index)
    : BaseWeatherChannel(index)
{
}

const std::string OpenMeteoChannel::name()
{
    return "OpenMeteo";
}

int16_t OpenMeteoChannel::fillWeather(CurrentWheatherData& currentWeather, ForecastDayWheatherData& todayWeather, ForecastDayWheatherData& tomorrowWeather, ForecastHourWheatherData& hour1Weather, ForecastHourWheatherData& hour2Weather)
{
    String url = OpenMeteoUrl;
    url += "&appid=";
    url += (const char*)ParamIW_APIKey;
    url += "&lat=";
    url += ParamIW_CHWeatherLocationType == 0 ? ParamBASE_Latitude : ParamIW_CHLatitude;
    url += "&lon=";
    url += ParamIW_CHWeatherLocationType == 0 ? ParamBASE_Longitude : ParamIW_CHLongitude;
    logDebugP("Call: %s", url.c_str());
    HTTPClient http;
#ifdef ARDUINO_ARCH_RP2040   
    http.setInsecure();
#endif
    http.begin(url);

    // Send HTTP GET request
    auto httpStatus = http.GET();
    if (httpStatus != 200)
        return httpStatus;

    JsonDocument doc;
    deserializeJson(doc, http.getString());

    JsonObject current = doc["current"];
    fillForecast(current, currentWeather);
  
    JsonArray daily = doc["daily"];
    JsonObject today = daily[0];
    fillForecast(today, todayWeather);
    JsonObject tomorrow = daily[1];
    fillForecast(tomorrow, tomorrowWeather);

    JsonArray hourly = doc["hourly"];
    JsonObject hour1 = hourly[1];
    fillForecast(hour1, hour1Weather);
    JsonObject hour2 = hourly[2];
    fillForecast(hour2, hour2Weather);

    return httpStatus;
}

void OpenMeteoChannel::fillForecast(JsonObject& json, CurrentWheatherData& wheater)
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

void OpenMeteoChannel::fillForecast(JsonObject& json, ForecastHourWheatherData& wheater)
{
    fillForecast(json, (CurrentWheatherData&) wheater);
    wheater.probabilityOfPrecipitation = round(100. * (float) json["pop"]);    // 0.70
}

void OpenMeteoChannel::fillForecast(JsonObject& json, ForecastDayWheatherData& wheater)
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
    wheater.probabilityOfPrecipitation = round(100. * (float) json["pop"]);    // 0.70
    wheater.uvi = json["uvi"];                           // 6.29
    wheater.clouds = json["clouds"];                     // 40
}
