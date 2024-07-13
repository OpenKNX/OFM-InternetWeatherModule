#pragma once
#include "OpenKNX.h"
#include "ChannelOwnerModule.h"

class InternetWeatherModule : public IWChannelOwnerModule
{
  public:
    InternetWeatherModule();
    const std::string name() override;
    const std::string version() override;
    OpenKNX::Channel* createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */) override;
    void showHelp() override;
    bool processCommand(const std::string cmd, bool diagnoseKo) override;
 
};

extern InternetWeatherModule openknxInternetWeatherModule;