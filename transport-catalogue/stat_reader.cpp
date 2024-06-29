#include "input_reader.h"
#include "transport_catalogue.h"
#include <iomanip>
#include <string_view>

namespace print
{
    void StatBus(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out)
    {
        request.remove_prefix(4);
        const tc::Bus* bus = tansport_catalogue.GetBus(request);

        if (bus != nullptr) 
        {   
            tc::Info info = tansport_catalogue.GetInfo(bus);

            out << "Bus " << bus->name_ << ": "
                << info.total_stops << " stops on route, "
                << info.unique_stops << " unique stops, " 
                << std::setprecision(6) << info.route_length << " route length, " 
                << std::setprecision(6) << info.curvature << " curvature"
                << std::endl;
        }   
        
        else 
        {      
            out << "Bus " << request << ": not found" << std::endl;
        }
    }

    void StatStop(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out)
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

        else if (stop == nullptr)
        {
            out << "Stop " << request << ": not found" << std::endl;
        }
    }

    void GeneralStat(tc::TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& out) 
    {
        request = parse::Trim(request);

        if (std::string(request.begin(), request.begin() + 3) == "Bus")
        {
            print::StatBus(tansport_catalogue, request, out);
        }

        else if (std::string(request.begin(), request.begin() + 4) == "Stop")
        {
            print::StatStop(tansport_catalogue, request, out);
        }
    }
} // namespace print