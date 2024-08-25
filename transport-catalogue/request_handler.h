#pragma once

#include <sstream>
#include <optional>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

class RequestHandler 
{
    public:
    
        RequestHandler(const tc::TransportCatalogue& catalogue, const renderer::MapRenderer& renderer, const tc::TransportRouter& router)
            : catalogue_(catalogue)
            , renderer_(renderer)
            , router_(router)
            {}

        // Возврашает список автобусов по остановке
        const std::set<std::string> GetBusesByStop(std::string_view stop_name) const;
        // Возвращает наиболее оптимальный маршрут от остановки
        const std::optional<graph::Router<double>::RouteInfo> GetRoute(const tc::Stop* stop_from, const tc::Stop* stop_to) const;
        const graph::DirectedWeightedGraph<double>& GetGraph() const;
        svg::Document RenderMap() const;

    private:

        // RequestHandler использует агрегацию объектов "Транспортный Справочник", "Визуализатор Карты" и "Транспортный роутер"
        const tc::TransportCatalogue& catalogue_;
        const renderer::MapRenderer& renderer_;
        const tc::TransportRouter& router_;
};