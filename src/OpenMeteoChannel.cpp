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
    // SAME for hourly ...
    url += "temperature_2m,apparent_temperature,relative_humidity_2m,surface_pressure,wind_speed_10m,wind_gusts_10m,wind_direction_10m,rain,snowfall,cloud_cover";
    // "Every weather variable available in hourly data, is available as current condition as well." [Open-Meteo API-Doc]
    url += ",uv_index";
    // ... SAME for hourly

    url += "&hourly=";
    // SAME as current ...
    url += "temperature_2m,apparent_temperature,relative_humidity_2m,surface_pressure,wind_speed_10m,wind_gusts_10m,wind_direction_10m,rain,snowfall,cloud_cover";
    // "Every weather variable available in hourly data, is available as current condition as well." [Open-Meteo API-Doc]
    url += ",uv_index";
    // ... SAME as current
    // additional value for forecast:
    url += ",precipitation_probability";

    url += "&daily=";
    url += "temperature_2m_min,temperature_2m_max,wind_speed_10m_max,wind_gusts_10m_max,wind_direction_10m_dominant,rain_sum,snowfall_sum,precipitation_probability_max,uv_index_max";
    url += "&timeformat=unixtime";

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
    "daily_units": {
        "time": "iso8601",
        "temperature_2m_max": "°C",
        "temperature_2m_min": "°C"
    },
    "daily": {
        "time": [
            "2024-08-03",
            "2024-08-04"
        ],
        "temperature_2m_max": [
            28.1,
            21.7
        ],
        "temperature_2m_min": [
            17,
            18.3
        ]
    }
    */
    JsonObject daily = doc["daily"];
    JsonObject hourly = doc["hourly"];
    fillForecast(daily, hourly, 0, todayWeather);
    fillForecast(daily, hourly, 1, tomorrowWeather);

    /*
    "hourly_units": {
        "time": "iso8601",
        "temperature_2m": "°C"
    },
    "hourly": {
        "time": [
            "2024-08-03T00:00",
            ..
            "2024-08-04T23:00"
        ],
        "temperature_2m": [
            20.7,
            ..
            18.3
        ]
    },
    */

    int hour = 17; // TODO FIXME calc based on current time, or response.current.time &timeformat=unixtime to next hour1 and hour2
    fillForecast(hourly, hour + 0, hour1Weather);
    fillForecast(hourly, hour + 1, hour2Weather);

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

void OpenMeteoChannel::fillForecast(JsonObject& json, int vi, ForecastHourWheatherData& wheater)
{
    // TODO fill with data from OpenMeteo
    /*
    // this will not work, due to different structure
    fillForecast(json, (CurrentWheatherData&) wheater);
    wheater.probabilityOfPrecipitation = round(100. * (float) json["pop"]);    // 0.70
    */

    // same as for current, but value-arrays instead of values
    wheater.temperature = json["temperature_2m"][vi];
    wheater.temperatureFeelsLike = json["apparent_temperature"][vi];
    wheater.humidity = json["relative_humidity_2m"][vi];
    wheater.pressure = json["surface_pressure"][vi];         // possible alternative: `pressure_msl` (at sealevel)
    wheater.windSpeed = json["wind_speed_10m"][vi];          // TODO check required conversion and handling of units // 3.6 * (float)json["wind_speed"];
    wheater.windGust = json["wind_gusts_10m"][vi];           // TODO check required conversion and handling of units // 3.6 * (float)json["wind_gust"];
    wheater.windDirection = json["wind_direction_10m"][vi];  // TODO check representation and required conversion
    wheater.rain = json["rain"][vi];
    wheater.snow = json["snowfall"][vi];
    wheater.uvi = json["uv_index"][vi];
    wheater.clouds = json["cloud_cover"][vi];

    wheater.probabilityOfPrecipitation = json["precipitation_probability"][vi]; //  = round(100. * (float) json["pop"]);    // 0.70
}

void OpenMeteoChannel::fillForecast(JsonObject& json, JsonObject& jsonHourly, int vi, ForecastDayWheatherData& wheater)
{
    // TODO check calculation of humidity, pressure and coulds by AVG of hourly values?

    const int vih = 24 * vi;
    const int vihDay = vih + 12;
    const int vihNight = vih + 0;
    const int vihEve = vih + 18;
    const int vihMorn = vih + 06;

    JsonArray temperature = jsonHourly["temperature_2m"];
    wheater.temperatureDay = temperature[vihDay];
    wheater.temperatureNight = temperature[vihNight];
    wheater.temperatureEvening = temperature[vihEve];
    wheater.temperatureMorning = temperature[vihMorn];
    wheater.temperatureMin = json["temperature_2m_min"][vi];
    wheater.temperatureMax = json["temperature_2m_max"][vi];

    JsonArray apparentTemperature = jsonHourly["apparent_temperature"];
    wheater.temperatureFeelsLikeDay = apparentTemperature[vihDay];
    wheater.temperatureFeelsLikeNight = apparentTemperature[vihNight];
    wheater.temperatureFeelsLikeEvening = apparentTemperature[vihEve];
    wheater.temperatureFeelsLikeMorning = apparentTemperature[vihMorn];

    /*
    wheater.humidity = json["humidity"];                 // 69
    wheater.pressure = json["pressure"];                 // 1006
    */
    wheater.windSpeed = json["wind_speed_10m_max"][vi];      // 3.6 * (float)json["wind_speed"]; // 69
    wheater.windGust = json["wind_gusts_10m_max"][vi];      // 3.6 * (float)json["wind_gust"];   // 69
    wheater.windDirection = json["wind_direction_10m_dominant"][vi];

    wheater.rain = json["rain_sum"][vi];
    wheater.snow = json["snowfall_sum"][vi];
    wheater.probabilityOfPrecipitation = json["precipitation_probability_max"][vi]; // round(100. * (float) json["pop"]);    // 0.70
    wheater.uvi = json["uv_index_max"][vi];
    /*
    wheater.clouds = json["clouds"];                     // 40
    */
}
