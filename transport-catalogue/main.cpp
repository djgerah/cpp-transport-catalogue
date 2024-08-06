#include <iostream>
#include <fstream>
#include "json_reader.h"
#include "request_handler.h"

int main() 
{
    tc::TransportCatalogue catalogue;
    // json_reader::JsonReader document(std::cin);
    std::ifstream input("input.json");
    std::ofstream output("output.svg");

    json_reader::JsonReader document(input);
    document.FillTransportCatalogue(catalogue);
    
    const json::Node& stat_requests = document.GetStatRequests();
    const json::Dict& render_settings = document.GetRenderSettings().AsDict();
    const renderer::MapRenderer& renderer = json_reader::FillRenderSettings(render_settings);

    RequestHandler request_handler(catalogue, renderer);
    document.ProcessRequests(stat_requests, catalogue, request_handler);
    
    auto map = request_handler.RenderMap();
    map.Render(output);

    return 0;
}