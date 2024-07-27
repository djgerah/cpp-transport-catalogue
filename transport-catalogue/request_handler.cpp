#include "request_handler.h"
#include <sstream>

namespace handler 
{
    std::pair<int, double> RequestHandler::GetRouteLength(const tc::Bus* bus) const
    {
        int route_length = 0;
        double geo_length = 0.0;

        for (size_t i = 0; i < bus->stops.size() - 1; ++i) 
        {
            auto from = bus->stops[i];
            auto to = bus->stops[i + 1];

            if (bus->is_roundtrip) 
            {
                route_length += catalogue_.GetDistance(from, to);
                geo_length += geo::ComputeDistance(from->coordinates, to->coordinates);
            }

            else 
            {
                route_length += catalogue_.GetDistance(from, to) + catalogue_.GetDistance(to, from);
                geo_length += geo::ComputeDistance(from->coordinates, to->coordinates) * 2;
            }
        }

        return { route_length, geo_length };
    }
    
    std::optional<tc::BusStat> RequestHandler::GetBusStat(const std::string_view bus_number) const 
    {
        tc::BusStat bus_stat{};
        const tc::Bus* bus = catalogue_.GetBus(bus_number);

        if (!bus)
        {
            throw std::invalid_argument("bus not found");
        }

        if (bus->is_roundtrip) 
        {
            bus_stat.total_stops = bus->stops.size();
        }

        else 
        {
            bus_stat.total_stops = bus->stops.size() * 2 - 1;
        }

        auto distance = GetRouteLength(bus);
        bus_stat.unique_stops = catalogue_.GetUniqStops(bus_number).size();
        bus_stat.route_length = distance.first;
        bus_stat.curvature = distance.first / distance.second;

        return bus_stat;
    }

    const json::Node RequestHandler::PrintBus(const json::Dict& request) const 
    {
        json::Dict result;

        const std::string& route_number = request.at("name").AsString();
        const tc::Bus* bus = catalogue_.GetBus(route_number);
        result["request_id"] = request.at("id").AsInt();

        if (bus != nullptr) 
        {
            result["curvature"] = GetBusStat(route_number)->curvature;
            result["route_length"] = GetBusStat(route_number)->route_length;
            result["stop_count"] = static_cast<int>(GetBusStat(route_number)->total_stops);
            result["unique_stop_count"] = static_cast<int>(GetBusStat(route_number)->unique_stops);
        }

        else 
        {
            result["error_message"] = json::Node{ static_cast<std::string>("not found") };
        }
        
        return json::Node{ result };
    }

    const json::Node RequestHandler::PrintStop(const json::Dict& request) const 
    {
        json::Dict result;

        const std::string& stop_name = request.at("name").AsString();
        const tc::Stop* stop = catalogue_.GetStop(stop_name);
        result["request_id"] = request.at("id").AsInt();

        if (stop != nullptr) 
        {
            json::Array buses;

            for (const auto& bus : GetBusesByStop(stop_name)) 
            {
                buses.push_back(bus);
            }
            
            result["buses"] = buses;
        }

        else 
        {
            result["error_message"] = json::Node{ static_cast<std::string>("not found") };
        }

        return json::Node{ result };
    }

    const json::Node RequestHandler::PrintMap(const json::Dict& request) const 
    {
        json::Dict result;

        result["request_id"] = request.at("id").AsInt();
        std::ostringstream strm;
        svg::Document map = RenderMap();
        map.Render(strm);
        result["map"] = strm.str();

        return json::Node{ result };
    }

    const std::set<std::string> RequestHandler::GetBusesByStop(std::string_view stop_name) const 
    {
        return catalogue_.GetStop(stop_name)->buses;
    }

    void RequestHandler::ProcessRequests(const json::Node& stat_requests) const 
    {
        json::Array result;

        for (auto& request : stat_requests.AsArray()) 
        {
            const auto& request_map = request.AsMap();
            const auto& type = request_map.at("type").AsString();

            if (type == "Stop") 
            {
                result.push_back(PrintStop(request_map).AsMap());
            }

            if (type == "Bus") 
            {
                result.push_back(PrintBus(request_map).AsMap());
            }

            if(type == "Map")
            {
                result.push_back(PrintMap(request_map).AsMap());
            }
        }
        
        json::Print(json::Document{ result }, std::cout);
    }

    svg::Document RequestHandler::RenderMap() const 
    {
        return renderer_.GetSVG(catalogue_.GetAllBuses());
    }
} // end namespace handler