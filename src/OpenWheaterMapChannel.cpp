#include "OpenWheaterMapChannel.h"


#define OpenWheaterMapUrl "https://api.openweathermap.org/data/2.5/onecall?units=metric&lang=de&exclude=minutely,dailyX,currentX,hourly"


OpenWheaterMapChannel::OpenWheaterMapChannel(uint8_t index)
 : BaseWheaterChannel(index)
{   
}

const std::string OpenWheaterMapChannel::name()
{
    return "OpenWheaterMap";
}

void OpenWheaterMapChannel::makeCall()
{
    HTTPClient http;
    String url = OpenWheaterMapUrl;
    url += "&appid=";
    url += (const char*) ParamIW_APIKey;
    url += "&lat=";
    url += ParamIW_CHWheaterLocationType == 0 ? ParamBASE_Latitude : ParamIW_CHLatitude;
    url += "&lon=";
    url += ParamIW_CHWheaterLocationType == 0 ? ParamBASE_Longitude : ParamIW_CHLongitude;
    
    logDebugP("Call: %s", url);
    http.begin(url);

    // Send HTTP GET request
    auto status = http.GET();
    if (status != 200)
    {
        logErrorP("Http result %d for '%s'", status, url);
        return;
    }

    JsonDocument doc;

    //  const char* json = "{\"lat\":47.07,\"lon\":15.42,\"timezone\":\"Europe/Vienna\",\"timezone_offset\":7200,\"current\":{\"dt\":1598723553,\"sunrise\":1598674453,\"sunset\":1598723074,\"temp\":22.34,\"feels_like\":21.95,\"pressure\":1006,\"humidity\":69,\"dew_point\":16.38,\"uvi\":6.29,\"clouds\":40,\"visibility\":10000,\"wind_speed\":3.6,\"wind_deg\":70,\"weather\":[{\"id\":802,\"main\":\"Clouds\",\"description\":\"Mäßig bewölkt\",\"icon\":\"03n\"}],\"rain\":{}}}";
    // const char* json =
    // deserializeJson(doc, json);
    deserializeJson(doc, http.getString());

    //  float lat = doc["lat"]; // 47.07
    //  float lon = doc["lon"]; // 15.42
    //  const char* timezone = doc["timezone"]; // "Europe/Vienna"
    //  int timezone_offset = doc["timezone_offset"]; // 7200

    JsonObject current = doc["current"];
    //  long current_dt = current["dt"]; // 1598723553
    //  long current_sunrise = current["sunrise"]; // 1598674453
    //  long current_sunset = current["sunset"]; // 1598723074
    float tempValue = current["temp"]; // 22.34
    logDebugP("Temperature: %f", tempValue);
    if ((float) KoIW_CHCurrentTemparatur.value(DPT_Value_Temp) != tempValue)
    {
        KoIW_CHCurrentTemparatur.value(tempValue, DPT_Value_Temp);
    }

                                    //  float current_feels_like = current["feels_like"]; // 21.95
                                    //  int current_pressure = current["pressure"]; // 1006
                                    //  int current_humidity = current["humidity"]; // 69
                                    //  float current_dew_point = current["dew_point"]; // 16.38
                                    //  float current_uvi = current["uvi"]; // 6.29
                                    //  int current_clouds = current["clouds"]; // 40
                                    //  int current_visibility = current["visibility"]; // 10000
                                    //  float current_wind_speed = current["wind_speed"]; // 3.6
                                    //  int current_wind_deg = current["wind_deg"]; // 70

  //  JsonObject current_weather_0 = current["weather"][0];
    //  int current_weather_0_id = current_weather_0["id"]; // 802
    //  const char* current_weather_0_main = current_weather_0["main"]; // "Clouds"
    //    current_weather_0_description = String((const char*) current_weather_0["description"]); // "Mäßig bewölkt"
    //  const char* current_weather_0_icon = current_weather_0["icon"]; // "03n"

  //  JsonArray daily = doc["daily"];

  //  JsonObject daily_0 = daily[tomorrow ? 1 : 0];
    // long daily_0_dt = daily_0["dt"]; // 1598695200
    // long daily_0_sunrise = daily_0["sunrise"]; // 1598674453
    // long daily_0_sunset = daily_0["sunset"]; // 1598723074

 //   JsonObject daily_0_temp = daily_0["temp"];
    // float daily_0_temp_day = daily_0_temp["day"]; // 21.03
 //   daily_0_temp_min = daily_0_temp["min"]; // 20.47
 //   daily_0_temp_max = daily_0_temp["max"]; // 21.03
                                            // float daily_0_temp_night = daily_0_temp["night"]; // 20.47
    // float daily_0_temp_eve = daily_0_temp["eve"]; // 21.03
    // float daily_0_temp_morn = daily_0_temp["morn"]; // 21.03

    // JsonObject daily_0_feels_like = daily_0["feels_like"];
    // float daily_0_feels_like_day = daily_0_feels_like["day"]; // 22.41
    // float daily_0_feels_like_night = daily_0_feels_like["night"]; // 22.05
    // float daily_0_feels_like_eve = daily_0_feels_like["eve"]; // 22.41
    // float daily_0_feels_like_morn = daily_0_feels_like["morn"]; // 22.41

    // int daily_0_pressure = daily_0["pressure"]; // 1006
    // int daily_0_humidity = daily_0["humidity"]; // 83
    // float daily_0_dew_point = daily_0["dew_point"]; // 18.03
    // float daily_0_wind_speed = daily_0["wind_speed"]; // 2.04
    // int daily_0_wind_deg = daily_0["wind_deg"]; // 80

    // JsonObject daily_0_weather_0 = daily_0["weather"][0];
    // int daily_0_weather_0_id = daily_0_weather_0["id"]; // 500
    // const char* daily_0_weather_0_main = daily_0_weather_0["main"]; // "Rain"
    // const char* daily_0_weather_0_description = daily_0_weather_0["description"]; // "Leichter Regen"
    // const char* daily_0_weather_0_icon = daily_0_weather_0["icon"]; // "10d"

    // int daily_0_clouds = daily_0["clouds"]; // 75
    // float daily_0_pop = daily_0["pop"]; // 0.8
  //  daily_0_rain = daily_0["rain"]; // 0.94
}