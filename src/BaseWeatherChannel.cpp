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
                bool select = (bool)KoIW_CHForecastSelection.value(DPT_Switch);
                logDebugP("changed switchable KO's to %s", select ? "tomorrow" : "today");
                updateDayForecastKo(select ? _tomorrow : _today, IW_KoOffset_Forecast);
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
        return true;
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
    logDebugP("Description: %s", description);
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

void BaseWeatherChannel::fetchData()
{
    CurrentWeatherData current = CurrentWeatherData();
    ForecastHourWeatherData hour1 = ForecastHourWeatherData();
    ForecastHourWeatherData hour2 = ForecastHourWeatherData();

    int16_t httpStatus = fillWeather(current, _today, _tomorrow, hour1, hour2);
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
    buildDescription(_today.description, _today.rain_mm, _today.snow_mm, _today.cloudsCover_percent, (const char*)ParamIW_TextPrefixDayCurrent);
    buildDescription(_tomorrow.description, _tomorrow.rain_mm, _tomorrow.snow_mm, _tomorrow.cloudsCover_percent, (const char*)ParamIW_TextPrefixDayNext);
    _available = true;

    if (ParamIW_CHOutCurrent)
    {
        logDebugP("Current:");
        logIndentUp();
        logDebugP("Temperature           : %.3f °C", current.temperature_C);
        setValueCompare(KoIW_CHCurrentTemperatur, current.temperature_C, DPT_Value_Temp);
        logDebugP("Temperature feels like: %.3f °C", current.temperatureFeelsLike_C);
        setValueCompare(KoIW_CHCurrentTemperaturFeelsLike, current.temperatureFeelsLike_C, DPT_Value_Temp);
        logDebugP("Humidity              : %.2f %%", current.humidity_percent);
        setValueCompare(KoIW_CHCurrentHumidity, current.humidity_percent, DPT_Value_Humidity);
        logDebugP("Pressure              : %.2f hPa", current.pressure_hPa);
        setValueCompare(KoIW_CHCurrentPressure, current.pressure_hPa, DPT_Value_Pres);
        logDebugP("Wind speed            : %.2f Km/h", current.windSpeed_Km_h);
        setValueCompare(KoIW_CHCurrentWind, current.windSpeed_Km_h, DPT_Value_Wsp_kmh);
        logDebugP("Wind gust             : %.2f Km/h", current.windGust_Km_h);
        setValueCompare(KoIW_CHCurrentWindGust, current.windGust_Km_h, DPT_Value_Wsp_kmh);
        logDebugP("Wind direction        : %d °", (int)current.windDirection_deg);
        setValueCompare(KoIW_CHCurrentWindDirection, current.windDirection_deg, DPT_Angle);
        logDebugP("Rain                  : %.2f mm/m²", current.rain_mm);
        setValueCompare(KoIW_CHCurrentRain, current.rain_mm, DPT_Rain_Amount);
        logDebugP("Snow                  : %.2f mm/m²", current.snow_mm);
        setValueCompare(KoIW_CHCurrentSnow, current.snow_mm, DPT_Length_mm);
        logDebugP("UVI                   : %f", current.uvi_unitOne);
        setValueCompare(KoIW_CHCurrentUVI, current.uvi_unitOne, DPT_Value_Tempd); // Workaround: no type for DPT 9.031 "coefficent", using temperature-difference instead
        logDebugP("Clouds                : %d %%", (int)current.cloudsCover_percent);
        setValueCompare(KoIW_CHCurrentClouds, current.cloudsCover_percent, DPT_Scaling);
        logIndentDown();
    }

    if (ParamIW_CHOutToday)
    {
        logDebugP("Today:");
        updateDayForecastKo(_today, 0);
    }

    if (ParamIW_CHOutTomorrow)
    {
        logDebugP("Tomorrow:");
        updateDayForecastKo(_tomorrow, IW_KoOffset_Tomorrow);
    }

    if (ParamIW_CHOutForecast)
    {
        bool select = (bool)KoIW_CHForecastSelection.value(DPT_Switch);
        logDebugP("update switchable KO's to %s", select ? "tomorrow" : "today");
        updateDayForecastKo(select ? _tomorrow : _today, IW_KoOffset_Forecast);
    }

    char description[15];
    if (ParamIW_CHOutHour1)
    {
        logDebugP("Hour + 1:");
        logIndentUp();
        buildDescription(description, hour1.rain_mm, hour1.snow_mm, hour1.cloudsCover_percent, "");
        logDebugP("Description                 : %s", description);
        setValueCompare(KoIW_CHHour1Description, description, DPT_String_8859_1);
        logDebugP("Temperature                 : %.3f °C", hour1.temperature_C);
        setValueCompare(KoIW_CHHour1Temperatur, hour1.temperature_C, DPT_Value_Temp);
        logDebugP("Temperature feels like      : %.3f °C", hour1.temperatureFeelsLike_C);
        setValueCompare(KoIW_CHHour1TemperaturFeelsLike, hour1.temperatureFeelsLike_C, DPT_Value_Temp);
        logDebugP("Humidity                    : %.2f %%", hour1.humidity_percent);
        setValueCompare(KoIW_CHHour1Humidity, hour1.humidity_percent, DPT_Value_Humidity);
        logDebugP("Pressure                    : %.2f hPa", hour1.pressure_hPa);
        setValueCompare(KoIW_CHHour1Pressure, hour1.pressure_hPa, DPT_Value_Pres);
        logDebugP("Wind speed                  : %.2f Km/h", hour1.windSpeed_Km_h);
        setValueCompare(KoIW_CHHour1Wind, hour1.windSpeed_Km_h, DPT_Value_Wsp_kmh);
        logDebugP("Wind gust                   : %.2f Km/h", hour1.windGust_Km_h);
        setValueCompare(KoIW_CHHour1WindGust, hour1.windGust_Km_h, DPT_Value_Wsp_kmh);
        logDebugP("Wind direction              : %d °", (int)hour1.windDirection_deg);
        setValueCompare(KoIW_CHHour1WindDirection, hour1.windDirection_deg, DPT_Angle);
        logDebugP("Rain                        : %.2f mm/m²", hour1.rain_mm);
        setValueCompare(KoIW_CHHour1Rain, hour1.rain_mm, DPT_Rain_Amount);
        logDebugP("Snow                        : %.2f mm/m²", hour1.rain_mm);
        setValueCompare(KoIW_CHHour1Snow, hour1.snow_mm, DPT_Length_mm);
        logDebugP("Probability of precipitation: %d %%", (int)hour1.probabilityOfPrecipitation_percent);
        setValueCompare(KoIW_CHHour1ProbabilityOfPrecipitation, hour1.probabilityOfPrecipitation_percent, DPT_Scaling);
        logDebugP("UVI                         : %f", hour1.uvi_unitOne);
        setValueCompare(KoIW_CHHour1UVI, hour1.uvi_unitOne, DPT_Value_Tempd); // Workaround: no type for DPT 9.031 "coefficent", using temperature-difference instead
        logDebugP("Clouds                      : %d %%", (int)hour1.cloudsCover_percent);
        setValueCompare(KoIW_CHHour1Clouds, hour1.cloudsCover_percent, DPT_Scaling);
        logIndentDown();
    }
    if (ParamIW_CHOutHour2)
    {
        logDebugP("Hour + 2:");
        logIndentUp();
        buildDescription(description, hour2.rain_mm, hour2.snow_mm, hour2.cloudsCover_percent, "");
        logDebugP("Description                 : %s", description);
        setValueCompare(KoIW_CHHour2Description, description, DPT_String_8859_1);
        logDebugP("Temperature                 : %.3f °C", hour2.temperature_C);
        setValueCompare(KoIW_CHHour2Temperatur, hour2.temperature_C, DPT_Value_Temp);
        logDebugP("Temperature feels like      : %.3f °C", hour2.temperatureFeelsLike_C);
        setValueCompare(KoIW_CHHour2TemperaturFeelsLike, hour2.temperatureFeelsLike_C, DPT_Value_Temp);
        logDebugP("Humidity                    : %.2f %%", hour2.humidity_percent);
        setValueCompare(KoIW_CHHour2Humidity, hour2.humidity_percent, DPT_Value_Humidity);
        logDebugP("Pressure                    : %.2f hPa", hour2.pressure_hPa);
        setValueCompare(KoIW_CHHour2Pressure, hour2.pressure_hPa, DPT_Value_Pres);
        logDebugP("Wind speed                  : %.2f Km/h", hour2.windSpeed_Km_h);
        setValueCompare(KoIW_CHHour2Wind, hour2.windSpeed_Km_h, DPT_Value_Wsp_kmh);
        logDebugP("Wind gust                   : %.2f Km/h", hour2.windGust_Km_h);
        setValueCompare(KoIW_CHHour2WindGust, hour2.windGust_Km_h, DPT_Value_Wsp_kmh);
        logDebugP("Wind direction              : %d °", (int)hour2.windDirection_deg);
        setValueCompare(KoIW_CHHour2WindDirection, hour2.windDirection_deg, DPT_Angle);
        logDebugP("Rain                        : %.2f mm/m²", hour2.rain_mm);
        setValueCompare(KoIW_CHHour2Rain, hour2.rain_mm, DPT_Rain_Amount);
        logDebugP("Snow                        : %.2f mm/m²", hour2.rain_mm);
        setValueCompare(KoIW_CHHour2Snow, hour2.snow_mm, DPT_Length_mm);
        logDebugP("Probability of precipitation: %d %%", (int)hour2.probabilityOfPrecipitation_percent);
        setValueCompare(KoIW_CHHour2ProbabilityOfPrecipitation, hour2.probabilityOfPrecipitation_percent, DPT_Scaling);
        logDebugP("UVI                         : %f", hour2.uvi_unitOne);
        setValueCompare(KoIW_CHHour2UVI, hour2.uvi_unitOne, DPT_Value_Tempd); // Workaround: no type for DPT 9.031 "coefficent", using temperature-difference instead
        logDebugP("Clouds                      : %d %%", (int)hour2.cloudsCover_percent);
        setValueCompare(KoIW_CHHour2Clouds, hour2.cloudsCover_percent, DPT_Scaling);
        logIndentDown();
    }
}

void BaseWeatherChannel::updateDayForecastKo(ForecastDayWeatherDataWithDescription& fd, int koOffset)
{
    logIndentUp();
    logDebugP("Description: %s", fd.description);
    setValueCompare(koOffset + IW_KoCHTodayDescription, fd.description, DPT_String_8859_1);

    logDebugP("Temperature:");
    logIndentUp();
    logDebugP("morning                   : %7.3f °C", fd.temperatureMorning_C);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturMorning, fd.temperatureMorning_C, DPT_Value_Temp);
    logDebugP("day                       : %7.3f °C", fd.temperatureDay_C);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturDay, fd.temperatureDay_C, DPT_Value_Temp);
    logDebugP("evening                   : %7.3f °C", fd.temperatureEvening_C);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturEvening, fd.temperatureEvening_C, DPT_Value_Temp);
    logDebugP("night                     : %7.3f °C", fd.temperatureNight_C);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturNight, fd.temperatureNight_C, DPT_Value_Temp);
    logDebugP("min                       : %7.3f °C", fd.temperatureMin_C);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturMin, fd.temperatureMin_C, DPT_Value_Temp);
    logDebugP("max                       : %7.3f °C", fd.temperatureMax_C);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturMax, fd.temperatureMax_C, DPT_Value_Temp);
    logDebugP("morning feels like        : %7.3f °C", fd.temperatureFeelsLikeMorning_C);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturMorningFeelsLike, fd.temperatureFeelsLikeEvening_C, DPT_Value_Temp);
    logDebugP("day feels like            : %7.3f °C", fd.temperatureFeelsLikeDay_C);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturDayFeelsLike, fd.temperatureFeelsLikeDay_C, DPT_Value_Temp);
    logDebugP("evening feels like        : %7.3f °C", fd.temperatureFeelsLikeEvening_C);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturEveningFeelsLike, fd.temperatureFeelsLikeEvening_C, DPT_Value_Temp);
    logDebugP("night feels like          : %7.3f °C", fd.temperatureFeelsLikeNight_C);
    setValueCompare(koOffset + IW_KoCHTodayTemperaturNightFeelsLike, fd.temperatureFeelsLikeNight_C, DPT_Value_Temp);
    logIndentDown();

    logDebugP("Humidity                    : %.2f %%", fd.humidity_percent);
    setValueCompare(koOffset + IW_KoCHTodayHumidity, fd.humidity_percent, DPT_Value_Humidity);
    logDebugP("Pressure                    : %.2f hPa", (int)fd.pressure_hPa);
    setValueCompare(koOffset + IW_KoCHTodayPressure, fd.pressure_hPa, DPT_Value_Pres);
    logDebugP("Wind speed                  : %.2f Km/h", fd.windSpeed_Km_h);
    setValueCompare(koOffset + IW_KoCHTodayWind, fd.windSpeed_Km_h, DPT_Value_Wsp_kmh);
    logDebugP("Wind gust                   : %.2f Km/h", fd.windGust_Km_h);
    setValueCompare(koOffset + IW_KoCHTodayWindGust, fd.windGust_Km_h, DPT_Value_Wsp_kmh);
    logDebugP("Wind direction              : %d °", (int)fd.windDirection_deg);
    setValueCompare(koOffset + IW_KoCHTodayWindDirection, fd.windDirection_deg, DPT_Angle);
    logDebugP("Rain                        : %.2f mm/m²", fd.rain_mm);
    setValueCompare(koOffset + IW_KoCHTodayRain, fd.rain_mm, DPT_Rain_Amount);
    logDebugP("Snow                        : %.2f mm/m²", fd.snow_mm);
    setValueCompare(koOffset + IW_KoCHTodaySnow, fd.snow_mm, DPT_Length_mm);
    logDebugP("Probability of precipitation: %d %%", (int)fd.probabilityOfPrecipitation_percent);
    setValueCompare(koOffset + IW_KoCHTodayProbabilityOfPrecipitation, fd.probabilityOfPrecipitation_percent, DPT_Scaling);
    logDebugP("UVI                         : %f", fd.uvi_unitOne);
    setValueCompare(koOffset + IW_KoCHTodayUVI, fd.uvi_unitOne, DPT_Value_Tempd); // Workaround: no type for DPT 9.031 "coefficent", using temperature-difference instead
    logDebugP("Clouds                      : %d %%", (int)fd.cloudsCover_percent);
    setValueCompare(koOffset + IW_KoCHTodayClouds, fd.cloudsCover_percent, DPT_Scaling);
    logIndentDown();
}
