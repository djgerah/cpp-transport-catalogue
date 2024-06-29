#pragma once

#include "geo.h"
#include "transport_catalogue.h"
#include <vector>

namespace reader
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
        std::string description;  // Параметры маршрута или кординаты
    };

    class InputReader 
    {
        public:
        /**
         * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
         */
        void ParseLine(std::string_view line);

        /**
         * Наполняет данными транспортный справочник, используя команды из commands_
         */
        void ApplyCommands(tc::TransportCatalogue& catalogue) const;

        private:
        
        std::vector<CommandDescription> commands_;
    };
}

namespace parse
{  
    geo::Coordinates Coordinates(std::string_view latitude, std::string_view longitude);
    std::string_view Trim(std::string_view string);
    std::vector<std::string_view> Split(std::string_view string, char delim);
    std::pair<std::vector<std::string_view>, bool> Route(std::string_view route);
    std::pair<std::string_view, int> Distance(std::string_view line);
    reader::CommandDescription Description(std::string_view line);  
} // namespace parse