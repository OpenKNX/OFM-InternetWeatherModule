#pragma once
#include "BaseWheaterChannel.h"


class OpenWheaterMapChannel : public BaseWheaterChannel
{
  public:
    OpenWheaterMapChannel(uint8_t index);

    void makeCall() override;
    const std::string name() override;
 };
