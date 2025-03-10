// SensorModule.h
#ifndef SENSORMODULE_H
#define SENSORMODULE_H

#include "EventManager.h"

class SensorModule {
public:
    SensorModule(EventManager &eventManager);
    ~SensorModule();
    void sendSensorData();

private:
    EventManager &eventManager;
    void onButtonPress(const EventManager::EventData &event);
};

#endif // SENSORMODULE_H
