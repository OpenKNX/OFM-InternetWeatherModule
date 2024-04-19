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
    url += (const char *)ParamIW_APIKey;
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
    deserializeJson(doc, http.getString());

    JsonObject current = doc["current"];
    if (current != nullptr)
    {
        float temp = current["temp"]; // 22.34
        logDebugP("Temperature: %f", temp);
        if (KoIW_CHCurrentTemparatur.valueNoSendCompare(temp, DPT_Value_Temp))
            KoIW_CHCurrentTemparatur.objectWritten();

        float feelsLike = current["feels_like"]; // 21.95
        logDebugP("Temperature Feels Like: %f", feelsLike);
        if (KoIW_CHCurrentTemparaturFeelsLike.valueNoSendCompare(feelsLike, DPT_Value_Temp))
            KoIW_CHCurrentTemparaturFeelsLike.objectWritten();

        float humidity = current["humidity"]; // 69
        logDebugP("Humidity: %f", humidity);
        if (KoIW_CHCurrentHumidity.valueNoSendCompare(humidity, DPT_Value_Humidity))
            KoIW_CHCurrentHumidity.objectWritten();

        int pressure = current["pressure"]; // 1006
        logDebugP("Pressure: %d", pressure);
        if (KoIW_CHCurrentPressure.valueNoSendCompare(pressure, DPT_Value_Pres))
            KoIW_CHCurrentPressure.objectWritten();

        float windSpeed = 3.6 * (float)current["wind_speed"]; // 69
        logDebugP("Wind Speed: %f", windSpeed);
        if (KoIW_CHCurrentWind.valueNoSendCompare(windSpeed, DPT_Value_Wsp_kmh))
            KoIW_CHCurrentWind.objectWritten();

        float windGust = 3.6 * (float)current["wind_gust"]; // 69
        logDebugP("Wind Gust: %f", windGust);
        if (KoIW_CHCurrentWindGust.valueNoSendCompare(windGust, DPT_Value_Wsp_kmh))
            KoIW_CHCurrentWindGust.objectWritten();

        int windDirection = current["wind_deg"]; // 70
        logDebugP("Wind Direction: %d", windDirection);
        if (KoIW_CHCurrentWindDirection.valueNoSendCompare(windDirection, DPT_Angle))
            KoIW_CHCurrentWindDirection.objectWritten();

        JsonObject rainObject = current["rain"];
        if (rainObject != nullptr)
        {
            float rain = rainObject["1h"]; // 2.5
            logDebugP("Rain: %f", rain);
            if (KoIW_CHCurrentRain.valueNoSendCompare(rain, DPT_Rain_Amount))
                KoIW_CHCurrentRain.objectWritten();
        }
        JsonObject snowObject = current["snow"];
        if (snowObject != nullptr)
        {
            float snow = snowObject["1h"]; // 2.5
            logDebugP("Snow: %f", snow);
            if (KoIW_CHCurrentSnow.valueNoSendCompare(snow, DPT_Length_mm))
                KoIW_CHCurrentSnow.objectWritten();
        }
        int uvi = round((float)current["uvi"]); // 6.29
        logDebugP("UVI: %d", uvi);
        if (KoIW_CHCurrentUVI.valueNoSendCompare(uvi, DPT_DecimalFactor))
            KoIW_CHCurrentUVI.objectWritten();  

        int clouds = current["clouds"]; // 40
        logDebugP("Clouds: %d", clouds);
        if (KoIW_CHCurrentClouds.valueNoSendCompare(clouds, DPT_Scaling))
            KoIW_CHCurrentClouds.objectWritten();
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
                if (KoIW_CHTodayTemparaturDay.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTodayTemparaturDay.objectWritten();

                temp = tempObject["night"]; // 21.95
                logDebugP("Today Night: %f", temp);
                if (KoIW_CHTodayTemparaturNight.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTodayTemparaturNight.objectWritten();

                temp = tempObject["eve"]; // 21.95
                logDebugP("Today Evening: %f", temp);
                if (KoIW_CHTodayTemparaturEvening.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTodayTemparaturEvening.objectWritten();

                temp = tempObject["morn"]; // 21.95
                logDebugP("Today Morning: %f", temp);
                if (KoIW_CHTodayTemparaturMorning.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTodayTemparaturMorning.objectWritten();

                temp = tempObject["min"]; // 21.95
                logDebugP("Today Min: %f", temp);
                if (KoIW_CHTodayTemparaturMin.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTodayTemparaturMin.objectWritten();

                temp = tempObject["max"]; // 21.95
                logDebugP("Today Max: %f", temp);
                if (KoIW_CHTodayTemparaturMax.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTodayTemparaturMax.objectWritten();
            }
            tempObject = today["feels_like"];
            if (tempObject != nullptr)
            {
                float temp = tempObject["day"]; // 21.95
                logDebugP("Today Day: %f", temp);
                if (KoIW_CHTodayTemparaturDayFeelsLike.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTodayTemparaturDayFeelsLike.objectWritten();

                temp = tempObject["night"]; // 21.95
                logDebugP("Today Night: %f", temp);
                if (KoIW_CHTodayTemparaturNightFeelsLike.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTodayTemparaturNightFeelsLike.objectWritten();

                temp = tempObject["eve"]; // 21.95
                logDebugP("Today Evening: %f", temp);
                if (KoIW_CHTodayTemparaturEveningFeelsLike.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTodayTemparaturEveningFeelsLike.objectWritten();

                temp = tempObject["morn"]; // 21.95
                logDebugP("Today Morning: %f", temp);
                if (KoIW_CHTodayTemparaturMorningFeelsLike.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTodayTemparaturMorningFeelsLike.objectWritten();
            }
            float humidity = today["humidity"]; // 69
            logDebugP("Today Humidity: %f", humidity);
            if (KoIW_CHTodayHumidity.valueNoSendCompare(humidity, DPT_Value_Humidity))
                KoIW_CHTodayHumidity.objectWritten();

            int pressure = today["pressure"]; // 1006
            logDebugP("Today Pressure: %d", pressure);
            if (KoIW_CHTodayPressure.valueNoSendCompare(pressure, DPT_Value_Pres))
                KoIW_CHTodayPressure.objectWritten();

            float windSpeed = 3.6 * (float)today["wind_speed"]; // 69
            logDebugP("Today Wind Speed: %f", windSpeed);
            if (KoIW_CHTodayWind.valueNoSendCompare(windSpeed, DPT_Value_Wsp_kmh))
                KoIW_CHTodayWind.objectWritten();

            float windGust = 3.6 * (float)today["wind_gust"]; // 69
            logDebugP("Today Wind Gust: %f", windGust);
            if (KoIW_CHTodayWindGust.valueNoSendCompare(windGust, DPT_Value_Wsp_kmh))
                KoIW_CHTodayWindGust.objectWritten();

            int windDirection = today["wind_deg"]; // 70
            logDebugP("Today Wind Direction: %d", windDirection);
            if (KoIW_CHTodayWindDirection.valueNoSendCompare(windDirection, DPT_Angle))
                KoIW_CHTodayWindDirection.objectWritten();

            float rain = today["rain"]; // 2.5
            logDebugP("Today Rain: %f", rain);
            if (KoIW_CHTodayRain.valueNoSendCompare(rain, DPT_Rain_Amount))
                KoIW_CHTodayRain.objectWritten();

            float snow = today["snow"]; // 2.5
            logDebugP("Today Snow: %f", snow);
            if (KoIW_CHTodaySnow.valueNoSendCompare(snow, DPT_Length_mm))
                KoIW_CHTodaySnow.objectWritten();

            int probabilityOfPrecipitation = today["pop"]; // 70
            logDebugP("Today Probability Of Precipitation: %d", probabilityOfPrecipitation);
            if (KoIW_CHTodayProbabilityOfPrecipitation.valueNoSendCompare(probabilityOfPrecipitation, DPT_Scaling))
                KoIW_CHTodayProbabilityOfPrecipitation.objectWritten();

            int uvi = round((float)today["uvi"]); // 6.29
            logDebugP("Today UVI: %d", uvi);
            if (KoIW_CHTodayUVI.valueNoSendCompare(uvi, DPT_DecimalFactor))
                KoIW_CHTodayUVI.objectWritten();

            int clouds = today["clouds"]; // 40
            logDebugP("Today Clouds: %d", clouds);
            if ((int)KoIW_CHTodayClouds.value(DPT_Scaling) != clouds)
            {
                KoIW_CHTodayClouds.value(clouds, DPT_Scaling);
            }
            setDescription(KoIW_CHTodayDescription, (const char *)ParamIW_WheaterConditionCurrentDayPrefix, rain, snow, clouds, probabilityOfPrecipitation);
        }
        JsonObject tomorrow = daily[1];
        if (tomorrow != nullptr)
        {
            JsonObject tempObject = tomorrow["temp"];
            if (tempObject != nullptr)
            {
                float temp = tempObject["day"]; // 21.95
                logDebugP("Tomorrow Day: %f", temp);
                if (KoIW_CHTomorrowTemparaturDay.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTomorrowTemparaturDay.objectWritten();

                temp = tempObject["night"]; // 21.95
                logDebugP("Tomorrow Night: %f", temp);
                if (KoIW_CHTomorrowTemparaturNight.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTomorrowTemparaturNight.objectWritten();

                temp = tempObject["eve"]; // 21.95
                logDebugP("Tomorrow Evening: %f", temp);
                if (KoIW_CHTomorrowTemparaturEvening.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTomorrowTemparaturEvening.objectWritten();

                temp = tempObject["morn"]; // 21.95
                logDebugP("Tomorrow Morning: %f", temp);
                if (KoIW_CHTomorrowTemparaturMorning.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTomorrowTemparaturMorning.objectWritten();

                temp = tempObject["min"]; // 21.95
                logDebugP("Tomorrow Min: %f", temp);
                if (KoIW_CHTomorrowTemparaturMin.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTomorrowTemparaturMin.objectWritten();

                temp = tempObject["max"]; // 21.95
                logDebugP("Tomorrow Max: %f", temp);
                if (KoIW_CHTomorrowTemparaturMax.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTomorrowTemparaturMax.objectWritten(); 
            }
            tempObject = tomorrow["feels_like"];
            if (tempObject != nullptr)
            {
                float temp = tempObject["day"]; // 21.95
                logDebugP("Tomorrow Day: %f", temp);
                if (KoIW_CHTomorrowTemparaturDayFeelsLike.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTomorrowTemparaturDayFeelsLike.objectWritten();

                temp = tempObject["night"]; // 21.95
                logDebugP("Tomorrow Night: %f", temp);
                if (KoIW_CHTomorrowTemparaturNightFeelsLike.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTomorrowTemparaturNightFeelsLike.objectWritten();

                temp = tempObject["eve"]; // 21.95
                logDebugP("Tomorrow Evening: %f", temp);
                if (KoIW_CHTomorrowTemparaturEveningFeelsLike.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTomorrowTemparaturEveningFeelsLike.objectWritten();

                temp = tempObject["morn"]; // 21.95
                logDebugP("Tomorrow Morning: %f", temp);
                if (KoIW_CHTomorrowTemparaturMorningFeelsLike.valueNoSendCompare(temp, DPT_Value_Temp))
                    KoIW_CHTomorrowTemparaturMorningFeelsLike.objectWritten();
            }
            float humidity = tomorrow["humidity"]; // 69
            logDebugP("Tomorrow Humidity: %f", humidity);
            if (KoIW_CHTomorrowHumidity.valueNoSendCompare(humidity, DPT_Value_Humidity))
                KoIW_CHTomorrowHumidity.objectWritten();

            int pressure = tomorrow["pressure"]; // 1006
            logDebugP("Tomorrow Pressure: %d", pressure);
            if (KoIW_CHTomorrowPressure.valueNoSendCompare(pressure, DPT_Value_Pres))
                KoIW_CHTomorrowPressure.objectWritten();

            float windSpeed = 3.6 * (float)tomorrow["wind_speed"]; // 69
            logDebugP("Tomorrow Wind Speed: %f", windSpeed);
            if (KoIW_CHTomorrowWind.valueNoSendCompare(windSpeed, DPT_Value_Wsp_kmh))
                KoIW_CHTomorrowWind.objectWritten();

            float windGust = 3.6 * (float)tomorrow["wind_gust"]; // 69
            logDebugP("Tomorrow Wind Gust: %f", windGust);
            if (KoIW_CHTomorrowWindGust.valueNoSendCompare(windGust, DPT_Value_Wsp_kmh))
                KoIW_CHTomorrowWindGust.objectWritten();

            int windDirection = tomorrow["wind_deg"]; // 70
            logDebugP("Tomorrow Wind Direction: %d", windDirection);
            if (KoIW_CHTomorrowWindDirection.valueNoSendCompare(windDirection, DPT_Angle))
                KoIW_CHTomorrowWindDirection.objectWritten();

            float rain = tomorrow["rain"]; // 2.5
            logDebugP("Tomorrow Rain: %f", rain);
            if (KoIW_CHTomorrowRain.valueNoSendCompare(rain, DPT_Rain_Amount))
                KoIW_CHTomorrowRain.objectWritten();

            float snow = tomorrow["snow"]; // 2.5
            logDebugP("Tomorrow Snow: %f", snow);
            if (KoIW_CHTomorrowSnow.valueNoSendCompare(snow, DPT_Length_mm))
                KoIW_CHTomorrowSnow.objectWritten();

            int probabilityOfPrecipitation = today["pop"]; // 70
            logDebugP("Tomorrow Probability Of Precipitation: %d", probabilityOfPrecipitation);
            if (KoIW_CHTomorrowProbabilityOfPrecipitation.valueNoSendCompare(probabilityOfPrecipitation, DPT_Scaling))
                KoIW_CHTomorrowProbabilityOfPrecipitation.objectWritten();

            int uvi = round((float)tomorrow["uvi"]); // 6.29
            logDebugP("Tomorrow UVI: %d", uvi);
            if (KoIW_CHTomorrowUVI.valueNoSendCompare(uvi, DPT_DecimalFactor))
                KoIW_CHTomorrowUVI.objectWritten();

            int clouds = tomorrow["clouds"]; // 40
            logDebugP("Tomorrow Clouds: %d", clouds);
            if ((int)KoIW_CHTomorrowClouds.value(DPT_Scaling) != clouds)
            {
                KoIW_CHTomorrowClouds.value(clouds, DPT_Scaling);
            }
            setDescription(KoIW_CHTomorrowDescription, (const char *)ParamIW_WheaterConditionNextDayPrefix, rain, snow, clouds, probabilityOfPrecipitation);            
        }
    }
}