#pragma once

#include "transport_catalogue.h"

namespace print
{
    void StatBus(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out);
    void StatStop(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out);
    void GeneralStat(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out);
} // namespace print