#include "OpenWheaterMapChannel.h"
#include "NetworkModule.h"

OpenWheaterMapChannel::OpenWheaterMapChannel(uint8_t index)
{
    _channelIndex = index;
}

const std::string OpenWheaterMapChannel::name()
{
    return "OpenWheaterMap";
}

void OpenWheaterMapChannel::setup()
{
    // Debug
 //   logTraceP("paramActive: %i", ParamBI_ChannelActive);
}

void OpenWheaterMapChannel::loop()
{
   if (openknxNetwork.connected())
   {
        auto now = millis();
        if (now == 0)
            now++; // Do not use 0 because it is used as marker for unitialized
        if (_lastApiCall == 0 || (now - _lastApiCall > 600000))
        {
            _lastApiCall = now;
            makeCall();
        }
   }
}

void OpenWheaterMapChannel::makeCall()
{
    
}