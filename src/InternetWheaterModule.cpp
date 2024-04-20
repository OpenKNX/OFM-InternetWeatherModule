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
    switch (ParamIW_CHWheaterChannelType)
    {
        case 1:
            return new OpenWheaterMapChannel(_channelIndex);
         default:
            return nullptr;
    }
}

void InternetWheaterModule::showHelp()
{
   openknx.console.printHelpLine("iw<CC>", "Update wheater data of channel CC. i.e. iw01");
   openknx.console.printHelpLine("iw<CC> s0", "Set forecast of channel CC to today. i.e. iw1 s0");
   openknx.console.printHelpLine("iw<CC> s1", "Set forecast of channel CC to tomorrow. i.e. iw1 s1");
}

bool InternetWheaterModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd.rfind("iw", 0) == 0)
    {
        auto channelString = cmd.substr(2);
        if (channelString.length() > 0)
        {
            auto pos = channelString.find_first_of(' ');
            std::string channelNumberString;
            std::string channelCmd;
            if (pos > 0 && pos != std::string::npos)
            {
                channelNumberString = channelString.substr(0, pos);
                channelCmd = channelString.substr(pos + 1);
            }
            else
            {
                channelNumberString = channelString;
                channelCmd = "";
            }
            auto channel = atoi(channelNumberString.c_str());
            if (channel < 1 || channel > getNumberOfChannels())
            {
                Serial.println();
                Serial.printf("Channel %d not found\r\n", channel);
                return true;
            }
            auto wheaterChannel = (BaseWheaterChannel*) getChannel(channel - 1);
            if (wheaterChannel == nullptr)
            {
                Serial.println();
                Serial.printf("Channel not %d activated\r\n", channel);
                return true;
            }
            if (channelCmd.length() == 0)
            {
                wheaterChannel->fetchData();
                return true;
            }
            else
            {
                return wheaterChannel->processCommand(channelCmd, diagnoseKo);
            }
        }
    }
    return false;
}
 

InternetWheaterModule openknxInternetWheaterModule;