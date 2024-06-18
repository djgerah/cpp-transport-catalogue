#pragma once

#include "geo.h"
#include <deque>
#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <unordered_map>

namespace tc
{
    struct Bus 
    { 
        std::string name_;
        std::deque<std::string> stops_;
        bool is_circle_;

        auto AsTuple() const 
        {
            return tie(name_, stops_, is_circle_);
        }

        bool operator==(const Bus& other) const 
        {
            return AsTuple() == other.AsTuple();
        }
    };

    struct Stop 
    {    
        std::string name_;
        geo::Coordinates coordinates;
        
        auto AsTuple() const 
        {
            return tie(name_, coordinates.lat, coordinates.lng);
        }

        bool operator==(const Stop& other) const 
        {
            return AsTuple() == other.AsTuple();
        }
    };

    struct Hasher
    {
        size_t operator()(const Stop* stop) const 
        {
            return ((hasher(stop->coordinates.lat) * 37) + (hasher(stop->coordinates.lng) * (37 * 37)));
        }

        private:

        std::hash<double> hasher;
    };

    // Реализуйте класс самостоятельно
    class TransportCatalogue 
    {
        using HashedStops = std::unordered_set<const Stop*, Hasher>;
        using BusMap = std::unordered_map<std::string_view, const Bus*>;
        using StopMap = std::unordered_map<std::string_view, const Stop*>;

        public:   
        // добавление автобуса в базу
        void AddBus(Bus& bus);
        // добавление остановки в базу
        void AddStop(Stop& stop);
        // поиск автобуса по номеру
        const Bus* GetBus(std::string_view stop);
        // поиск остановки по названию
        const Stop* GetStop(std::string_view stop);
        // получение количества уникальных остановок автобуса
        HashedStops GetUniqStops(std::string_view bus_name);
        // Рассчет длины маршрута автобуса
        double GetRouteLength(const Bus* bus);
        std::set<std::string> GetBusesByStop(std::string_view stop_name);

        private:
        // База автобусов
        std::deque<Bus> buses_;
        BusMap busname_to_bus;
        // База остановок
        std::deque<Stop> stops_;
        StopMap stopname_to_stop;
    };
} // namespace tc