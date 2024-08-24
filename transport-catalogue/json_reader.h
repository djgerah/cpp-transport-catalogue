#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace json_reader
{
    struct CommandDescription 
    {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const 
        {
            return !command.empty();
        }

        bool operator!() const 
        {
            return !operator bool();
        }

        std::string command;      // Название команды (Stop или Bus)
        std::string id;           // Номер маршрута или название остановки
        json::Dict description;   // Параметры маршрута или кординаты
    };

    class JsonReader 
    {
        public:
        
            JsonReader(std::istream& document)
                : document_(json::Load(document))
                {}

            const json::Node& GetBaseRequests() const;
            const json::Node& GetStatRequests() const;
            const json::Node& GetRenderSettings() const;
            const json::Node& GetRoutingSettings() const;
            const json::Node PrintBus(const json::Dict& request, tc::TransportCatalogue& catalogue_) const;
            const json::Node PrintStop(const json::Dict& request, tc::TransportCatalogue& catalogue_, RequestHandler& request_handler) const;
            const json::Node PrintMap(const json::Dict& request, RequestHandler& request_handler) const;
            const json::Node PrintRoute(const json::Dict& request, tc::TransportCatalogue& catalogue_, RequestHandler& request_handler) const;
            void ProcessRequests(const json::Node& stat_requests, tc::TransportCatalogue& catalogue, RequestHandler& request_handler) const;
            void FillTransportCatalogue(tc::TransportCatalogue& catalogue);
            renderer::MapRenderer FillRenderSettings(const json::Node& settings) const;
            tc::RoutingSettings FillRoutingSettings(const json::Node& settings) const;

        private:
            
            tc::Stop MakeStop(const json_reader::CommandDescription& c) const;
            tc::Bus MakeBus(const json_reader::CommandDescription& c, tc::TransportCatalogue& catalogue) const;
            void ProcessColors(const json::Dict& request, renderer::RenderSettings& render_settings) const;
            svg::Rgb MakeRGB(const json::Array& type) const;
            svg::Rgba MakeRGBA(const json::Array& type) const;
            void AddDistance(const json_reader::CommandDescription& c, tc::TransportCatalogue& catalogue) const;
            void ParseRequest(const json::Node &request);
            static CommandDescription ParseCommandDescription(const json::Node &request);
            /*
            * Наполняет данными транспортный справочник, используя команды из commands_
            */
            void ApplyCommands(tc::TransportCatalogue &catalogue) const;
            static std::vector<const tc::Stop*> ParseRoute(const json::Dict& description, tc::TransportCatalogue& catalogue);
            
            json::Document document_;
            std::vector<CommandDescription> commands_;
    };
} // end namespace json_reader