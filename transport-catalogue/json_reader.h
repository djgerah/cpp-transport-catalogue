#pragma once

#include <string>
#include <vector>
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

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

            explicit JsonReader(std::istream& document)
                : document_(json::Load(document)) 
                {}          

            const json::Node& GetBaseRequests() const;
            const json::Node& GetStatRequests() const;
            const json::Node& GetRenderSettings() const;

            void FillTransportCatalogue(tc::TransportCatalogue &catalogue);

        private:
        
            void ParseRequest(const json::Node &request);
            static CommandDescription ParseCommandDescription(const json::Node &request);
            /*
            * Наполняет данными транспортный справочник, используя команды из commands_
            */
            void ApplyCommands(tc::TransportCatalogue &catalogue) const;
            static std::vector<std::string_view> ParseRoute(const json::Dict& description);
            
            json::Document document_;
            std::vector<CommandDescription> commands_;
    };

    const renderer::MapRenderer FillRenderSettings(const json::Dict& request_map);
} // end namespace json_reader