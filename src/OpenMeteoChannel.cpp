#include "OpenMeteoChannel.h"
#define OpenMeteoUrl "https://api.open-meteo.com/v1/forecast"

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
    // TODO check using unixtimestamp format
    // TODO check using csv-result

    String url = OpenMeteoUrl;
    // TODO add optional key, check combination with other endpoint required
    // url += "&appid=";
    // url += (const char*)ParamIW_APIKey;
    // TODO set timezone
    url += "?timezone=Europe%2FBerlin";
    url += "&latitude=";
    url += ParamIW_CHWeatherLocationType == 0 ? ParamBASE_Latitude : ParamIW_CHLatitude;
    url += "&longitude=";
    url += ParamIW_CHWeatherLocationType == 0 ? ParamBASE_Longitude : ParamIW_CHLongitude;
    url += "&current=";
    // TODO select based on configuration?
    url += "temperature_2m,apparent_temperature,relative_humidity_2m,surface_pressure,wind_speed_10m,wind_gusts_10m,wind_direction_10m,rain,snowfall,cloud_cover";
    // "Every weather variable available in hourly data, is available as current condition as well." [Open-Meteo API-Doc]
    url += ",uv_index";

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
  
    /*
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
    */

    return httpStatus;
}

void OpenMeteoChannel::fillForecast(JsonObject& json, CurrentWheatherData& wheater)
{
    wheater.temperature = json["temperature_2m"];
    wheater.temperatureFeelsLike = json["apparent_temperature"];
    wheater.humidity = json["relative_humidity_2m"];
    wheater.pressure = json["surface_pressure"];         // possible alternative: `pressure_msl` (at sealevel)
    wheater.windSpeed = json["wind_speed_10m"];          // TODO check required conversion and handling of units // 3.6 * (float)json["wind_speed"];
    wheater.windGust = json["wind_gusts_10m"];           // TODO check required conversion and handling of units // 3.6 * (float)json["wind_gust"];
    wheater.windDirection = json["wind_direction_10m"];  // TODO check representation and required conversion
    wheater.rain = json["rain"];
    wheater.snow = json["snowfall"];
    wheater.uvi = json["uv_index"];
    wheater.clouds = json["cloud_cover"];

    // TODO check integraten of additional fields from Open-Meteo:
    // * Is Day or Night [0/1] - not planned, as day-calc is available in LOG 
    // * Precipitation   [mm] Gesamtniederschlagsmenge, also womöglich "nur" die Summe aller Arten ?
    // * Showers         [mm] ?
    // * Weather Code    [0..100] - see end of https://open-meteo.com/en/docs
}

void OpenMeteoChannel::fillForecast(JsonObject& json, ForecastHourWheatherData& wheater)
{
    // TODO fill with data from OpenMeteo
    /*
    fillForecast(json, (CurrentWheatherData&) wheater);
    wheater.probabilityOfPrecipitation = round(100. * (float) json["pop"]);    // 0.70
    */
}

void OpenMeteoChannel::fillForecast(JsonObject& json, ForecastDayWheatherData& wheater)
{
    // TODO fill with data from OpenMeteo
    /*
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
    */
}
