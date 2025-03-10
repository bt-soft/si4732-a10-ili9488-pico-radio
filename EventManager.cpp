#include "EventManager.h"
#include <CoreMutex.h>
#include <algorithm>

// Core lock
auto_init_mutex(eventManagerMutex);

/**
 *
 */
EventManager::EventManager() {}

/**
 *
 */
void EventManager::subscribe(int eventType, EventCallback callback) {
    CoreMutex mtx(&eventManagerMutex); // Lock
    subscribers.push_back({eventType, callback});
}

/**
 *
 */
void EventManager::unsubscribe(int eventType, EventCallback callback) {
    CoreMutex mtx(&eventManagerMutex); // Lock
    subscribers.erase(
        std::remove_if(subscribers.begin(),
                       subscribers.end(),
                       [eventType, &callback](const Subscriber &sub) {
                           return sub.eventType == eventType && sub.callback.target<void(const EventData &)>() == callback.target<void(const EventData &)>();
                       }),
        subscribers.end());
}

/**
 *
 */
void EventManager::publish(int eventType, void *eventData) {
    CoreMutex mtx(&eventManagerMutex); // Lock

    EventData event{eventType, eventData};
    for (const auto &subscriber : subscribers) {
        if (subscriber.eventType == EventManager::ALL_EVENTS || subscriber.eventType == eventType) {
            subscriber.callback(event);
        }
    }
}
