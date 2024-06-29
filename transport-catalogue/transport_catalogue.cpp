#include "transport_catalogue.h"

namespace tc
{
    // Значение передается по lvalue ссылке, которая запрещает перемещение
    void TransportCatalogue::AddBus(const std::string& bus_name, std::vector<std::string_view>& stops, bool is_circle) 
    {
        std::deque<const Stop*> temp;

        for (auto stop : stops)
        {
            temp.push_back(GetStop(stop));
        }

        buses_.push_back({ bus_name, temp, is_circle });

        busname_to_bus[buses_.back().name_] = &buses_.back();
    }

    const Bus* TransportCatalogue::GetBus(std::string_view bus_name) 
    {   
        if (busname_to_bus.find(bus_name) == busname_to_bus.end())
        {
            return nullptr;
        }

        else
        {
            return busname_to_bus.at(bus_name);
        }
    }

    void TransportCatalogue::AddStop(const std::string& stop_name, geo::Coordinates& coordinates) 
    {
        stops_.push_back({ stop_name, coordinates });

        stopname_to_stop[stops_.back().name_] = &stops_.back();
    }

    const Stop* TransportCatalogue::GetStop(std::string_view stop_name) 
    {   
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
    std::set<std::string_view> TransportCatalogue::GetBusesByStop(std::string_view stop_name)
    {
        std::set<std::string_view> buses_by_stop;

        for (const auto& bus : buses_)
        {
            for (const auto& stop : bus.stops_)
            {
                if (stop->name_ == stop_name)
                {
                    buses_by_stop.insert(bus.name_);
                }
            }
        }

        return buses_by_stop;
    }

    std::unordered_set<const Stop*, Hasher> TransportCatalogue::GetUniqStops(std::string_view bus_name)
    {
        std::unordered_set<const Stop*, Hasher> unique_stops;
        
        for (const auto& stop : busname_to_bus.at(bus_name)->stops_) 
        {
            if (stopname_to_stop.count(stop->name_))
            {
                unique_stops.insert(stopname_to_stop.at(stop->name_));
            }
        }
        
        return unique_stops;
    }

    // Задает дистанции между остановками
    void TransportCatalogue::SetDistance(std::string_view from, std::string_view to, int dist)
    {
        distance_[{ GetStop(from), GetStop(to) }] = dist; 
    }

    // Возвращает дистанции между остановками
    int TransportCatalogue::GetDistance(const Stop* from, const Stop* to)
    {
        if (distance_.count({ from, to })) 
        {
            return distance_.at({ from, to });
        }

        else if (distance_.count({ to, from }))
        {
            return distance_.at({ to, from });
        }
        
        else 
        {
            return 0;
        }
    }

    std::pair<int, double> TransportCatalogue::GetRouteLength(const Bus* bus) 
    {
            int distance = 0;
            double route_length = 0.0;
            
            for (auto i = 1; i < bus->stops_.size(); ++i)
            {
                distance += GetDistance(bus->stops_[i - 1], bus->stops_[i]);
            }

            if  (bus->is_circle_) 
            {
                for (auto i = 1; i < bus->stops_.size(); ++i)
                {
                    route_length += geo::ComputeDistance(bus->stops_[i - 1]->coordinates, bus->stops_[i]->coordinates);
                }
            }
            
            else
            {
                for (auto i = bus->stops_.size() - 1; i > 0; --i)
                {
                    route_length += geo::ComputeDistance(bus->stops_[i]->coordinates, bus->stops_[i - 1]->coordinates);   
                }   
            }

        return { distance, route_length };
    }

    Info TransportCatalogue::GetInfo(const Bus* bus)
    {
        Info info;
        
        info.total_stops = bus->stops_.size();
        info.unique_stops = TransportCatalogue::GetUniqStops(bus->name_).size();
        
        auto route_length =  TransportCatalogue::GetRouteLength(bus);
        info.route_length = route_length.first;
        info.curvature = route_length.first / route_length.second;

        return info;
    }
} // namespace tc