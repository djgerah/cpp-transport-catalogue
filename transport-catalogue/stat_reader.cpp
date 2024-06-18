#include "input_reader.h"
#include "transport_catalogue.h"
#include <iomanip>
#include <string_view>
 
void ParseAndPrintStat(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out) 
{
    request = parse::Trim(request);
    std::string_view processed_request;

    if (std::string(request.begin(), request.begin() + 4) == "Stop")
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

    else if (std::string(request.begin(), request.begin() + 3) == "Bus")
    {
        request.remove_prefix(4);
        const tc::Bus* bus = tansport_catalogue.GetBus(request);

        if (bus != nullptr) 
        {   
            out << "Bus " << bus->name_ << ": "
                << bus->stops_.size() << " stops on route, "
                << tansport_catalogue.GetUniqStops(bus->name_).size() << " unique stops, "
                << std::setprecision(6) << tansport_catalogue.GetRouteLength(bus) << " route length" 
                << std::endl;
        } 
        
        else 
        {      
            out << "Bus " << request << ": not found" << std::endl;
        }
    }
}