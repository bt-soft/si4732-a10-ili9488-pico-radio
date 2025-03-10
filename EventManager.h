#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <functional>
#include <mutex>
#include <vector>

class EventManager {
public:
    enum EventType {
        ALL_EVENTS = -1,
        BUTTON_PRESS = 1,
        SENSOR_UPDATE = 2,
        ERROR_EVENT = 3
    };

    struct ButtonEventData {
        int buttonId;
        bool pressed;
    };

    struct SensorEventData {
        float temperature;
        float humidity;
    };

    struct ErrorEventData {
        int errorCode;
        const char *message;
    };

    struct EventData {
        int type;
        void *data;
    };

    using EventCallback = std::function<void(const EventData &)>;

    EventManager();
    void subscribe(int eventType, EventCallback callback);
    void unsubscribe(int eventType, EventCallback callback);
    void publish(int eventType, void *eventData = nullptr);

private:
    struct Subscriber {
        int eventType;
        EventCallback callback;
    };
    std::vector<Subscriber> subscribers;
};
// Globálisan deklarálva
extern EventManager eventManager;
#endif // EVENTMANAGER_H