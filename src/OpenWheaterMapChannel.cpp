#include "OpenWheaterMapChannel.h"

#define OpenWheaterMapUrl "https://api.openweathermap.org/data/3.0/onecall?units=metric&lang=de&exclude=minutely,hourly"


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
    logDebugP("Call: %s", url.c_str());
    http.begin(url);

    // Send HTTP GET request
    auto status = http.GET();
    KoIW_CHHTTPStatus.value(status, DPT_Value_2_Count);
    if (status != 200)
    {
        logErrorP("Http result %d for '%s'", status, url.c_str());
        return;
    }

    JsonDocument doc;

    //  const char* json = "{\"lat\":47.07,\"lon\":15.42,\"timezone\":\"Europe/Vienna\",\"timezone_offset\":7200,\"current\":{\"dt\":1598723553,\"sunrise\":1598674453,\"sunset\":1598723074,\"temp\":22.34,\"feels_like\":21.95,\"pressure\":1006,\"humidity\":69,\"dew_point\":16.38,\"uvi\":6.29,\"clouds\":40,\"visibility\":10000,\"wind_speed\":3.6,\"wind_deg\":70,\"weather\":[{\"id\":802,\"main\":\"Clouds\",\"description\":\"Mäßig bewölkt\",\"icon\":\"03n\"}],\"rain\":{}}}";
    // const char* json =
    // deserializeJson(doc, json);
    deserializeJson(doc, http.getString());

    JsonObject current = doc["current"];
    if (current != nullptr)
    {
        float temp = current["temp"]; // 22.34
        logDebugP("Temperature: %f", temp);
        if ((float) KoIW_CHCurrentTemparatur.value(DPT_Value_Temp) != temp)
        {
            KoIW_CHCurrentTemparatur.value(temp, DPT_Value_Temp);
        }
        float feelsLike = current["feels_like"]; // 21.95
        logDebugP("Temperature Feels Like: %f", feelsLike);
        if ((float) KoIW_CHCurrentTemparaturFeelsLike.value(DPT_Value_Temp) != feelsLike)
        {
            KoIW_CHCurrentTemparaturFeelsLike.value(feelsLike, DPT_Value_Temp);
        }
        float humidity = current["humidity"]; // 69
        logDebugP("Humidity: %f", humidity);
        if ((float) KoIW_CHCurrentHumidity.value(DPT_Value_Humidity) != humidity)
        {
            KoIW_CHCurrentHumidity.value(humidity, DPT_Value_Humidity);
        }
        int pressure = current["pressure"]; // 1006
        logDebugP("Pressure: %d", pressure);
        if ((float) KoIW_CHCurrentPressure.value(DPT_Value_Pres) != pressure)
        {
            KoIW_CHCurrentPressure.value(pressure, DPT_Value_Pres);
        }
        float windSpeed = 3.6 * (float) current["wind_speed"]; // 69
        logDebugP("Wind Speed: %f", windSpeed);   
        if ((float) KoIW_CHCurrentWind.value(DPT_Value_Wsp_kmh) != windSpeed)
        {
            KoIW_CHCurrentWind.value(windSpeed, DPT_Value_Wsp_kmh);
        }
        float windGust = 3.6 * (float) current["wind_gust"]; // 69
        logDebugP("Wind Gust: %f", windGust);   
        if ((float) KoIW_CHCurrentWindGust.value(DPT_Value_Wsp_kmh) != windGust)
        {
            KoIW_CHCurrentWindGust.value(windGust, DPT_Value_Wsp_kmh);
        }
        int windDirection = current["wind_deg"]; // 70
        logDebugP("Wind Direction: %d", windDirection);   
        if ((float) KoIW_CHCurrentWindDirection.value(DPT_Angle) != windDirection)
        {
            KoIW_CHCurrentWindDirection.value(windDirection, DPT_Angle);
        }   
        JsonObject rainObject = current["rain"];    
        if (rainObject != nullptr)
        {
            float rain = rainObject["1h"]; // 2.5
            logDebugP("Rain: %f", rain); 
            if ((float) KoIW_CHCurrentRain.value(DPT_Rain_Amount) != rain)
            {
                KoIW_CHCurrentRain.value(rain, DPT_Rain_Amount);
            }   
        }  
        JsonObject snowObject = current["snow"];    
        if (snowObject != nullptr)
        {
            float snow = snowObject["1h"]; // 2.5
            logDebugP("Snow: %f", snow); 
            if ((float) KoIW_CHCurrentSnow.value(DPT_Length_mm) != snow)
            {
                KoIW_CHCurrentSnow.value(snow, DPT_Length_mm);
            }   
        }            
        int uvi = round((float)current["uvi"]); // 6.29   
        logDebugP("UVI: %d", uvi);   
        if ((int) KoIW_CHCurrentUVI.value(DPT_DecimalFactor) != uvi)
        {
            KoIW_CHCurrentUVI.value(uvi, DPT_DecimalFactor);
        }         
        int clouds = current["clouds"]; // 40
        logDebugP("Clouds: %d", clouds);   
        if ((int) KoIW_CHCurrentClouds.value(DPT_Scaling) != clouds)
        {
            KoIW_CHCurrentClouds.value(clouds, DPT_Scaling);
        }   
    }
    JsonArray daily = doc["daily"];
    if (daily != nullptr)
    {
        JsonObject today = daily[0];
        if (today != nullptr)
        {
            JsonObject tempObject = today["temp"]; 
            if (tempObject != nullptr)
            {
                float temp = tempObject["day"]; // 21.95
                logDebugP("Today Day: %f", temp);
                if ((float) KoIW_CHTodayTemparaturDay.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTodayTemparaturDay.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["night"]; // 21.95
                logDebugP("Today Night: %f", temp);
                if ((float) KoIW_CHTodayTemparaturNight.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTodayTemparaturNight.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["eve"]; // 21.95
                logDebugP("Today Evening: %f", temp);
                if ((float) KoIW_CHTodayTemparaturEvening.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTodayTemparaturEvening.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["morn"]; // 21.95
                logDebugP("Today Morning: %f", temp);
                if ((float) KoIW_CHTodayTemparaturMorning.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTodayTemparaturMorning.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["min"]; // 21.95
                logDebugP("Today Min: %f", temp);
                if ((float) KoIW_CHTodayTemparaturMin.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTodayTemparaturMin.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["max"]; // 21.95
                logDebugP("Today Max: %f", temp);
                if ((float) KoIW_CHTodayTemparaturMax.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTodayTemparaturMax.value(temp, DPT_Value_Temp);
                }
            }
            tempObject = today["feels_like"]; 
            if (tempObject != nullptr)
            {
                float temp = tempObject["day"]; // 21.95
                logDebugP("Today Day: %f", temp);
                if ((float) KoIW_CHTodayTemparaturDayFeelsLike.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTodayTemparaturDay.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["night"]; // 21.95
                logDebugP("Today Night: %f", temp);
                if ((float) KoIW_CHTodayTemparaturNightFeelsLike.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTodayTemparaturNight.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["eve"]; // 21.95
                logDebugP("Today Evening: %f", temp);
                if ((float) KoIW_CHTodayTemparaturEveningFeelsLike.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTodayTemparaturEvening.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["morn"]; // 21.95
                logDebugP("Today Morning: %f", temp);
                if ((float) KoIW_CHTodayTemparaturMorningFeelsLike.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTodayTemparaturMorning.value(temp, DPT_Value_Temp);
                }
            }
            float humidity = today["humidity"]; // 69
            logDebugP("Today Humidity: %f", humidity);
            if ((float) KoIW_CHTodayHumidity.value(DPT_Value_Humidity) != humidity)
            {
                KoIW_CHTodayHumidity.value(humidity, DPT_Value_Humidity);
            }
            int pressure = today["pressure"]; // 1006
            logDebugP("Today Pressure: %d", pressure);
            if ((float) KoIW_CHTodayPressure.value(DPT_Value_Pres) != pressure)
            {
                KoIW_CHTodayPressure.value(pressure, DPT_Value_Pres);
            }
            float windSpeed = 3.6 * (float) today["wind_speed"]; // 69
            logDebugP("Today Wind Speed: %f", windSpeed);   
            if ((float) KoIW_CHTodayWind.value(DPT_Value_Wsp_kmh) != windSpeed)
            {
                KoIW_CHTodayWind.value(windSpeed, DPT_Value_Wsp_kmh);
            }
            float windGust = 3.6 * (float) today["wind_gust"]; // 69
            logDebugP("Today Wind Gust: %f", windGust);   
            if ((float) KoIW_CHTodayWindGust.value(DPT_Value_Wsp_kmh) != windGust)
            {
                KoIW_CHTodayWindGust.value(windGust, DPT_Value_Wsp_kmh);
            }
            int windDirection = today["wind_deg"]; // 70
            logDebugP("Today Wind Direction: %d", windDirection);   
            if ((float) KoIW_CHTodayWindDirection.value(DPT_Angle) != windDirection)
            {
                KoIW_CHTodayWindDirection.value(windDirection, DPT_Angle);
            }   
            float rain = today["rain"]; // 2.5
            logDebugP("Today Rain: %f", rain); 
            if ((float) KoIW_CHTodayRain.value(DPT_Rain_Amount) != rain)
            {
                KoIW_CHTodayRain.value(rain, DPT_Rain_Amount);
            }   
            float snow = today["snow"]; // 2.5
            logDebugP("Today Snow: %f", snow); 
            if ((float) KoIW_CHTodaySnow.value(DPT_Length_mm) != snow)
            {
                KoIW_CHTodaySnow.value(snow, DPT_Length_mm);
            }  
            int probabilityOfPrecipitation = today["pop"]; // 70
            logDebugP("Today Probability Of Precipitation: %d", probabilityOfPrecipitation);   
            if ((float) KoIW_CHTodayProbabilityOfPrecipitation.value(DPT_Scaling) != probabilityOfPrecipitation)
            {
                KoIW_CHTodayProbabilityOfPrecipitation.value(probabilityOfPrecipitation, DPT_Scaling);
            }               
            int uvi = round((float)today["uvi"]); // 6.29   
            logDebugP("Today UVI: %d", uvi);   
            if ((int) KoIW_CHTodayUVI.value(DPT_DecimalFactor) != uvi)
            {
                KoIW_CHTodayUVI.value(uvi, DPT_DecimalFactor);
            }         
            int clouds = today["clouds"]; // 40
            logDebugP("Today Clouds: %d", clouds);   
            if ((int) KoIW_CHTodayClouds.value(DPT_Scaling) != clouds)
            {
                KoIW_CHTodayClouds.value(clouds, DPT_Scaling);
            }   
        }
        JsonObject tomorrow = daily[0];
        if (tomorrow != nullptr)
        {
            JsonObject tempObject = tomorrow["temp"]; 
            if (tempObject != nullptr)
            {
                float temp = tempObject["day"]; // 21.95
                logDebugP("Tomorrow Day: %f", temp);
                if ((float) KoIW_CHTomorrowTemparaturDay.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTomorrowTemparaturDay.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["night"]; // 21.95
                logDebugP("Tomorrow Night: %f", temp);
                if ((float) KoIW_CHTomorrowTemparaturNight.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTomorrowTemparaturNight.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["eve"]; // 21.95
                logDebugP("Tomorrow Evening: %f", temp);
                if ((float) KoIW_CHTomorrowTemparaturEvening.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTomorrowTemparaturEvening.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["morn"]; // 21.95
                logDebugP("Tomorrow Morning: %f", temp);
                if ((float) KoIW_CHTomorrowTemparaturMorning.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTomorrowTemparaturMorning.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["min"]; // 21.95
                logDebugP("Tomorrow Min: %f", temp);
                if ((float) KoIW_CHTomorrowTemparaturMin.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTomorrowTemparaturMin.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["max"]; // 21.95
                logDebugP("Tomorrow Max: %f", temp);
                if ((float) KoIW_CHTomorrowTemparaturMax.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTomorrowTemparaturMax.value(temp, DPT_Value_Temp);
                }
            }
            tempObject = tomorrow["feels_like"]; 
            if (tempObject != nullptr)
            {
                float temp = tempObject["day"]; // 21.95
                logDebugP("Tomorrow Day: %f", temp);
                if ((float) KoIW_CHTomorrowTemparaturDayFeelsLike.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTomorrowTemparaturDay.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["night"]; // 21.95
                logDebugP("Tomorrow Night: %f", temp);
                if ((float) KoIW_CHTomorrowTemparaturNightFeelsLike.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTomorrowTemparaturNight.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["eve"]; // 21.95
                logDebugP("Tomorrow Evening: %f", temp);
                if ((float) KoIW_CHTomorrowTemparaturEveningFeelsLike.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTomorrowTemparaturEvening.value(temp, DPT_Value_Temp);
                }
                temp = tempObject["morn"]; // 21.95
                logDebugP("Tomorrow Morning: %f", temp);
                if ((float) KoIW_CHTomorrowTemparaturMorningFeelsLike.value(DPT_Value_Temp) != temp)
                {
                    KoIW_CHTomorrowTemparaturMorning.value(temp, DPT_Value_Temp);
                }
            }
            float humidity = tomorrow["humidity"]; // 69
            logDebugP("Tomorrow Humidity: %f", humidity);
            if ((float) KoIW_CHTomorrowHumidity.value(DPT_Value_Humidity) != humidity)
            {
                KoIW_CHTomorrowHumidity.value(humidity, DPT_Value_Humidity);
            }
            int pressure = tomorrow["pressure"]; // 1006
            logDebugP("Tomorrow Pressure: %d", pressure);
            if ((float) KoIW_CHTomorrowPressure.value(DPT_Value_Pres) != pressure)
            {
                KoIW_CHTomorrowPressure.value(pressure, DPT_Value_Pres);
            }
            float windSpeed = 3.6 * (float) tomorrow["wind_speed"]; // 69
            logDebugP("Tomorrow Wind Speed: %f", windSpeed);   
            if ((float) KoIW_CHTomorrowWind.value(DPT_Value_Wsp_kmh) != windSpeed)
            {
                KoIW_CHTomorrowWind.value(windSpeed, DPT_Value_Wsp_kmh);
            }
            float windGust = 3.6 * (float) tomorrow["wind_gust"]; // 69
            logDebugP("Tomorrow Wind Gust: %f", windGust);   
            if ((float) KoIW_CHTomorrowWindGust.value(DPT_Value_Wsp_kmh) != windGust)
            {
                KoIW_CHTomorrowWindGust.value(windGust, DPT_Value_Wsp_kmh);
            }
            int windDirection = tomorrow["wind_deg"]; // 70
            logDebugP("Tomorrow Wind Direction: %d", windDirection);   
            if ((float) KoIW_CHTomorrowWindDirection.value(DPT_Angle) != windDirection)
            {
                KoIW_CHTomorrowWindDirection.value(windDirection, DPT_Angle);
            }              
            float rain = tomorrow["rain"]; // 2.5
            logDebugP("Tomorrow Rain: %f", rain); 
            if ((float) KoIW_CHTomorrowRain.value(DPT_Rain_Amount) != rain)
            {
                KoIW_CHTomorrowRain.value(rain, DPT_Rain_Amount);
            }   
            float snow = tomorrow["snow"]; // 2.5
            logDebugP("Tomorrow Snow: %f", snow); 
            if ((float) KoIW_CHTomorrowSnow.value(DPT_Length_mm) != snow)
            {
                KoIW_CHTomorrowSnow.value(snow, DPT_Length_mm);
            }   
            int probabilityOfPrecipitation = today["pop"]; // 70
            logDebugP("Tomorrow Probability Of Precipitation: %d", probabilityOfPrecipitation);   
            if ((float) KoIW_CHTomorrowProbabilityOfPrecipitation.value(DPT_Scaling) != probabilityOfPrecipitation)
            {
                KoIW_CHTomorrowProbabilityOfPrecipitation.value(probabilityOfPrecipitation, DPT_Scaling);
            }
            int uvi = round((float)tomorrow["uvi"]); // 6.29   
            logDebugP("Tomorrow UVI: %d", uvi);   
            if ((int) KoIW_CHTomorrowUVI.value(DPT_DecimalFactor) != uvi)
            {
                KoIW_CHTomorrowUVI.value(uvi, DPT_DecimalFactor);
            }         
            int clouds = tomorrow["clouds"]; // 40
            logDebugP("Tomorrow Clouds: %d", clouds);   
            if ((int) KoIW_CHTomorrowClouds.value(DPT_Scaling) != clouds)
            {
                KoIW_CHTomorrowClouds.value(clouds, DPT_Scaling);
            }   
        }
    }







// #define IW_KoCHCurrentUVI 10
// #define IW_KoCHCurrentClouds 11
// #define IW_KoCHTodayDescription 12
// #define IW_KoCHTodayTemparaturDay 13
// #define IW_KoCHTodayTemparaturNight 14
// #define IW_KoCHTodayTemparaturMorning 15
// #define IW_KoCHTodayTemparaturEvening 16
// #define IW_KoCHTodayTemparaturMin 17
// #define IW_KoCHTodayTemparaturMax 18
// #define IW_KoCHTodayTemparaturDayFeelsLike 19
// #define IW_KoCHTodayTemparaturNightFeelsLike 20
// #define IW_KoCHTodayTemparaturMorningFeelsLike 21
// #define IW_KoCHTodayTemparaturEveningFeelsLike 22
// #define IW_KoCHTodayHumidity 23
// #define IW_KoCHTodayPressure 24
// #define IW_KoCHTodayWind 25
// #define IW_KoCHTodayWindGust 26
// #define IW_KoCHTodayWindDirection 27
// #define IW_KoCHTodayRain 28
// #define IW_KoCHTodaySnow 29
// #define IW_KoCHTodayUVI 30
// #define IW_KoCHTodayClouds 31
// #define IW_KoCHTomorrowDescription 32
// #define IW_KoCHTomorrowTemparaturDay 33
// #define IW_KoCHTomorrowTemparaturNight 34
// #define IW_KoCHTomorrowTemparaturMorning 35
// #define IW_KoCHTomorrowTemparaturEvening 36
// #define IW_KoCHTomorrowTemparaturMin 37
// #define IW_KoCHTomorrowTemparaturMax 38
// #define IW_KoCHTomorrowTemparaturDayFeelsLike 39
// #define IW_KoCHTomorrowTemparaturNightFeelsLike 40
// #define IW_KoCHTomorrowTemparaturMorningFeelsLike 41
// #define IW_KoCHTomorrowTemparaturEveningFeelsLike 42
// #define IW_KoCHTomorrowHumidity 43
// #define IW_KoCHTomorrowPressure 44
// #define IW_KoCHTomorrowWind 45
// #define IW_KoCHTomorrowWindGust 46
// #define IW_KoCHTomorrowWindDirection 47
// #define IW_KoCHTomorrowRain 48
// #define IW_KoCHTomorrowSnow 49
// #define IW_KoCHTomorrowUVI 50
// #define IW_KoCHTomorrowClouds 51





                                    //  int current_pressure = current["pressure"]; // 1006
                                    //  int current_humidity = current["humidity"]; // 69
                                    //  float current_dew_point = current["dew_point"]; // 16.38
                                  
                                   
                                    //  int current_visibility = current["visibility"]; // 10000
                                    //  float current_wind_speed = current["wind_speed"]; // 3.6
                          

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