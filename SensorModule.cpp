// SensorModule.cpp
#include "SensorModule.h"
#include <Arduino.h>

SensorModule::SensorModule(EventManager &eventManager) : eventManager(eventManager) {
    eventManager.subscribe(EventManager::BUTTON_PRESS, std::bind(&SensorModule::onButtonPress, this, std::placeholders::_1));
}

SensorModule::~SensorModule() {
    eventManager.unsubscribe(EventManager::BUTTON_PRESS, std::bind(&SensorModule::onButtonPress, this, std::placeholders::_1));
    Serial.println("SensorModule törölve és leiratkozva");
}

void SensorModule::sendSensorData() {
    EventManager::SensorEventData sensorData = {22.3, 55.4};
    eventManager.publish(EventManager::SENSOR_UPDATE, &sensorData);
}

void SensorModule::onButtonPress(const EventManager::EventData &event) {
    auto *data = static_cast<EventManager::ButtonEventData *>(event.data);
    Serial.print("SensorModule észlelte a gombnyomást - ID: ");
    Serial.println(data->buttonId);
}
