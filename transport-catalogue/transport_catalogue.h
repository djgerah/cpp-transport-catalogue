#pragma once

#include <deque>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"
#include "geo.h"

namespace tc 
{
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

    class TransportCatalogue 
    {
        using StopMap = std::unordered_map<std::string_view, const Stop*>;
        using BusMap = std::unordered_map<std::string_view, const Bus*>;
        using HashedStops = std::unordered_set<const Stop*, Hasher>;
        using HashedDistanceBtwStops = std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher>;

        public:

            // добавление остановки в базу
            void AddStop(tc::Stop stop);
            // поиск остановки по названию
            const Stop* GetStop(std::string_view stop_name) const;
            // добавление автобуса в базу
            void AddBus(tc::Bus bus);
            // поиск автобуса по номеру
            const Bus* GetBus(std::string_view bus_name) const;
            // получение количества уникальных остановок автобуса
            std::unordered_set<const Stop*, Hasher> GetUniqueStops(std::string_view bus_number) const;
            // получение списка всех остановок
            const std::map<std::string_view, const Stop*> GetAllStops() const;
            // получение всех автобусов парка
            const std::map<std::string_view, const Bus*> GetAllBuses() const;
            // устанавливает расстояние между парой остановок
            void SetDistance(const Stop* from, const Stop* to, const int distance);
            // рассчет расстояния между остановками
            int GetDistance(const Stop* from, const Stop* to) const;
            // Рассчитывает протяженность маршрута
            std::pair<int, double> GetRouteLength(const tc::Bus* bus) const;
            // Возвращает информацию о маршруте (запрос Bus)
            std::optional<tc::BusStat> GetBusStat(const std::string_view bus_number) const;

        private:
            // База остановок
            std::deque<Stop> stops_;
            StopMap stopname_to_stop_;
            // База автобусов
            std::deque<Bus> buses_;
            BusMap busname_to_bus_;

            HashedDistanceBtwStops dist_btw_stops;
    };
}  // end namespace tc