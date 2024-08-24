#include "transport_router.h"

namespace tc 
{
    graph::DirectedWeightedGraph<double> tc::TransportRouter::AddStopEdge(const TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double> graph_to_build) 
    {
        graph::VertexId vertex_id = 0;

        for (const auto& [stop_name, stop_ptr] : catalogue.GetAllStops()) 
        {
            stop_to_vertex_id_[stop_ptr] = vertex_id;
            graph_to_build.AddEdge({ stop_ptr->name, 0,vertex_id, ++vertex_id, static_cast<double>(routing_settings_.bus_wait_time_) });
            
            ++vertex_id;
        }

        return graph_to_build;
    }

    graph::DirectedWeightedGraph<double> tc::TransportRouter::AddBusEdge(const TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double> graph_to_build)
    {
        for (auto& [name, bus_ptr] : catalogue.GetAllBuses())
        {

            for (size_t i = 0; i < bus_ptr->stops.size(); ++i) 
            {
                size_t span_count = 1;
                
                for (size_t j = i + 1; j < bus_ptr->stops.size(); ++j) 
                {
                    int A_to_B = 0;
                    int B_to_A = 0;

                    for (size_t d = i + 1; d <= j; ++d) 
                    {
                        // Получаем расстояние от остановки А до остановки В
                        A_to_B += catalogue.GetDistance(bus_ptr->stops[d - 1], bus_ptr->stops[d]);
                        // И от В до А, т.к. расстояние от остановки A до остановки B может быть не равно расстоянию от B до A
                        B_to_A += catalogue.GetDistance(bus_ptr->stops[d], bus_ptr->stops[d - 1]);
                    }

                    const Stop* from = bus_ptr->stops[i];
                    const Stop* to = bus_ptr->stops[j];

                    // Добавляем ребро "Остановка А - "Остановка B" для каждого маршрута
                    graph_to_build.AddEdge({ bus_ptr->number, span_count,
                                            stop_to_vertex_id_.at(from) + 1, stop_to_vertex_id_.at(to),
                                            // Разделив расстояние на среднюю скорость движения (скорость / время * 100), 
                                            // получаем время за которое было преодалено это расстояние
                                            A_to_B / (routing_settings_.bus_velocity_ / 6 * 100)
                                            });
                    
                    // Если маршрут некольцевой - так же добавляем ребро "Остановка B - Остановка A"
                    if (!bus_ptr->is_roundtrip) 
                    {
                        graph_to_build.AddEdge({ bus_ptr->number, span_count, 
                                                stop_to_vertex_id_.at(to) + 1, stop_to_vertex_id_.at(from),
                                                B_to_A / (routing_settings_.bus_velocity_ / 6 * 100)
                                                });
                    }
                    
                    ++span_count;
                }
            }
        }

        return graph_to_build;
    } 

    graph::DirectedWeightedGraph<double> tc::TransportRouter::BuildGraph(const TransportCatalogue& catalogue) 
    {
       graph::DirectedWeightedGraph<double> graph_to_build(catalogue.GetAllStops().size() * 2);
       
       return AddBusEdge(catalogue, AddStopEdge(catalogue, graph_to_build));
    }

    const std::optional<graph::Router<double>::RouteInfo> TransportRouter::GetRoute(const tc::Stop* from, const tc::Stop* to) const 
    {
        return router_->BuildRoute(stop_to_vertex_id_.at(from), stop_to_vertex_id_.at(to));
    }

    const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const 
    {
        return graph_;
    }
} // end namespace tc