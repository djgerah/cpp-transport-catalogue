#include "input_reader.h"
#include <algorithm>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates parse::Coordinates(std::string_view latitude, std::string_view longitude) 
{
    double lat = std::stod(std::string(latitude));
    double lng = std::stod(std::string(longitude));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view parse::Trim(std::string_view string) 
{
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) 
    {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> parse::Split(std::string_view string, char delim) 
{
    std::vector<std::string_view> result;
    size_t pos = 0;

    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) 
    {
        auto delim_pos = string.find(delim, pos);

        if (delim_pos == string.npos) 
        {
            delim_pos = string.size();
        }

        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) 
        {
            result.push_back(substr);
        }

        pos = delim_pos + 1;
    }

    return result;
}

/*
* Парсит маршрут.
* Для кольцевого маршрута (A>B>C>A) возвращает true и массив названий остановок [A,B,C,A]
* Для некольцевого маршрута (A-B-C-D) возвращает false и массив названий остановок [A,B,C,D,C,B,A]
*/
std::pair<std::vector<std::string_view>, bool> parse::Route(std::string_view route) 
{
    // Не кольцевой маршрут
    if (route.find('>') != route.npos) 
    {
        return std::make_pair(Split(route, '>'), false);
    }
    
    // Кольцевой маршрут
    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());

    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());
    
    return std::make_pair(results, true);
}

/*
 * Парсит строку вида "3900m to Marushkino", возвращает название остановки и расстояние до этой остановки в метрах
 */
std::pair<std::string_view, int> parse::Distance(std::string_view line) 
{
    using namespace std::string_literals;
    
    auto to_pos = line.find("to"s);

    if (to_pos == std::string_view::npos) 
    {
        return {};
    }

    auto m_pos = line.find('m');

    if (m_pos >= to_pos) 
    {
        return {};
    }
    
    return {line.substr(to_pos + 3), std::stoi(std::string(line.substr(0, m_pos)))};
}

reader::CommandDescription parse::Description(std::string_view line) 
{
    auto colon_pos = line.find(':');

    if (colon_pos == line.npos) 
    {
        return {};
    }

    auto space_pos = line.find(' ');

    if (space_pos >= colon_pos) 
    {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);

    if (not_space >= colon_pos) 
    {
        return {};
    }

    return  { 
              std::string(line.substr(0, space_pos)),                         // Параметры маршрута или кординаты
              std::string(line.substr(not_space, colon_pos - not_space)),     // Номер маршрута или название остановки
              std::string(line.substr(colon_pos + 1))                         // Название команды (Stop или Bus)
            };                      
}

/*
 * Парсит строку запроса, заполняет структуру типа CommandDescription
 */
void reader::InputReader::ParseLine(std::string_view line) 
{
    auto command_description = parse::Description(line);
    
    if (command_description) 
    {
        commands_.push_back(std::move(command_description));
    }
}

/*
 * Обрабатывает поля CommandDescription
 */
void reader::InputReader::ApplyCommands([[maybe_unused]] tc::TransportCatalogue& catalogue) const 
{
    // Реализуйте метод самостоятельно
    // [stop.id] = { координаты (latitude и longitude), следующая остановка, расстояние до неё };
    std::unordered_map<std::string_view, std::vector<std::string_view>> route_descriptions;

    for (const auto& c: commands_) 
    {
        // command: автобус или остановка 
        if (c.command == "Stop") 
        {
            // id: номер автобуса или название остановки
            // description: маршрут или координаты
            route_descriptions[c.id] = parse::Split(c.description, ',');

            if (!route_descriptions[c.id].empty()) 
            {
                auto parameters = parse::Coordinates(route_descriptions[c.id][0], route_descriptions[c.id][1]); // <- latitude и longitude
                catalogue.AddStop(c.id, parameters);
            }
        }
    }
    
    for (const auto& c: commands_) 
    {
        // command: автобус или остановка 
        if (c.command == "Stop") 
        {
            // Если кроме latitude и longitude, description содержит информацию о расстоянии до других остановок
            for (int i = 2; i < route_descriptions[c.id].size(); ++i) 
            {
                std::pair<std::string_view, int> distance = parse::Distance(route_descriptions[c.id][i]);
                catalogue.SetDistance(c.id, distance.first, distance.second);
            }
        }  
    }

    for (const auto& c: commands_) 
    {
        // command: автобус или остановка
        if (c.command == "Bus")
        {
            // id: номер автобуса или название остановки
            // description: маршрут или координаты
            auto parameters = parse::Route(c.description);
            catalogue.AddBus(c.id, parameters.first, parameters.second);
        }
    }
}