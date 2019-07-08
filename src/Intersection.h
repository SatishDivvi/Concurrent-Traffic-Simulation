#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <vector>
#include <future>
#include <mutex>
#include <memory>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Street;
class Vehicle;

// auxiliary class to queue and dequeue waiting vehicles in a thread-safe manner
class WaitingVehicles
{
public:
    // getters / setters
    int getSize();

    // typical behaviour methods
    void pushBack(std::shared_ptr<Vehicle> vehicle, std::promise<void> &&promise);
    void permitEntryToFirstQueue();

private:
    std::vector<std::shared_ptr<Vehicle>> _vehicles; // list of all vehicles waiting to enter this intersection
    std::vector<std::promise<void>> _promises; // list of associated promises

}


class Intersection : public TrafficObject
{
public:
    // constructor / desctructor
    Intersection();

    // getters / setters

    // typical behaviour methods
    void addStreet(std::shared_ptr<Street> street);
    std::vector<std::shared_ptr<Street>> queryStreets(std::shared_ptr<Street> incoming); // return pointer to current list of all outgoing streets

private:

    // typical behaviour methods
    std::vector<std::shared_ptr<Street>> _streets;   // list of all streets connected to this intersection
};

#endif