#include "BaseWheaterChannel.h"

BaseWheaterChannel::BaseWheaterChannel(uint8_t index)
{
    _channelIndex = index;
}

void BaseWheaterChannel::setup()
{
    // <Enumeration Text="Keine"        Value="0" Id="%ENID%" />
    // <Enumeration Text="10 Minuten"   Value="1" Id="%ENID%" />
    // <Enumeration Text="30 Minuten"   Value="2" Id="%ENID%" />
    // <Enumeration Text="Jede Stunde"  Value="3" Id="%ENID%" />
    switch (ParamIW_WheaterRefreshInterval)
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
}

void BaseWheaterChannel::processInputKo(GroupObject& ko)
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
         case IW_KoRefreshWheaterData: 
            if (ko.value(DPT_Trigger))
                fetchData();
            break;
    }
}

void BaseWheaterChannel::loop()
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

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

void BaseWheaterChannel::setDescription(GroupObject& ko, const char* prefix, float rain, float snow, int clouds, int probabilityOfPrecipitation)
{
    char description[15] = {0}; // Must be 0 terminated
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
            formatText = (const char*)ParamIW_WheaterConditionSnow;
            value = snow;
        }
        else
        {
            formatText = (const char*)ParamIW_WheaterConditionRain;
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
            snprintf(buffer, bufferSize, "%s", (const char*)ParamIW_WheaterConditionSun);
        }
        else
        {
            std::string formatText((const char*)ParamIW_WheaterConditionClouds);
            replaceAll(formatText, "%", "%%");
            replaceAll(formatText, "XXX", "%d");
            snprintf(buffer, bufferSize, formatText.c_str(), clouds);
        }
    }
    if (ko.valueNoSendCompare(description, DPT_String_8859_1))
    {
        ko.objectWritten();
        logDebugP("written: %s", description);
    }
    else
    {
        logDebugP("not written: %s", description);
    }
}
void BaseWheaterChannel::fetchData()
{
    makeCall();
    updateSwitchableKos();
}

void BaseWheaterChannel::copy(GroupObject& koTarget, bool select, GroupObject& ko1, GroupObject& ko2)
{
    auto koSource = select ? ko2 : ko1;
    bool intialized = koTarget.initialized();
    if (intialized && memcmp(koTarget.valueRef(),  koSource.valueRef(), koTarget.valueSize()) == 0)
        return;
    memcpy(koTarget.valueRef(), koSource.valueRef(), koSource.valueSize());
    koTarget.objectWritten();
}
void BaseWheaterChannel::updateSwitchableKos()
{
    auto select = (bool) KoIW_CHForecastSelection.value(DPT_Switch);
    logDebugP("update switchabel KO's to %d", select);
    copy(KoIW_CHForecastDescription, select, KoIW_CHTodayDescription, KoIW_CHTomorrowDescription);
    copy(KoIW_CHForecastTemparaturDay, select, KoIW_CHTodayTemparaturDay, KoIW_CHTomorrowTemparaturDay);
    copy(KoIW_CHForecastTemparaturNight, select, KoIW_CHTodayTemparaturNight, KoIW_CHTomorrowTemparaturNight);
    copy(KoIW_CHForecastTemparaturEvening, select, KoIW_CHTodayTemparaturEvening, KoIW_CHTomorrowTemparaturEvening);
    copy(KoIW_CHForecastTemparaturMorning, select, KoIW_CHTodayTemparaturMorning, KoIW_CHTomorrowTemparaturMorning);
    copy(KoIW_CHForecastTemparaturMin, select, KoIW_CHTodayTemparaturMin, KoIW_CHTomorrowTemparaturMin);
    copy(KoIW_CHForecastTemparaturMax, select, KoIW_CHTodayTemparaturMax, KoIW_CHTomorrowTemparaturMax);
    copy(KoIW_CHForecastTemparaturDayFeelsLike, select, KoIW_CHTodayTemparaturDayFeelsLike, KoIW_CHTomorrowTemparaturDayFeelsLike);
    copy(KoIW_CHForecastTemparaturNightFeelsLike, select, KoIW_CHTodayTemparaturNightFeelsLike, KoIW_CHTomorrowTemparaturNightFeelsLike);
    copy(KoIW_CHForecastTemparaturEveningFeelsLike, select, KoIW_CHTodayTemparaturEveningFeelsLike, KoIW_CHTomorrowTemparaturEveningFeelsLike);
    copy(KoIW_CHForecastTemparaturMorningFeelsLike, select, KoIW_CHTodayTemparaturMorningFeelsLike, KoIW_CHTomorrowTemparaturMorningFeelsLike);
    copy(KoIW_CHForecastHumidity, select, KoIW_CHTodayHumidity, KoIW_CHTomorrowHumidity);
    copy(KoIW_CHForecastPressure, select, KoIW_CHTodayPressure, KoIW_CHTomorrowPressure);
    copy(KoIW_CHForecastWind, select, KoIW_CHTodayWind, KoIW_CHTomorrowWind);
    copy(KoIW_CHForecastWindGust, select, KoIW_CHTodayWindGust, KoIW_CHTomorrowWindGust);
    copy(KoIW_CHForecastWindDirection, select, KoIW_CHTodayWindDirection, KoIW_CHTomorrowWindDirection);
    copy(KoIW_CHForecastRain, select, KoIW_CHTodayRain, KoIW_CHTomorrowRain);
    copy(KoIW_CHForecastSnow, select, KoIW_CHTodaySnow, KoIW_CHTomorrowSnow);
    copy(KoIW_CHForecastProbabilityOfPrecipitation, select, KoIW_CHTodayProbabilityOfPrecipitation, KoIW_CHTomorrowProbabilityOfPrecipitation);
    copy(KoIW_CHForecastUVI, select, KoIW_CHTodayUVI, KoIW_CHTomorrowUVI);
    copy(KoIW_CHForecastClouds, select, KoIW_CHTodayClouds, KoIW_CHTomorrowClouds);
  }