#include "InternetWheaterModule.h"
#include "OpenWheaterMapChannel.h"

InternetWheaterModule::InternetWheaterModule()
    : ChannelOwnerModule(IW_ChannelCount)
{

}

const std::string InternetWheaterModule::name()
{
    return "InternetWheaterModule";
}

const std::string InternetWheaterModule::version()
{
    // hides the module in the version output on the console, because the firmware version is sufficient.
    return "";
}

OpenKNX::Channel* InternetWheaterModule::createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */)
{
    return new OpenWheaterMapChannel(_channelIndex);
}


InternetWheaterModule openknxInternetWheaterModule;