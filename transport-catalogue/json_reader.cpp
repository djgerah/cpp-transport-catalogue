#include <sstream>
#include "json_builder.h"
#include "json_reader.h"
#include "transport_catalogue.h"

namespace json_reader 
{
    using namespace std::string_literals;

    CommandDescription JsonReader::ParseCommandDescription(const json::Node& request) 
    {
        // description: маршрут или координаты
        json::Dict description;

        for (const auto& item : request.AsDict()) 
        {
            if (item.first != "type"s && item.first != "name"s) 
            {
                description.insert(item);
            }
        }

        return {request.AsDict().at("type"s).AsString(), // Название команды (Stop или Bus)
                request.AsDict().at("name"s).AsString(), // Номер маршрута или название остановки
                description };                          // Параметры маршрута или кординаты
    }

    void JsonReader::ParseRequest(const json::Node& request) 
    {
        json_reader::CommandDescription command_description = ParseCommandDescription(request);
        
        commands_.push_back(std::move(command_description));
    }

    void JsonReader::FillTransportCatalogue(tc::TransportCatalogue& catalogue) 
    {
        auto base_requests = document_.GetRoot().AsDict().at("base_requests"s).AsArray();
        
        for (const auto& base_request : base_requests) 
        {
            ParseRequest(base_request);
        }

        ApplyCommands(catalogue);
    }


    const json::Node& JsonReader::GetRenderSettings() const 
    {
        return document_.GetRoot().AsDict().at("render_settings"s);
    }

    const json::Node& JsonReader::GetBaseRequests() const 
    {
        return document_.GetRoot().AsDict().at("base_requests");
    }

    const json::Node& JsonReader::GetStatRequests() const 
    {
        return document_.GetRoot().AsDict().at("stat_requests"s);
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
        for (const auto &road_distance : command.description.at("road_distances"s).AsDict()) 
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

    svg::Rgb MakeRGB(const json::Array& type)
    {
        return svg::Rgb(type[0].AsInt(), type[1].AsInt(), type[2].AsInt());
    }

    svg::Rgba MakeRGBA(const json::Array& type)
    {
        return svg::Rgba(type[0].AsInt(), type[1].AsInt(), type[2].AsInt(), type[3].AsDouble());
    }

    void ProcessTheColor(const json::Dict& request, renderer::RenderSettings& render_settings)
    {
        if (request.at("underlayer_color"s).IsString()) 
        {
            render_settings.underlayer_color = request.at("underlayer_color"s).AsString();
        } 
        
        else if (request.at("underlayer_color"s).IsArray()) 
        {
            const json::Array& type = request.at("underlayer_color"s).AsArray();

            if (type.size() == 3) 
            {
                render_settings.underlayer_color = MakeRGB(type);
            } 
            
            else if (type.size() == 4) 
            {
                render_settings.underlayer_color = MakeRGBA(type);
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
                    render_settings.color_palette.emplace_back(MakeRGB(type));
                }

                else if (type.size() == 4) 
                {
                    render_settings.color_palette.emplace_back(MakeRGBA(type));
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
            const auto& request_map = request.AsDict();
            const auto& type = request_map.at("type").AsString();

            if (type == "Stop") 
            {
                result.push_back(PrintStop(request_map, catalogue, request_handler).AsDict());
            }

            if (type == "Bus") 
            {
                result.push_back(PrintBus(request_map, catalogue).AsDict());
            }

            if(type == "Map")
            {
                result.push_back(PrintMap(request_map, request_handler).AsDict());
            }
        }
        
        json::Print(json::Document{ result }, std::cout);
    }

    const json::Node JsonReader::PrintBus(const json::Dict& request, tc::TransportCatalogue& catalogue_) const 
    {
        json::Node result;
        const tc::Bus* bus = catalogue_.GetBus(request.at("name").AsString());

        if (bus != nullptr) 
        {
            auto bus_stat = catalogue_.GetBusStat(request.at("name").AsString());

            result = json::Builder{}.StartDict()
                                    .Key("request_id").Value(request.at("id").AsInt())
                                    .Key("curvature").Value(bus_stat->curvature)
                                    .Key("route_length").Value(bus_stat->route_length)
                                    .Key("stop_count").Value(static_cast<int>(bus_stat->total_stops))
                                    .Key("unique_stop_count").Value(static_cast<int>(bus_stat->unique_stops))
                                    .EndDict().Build();
        }

        else 
        {
            result = json::Builder{}.StartDict()
                                    .Key("request_id").Value(request.at("id").AsInt())
                                    .Key("error_message").Value("not found")
                                    .EndDict().Build();
        }
        
        return json::Node{ result };
    }

    const json::Node JsonReader::PrintStop(const json::Dict& request, tc::TransportCatalogue& catalogue_, RequestHandler& request_handler) const 
    {
        json::Node result;
        const tc::Stop* stop = catalogue_.GetStop(request.at("name").AsString());

        if (stop != nullptr) 
        {
            json::Array buses;

            for (const auto& bus : request_handler.GetBusesByStop(request.at("name").AsString())) 
            {
                buses.push_back(bus);
            }
            
            result = json::Builder{}.StartDict()
                                    .Key("request_id").Value(request.at("id").AsInt())
                                    .Key("buses").Value(buses)
                                    .EndDict().Build();
        }

        else 
        {
            result = json::Builder{}.StartDict()
                                    .Key("request_id").Value(request.at("id").AsInt())
                                    .Key("error_message").Value("not found")
                                    .EndDict().Build();
        }

        return json::Node{ result };
    }

    const json::Node JsonReader::PrintMap(const json::Dict& request, RequestHandler& request_handler) const 
    {
        json::Node result;

        std::ostringstream strm;
        svg::Document map = request_handler.RenderMap();
        map.Render(strm);

        result = json::Builder{}.StartDict()
                                .Key("request_id").Value(request.at("id").AsInt())
                                .Key("map").Value(strm.str())
                                .EndDict().Build();

        return json::Node{ result };
    }
} // end namespace json_reader