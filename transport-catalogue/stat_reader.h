#pragma once

#include "transport_catalogue.h"

void PrintStatStop(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out);
void PrintStatBus(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out);
void ParseAndPrintStat(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out);