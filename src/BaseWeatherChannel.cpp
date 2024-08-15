#include "BaseWeatherChannel.h"
#ifdef WLAN_WifiSSID
#include "WiFi.h"
#else
#include "NetworkModule.h"
#endif
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
            if (_available)
            {
                bool select = (bool) KoIW_CHForecastSelection.value(DPT_Switch);
                logDebugP("changed switchable KO's to %s", select ? "tomorrow" : "today");
                updateDayForecastKo(select ? tomorrow : today, IW_KoOffset_Forecast);
            }
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
    if (WiFi.isConnected())
#else
    if (openknxNetwork.established())
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
            formatText = (const char*)ParamIW_TextSnow;
            value = snow;
        }
        else
        {
            formatText = (const char*)ParamIW_TextRain;
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
            snprintf(buffer, bufferSize, "%s", (const char*)ParamIW_TextSun);
        }
        else
        {
            std::string formatText((const char*)ParamIW_TextClouds);
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

void BaseWeatherChannel::setValueCompare(uint goNumber, const KNXValue& value, const Dpt& type)
{
    GroupObject& groupObject = (knx.getGroupObject(IW_KoCalcNumber(goNumber)));
    setValueCompare(groupObject, value, type);
}

void BaseWeatherChannel::updateUviKo(GroupObject& groupObject, float uviFloatValue)
{
    logDebugP("UVI: %f", uviFloatValue);
    setValueCompare(groupObject, (uint8_t)(max((uint8_t) 0, (uint8_t) min(round(uviFloatValue), (float) 255))), DPT_DecimalFactor);
}

void BaseWeatherChannel::fetchData()
{
    CurrentWheatherData current = CurrentWheatherData();
    /*
    ForecastDayWheatherDataWithDescription today = ForecastDayWheatherDataWithDescription();
    ForecastDayWheatherDataWithDescription tomorrow = ForecastDayWheatherDataWithDescription();
    */
    ForecastHourWheatherData hour1 = ForecastHourWheatherData();
    ForecastHourWheatherData hour2 = ForecastHourWheatherData();

    int16_t httpStatus = fillWeather(current, today, tomorrow, hour1, hour2);
    KoIW_CHHTTPStatus.value(httpStatus, DPT_Value_2_Count);
    if (httpStatus != 200)
    {
        _available = false;
        logErrorP("Http result %d", httpStatus);
        return;
    }
    else
    {
        // set _available to true after description is 
        logDebugP("Http result %d", httpStatus);
    } 
    buildDescription(today.description, today.rain, today.snow, today.clouds, (const char*)ParamIW_TextPrefixDayCurrent);
    buildDescription(tomorrow.description, tomorrow.rain, tomorrow.snow, tomorrow.clouds, (const char*)ParamIW_TextPrefixDayNext);
    _available = true;

    if (ParamIW_CHOutCurrent)
    {
        logDebugP("Current:");
        logIndentUp();
        logDebugP("Temperature: %f", current.temperature);
        setValueCompare(KoIW_CHCurrentTemperatur, current.temperature, DPT_Value_Temp);
        logDebugP("Temperature feels like: %f", current.temperatureFeelsLike);
        setValueCompare(KoIW_CHCurrentTemperaturFeelsLike, current.temperatureFeelsLike, DPT_Value_Temp);
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
        updateUviKo(KoIW_CHCurrentUVI, current.uvi);
        logDebugP("Clouds: %d", (int) current.clouds);
        setValueCompare(KoIW_CHCurrentClouds, current.clouds, DPT_Scaling);
        logIndentDown();
    }

    if (ParamIW_CHOutToday)
    {
        logDebugP("Today:");
        updateDayForecastKo(today, 0);
    }

    if (ParamIW_CHOutTomorrow)
    {
        logDebugP("Tomorrow:");
        updateDayForecastKo(tomorrow, IW_KoOffset_Tomorrow);
    }

    if (ParamIW_CHOutForecast)
    {
        bool select = (bool) KoIW_CHForecastSelection.value(DPT_Switch);
        logDebugP("update switchable KO's to %s", select ? "tomorrow" : "today");
        updateDayForecastKo(select ? tomorrow : today, IW_KoOffset_Forecast);
    }

    char description[15];
    if (ParamIW_CHOutHour1)
    {
        logDebugP("Hour + 1:");
        logIndentUp();
        buildDescription(description, hour1.rain, hour1.snow, hour1.clouds, "");
        logDebugP("Description: %s", description);
        setValueCompare(KoIW_CHHour1Description, description, DPT_String_8859_1);
        logDebugP("Temperature: %f", hour1.temperature);
        setValueCompare(KoIW_CHHour1Temperatur, hour1.temperature, DPT_Value_Temp);
        logDebugP("Temperature feels like: %f", hour1.temperatureFeelsLike);
        setValueCompare(KoIW_CHHour1TemperaturFeelsLike, hour1.temperatureFeelsLike, DPT_Value_Temp);
        logDebugP("Humidity: %f", hour1.humidity);
        setValueCompare(KoIW_CHHour1Humidity, hour1.humidity, DPT_Value_Humidity);
        logDebugP("Pressure: %f", hour1.pressure);
        setValueCompare(KoIW_CHHour1Pressure, hour1.pressure, DPT_Value_Pres);
        logDebugP("Wind speed: %f", hour1.windSpeed);
        setValueCompare(KoIW_CHHour1Wind, hour1.windSpeed, DPT_Value_Wsp_kmh);
        logDebugP("Wind gust: %f", hour1.windGust);
        setValueCompare(KoIW_CHHour1WindGust, hour1.windGust, DPT_Value_Wsp_kmh);
        logDebugP("Wind direction: %d", (int) hour1.windDirection);
        setValueCompare(KoIW_CHHour1WindDirection, hour1.windDirection, DPT_Angle);
        logDebugP("Rain: %f", hour1.rain);
        setValueCompare(KoIW_CHHour1Rain, hour1.rain, DPT_Rain_Amount);
        logDebugP("Snow: %f", hour1.rain);
        setValueCompare(KoIW_CHHour1Snow, hour1.snow, DPT_Length_mm);
        logDebugP("Probability of precipitation: %d", (int) hour1.probabilityOfPrecipitation);
        setValueCompare(KoIW_CHHour1ProbabilityOfPrecipitation, hour1.probabilityOfPrecipitation, DPT_Scaling);
        updateUviKo(KoIW_CHHour1UVI, hour1.uvi);
        logDebugP("Clouds: %d", (int) hour1.clouds);
        setValueCompare(KoIW_CHHour1Clouds, hour1.clouds, DPT_Scaling);
        logIndentDown();
    }
    if (ParamIW_CHOutHour2)
    {
        logDebugP("Hour + 2:");
        logIndentUp();
        buildDescription(description, hour2.rain, hour2.snow, hour2.clouds, "");
        logDebugP("Description: %s", description);
        setValueCompare(KoIW_CHHour2Description, description, DPT_String_8859_1);
        logDebugP("Temperature: %f", hour2.temperature);
        setValueCompare(KoIW_CHHour2Temperatur, hour2.temperature, DPT_Value_Temp);
        logDebugP("Temperature feels like: %f", hour2.temperatureFeelsLike);
        setValueCompare(KoIW_CHHour2TemperaturFeelsLike, hour2.temperatureFeelsLike, DPT_Value_Temp);
        logDebugP("Humidity: %f", hour2.humidity);
        setValueCompare(KoIW_CHHour2Humidity, hour2.humidity, DPT_Value_Humidity);
        logDebugP("Pressure: %f", hour2.pressure);
        setValueCompare(KoIW_CHHour2Pressure, hour2.pressure, DPT_Value_Pres);
        logDebugP("Wind speed: %f", hour2.windSpeed);
        setValueCompare(KoIW_CHHour2Wind, hour2.windSpeed, DPT_Value_Wsp_kmh);
        logDebugP("Wind gust: %f", hour2.windGust);
        setValueCompare(KoIW_CHHour2WindGust, hour2.windGust, DPT_Value_Wsp_kmh);
        logDebugP("Wind direction: %d", (int) hour2.windDirection);
        setValueCompare(KoIW_CHHour2WindDirection, hour2.windDirection, DPT_Angle);
        logDebugP("Rain: %f", hour2.rain);
        setValueCompare(KoIW_CHHour2Rain, hour2.rain, DPT_Rain_Amount);
        logDebugP("Snow: %f", hour2.rain);
        setValueCompare(KoIW_CHHour2Snow, hour2.snow, DPT_Length_mm);
        logDebugP("Probability of precipitation: %d", (int) hour2.probabilityOfPrecipitation);
        setValueCompare(KoIW_CHHour2ProbabilityOfPrecipitation, hour2.probabilityOfPrecipitation, DPT_Scaling);
        updateUviKo(KoIW_CHHour2UVI, hour2.uvi);
        logDebugP("Clouds: %d", (int) hour2.clouds);
        setValueCompare(KoIW_CHHour2Clouds, hour2.clouds, DPT_Scaling);
        logIndentDown();
    }
}

void BaseWeatherChannel::updateDayForecastKo(ForecastDayWheatherDataWithDescription fd, int koOffset)
{
    // TODO use reference for fd

    logIndentUp();
    logDebugP("description: %s", fd.description);
    setValueCompare(koOffset + IW_KoCHTodayDescription, fd.description, DPT_String_8859_1);
    logDebugP("temperature morning: %f", fd.temperatureMorning);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturMorning, fd.temperatureMorning, DPT_Value_Temp);
    logDebugP("temperature day: %f", fd.temperatureDay);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturDay, fd.temperatureDay, DPT_Value_Temp);
    logDebugP("temperature evening: %f", fd.temperatureEvening);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturEvening, fd.temperatureEvening, DPT_Value_Temp);
    logDebugP("temperature night: %f", fd.temperatureNight);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturNight, fd.temperatureNight, DPT_Value_Temp);
    logDebugP("temperature min: %f", fd.temperatureMin);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturMin, fd.temperatureMin, DPT_Value_Temp);
    logDebugP("temperature max: %f", fd.temperatureMax);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturMax, fd.temperatureMax, DPT_Value_Temp);
    logDebugP("temperature morning feels like: %f", fd.temperatureFeelsLikeMorning);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturMorningFeelsLike, fd.temperatureFeelsLikeEvening, DPT_Value_Temp);
    logDebugP("temperature day feels like: %f", fd.temperatureFeelsLikeDay);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturDayFeelsLike, fd.temperatureFeelsLikeDay, DPT_Value_Temp);
    logDebugP("temperature evening feels like: %f", fd.temperatureFeelsLikeEvening);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturEveningFeelsLike, fd.temperatureFeelsLikeEvening, DPT_Value_Temp);
    logDebugP("temperature night feels like: %f", fd.temperatureFeelsLikeNight);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturNightFeelsLike, fd.temperatureFeelsLikeNight, DPT_Value_Temp);
    logDebugP("humidity: %f", fd.humidity);
    setValueCompare(koOffset + IW_KoCHTodayHumidity, fd.humidity, DPT_Value_Humidity);
    logDebugP("pressure: %d", (int) fd.pressure);
    setValueCompare(koOffset + IW_KoCHTodayPressure, fd.pressure, DPT_Value_Pres);
    logDebugP("wind speed: %f", fd.windSpeed);
    setValueCompare(koOffset + IW_KoCHTodayWind, fd.windSpeed, DPT_Value_Wsp_kmh);
    logDebugP("wind gust: %f", fd.windGust);
    setValueCompare(koOffset + IW_KoCHTodayWindGust, fd.windGust, DPT_Value_Wsp_kmh);
    logDebugP("wind direction: %d", (int) fd.windDirection);
    setValueCompare(koOffset + IW_KoCHTodayWindDirection, fd.windDirection, DPT_Angle);
    logDebugP("rain: %f", fd.rain);
    setValueCompare(koOffset + IW_KoCHTodayRain, fd.rain, DPT_Rain_Amount);
    logDebugP("snow: %f", fd.snow);
    setValueCompare(koOffset + IW_KoCHTodaySnow, fd.snow, DPT_Length_mm);
    logDebugP("probability of precipitation: %d", (int) fd.probabilityOfPrecipitation);
    setValueCompare(koOffset + IW_KoCHTodayProbabilityOfPrecipitation, fd.probabilityOfPrecipitation, DPT_Scaling);

    updateUviKo((knx.getGroupObject(IW_KoCalcNumber(IW_KoCHTodayUVI))), fd.uvi);

    logDebugP("clouds: %d", (int) fd.clouds);
    setValueCompare(koOffset + IW_KoCHTodayClouds, fd.clouds, DPT_Scaling);
    logIndentDown();
}
