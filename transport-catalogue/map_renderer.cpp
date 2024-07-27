#include "map_renderer.h"

using namespace std::literals;

namespace renderer 
{
    bool IsZero(double value) 
    {
        return std::abs(value) < EPSILON;
    }

    std::vector<svg::Polyline> MapRenderer::RenderRouteLines(const std::map<std::string_view, const tc::Bus*>& buses, const SphereProjector& sphere_projector) const 
    {
        std::vector<svg::Polyline> lines;
        // Первый по алфавиту маршрут должен получить первый цвет, второй маршрут — второй цвет и так далее
        size_t color = 0;

        for (const auto &[bus_number, bus] : buses) 
        {
            if (!bus->stops.empty()) 
            {
                std::vector<const tc::Stop*> route_stops{ bus->stops.begin(), bus->stops.end() };

                // Если маршрут некольцевой, то есть "is_roundtrip": false, 
                // каждый отрезок между соседними остановками должен быть нарисован дважды: 
                // сначала в прямом, а потом в обратном направлении
                if (!bus->is_roundtrip) 
                {
                    route_stops.insert(route_stops.end(), std::next(bus->stops.rbegin()), bus->stops.rend());
                }

                svg::Polyline line;

                for (const auto &stop : route_stops) 
                {
                    line.AddPoint(sphere_projector(stop->coordinates));
                }

                // Цвет линии stroke определён по правилам выше
                line.SetStrokeColor(render_settings_.color_palette[color]);
                // Цвет заливки fill должен иметь значение none
                line.SetFillColor("none");
                // Толщина линии stroke-width равна настройке line_width
                line.SetStrokeWidth(render_settings_.line_width);
                // Формы конца линии stroke-linecap и соединений stroke-linejoin равны round
                line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                
                if (color < render_settings_.color_palette.size() - 1) 
                {
                    ++color;
                } 
                
                else 
                {
                    color = 0;
                }
                
                lines.push_back(line);
            }
        }
        
        return lines;
    }

    std::vector<svg::Text> MapRenderer::RenderBusLabel(const std::map<std::string_view, const tc::Bus*>& buses, const SphereProjector& sphere_projector) const 
    {
        svg::Text text;
        svg::Text underlayer;
        std::vector<svg::Text> bus_labels;
        // Первый по алфавиту маршрут должен получить первый цвет, второй маршрут — второй цвет и так далее
        size_t color = 0;

        for (const auto& [bus_number, bus] : buses) 
        {
            // Если остановок у маршрута нет, его название выводиться не должно
            if (!bus->stops.empty()) 
            {
                // x и y — координаты соответствующей конечной остановки (конечной считается первая остановка маршрута)
                text.SetPosition(sphere_projector(bus->stops.front()->coordinates));
                // смещение dx и dy равно настройке bus_label_offset;
                text.SetOffset(render_settings_.bus_label_offset);
                // размер шрифта font-size равен настройке bus_label_font_size
                text.SetFontSize(render_settings_.bus_label_font_size);
                // название шрифта font-family — "Verdana"
                text.SetFontFamily("Verdana"s);
                // толщина шрифта font-weight — "bold"
                text.SetFontWeight("bold"s);
                // содержимое — название автобуса
                text.SetData(bus->number);
                // Цвет маршрута
                text.SetFillColor(render_settings_.color_palette[color]);
                
                // Дополнительные свойства подложки:
                underlayer.SetPosition(sphere_projector(bus->stops.front()->coordinates));
                underlayer.SetOffset(render_settings_.bus_label_offset);
                underlayer.SetFontSize(render_settings_.bus_label_font_size);
                underlayer.SetFontFamily("Verdana"s);
                underlayer.SetFontWeight("bold"s);
                underlayer.SetData(bus->number);
                // цвет заливки fill и цвет линий stroke равны настройке underlayer_color
                underlayer.SetFillColor(render_settings_.underlayer_color);
                underlayer.SetStrokeColor(render_settings_.underlayer_color);
                // толщина линий stroke-width равна настройке underlayer_width
                underlayer.SetStrokeWidth(render_settings_.underlayer_width);
                // формы конца линии stroke-linecap и соединений stroke-linejoin равны round
                underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                
                bus_labels.push_back(underlayer);
                bus_labels.push_back(text);
                
                // Название маршрута должно отрисовываться у каждой из его конечных остановок.
                // В некольцевом маршруте — когда "is_roundtrip": false — конечной считается первая и последняя остановки маршрута
                if (!bus->is_roundtrip && bus->stops.front() != bus->stops.back()) 
                {
                    svg::Text text_2 { text };
                    svg::Text underlayer_2 { underlayer };

                    text_2.SetPosition(sphere_projector(bus->stops.back()->coordinates));
                    underlayer_2.SetPosition(sphere_projector(bus->stops.back()->coordinates));
                    
                    bus_labels.push_back(underlayer_2);
                    bus_labels.push_back(text_2);
                }

                if (color < render_settings_.color_palette.size() - 1) 
                {
                    ++color;
                }
                
                else 
                {
                    color = 0;
                }
            }
        }
        
        return bus_labels;
    }

    std::vector<svg::Circle> MapRenderer::RenderStopPoints(const std::map<std::string_view, const tc::Stop*>& stops, const SphereProjector& sphere_projector) const 
    {
        // Каждая остановка маршрута изображается на карте в виде кружочков белого цвета
        svg::Circle circle;
        std::vector<svg::Circle> circles;

        for (const auto &[stop_name, stop] : stops) 
        {    
            // координаты центра cx и cy — координаты соответствующей остановки на карте
            circle.SetCenter(sphere_projector(stop->coordinates));
            // радиус r равен настройке stop_radius из словаря render_settings
            circle.SetRadius(render_settings_.stop_radius);
            // цвет заливки fill — "white"
            circle.SetFillColor("white"s);

            circles.push_back(circle);
        }

        return circles;
    }

    std::vector<svg::Text> MapRenderer::RenderStopLabel(const std::map<std::string_view, const tc::Stop*>& stops, const SphereProjector& sphere_projector) const 
    {
        // Для каждой остановки выведите два текстовых объекта: подложку и саму надпись
        svg::Text text;
        svg::Text underlayer;
        std::vector<svg::Text> stop_labels;

        for (const auto& [stop_name, stop] : stops) 
        {
            text.SetFillColor("black"s);
            // x и y — координаты соответствующей остановки
            text.SetPosition(sphere_projector(stop->coordinates));
            // смещение dx и dy равно настройке stop_label_offset
            text.SetOffset(render_settings_.stop_label_offset);
            // размер шрифта font-size равен настройке stop_label_font_size
            text.SetFontSize(render_settings_.stop_label_font_size);
            // название шрифта font-family — "Verdana"
            text.SetFontFamily("Verdana"s);
            // свойства font-weight быть не должно, содержимое — название остановки
            text.SetData(stop->name);
            
            // Дополнительные свойства подложки:
            underlayer.SetPosition(sphere_projector(stop->coordinates));
            underlayer.SetOffset(render_settings_.stop_label_offset);
            underlayer.SetFontSize(render_settings_.stop_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetData(stop->name);

            // цвет заливки fill и цвет линий stroke равны настройке underlayer_color
            underlayer.SetFillColor(render_settings_.underlayer_color);
            underlayer.SetStrokeColor(render_settings_.underlayer_color);
            // толщина линий stroke-width равна настройке underlayer_width
            underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            // формы конца линии stroke-linecap и соединений stroke-linejoin равны "round"
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            
            stop_labels.push_back(underlayer);
            stop_labels.push_back(text);
        }
        
        return stop_labels;
    }

    svg::Document MapRenderer::GetSVG(const std::map<std::string_view, const tc::Bus*>& buses) const 
    {
        svg::Document document;
        std::vector<geo::Coordinates> stop_coordinates;
        std::map<std::string_view, const tc::Stop*> stops;

        for (const auto& [bus_number, bus] : buses) 
        {
            for (const auto& stop : bus->stops) 
            {
                stop_coordinates.push_back(stop->coordinates);
                stops[stop->name] = stop;
            }
        }

        SphereProjector sphere_projector(stop_coordinates.begin(), stop_coordinates.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
        
        for (const auto& line : RenderRouteLines(buses, sphere_projector))
        { 
            document.Add(line);
        }
        
        for (const auto& text : RenderBusLabel(buses, sphere_projector)) 
        {
            document.Add(text);
        }
        
        for (const auto& circle : RenderStopPoints(stops, sphere_projector)) 
        {
            document.Add(circle);
        }
        
        for (const auto& text : RenderStopLabel(stops, sphere_projector)) 
        {
            document.Add(text);
        }

        return document;
    }
} // end namespace renderer