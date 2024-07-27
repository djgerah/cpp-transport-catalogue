#include "json_reader.h"
#include "transport_catalogue.h"

namespace json_reader 
{
    using namespace std::string_literals;

    CommandDescription JsonReader::ParseCommandDescription(const json::Node& request) 
    {
        // description: маршрут или координаты
        json::Dict description;

        for (const auto& item : request.AsMap()) 
        {
            if (item.first != "type"s && item.first != "name"s) 
            {
                description.insert(item);
            }
        }

        return {request.AsMap().at("type"s).AsString(), // Название команды (Stop или Bus)
                request.AsMap().at("name"s).AsString(),      // Номер маршрута или название остановки
                description };                        // Параметры маршрута или кординаты
    }

    void JsonReader::ParseRequest(const json::Node& request) 
    {
        json_reader::CommandDescription command_description = ParseCommandDescription(request);
        
        commands_.push_back(std::move(command_description));
    }

    void JsonReader::FillTransportCatalogue(tc::TransportCatalogue& catalogue) 
    {
        auto base_requests = document_.GetRoot().AsMap().at("base_requests"s).AsArray();
        
        for (const auto& base_request : base_requests) 
        {
            ParseRequest(base_request);
        }

        ApplyCommands(catalogue);
    }


    const json::Node& JsonReader::GetRenderSettings() const 
    {
        return document_.GetRoot().AsMap().at("render_settings"s);
    }

    const json::Node& JsonReader::GetBaseRequests() const 
    {
        return document_.GetRoot().AsMap().at("base_requests");
    }

    const json::Node& JsonReader::GetStatRequests() const 
    {
        return document_.GetRoot().AsMap().at("stat_requests"s);
    }


    std::vector<std::string_view> JsonReader::ParseRoute(const json::Dict& description) 
    {
        std::vector<std::string_view> results;
        
        for (const auto &stop : description.at("stops"s).AsArray()) 
        {
            results.push_back(stop.AsString());
        }
        
        return results;
    }

    /*
    * Обрабатывает поля CommandDescription
    */
    void JsonReader::ApplyCommands(tc::TransportCatalogue &catalogue) const 
    {    
        for (const auto& c : commands_) 
        {
            // command: автобус или остановка 
            if (c.command == "Stop"s) 
            {
                // id: номер автобуса или название остановки
                // description: маршрут или координаты
                catalogue.AddStop(c.id, {c.description.at("latitude"s).AsDouble(), c.description.at("longitude"s).AsDouble()});
            }
        }

        for (const auto& c : commands_) 
        {
            // command: автобус или остановка 
            if (c.command == "Stop"s && c.description.count("road_distances"s)) 
            {
                // id: номер автобуса или название остановки
                // description: маршрут или координаты
                for (const auto &road_distance : c.description.at("road_distances"s).AsMap()) 
                {
                    catalogue.SetDistance(catalogue.GetStop(c.id), catalogue.GetStop(road_distance.first), road_distance.second.AsInt());
                }
            }
        }

        for (const auto& c : commands_) 
        {
            // command: автобус или остановка
            if (c.command == "Bus"s) 
            {
                // id: номер автобуса или название остановки
                // description: маршрут или координаты
                catalogue.AddBus(c.id, ParseRoute(c.description), c.description.at("is_roundtrip"s).AsBool());
            }
        }
    }

    const renderer::MapRenderer FillRenderSettings(const json::Dict& request)
    {
        renderer::RenderSettings render_settings;

        render_settings.width = request.at("width"s).AsDouble();
        render_settings.height = request.at("height"s).AsDouble();
        render_settings.padding = request.at("padding"s).AsDouble();
        render_settings.stop_radius = request.at("stop_radius"s).AsDouble();
        render_settings.line_width = request.at("line_width"s).AsDouble();
        render_settings.bus_label_font_size = request.at("bus_label_font_size"s).AsInt();
        const json::Array& bus_label_offset = request.at("bus_label_offset"s).AsArray();
        render_settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };

        render_settings.stop_label_font_size = request.at("stop_label_font_size"s).AsInt();
        const json::Array& stop_label_offset = request.at("stop_label_offset"s).AsArray();
        render_settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };
        
        if (request.at("underlayer_color"s).IsString()) 
        {
            render_settings.underlayer_color = request.at("underlayer_color"s).AsString();
        } 
        
        else if (request.at("underlayer_color"s).IsArray()) 
        {
            const json::Array& underlayer_color = request.at("underlayer_color"s).AsArray();

            if (underlayer_color.size() == 3) 
            {
                render_settings.underlayer_color = svg::Rgb(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt());
            } 
            
            else if (underlayer_color.size() == 4) 
            {
                render_settings.underlayer_color = svg::Rgba(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt(), underlayer_color[3].AsDouble());
            } 
            
            else 
            {
                throw std::logic_error("wrong underlayer color type"s);
            }
        } 
        
        else 
        {
            throw std::logic_error("wrong underlayer color"s);
        }
        
        render_settings.underlayer_width = request.at("underlayer_width"s).AsDouble();
        const json::Array& color_palette = request.at("color_palette"s).AsArray();
        
        for (const auto& color : color_palette) 
        {
            if (color.IsString()) 
            {
                render_settings.color_palette.emplace_back(color.AsString());
            } 
            
            else if (color.IsArray()) 
            {
                const json::Array& type = color.AsArray();

                if (type.size() == 3) 
                {
                    render_settings.color_palette.emplace_back(svg::Rgb(type[0].AsInt(), type[1].AsInt(), type[2].AsInt()));
                }

                else if (type.size() == 4) 
                {
                    render_settings.color_palette.emplace_back(svg::Rgba(type[0].AsInt(), type[1].AsInt(), type[2].AsInt(), type[3].AsDouble()));
                } 
                
                else 
                {
                    throw std::logic_error("wrong color palette type"s);
                }
            } 
            
            else 
            {
                throw std::logic_error("wrong color palette"s);
            }
        }
        
        return render_settings;
    }
} // end namespace json_reader