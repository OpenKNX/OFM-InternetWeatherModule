#pragma once
#include "OpenKNX.h"

class OpenWheaterMapChannel : public OpenKNX::Channel
{
  protected:
    unsigned long _lastApiCall = 0;
  public:
    OpenWheaterMapChannel(uint8_t index);

    void setup() override;
    void loop() override;
    void makeCall();
    const std::string name() override;

 };
