#pragma once

#include "geo.h"
#include <deque>
#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace tc
{
    struct Stop 
    {    
        std::string name_;
        geo::Coordinates coordinates;
    };

    struct Bus 
    { 
        std::string name_;
        std::deque<const Stop*> stops_;
        bool is_circle_;
    };

    struct Info 
    {
        size_t total_stops = 0;
        size_t unique_stops = 0;
        double route_length = 0.0;
        double curvature = 0.0;
    };

    struct Hasher
    {
        size_t operator()(const Stop* stop) const noexcept  
        {
            return ((hasher_db(stop->coordinates.lat) * 37) + (hasher_db(stop->coordinates.lng) * (37 * 37)));
        }

        size_t operator()(const std::pair<const Stop*, const Stop*> pair_stops) const noexcept 
        {
            auto hash_1 = static_cast<const void*>(pair_stops.first);
            auto hash_2 = static_cast<const void*>(pair_stops.second);

            return (hasher_ptr(hash_1) * 37) + (hasher_ptr(hash_2) * (37 * 37));
        }    

        private:

        std::hash<double> hasher_db;
        std::hash<const void*> hasher_ptr;
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
        // поиск автобусов по названию остановки
        std::set<std::string_view> GetBusesByStop(std::string_view stop_name);
        // получение количества уникальных остановок автобуса
        HashedStops GetUniqStops(std::string_view bus_name);
        // устанавливает расстояние между парой остановок
        void SetDistance(std::string_view from, std::string_view to, int dist);
        // рассчет расстояния между остановками
        int GetDistance(const Stop* from, const Stop* to);
        // рассчет длины маршрута автобуса
        std::pair<int, double> GetRouteLength(const Bus* bus);
        // получение полной информации о маршруте
        Info GetInfo(const Bus* bus);

        private:
        // База автобусов
        std::deque<Bus> buses_;
        BusMap busname_to_bus;
        // База остановок
        std::deque<Stop> stops_;
        StopMap stopname_to_stop;
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher> distance_;
    };
} // namespace tc