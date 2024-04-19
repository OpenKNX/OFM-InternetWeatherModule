#pragma once
#include "BaseWheaterChannel.h"


class OpenWheaterMapChannel : public BaseWheaterChannel
{
  protected:
      void makeCall() override;

  public:
    OpenWheaterMapChannel(uint8_t index);
    const std::string name() override;
 };
