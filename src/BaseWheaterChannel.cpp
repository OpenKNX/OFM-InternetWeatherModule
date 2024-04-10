#include "BaseWheaterChannel.h"

BaseWheaterChannel::BaseWheaterChannel(uint8_t index)
{
    _channelIndex = index;
     // <Enumeration Text="Keine"        Value="0" Id="%ENID%" />
    // <Enumeration Text="10 Minuten"   Value="1" Id="%ENID%" />
    // <Enumeration Text="30 Minuten"   Value="2" Id="%ENID%" />   
    // <Enumeration Text="Jede Stunde"  Value="3" Id="%ENID%" />    
    switch(ParamIW_WheaterRefreshInterval)
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
    logDebugP("Update interval: %dms", _updateIntervalInMs);
}

void BaseWheaterChannel::processInputKo(GroupObject &ko)
{
    // auto index = IW_KoCalcIndex(ko.asap());
    // switch (index)
    // {
    //     case IW_KoRefreshWheaterData:
    //     {
    //         if (ko.value(DPT_Trigger))
    //             makeCall();
    //         break;
    //     }
    // }    
}


void BaseWheaterChannel::loop()
{
// #ifdef WLAN_WifiSSID
//     if (openknxWLANModule.connected())
// #else
//     if (openknxNetwork.connected())
// #endif
//     {
//         auto now = millis();
//         if (now == 0)
//             now++; // Do not use 0 because it is used as marker for unitialized

//         if (_updateIntervalInMs > 0 && 
//             (_lastApiCall == 0 || (now - _lastApiCall > _updateIntervalInMs)))
//         {
//             _lastApiCall = now;
//          //   makeCall();
//         }
//     }
}