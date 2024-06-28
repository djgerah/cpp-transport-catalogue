#pragma once

#include "geo.h"
#include <algorithm>
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace tc
{
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

        bool operator!=(const Stop& other) const 
        {
            return (AsTuple() != other.AsTuple());
        }
    };

    struct Bus 
    { 
        std::string name_;
        std::deque<const Stop*> stops_ptr_;
        bool is_circle_;

        auto AsTuple() const 
        {
            return tie(name_, stops_ptr_, is_circle_);
        }

        bool operator==(const Bus& other) const 
        {
            return AsTuple() == other.AsTuple();
        }

        bool operator!=(const Bus& other) const 
        {
            return (AsTuple() != other.AsTuple());
        }
    };

    struct Info 
    {
        size_t total_stops = 0;
        size_t unique_stops = 0;
        double route_length = 0.0;
    };

    struct Hasher
    {
        size_t operator()(const Stop* stop) const 
        {
            return ((hasher_db(stop->coordinates.lat) * 37) + (hasher_db(stop->coordinates.lng) * (37 * 37)));
        }

        private:

        std::hash<double> hasher_db;
    };

    // Реализуйте класс самостоятельно
    class TransportCatalogue 
    {
        using HashedStops = std::unordered_set<const Stop*, Hasher>;
        using BusMap = std::unordered_map<std::string_view, const Bus*>;
        using StopMap = std::unordered_map<std::string_view, const Stop*>;

        public:   
        // добавление автобуса в базу
        void AddBus(const std::string& bus_name, std::vector<std::string_view>& stops, bool is_circle);
        // добавление остановки в базу
        void AddStop(const std::string& stop_name, geo::Coordinates& coordinates);
        // поиск автобуса по номеру
        const Bus* GetBus(std::string_view stop);
        // поиск остановки по названию
        const Stop* GetStop(std::string_view stop);
        // получение количества уникальных остановок автобуса
        HashedStops GetUniqStops(std::string_view bus_name);
        // Рассчет длины маршрута автобуса
        double GetRouteLength(const Bus* bus);
        // Получение списка автобусов по названию остановки
        std::set<std::string_view> GetBusesByStop(std::string_view stop_name);
        // Получение информации о маршруте
        Info GetInfo(const Bus* bus);

        private:
        // База автобусов
        std::deque<Bus> buses_;
        BusMap busname_to_bus;
        // База остановок
        std::deque<Stop> stops_;
        StopMap stopname_to_stop;
    };
} // namespace tc