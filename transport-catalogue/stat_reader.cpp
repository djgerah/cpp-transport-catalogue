#include "input_reader.h"
#include "transport_catalogue.h"
#include <iomanip>
#include <string_view>

void PrintStatStop(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out)
{
        request.remove_prefix(5);
        const tc::Stop* stop = tansport_catalogue.GetStop(request);

        if (stop != nullptr)
        {
            out << "Stop " << stop->name_ << ": ";

            auto buses_by_stop = tansport_catalogue.GetBusesByStop(stop->name_);

            if (!buses_by_stop.empty())
            {
                out << "buses ";

                for (auto& bus : buses_by_stop)
                {
                    out << bus << " ";
                }
                
                out << std::endl;
            }

            else
            {
                out << "no buses" << std::endl;
            }   
        }

        else
        {
            out << "Stop " << request << ": not found" << std::endl;
        }
}

void PrintStatBus(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out)
{
        request.remove_prefix(4);
        const tc::Bus* bus = tansport_catalogue.GetBus(request);

        if (bus != nullptr) 
        {   
            out << "Bus " << bus->name_ << ": "
                << tansport_catalogue.GetInfo(bus).total_stops << " stops on route, "
                // << bus->stops_.size() << " stops on route, "
                << tansport_catalogue.GetInfo(bus).unique_stops << " unique stops, " 
                // << tansport_catalogue.GetUniqStops(bus->name_).size() << " unique stops, "
                << std::setprecision(6) << tansport_catalogue.GetInfo(bus).route_length << " route length" 
                // << std::setprecision(6) << tansport_catalogue.GetRouteLength(bus) << " route length" 
                << std::endl;
        } 
        
        else 
        {      
            out << "Bus " << request << ": not found" << std::endl;
        }
}

void ParseAndPrintStat(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out) 
{
    request = parse::Trim(request);

    if (std::string(request.begin(), request.begin() + 4) == "Stop")
    {
        PrintStatStop(tansport_catalogue, request, out);
    }

    else if (std::string(request.begin(), request.begin() + 3) == "Bus")
    {
        PrintStatBus(tansport_catalogue, request, out);
    }
}