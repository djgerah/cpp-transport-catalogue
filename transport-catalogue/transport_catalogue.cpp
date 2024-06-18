#include "transport_catalogue.h"

void tc::TransportCatalogue::AddBus(Bus& bus) 
{
    buses_.push_back(std::move(bus));
    
    busname_to_bus[buses_.back().name_] = &buses_.back();
}

const tc::Bus* tc::TransportCatalogue::GetBus(std::string_view bus_name) 
{   
    return busname_to_bus.count(bus_name) ? busname_to_bus.at(bus_name) : nullptr;
}

void tc::TransportCatalogue::AddStop(Stop& stop) 
{
    stops_.push_back(std::move(stop));

    stopname_to_stop[stops_.back().name_] = &stops_.back();
}

const tc::Stop* tc::TransportCatalogue::GetStop(std::string_view stop_name) 
{   
    return stopname_to_stop.count(stop_name) ? stopname_to_stop.at(stop_name) : nullptr;
}

// Метод для получения списка автобусов по остановке.
std::set<std::string> tc::TransportCatalogue::GetBusesByStop(std::string_view stop_name)
{
    std::set<std::string> buses_by_stop;

    for (const auto& bus : buses_)
    {
        for (const auto& stop : bus.stops_)
        {
            if (stop == stop_name)
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
                const Stop* from = TransportCatalogue::GetStop(bus->stops_.at(i - 1));
                const Stop* to = TransportCatalogue::GetStop(bus->stops_.at(i));

                route_length += geo::ComputeDistance(from->coordinates, to->coordinates);    
            }
        }
        
        else
        {
            for (auto i = bus->stops_.size() - 1; i > 0; --i)
            {
                const Stop* from = TransportCatalogue::GetStop(bus->stops_.at(i));
                const Stop* to = TransportCatalogue::GetStop(bus->stops_.at(i - 1));

                route_length += geo::ComputeDistance(from->coordinates, to->coordinates);    
            }   
        }

    return route_length;
}