#include "transport_catalogue.h"

// Значение передается по lvalue ссылке, которая запрещает перемещение
void tc::TransportCatalogue::AddBus(Bus& bus) 
{
    buses_.push_back(bus);
    
    busname_to_bus[buses_.back().name_] = &buses_.back();
}

const tc::Bus* tc::TransportCatalogue::GetBus(std::string_view bus_name) 
{   
    // return busname_to_bus.count(bus_name) ? busname_to_bus.at(bus_name) : nullptr;

    if (busname_to_bus.find(bus_name) == busname_to_bus.end())
    {
        return nullptr;
    }

    else
    {
        return busname_to_bus.at(bus_name);
    }
}

void tc::TransportCatalogue::AddStop(Stop& stop) 
{
    stops_.push_back(stop);

    stopname_to_stop[stops_.back().name_] = &stops_.back();
}

const tc::Stop* tc::TransportCatalogue::GetStop(std::string_view stop_name) 
{   
    // return stopname_to_stop.count(stop_name) ? stopname_to_stop.at(stop_name) : nullptr;

    if (stopname_to_stop.find(stop_name) == stopname_to_stop.end())
    {
        return nullptr;
    }

    else
    {
        return stopname_to_stop.at(stop_name);
    }
}

// Метод для получения списка автобусов по остановке.
std::set<std::string_view> tc::TransportCatalogue::GetBusesByStop(std::string_view stop_name)
{
    std::set<std::string_view> buses_by_stop;

    for (const auto& bus : buses_)
    {
        for (const auto& stop : bus.stops_ptr_)
        {
            if (stop->name_ == stop_name)
            {
                buses_by_stop.insert(bus.name_);
            }
        }
    }

    return buses_by_stop;
}

std::unordered_set<const tc::Stop*, tc::Hasher> tc::TransportCatalogue::GetUniqStops(std::string_view bus_name)
{
    std::unordered_set<const Stop*, Hasher> unique_stops;
    
    for (const auto& stop : busname_to_bus.at(bus_name)->stops_) 
    {
        if (stopname_to_stop.count(stop))
        {
            unique_stops.insert(stopname_to_stop.at(stop));
        }
    }
    
    return unique_stops;
}

double tc::TransportCatalogue::GetRouteLength(const Bus* bus) 
{
        double route_length = 0.0;

        if  (bus->is_circle_) 
        {
            for (auto i = 1; i < bus->stops_.size(); ++i)
            {
                route_length += geo::ComputeDistance(bus->stops_ptr_[i - 1]->coordinates, bus->stops_ptr_[i]->coordinates);    
            }
        }
        
        else
        {
            for (auto i = bus->stops_.size() - 1; i > 0; --i)
            {
                route_length += geo::ComputeDistance(bus->stops_ptr_[i]->coordinates, bus->stops_ptr_[i - 1]->coordinates);    
            }   
        }

    return route_length;
}

tc::Info tc::TransportCatalogue::GetInfo(const tc::Bus* bus)
{
    tc::Info info;
    
    info.route_length = tc::TransportCatalogue::GetRouteLength(bus);
    info.total_stops = bus->stops_ptr_.size();
    info.unique_stops = tc::TransportCatalogue::GetUniqStops(bus->name_).size();

    return info;
}