#include "input_reader.h"
#include "iostream"
#include <algorithm>
#include <cassert>
#include <iterator>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates parse::Coordinates(std::string_view str) 
{
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) 
    {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

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

CommandDescription parse::TheCommandDescription(std::string_view line) 
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

    return { std::string(line.substr(0, space_pos)),                        // Параметры маршрута или кординаты
            std::string(line.substr(not_space, colon_pos - not_space)),     // Номер маршрута или название остановки
            std::string(line.substr(colon_pos + 1)) };                      // Название команды (Stop или Bus)
}


void parse::InputReader::ParseLine(std::string_view line) 
{
    auto command_description = parse::TheCommandDescription(line);
    
    if (command_description) 
    {
        commands_.push_back(std::move(command_description));
    }
}

void parse::InputReader::ApplyCommands([[maybe_unused]] tc::TransportCatalogue& catalogue) const 
{
    // Реализуйте метод самостоятельно
    tc::Bus bus;
    tc::Stop stop;

    for (auto& c : commands_)
    {  
        // command: автобус или остановка 
        if (c.command == "Stop")    
        {
            // id: номер автобуса или название остановки
            stop.name_ = c.id;
            // description: маршрут или координаты
            stop.coordinates = parse::Coordinates(c.description);

            catalogue.AddStop(stop);
        }

        // command: автобус или остановка
        else if (c.command == "Bus")
        {
            // id: номер автобуса или название остановки
            bus.name_ = c.id;
            // description: маршрут или координаты    
            auto parameters = parse::Route(c.description);

            bus.stops_.assign(parameters.first.begin(), parameters.first.end());

            for (auto stop : bus.stops_)
            {
                auto temp = catalogue.GetStop(stop);
                if (temp != nullptr)
                {
                    bus.stops_ptr_.push_back(temp); 
                }

                else
                {
                    std::cout << "Current stop is not exist yet." << std::endl;
                    continue;;
                }
            }

            bus.is_circle_ = parameters.second;

            catalogue.AddBus(bus);
        }
    }
}