#include "json_reader.h"
#include "request_handler.h"

int main() 
{
    tc::TransportCatalogue catalogue;

    json_reader::JsonReader document (std::cin);
    document.FillTransportCatalogue(catalogue);

    const json::Node& stat_requests = document.GetStatRequests();
    const renderer::MapRenderer& renderer = document.FillRenderSettings(document.GetRenderSettings());
    const tc::RoutingSettings routing_settings = document.FillRoutingSettings(document.GetRoutingSettings());
    const tc::TransportRouter router = { routing_settings, catalogue };

    RequestHandler request_handler(catalogue, renderer, router);
    document.ProcessRequests(stat_requests, catalogue, request_handler);

    return 0;
}