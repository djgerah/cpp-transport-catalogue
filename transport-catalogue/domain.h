#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include "geo.h"

namespace tc 
{
    struct Stop 
    {
        std::string name;
        geo::Coordinates coordinates;
        std::set<std::string> buses;
    };

    struct Bus 
    {
        std::string number;
        std::vector<const Stop*> stops;
        bool is_roundtrip;
    };

    struct BusStat 
    {
        size_t total_stops = 0;
        size_t unique_stops = 0;
        double route_length = 0.0;
        double curvature = 0.0;
    };
} // end namespace tc