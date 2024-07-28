#include <sstream>
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


    std::vector<const tc::Stop*> JsonReader::ParseRoute(const json::Dict& description, tc::TransportCatalogue& catalogue) 
    {
        std::vector<const tc::Stop*> stop_ptr;
        
        for (const auto &stop : description.at("stops"s).AsArray()) 
        {
            stop_ptr.push_back(catalogue.GetStop(stop.AsString()));
        }
        
        return stop_ptr;
    }

    tc::Stop JsonReader::MakeStop(const json_reader::CommandDescription& command) const
    {
        // id: номер автобуса или название остановки
        // description: маршрут или координаты
        tc::Stop stop = {command.id, { command.description.at("latitude"s).AsDouble(), command.description.at("longitude"s).AsDouble()}, {} };
     
        return stop;
    }

    void JsonReader::AddDistance(const json_reader::CommandDescription& command, tc::TransportCatalogue& catalogue) const
    {
        for (const auto &road_distance : command.description.at("road_distances"s).AsMap()) 
        {
            // id: номер автобуса или название остановки
            // description: маршрут или координаты
            catalogue.SetDistance(catalogue.GetStop(command.id), catalogue.GetStop(road_distance.first), road_distance.second.AsInt());
        }
    }

    tc::Bus JsonReader::MakeBus(const json_reader::CommandDescription& command, tc::TransportCatalogue& catalogue) const
    {
        // id: номер автобуса или название остановки
        // description: маршрут или координаты
        tc::Bus bus = { command.id, ParseRoute(command.description, catalogue), command.description.at("is_roundtrip"s).AsBool() };
     
        return bus;
    }

    /*
    * Обрабатывает поля CommandDescription
    */
    void JsonReader::ApplyCommands(tc::TransportCatalogue& catalogue) const 
    {    
        for (auto& c : commands_) 
        {
            // command: автобус или остановка s
            if (c.command == "Stop"s) 
            {
                catalogue.AddStop(MakeStop(c));
            }
        }

        for (auto& c : commands_) 
        {
            // command: автобус или остановка 
            if (c.command == "Stop"s && c.description.count("road_distances"s)) 
            {
                AddDistance(c, catalogue);
            }
        }

        for (auto& c : commands_) 
        {
            // command: автобус или остановка
            if (c.command == "Bus"s) 
            {
                catalogue.AddBus(MakeBus(c, catalogue));
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
        
        ProcessTheColor(request, render_settings);

        return render_settings;
    }

    void ProcessTheColor(const json::Dict& request, renderer::RenderSettings& render_settings)
    {
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
    }

    void JsonReader::ProcessRequests(const json::Node& stat_requests, tc::TransportCatalogue& catalogue, RequestHandler& request_handler) const 
    {
        json::Array result;

        for (auto& request : stat_requests.AsArray()) 
        {
            const auto& request_map = request.AsMap();
            const auto& type = request_map.at("type").AsString();

            if (type == "Stop") 
            {
                result.push_back(PrintStop(request_map, catalogue, request_handler).AsMap());
            }

            if (type == "Bus") 
            {
                result.push_back(PrintBus(request_map, catalogue).AsMap());
            }

            if(type == "Map")
            {
                result.push_back(PrintMap(request_map, request_handler).AsMap());
            }
        }
        
        json::Print(json::Document{ result }, std::cout);
    }

    const json::Node JsonReader::PrintBus(const json::Dict& request, tc::TransportCatalogue& catalogue_) const 
    {
        json::Dict result;

        const std::string& route_number = request.at("name").AsString();
        const tc::Bus* bus = catalogue_.GetBus(route_number);
        result["request_id"] = request.at("id").AsInt();

        if (bus != nullptr) 
        {
            auto bus_stat = catalogue_.GetBusStat(route_number);

            result["curvature"] = bus_stat->curvature;
            result["route_length"] = bus_stat->route_length;
            result["stop_count"] = static_cast<int>(bus_stat->total_stops);
            result["unique_stop_count"] = static_cast<int>(bus_stat->unique_stops);
        }

        else 
        {
            result["error_message"] = json::Node{ static_cast<std::string>("not found") };
        }
        
        return json::Node{ result };
    }

    const json::Node JsonReader::PrintStop(const json::Dict& request, tc::TransportCatalogue& catalogue_, RequestHandler& request_handler) const 
    {
        json::Dict result;

        const std::string& stop_name = request.at("name").AsString();
        const tc::Stop* stop = catalogue_.GetStop(stop_name);
        result["request_id"] = request.at("id").AsInt();

        if (stop != nullptr) 
        {
            json::Array buses;

            for (const auto& bus : request_handler.GetBusesByStop(stop_name)) 
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

    const json::Node JsonReader::PrintMap(const json::Dict& request, RequestHandler& request_handler) const 
    {
        json::Dict result;

        result["request_id"] = request.at("id").AsInt();
        std::ostringstream strm;
        svg::Document map = request_handler.RenderMap();
        map.Render(strm);
        result["map"] = strm.str();

        return json::Node{ result };
    }
} // end namespace json_reader