#include "request_handler.h"

const std::set<std::string> RequestHandler::GetBusesByStop(std::string_view stop_name) const 
{
    return catalogue_.GetStop(stop_name)->buses;
}

svg::Document RequestHandler::RenderMap() const 
{
    return renderer_.GetSVG(catalogue_.GetAllBuses());
}