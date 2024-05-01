#include "BaseWeatherChannel.h"
#include "WLANModule.h"
#include "NetworkModule.h"

BaseWeatherChannel::BaseWeatherChannel(uint8_t index)
{
    _channelIndex = index;
}

void BaseWeatherChannel::setup()
{
    // <Enumeration Text="Keine"        Value="0" Id="%ENID%" />
    // <Enumeration Text="10 Minuten"   Value="1" Id="%ENID%" />
    // <Enumeration Text="30 Minuten"   Value="2" Id="%ENID%" />
    // <Enumeration Text="Jede Stunde"  Value="3" Id="%ENID%" />
    switch (ParamIW_WeatherRefreshInterval)
    {
        case 1:
            _updateIntervalInMs = 10 * 60 * 1000;
            break;
        case 2:
            _updateIntervalInMs = 30 * 60 * 1000;
            break;
        case 3:
            _updateIntervalInMs = 60 * 60 * 1000;
            break;
    }
    logDebugP("Update interval: %ldms", _updateIntervalInMs);
    KoIW_CHForecastSelection.value(false, DPT_Switch);
}

void BaseWeatherChannel::processInputKo(GroupObject& ko)
{
    // channel ko
    auto index = IW_KoCalcIndex(ko.asap());
    switch (index)
    {
        case IW_KoCHForecastSelection:
            updateSwitchableKos();
            break;
    }
    // module ko
    switch (ko.asap())
    {
        case IW_KoRefreshWeatherData:
            if (ko.value(DPT_Trigger))
                fetchData();
            break;
    }
}

void BaseWeatherChannel::loop()
{
#ifdef WLAN_WifiSSID
    if (openknxWLANModule.connected())
#else
    if (openknxNetwork.connected())
#endif
    {
        auto now = millis();
        if (now == 0)
            now++; // Do not use 0 because it is used as marker for unitialized

        if (_updateIntervalInMs > 0 &&
            (_lastApiCall == 0 || (now - _lastApiCall > _updateIntervalInMs)))
        {
            _lastApiCall = now;
            fetchData();
        }
    }
}

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

bool BaseWeatherChannel::processCommand(const std::string cmd, bool diagnoseKo)
{  
    if (cmd == "s0")
    {
        KoIW_CHForecastSelection.value(false, DPT_Switch);
        return true;
    }
    else if (cmd == "s1")
    {
        KoIW_CHForecastSelection.value(true, DPT_Switch);
        return true;
    }
    else if (cmd == "update")
    {
        fetchData();
    }
    return false;
}

void BaseWeatherChannel::buildDescription(char* description, float rain, float snow, uint8_t clouds, const char* prefix)
{
    memset(description, 0, 15) ; // Must be 0 terminated
    char* buffer = description;
    int bufferSize = 15;
    auto prefixLen = strlen(prefix);
    if (prefixLen > 0)
    {
        strncpy(buffer, prefix, bufferSize - 1);
        bufferSize -= prefixLen;
        buffer += prefixLen;
    }
    auto all = rain + snow;
    if (all > 0.5)
    {
        std::string formatText;
        float value;
        if (snow > rain)
        {
            formatText = (const char*)ParamIW_WeatherConditionSnow;
            value = snow;
        }
        else
        {
            formatText = (const char*)ParamIW_WeatherConditionRain;
            value = rain;
        }
        replaceAll(formatText, "%", "%%");
        replaceAll(formatText, "XXX", all >= 10 ? "%.0f" : "%.1f");
        snprintf(buffer, bufferSize, formatText.c_str(), value);
    }
    else
    {
        if (clouds <= 5)
        {
            snprintf(buffer, bufferSize, "%s", (const char*)ParamIW_WeatherConditionSun);
        }
        else
        {
            std::string formatText((const char*)ParamIW_WeatherConditionClouds);
            replaceAll(formatText, "%", "%%");
            replaceAll(formatText, "XXX", "%d");
            snprintf(buffer, bufferSize, formatText.c_str(), clouds);
        }
    }
}

void BaseWeatherChannel::setValueCompare(GroupObject& groupObject, const KNXValue& value, const Dpt& type)
{
    if (groupObject.valueNoSendCompare(value, type))
        groupObject.objectWritten();
}
void BaseWeatherChannel::fetchData()
{
    CurrentWheatherData current = CurrentWheatherData();
    ForecastDayWheatherDataWithDescription today = ForecastDayWheatherDataWithDescription();
    ForecastDayWheatherDataWithDescription tomorrow = ForecastDayWheatherDataWithDescription();
    ForecastHourWheatherData hour1 = ForecastHourWheatherData();
    ForecastHourWheatherData hour2 = ForecastHourWheatherData();

    int16_t httpStatus = fillWeather(current, today, tomorrow, hour1, hour2);
    KoIW_CHHTTPStatus.value(httpStatus, DPT_Value_2_Count);
    if (httpStatus != 200)
    {
        logErrorP("Http result %d", httpStatus);
        return;
    }
    else
    {
        logDebugP("Http result %d", httpStatus);
    } 
    buildDescription(today.description, today.rain, today.snow, today.clouds, (const char*)ParamIW_WeatherConditionCurrentDayPrefix);
    _descriptionToday = today.description;
    buildDescription(tomorrow.description, tomorrow.rain, tomorrow.snow, tomorrow.clouds, (const char*)ParamIW_WeatherConditionNextDayPrefix);
    _descriptionTomorrow = tomorrow.description;

    logDebugP("Temperature: %f", current.temperature);
    setValueCompare(KoIW_CHCurrentTemparatur, current.temperature, DPT_Value_Temp);
    logDebugP("Temperature feels like: %f", current.temperatureFeelsLike);
    setValueCompare(KoIW_CHCurrentTemparaturFeelsLike, current.temperatureFeelsLike, DPT_Value_Temp);
    logDebugP("Humidity: %f", current.humidity);
    setValueCompare(KoIW_CHCurrentHumidity, current.humidity, DPT_Value_Humidity);
    logDebugP("Pressure: %f", current.pressure);
    setValueCompare(KoIW_CHCurrentPressure, current.pressure, DPT_Value_Pres);
    logDebugP("Wind speed: %f", current.windSpeed);
    setValueCompare(KoIW_CHCurrentWind, current.windSpeed, DPT_Value_Wsp_kmh);
    logDebugP("Wind gust: %f", current.windGust);
    setValueCompare(KoIW_CHCurrentWindGust, current.windGust, DPT_Value_Wsp_kmh);
    logDebugP("Wind direction: %d", (int) current.windDirection);
    setValueCompare(KoIW_CHCurrentWindDirection, current.windDirection, DPT_Angle);
    logDebugP("Rain: %f", current.rain);
    setValueCompare(KoIW_CHCurrentRain, current.rain, DPT_Rain_Amount);
    logDebugP("Snow: %f", current.rain);
    setValueCompare(KoIW_CHCurrentSnow, current.snow, DPT_Length_mm);
    logDebugP("UVI: %f", current.uvi);
    setValueCompare(KoIW_CHCurrentUVI, (uint8_t)round(current.uvi), DPT_DecimalFactor);
    logDebugP("Clouds: %d", (int) current.clouds);
    setValueCompare(KoIW_CHCurrentClouds, current.clouds, DPT_Scaling);

    logDebugP("Today description: %s", today.description);
    setValueCompare(KoIW_CHTodayDescription, today.description, DPT_String_8859_1);
    logDebugP("Today temperature morning: %f", today.temperatureMorning);
    setValueCompare(KoIW_CHTodayTemparaturMorning, today.temperatureMorning, DPT_Value_Temp);
    logDebugP("Today temperature day: %f", today.temperatureDay);
    setValueCompare(KoIW_CHTodayTemparaturDay, today.temperatureDay, DPT_Value_Temp);
    logDebugP("Today temperature evening: %f", today.temperatureEvening);
    setValueCompare(KoIW_CHTodayTemparaturEvening, today.temperatureEvening, DPT_Value_Temp);
    logDebugP("Today temperature night: %f", today.temperatureNight);
    setValueCompare(KoIW_CHTodayTemparaturNight, today.temperatureNight, DPT_Value_Temp);
    logDebugP("Today temperature min: %f", today.temperatureMin);
    setValueCompare(KoIW_CHTodayTemparaturMin, today.temperatureMin, DPT_Value_Temp);
    logDebugP("Today temperature max: %f", today.temperatureMax);
    setValueCompare(KoIW_CHTodayTemparaturMax, today.temperatureMax, DPT_Value_Temp);
    logDebugP("Today temperature morning feels like: %f", today.temperatureFeelsLikeMorning);
    setValueCompare(KoIW_CHTodayTemparaturMorningFeelsLike, today.temperatureFeelsLikeEvening, DPT_Value_Temp);
    logDebugP("Today temperature day feels like: %f", today.temperatureFeelsLikeDay);
    setValueCompare(KoIW_CHTodayTemparaturDayFeelsLike, today.temperatureFeelsLikeDay, DPT_Value_Temp);
    logDebugP("Today temperature evening feels like: %f", today.temperatureFeelsLikeEvening);
    setValueCompare(KoIW_CHTodayTemparaturEveningFeelsLike, today.temperatureFeelsLikeEvening, DPT_Value_Temp);
    logDebugP("Today temperature night feels like: %f", today.temperatureFeelsLikeNight);
    setValueCompare(KoIW_CHTodayTemparaturNightFeelsLike, today.temperatureFeelsLikeNight, DPT_Value_Temp);
    logDebugP("Today humidity: %f", today.humidity);
    setValueCompare(KoIW_CHTodayHumidity, today.humidity, DPT_Value_Humidity);
    logDebugP("Today pressure: %d", (int) today.pressure);
    setValueCompare(KoIW_CHTodayPressure, today.pressure, DPT_Value_Pres);
    logDebugP("Today wind speed: %f", today.windSpeed);
    setValueCompare(KoIW_CHTodayWind, today.windSpeed, DPT_Value_Wsp_kmh);
    logDebugP("Today wind gust: %f", today.windGust);
    setValueCompare(KoIW_CHTodayWindGust, today.windGust, DPT_Value_Wsp_kmh);
    logDebugP("Today wind direction: %d", (int) today.windDirection);
    setValueCompare(KoIW_CHTodayWindDirection, today.windDirection, DPT_Angle);
    logDebugP("Today rain: %f", today.rain);
    setValueCompare(KoIW_CHTodayRain, today.rain, DPT_Rain_Amount);
    logDebugP("Today snow: %f", today.snow);
    setValueCompare(KoIW_CHTodaySnow, today.snow, DPT_Length_mm);
    logDebugP("Today probability of precipitation: %d", (int) today.probabilityOfPrecipitation);
    setValueCompare(KoIW_CHTodayProbabilityOfPrecipitation, today.probabilityOfPrecipitation, DPT_Scaling);
    logDebugP("Today UVI: %f", today.uvi);
    setValueCompare(KoIW_CHTodayUVI, (uint8_t)round(today.uvi), DPT_DecimalFactor);
    logDebugP("Today clouds: %d", (int) today.clouds);
    setValueCompare(KoIW_CHTodayClouds, today.clouds, DPT_Scaling);

    logDebugP("Tomorrow description: %s", tomorrow .description);
    setValueCompare(KoIW_CHTomorrowDescription, tomorrow .description, DPT_String_8859_1);
    logDebugP("Tomorrow temperature morning: %f", tomorrow .temperatureMorning);
    setValueCompare(KoIW_CHTomorrowTemparaturMorning, tomorrow .temperatureMorning, DPT_Value_Temp);
    logDebugP("Tomorrow temperature day: %f", tomorrow .temperatureDay);
    setValueCompare(KoIW_CHTomorrowTemparaturDay, tomorrow .temperatureDay, DPT_Value_Temp);
    logDebugP("Tomorrow temperature evening: %f", tomorrow .temperatureEvening);
    setValueCompare(KoIW_CHTomorrowTemparaturEvening, tomorrow .temperatureEvening, DPT_Value_Temp);
    logDebugP("Tomorrow temperature night: %f", tomorrow .temperatureNight);
    setValueCompare(KoIW_CHTomorrowTemparaturNight, tomorrow .temperatureNight, DPT_Value_Temp);
    logDebugP("Tomorrow temperature min: %f", tomorrow .temperatureMin);
    setValueCompare(KoIW_CHTomorrowTemparaturMin, tomorrow .temperatureMin, DPT_Value_Temp);
    logDebugP("Tomorrow temperature max: %f", tomorrow .temperatureMax);
    setValueCompare(KoIW_CHTomorrowTemparaturMax, tomorrow .temperatureMax, DPT_Value_Temp);
    logDebugP("Tomorrow temperature morning feels like: %f", tomorrow .temperatureFeelsLikeMorning);
    setValueCompare(KoIW_CHTomorrowTemparaturMorningFeelsLike, tomorrow .temperatureFeelsLikeEvening, DPT_Value_Temp);
    logDebugP("Tomorrow temperature day feels like: %f", tomorrow .temperatureFeelsLikeDay);
    setValueCompare(KoIW_CHTomorrowTemparaturDayFeelsLike, tomorrow .temperatureFeelsLikeDay, DPT_Value_Temp);
    logDebugP("Tomorrow temperature evening feels like: %f", tomorrow .temperatureFeelsLikeEvening);
    setValueCompare(KoIW_CHTomorrowTemparaturEveningFeelsLike, tomorrow .temperatureFeelsLikeEvening, DPT_Value_Temp);
    logDebugP("Tomorrow temperature night feels like: %f", tomorrow .temperatureFeelsLikeNight);
    setValueCompare(KoIW_CHTomorrowTemparaturNightFeelsLike, tomorrow .temperatureFeelsLikeNight, DPT_Value_Temp);
    logDebugP("Tomorrow humidity: %f", tomorrow .humidity);
    setValueCompare(KoIW_CHTomorrowHumidity, tomorrow .humidity, DPT_Value_Humidity);
    logDebugP("Tomorrow pressure: %d", (int) tomorrow .pressure);
    setValueCompare(KoIW_CHTomorrowPressure, tomorrow .pressure, DPT_Value_Pres);
    logDebugP("Tomorrow wind speed: %f", tomorrow .windSpeed);
    setValueCompare(KoIW_CHTomorrowWind, tomorrow .windSpeed, DPT_Value_Wsp_kmh);
    logDebugP("Tomorrow wind gust: %f", tomorrow .windGust);
    setValueCompare(KoIW_CHTomorrowWindGust, tomorrow .windGust, DPT_Value_Wsp_kmh);
    logDebugP("Tomorrow wind direction: %d", (int) tomorrow .windDirection);
    setValueCompare(KoIW_CHTomorrowWindDirection, tomorrow .windDirection, DPT_Angle);
    logDebugP("Tomorrow rain: %f", tomorrow .rain);
    setValueCompare(KoIW_CHTomorrowRain, tomorrow .rain, DPT_Rain_Amount);
    logDebugP("Tomorrow snow: %f", tomorrow .snow);
    setValueCompare(KoIW_CHTomorrowSnow, tomorrow .snow, DPT_Length_mm);
    logDebugP("Tomorrow probability of precipitation: %d", (int) tomorrow .probabilityOfPrecipitation);
    setValueCompare(KoIW_CHTomorrowProbabilityOfPrecipitation, tomorrow .probabilityOfPrecipitation, DPT_Scaling);
    logDebugP("Tomorrow UVI: %f", tomorrow .uvi);
    setValueCompare(KoIW_CHTomorrowUVI, (uint8_t)round(tomorrow .uvi), DPT_DecimalFactor);
    logDebugP("Tomorrow clouds: %d", (int) tomorrow .clouds);
    setValueCompare(KoIW_CHTomorrowClouds, tomorrow .clouds, DPT_Scaling);

    updateSwitchableKos();

    char description[15];
    buildDescription(description, hour1.rain, hour1.snow, hour1.clouds, "");
    logDebugP("Hour + 1 Description: %s", description);
    setValueCompare(KoIW_CHHour1Description, description, DPT_String_8859_1);
    logDebugP("Hour + 1 Temperature: %f", hour1.temperature);
    setValueCompare(KoIW_CHHour1Temparatur, hour1.temperature, DPT_Value_Temp);
    logDebugP("Hour + 1 Temperature feels like: %f", hour1.temperatureFeelsLike);
    setValueCompare(KoIW_CHHour1TemparaturFeelsLike, hour1.temperatureFeelsLike, DPT_Value_Temp);
    logDebugP("Hour + 1 Humidity: %f", hour1.humidity);
    setValueCompare(KoIW_CHHour1Humidity, hour1.humidity, DPT_Value_Humidity);
    logDebugP("Hour + 1 Pressure: %f", hour1.pressure);
    setValueCompare(KoIW_CHHour1Pressure, hour1.pressure, DPT_Value_Pres);
    logDebugP("Hour + 1 Wind speed: %f", hour1.windSpeed);
    setValueCompare(KoIW_CHHour1Wind, hour1.windSpeed, DPT_Value_Wsp_kmh);
    logDebugP("Hour + 1 Wind gust: %f", hour1.windGust);
    setValueCompare(KoIW_CHHour1WindGust, hour1.windGust, DPT_Value_Wsp_kmh);
    logDebugP("Hour + 1 Wind direction: %d", (int) hour1.windDirection);
    setValueCompare(KoIW_CHHour1WindDirection, hour1.windDirection, DPT_Angle);
    logDebugP("Hour + 1 Rain: %f", hour1.rain);
    setValueCompare(KoIW_CHHour1Rain, hour1.rain, DPT_Rain_Amount);
    logDebugP("Hour + 1 Snow: %f", hour1.rain);
    setValueCompare(KoIW_CHHour1Snow, hour1.snow, DPT_Length_mm);
    logDebugP("Hour + 1 Probability of precipitation: %d", (int) hour1.probabilityOfPrecipitation);
    setValueCompare(KoIW_CHHour1ProbabilityOfPrecipitation, hour1.probabilityOfPrecipitation, DPT_Scaling);
    logDebugP("Hour + 1 UVI: %f", hour1.uvi);
    setValueCompare(KoIW_CHHour1UVI, (uint8_t)round(hour1.uvi), DPT_DecimalFactor);
    logDebugP("Hour + 1 Clouds: %d", (int) hour1.clouds);
    setValueCompare(KoIW_CHHour1Clouds, hour1.clouds, DPT_Scaling);

    buildDescription(description, hour2.rain, hour2.snow, hour2.clouds, "");
    logDebugP("Hour + 2 Description: %s", description);
    setValueCompare(KoIW_CHHour2Description, description, DPT_String_8859_1);
    logDebugP("Hour + 2 Temperature: %f", hour2.temperature);
    setValueCompare(KoIW_CHHour2Temparatur, hour2.temperature, DPT_Value_Temp);
    logDebugP("Hour + 2 Temperature feels like: %f", hour2.temperatureFeelsLike);
    setValueCompare(KoIW_CHHour2TemparaturFeelsLike, hour2.temperatureFeelsLike, DPT_Value_Temp);
    logDebugP("Hour + 2 Humidity: %f", hour2.humidity);
    setValueCompare(KoIW_CHHour2Humidity, hour2.humidity, DPT_Value_Humidity);
    logDebugP("Hour + 2 Pressure: %f", hour2.pressure);
    setValueCompare(KoIW_CHHour2Pressure, hour2.pressure, DPT_Value_Pres);
    logDebugP("Hour + 2 Wind speed: %f", hour2.windSpeed);
    setValueCompare(KoIW_CHHour2Wind, hour2.windSpeed, DPT_Value_Wsp_kmh);
    logDebugP("Hour + 2 Wind gust: %f", hour2.windGust);
    setValueCompare(KoIW_CHHour2WindGust, hour2.windGust, DPT_Value_Wsp_kmh);
    logDebugP("Hour + 2 Wind direction: %d", (int) hour2.windDirection);
    setValueCompare(KoIW_CHHour2WindDirection, hour2.windDirection, DPT_Angle);
    logDebugP("Hour + 2 Rain: %f", hour2.rain);
    setValueCompare(KoIW_CHHour2Rain, hour2.rain, DPT_Rain_Amount);
    logDebugP("Hour + 2 Snow: %f", hour2.rain);
    setValueCompare(KoIW_CHHour2Snow, hour2.snow, DPT_Length_mm);
    logDebugP("Hour + 2 Probability of precipitation: %d", (int) hour2.probabilityOfPrecipitation);
    setValueCompare(KoIW_CHHour2ProbabilityOfPrecipitation, hour2.probabilityOfPrecipitation, DPT_Scaling);
    logDebugP("Hour + 2 UVI: %f", hour2.uvi);
    setValueCompare(KoIW_CHHour2UVI, (uint8_t)round(hour2.uvi), DPT_DecimalFactor);
    logDebugP("Hour + 2 Clouds: %d", (int) hour2.clouds);
    setValueCompare(KoIW_CHHour2Clouds, hour2.clouds, DPT_Scaling);
}

void BaseWeatherChannel::copyGroupObject(GroupObject& koTarget, bool select, GroupObject& ko1, GroupObject& ko2)
{
    auto koSource = select ? ko2 : ko1;
    bool intialized = koTarget.initialized();
    if (intialized && memcmp(koTarget.valueRef(),  koSource.valueRef(), koTarget.valueSize()) == 0)
        return;
    memcpy(koTarget.valueRef(), koSource.valueRef(), koSource.valueSize());
    koTarget.objectWritten();
}

void BaseWeatherChannel::updateSwitchableKos()
{
    auto select = (bool) KoIW_CHForecastSelection.value(DPT_Switch);
    logDebugP("update switchable KO's to %s", select ? "tomorrow" : "today");
    if (KoIW_CHForecastDescription.valueNoSendCompare(select ? _descriptionTomorrow.c_str() : _descriptionToday.c_str(), DPT_String_8859_1))
        KoIW_CHForecastDescription.objectWritten();
    copyGroupObject(KoIW_CHForecastTemparaturDay, select, KoIW_CHTodayTemparaturDay, KoIW_CHTomorrowTemparaturDay);
    copyGroupObject(KoIW_CHForecastTemparaturNight, select, KoIW_CHTodayTemparaturNight, KoIW_CHTomorrowTemparaturNight);
    copyGroupObject(KoIW_CHForecastTemparaturEvening, select, KoIW_CHTodayTemparaturEvening, KoIW_CHTomorrowTemparaturEvening);
    copyGroupObject(KoIW_CHForecastTemparaturMorning, select, KoIW_CHTodayTemparaturMorning, KoIW_CHTomorrowTemparaturMorning);
    copyGroupObject(KoIW_CHForecastTemparaturMin, select, KoIW_CHTodayTemparaturMin, KoIW_CHTomorrowTemparaturMin);
    copyGroupObject(KoIW_CHForecastTemparaturMax, select, KoIW_CHTodayTemparaturMax, KoIW_CHTomorrowTemparaturMax);
    copyGroupObject(KoIW_CHForecastTemparaturDayFeelsLike, select, KoIW_CHTodayTemparaturDayFeelsLike, KoIW_CHTomorrowTemparaturDayFeelsLike);
    copyGroupObject(KoIW_CHForecastTemparaturNightFeelsLike, select, KoIW_CHTodayTemparaturNightFeelsLike, KoIW_CHTomorrowTemparaturNightFeelsLike);
    copyGroupObject(KoIW_CHForecastTemparaturEveningFeelsLike, select, KoIW_CHTodayTemparaturEveningFeelsLike, KoIW_CHTomorrowTemparaturEveningFeelsLike);
    copyGroupObject(KoIW_CHForecastTemparaturMorningFeelsLike, select, KoIW_CHTodayTemparaturMorningFeelsLike, KoIW_CHTomorrowTemparaturMorningFeelsLike);
    copyGroupObject(KoIW_CHForecastHumidity, select, KoIW_CHTodayHumidity, KoIW_CHTomorrowHumidity);
    copyGroupObject(KoIW_CHForecastPressure, select, KoIW_CHTodayPressure, KoIW_CHTomorrowPressure);
    copyGroupObject(KoIW_CHForecastWind, select, KoIW_CHTodayWind, KoIW_CHTomorrowWind);
    copyGroupObject(KoIW_CHForecastWindGust, select, KoIW_CHTodayWindGust, KoIW_CHTomorrowWindGust);
    copyGroupObject(KoIW_CHForecastWindDirection, select, KoIW_CHTodayWindDirection, KoIW_CHTomorrowWindDirection);
    copyGroupObject(KoIW_CHForecastRain, select, KoIW_CHTodayRain, KoIW_CHTomorrowRain);
    copyGroupObject(KoIW_CHForecastSnow, select, KoIW_CHTodaySnow, KoIW_CHTomorrowSnow);
    copyGroupObject(KoIW_CHForecastProbabilityOfPrecipitation, select, KoIW_CHTodayProbabilityOfPrecipitation, KoIW_CHTomorrowProbabilityOfPrecipitation);
    copyGroupObject(KoIW_CHForecastUVI, select, KoIW_CHTodayUVI, KoIW_CHTomorrowUVI);
    copyGroupObject(KoIW_CHForecastClouds, select, KoIW_CHTodayClouds, KoIW_CHTomorrowClouds);
  }