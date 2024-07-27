#include <iostream>
#include "json_reader.h"
#include "request_handler.h"

int main() 
{
    tc::TransportCatalogue catalogue;
    json_reader::JsonReader document(std::cin);
    
    document.FillTransportCatalogue(catalogue);
    
    const json::Node& stat_requests = document.GetStatRequests();
    const json::Dict& render_settings = document.GetRenderSettings().AsMap();
    const renderer::MapRenderer& renderer = json_reader::FillRenderSettings(render_settings);

    handler::RequestHandler request_handler(catalogue, renderer);
    // svg::Document doc = request_handler.RenderMap();
    // doc.Render(std::cout);
    request_handler.ProcessRequests(stat_requests);

    return 0;
}