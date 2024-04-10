#pragma once
#include "OpenKNX.h"
#ifdef WLAN_WifiSSID
#include "WLANModule.h"
#else
#include "NetworkModule.h"
#endif
#include "ArduinoJson.h"
#include "HTTPClient.h"

class BaseWheaterChannel : public OpenKNX::Channel
{
  private:
    unsigned long _lastApiCall = 0;
    unsigned long _updateIntervalInMs = 0;
  protected:
    BaseWheaterChannel(uint8_t index);
  public:
    void loop() override;
    virtual void makeCall() = 0;
    void processInputKo(GroupObject &ko);
 };