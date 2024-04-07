#include "ChannelOwnerModule.h"
#include "OpenKNX.h"

class InternetWheaterModule : public ChannelOwnerModule
{
  public:
    InternetWheaterModule();
    const std::string name() override;
    const std::string version() override;
    OpenKNX::Channel* createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */) override;
};

extern InternetWheaterModule openknxInternetWheaterModule;