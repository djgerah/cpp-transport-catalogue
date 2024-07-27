#include "transport_catalogue.h"

namespace tc 
{
    void TransportCatalogue::AddStop(const std::string& stop_name, const geo::Coordinates& coordinates) 
    {
        stops_.push_back({ stop_name, coordinates, {} });
        stopname_to_stop_[stops_.back().name] = &stops_.back();
    }

    const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const 
    {
        if (stopname_to_stop_.find(stop_name) == stopname_to_stop_.end())
        {
            return nullptr;
        }

        else
        {
            return stopname_to_stop_.at(stop_name);
        }
    }

    void TransportCatalogue::AddBus(const std::string& bus_number, const std::vector<std::string_view>& stops, bool is_roundtrip)
    {   
        std::vector<const Stop*> stop_ptr;

        for (const auto& bus_stop : stops) 
        {
            stop_ptr.push_back(GetStop(bus_stop));
        }
        
        buses_.push_back({ bus_number, stop_ptr, is_roundtrip });
        busname_to_bus_[buses_.back().number] = &buses_.back();

        for (const auto& bus_stop : stops) 
        {
            for (auto& stop : stops_) 
            {
                if (stop.name == GetStop(bus_stop)->name) 
                {
                    stop.buses.insert(bus_number);
                }
            }
        }
    }

    const Bus* TransportCatalogue::GetBus(std::string_view bus_name) const
    {   
        if (busname_to_bus_.find(bus_name) == busname_to_bus_.end())
        {
            return nullptr;
        }

        else
        {
            return busname_to_bus_.at(bus_name);
        }
    }

    std::unordered_set<const Stop*, Hasher> TransportCatalogue::GetUniqStops(std::string_view bus_number) const
    {
        std::unordered_set<const Stop*, Hasher> unique_stops;
        
        for (const auto& stop : busname_to_bus_.at(bus_number)->stops) 
        {
            if (stopname_to_stop_.count(stop->name))
            {
                unique_stops.insert(stopname_to_stop_.at(stop->name));
            }
        }
        
        return unique_stops;
    }

    const std::map<std::string_view, const Bus*> TransportCatalogue::GetAllBuses() const 
    {
        std::map<std::string_view, const Bus*> all_buses;

        for (const auto& bus : busname_to_bus_) 
        {
            all_buses.emplace(bus);
        }
        
        return all_buses;
    }

    void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, const int distance)
    {
        dist_btw_stops[{ from, to }] = distance; 
    }

    int TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const
    {
        if (dist_btw_stops.count({ from, to })) 
        {
            return dist_btw_stops.at({ from, to });
        }

        else if (dist_btw_stops.count({ to, from }))
        {
            return dist_btw_stops.at({ to, from });
        }
        
        else 
        {
            return 0;
        }
    }
} // end namespace tc