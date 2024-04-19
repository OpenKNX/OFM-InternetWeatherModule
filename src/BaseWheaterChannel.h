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
    void copy(GroupObject& koTarget, bool select, GroupObject& ko1, GroupObject& ko2);
  protected:
    BaseWheaterChannel(uint8_t index);
    virtual void makeCall() = 0;
    void setDescription(GroupObject &ko, const char* prefix, float rain, float snow, int clouds, int probabilityOfPrecipitation);
    void updateSwitchableKos();
 public:
    void loop() override;
    void setup() override;
    void fetchData();
    void processInputKo(GroupObject &ko) override;

 };