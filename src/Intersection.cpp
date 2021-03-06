#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <random>
#include <mutex>

#include "Street.h"
#include "Intersection.h"
#include "Vehicle.h"

/* Implementation of class "WaitingVehicles" */

// L3.1 : Safeguard all accesses to the private members _vehicles and _promises with an appropriate locking mechanism, 
// that will not cause a deadlock situation where access to the resources is accidentally blocked.

int WaitingVehicles::getSize()
{
    std::lock_gaurd<std::mutex> lock(_mutex);
    return _vehicles.size();
}

void WaitingVehicles::pushBack(std::shared_ptr<Vehicle> vehicle, std::promise<void> &&promise)
{
    std::lock_gaurd<std::mutex> lock(_mutex);
    _vehicles.push_back(vehicle);
    _promises.push_back(std::move(promise));
}

void WaitingVehicles::permitEntryToFirstInQueue()
{
    std::lock_gaurd<std::mutex> lock(_mutex);
    // L2.3 : First, get the entries from the front of _promises and _vehicles. 
    // Then, fulfill promise and send signal back that permission to enter has been granted.
    // Finally, remove the front elements from both queues. 
    auto first_vehicle = _vehicles.begin();
    auto first_promise = _promises.begin();
    first_promise->set_value();
    _vehicles.erase(first_vehicle);
    _promises.erase(first_promise);

}

/* Implementation of class "Intersection" */

Intersection::Intersection()
{
    _type = ObjectType::objectIntersection;
    _isBlocked = false;
}

void Intersection::addStreet(std::shared_ptr<Street> street)
{
    _streets.push_back(street);
}

std::vector<std::shared_ptr<Street>> Intersection::queryStreets(std::shared_ptr<Street> incoming)
{
    // store all outgoing streets in a vector ...
    std::vector<std::shared_ptr<Street>> outgoings;
    for (auto it : _streets)
    {
        if (incoming->getID() != it->getID()) // ... except the street making the inquiry
        {
            outgoings.push_back(it);
        }
    }

    return outgoings;
}

// adds a new vehicle to the queue and returns once the vehicle is allowed to enter
void Intersection::addVehicletoQueue(std::shared_ptr<Vehicle> vehicle)
{
    // L3.3 : Ensure that the text output locks the console as a shared resource. Use the mutex _mtxCout you have added to the base class TrafficObject in the previous task. Make sure that in between the two calls to std-cout at the beginning and at the end of addVehicleToQueue the lock is not held. 
    std::unique_lock<std::mutex> lock(_mtxCout);
    std::cout << "Intersection #" << _id << "::addVehicleToQueue: thread id = " << std::this_thread::get_id() << std::endl;
    lock.unlock();
    // L2.2 : First, add the new vehicle to the waiting line by creating a promise, a corresponding future and then adding both to _waitingVehicles. 
    // Then, wait until the vehicle has been granted entry. 
    std::promise<void> new_vehicle;
    std::future<void> ftr = new_vehicle.get_future();
    _waitingVehicles.pushBack(vehicle, std::move(new_vehicle));
    ftr.wait();
    lock.lock();
    std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " is granted entry." << std::endl;
    std::cout << "Traffic Light is: " << _trafficLight.getCurrentPhase() << std::endl; 
    lock.unlock();
    // FP.6b : use the methods TrafficLight::getCurrentPhase and TrafficLight::waitForGreen to block the execution until the traffic light turns green.
	if(_trafficLight.getCurrentPhase() == TrafficLightPhase::red) {
    	_trafficLight.waitForGreen();
    }
}

void Intersection::vehicleHasLeft(std::shared_ptr<Vehicle> vehicle)
{
    //std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " has left." << std::endl;

    // unblock queue processing
    this->setIsBlocked(false);
}

void Intersection::setIsBlocked(bool isBlocked)
{
    _isBlocked = isBlocked;
    //std::cout << "Intersection #" << _id << " isBlocked=" << isBlocked << std::endl;
}

// virtual function which is executed in a thread
void Intersection::simulate() // using threads + promises/futures + exceptions
{
    _trafficLight.simulate();
    // launch vehicle queue processing in a thread
    threads.emplace_back(std::thread(&Intersection::processVehicleQueue, this));
}

void Intersection::processVehicleQueue()
{
    // print id of the current thread
    //std::cout << "Intersection #" << _id << "::processVehicleQueue: thread id = " << std::this_thread::get_id() << std::endl;

    // continuously process the vehicle queue
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // only proceed when at least one vehicle is waiting in the queue
        if (_waitingVehicles.getSize() > 0 && !_isBlocked)
        {
            // set intersection to "blocked" to prevent other vehicles from entering
            this->setIsBlocked(true);

            // permit entry to first vehicle in the queue (FIFO)
            _waitingVehicles.permitEntryToFirstInQueue();
        }
    }
}