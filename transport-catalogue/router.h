#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <map>
#include <utility>
#include <vector>

#include "graph.h"
#include "domain.h"

/*
    Маршрутизатор — класс Router — класс, реализующий поиск кратчайшего пути во взвешенном ориентированном графе.
    Требует квадратичного относительно количества вершин объёма памяти, 
    не считая памяти, требуемой для хранения кэша маршрутов.
*/

namespace graph 
{
    template <typename Weight>
    class Router 
    {
        private:

            using Graph = DirectedWeightedGraph<Weight>;

        public:

            // Конструктор маршрутизатора имеет сложность 
            // 𝑂(𝑉3+𝐸)O(V 3+E), где 𝑉 — количество вершин графа, 𝐸 — количество рёбер.
            Router(const Graph& graph)
                : graph_(graph)
                , routes_internal_data_(graph.GetVertexCount(), std::vector<std::optional<RouteInternalData>>(graph.GetVertexCount()))
                {
                    InitializeRoutesInternalData(graph);

                    const size_t vertex_count = graph.GetVertexCount();
                    
                    for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) 
                    {
                        RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
                    }
                }

            struct RouteInfo 
            {
                Weight weight;
                std::vector<EdgeId> edges;
            };

            // Построение маршрута на готовом маршрутизаторе линейно относительно количества рёбер в маршруте. 
            // Таким образом, основная нагрузка построения оптимальных путей ложится на конструктор.
            std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const;
            void SetVertexId(std::map<const tc::Stop*, graph::VertexId> stop_to_vertex_id);
            graph::VertexId GetVertexId(const tc::Stop* stop);
            const graph::DirectedWeightedGraph<double>& GetGraph() const;

        private:

            struct RouteInternalData 
            {
                Weight weight;
                std::optional<EdgeId> prev_edge;
            };

            using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

            void InitializeRoutesInternalData(const Graph& graph) 
            {
                const size_t vertex_count = graph.GetVertexCount();
                for (VertexId vertex = 0; vertex < vertex_count; ++vertex) 
                {
                    routes_internal_data_[vertex][vertex] = RouteInternalData{ ZERO_WEIGHT, std::nullopt };
                    
                    for (const EdgeId edge_id : graph.GetIncidentEdges(vertex)) 
                    {
                        const auto& edge = graph.GetEdge(edge_id);
                        
                        if (edge.weight < ZERO_WEIGHT) 
                        {
                            // Маршрутизатор не работает с графами, имеющими рёбра отрицательного веса.
                            throw std::domain_error("Edges' weights should be non-negative");
                        }
                        auto& route_internal_data = routes_internal_data_[vertex][edge.to];
                        if (!route_internal_data || route_internal_data->weight > edge.weight) {
                            route_internal_data = RouteInternalData{ edge.weight, edge_id };
                        }
                    }
                }
            }

            void RelaxRoute(VertexId vertex_from, VertexId vertex_to, const RouteInternalData& route_from, const RouteInternalData& route_to) 
            {
                auto& route_relaxing = routes_internal_data_[vertex_from][vertex_to];
                const Weight candidate_weight = route_from.weight + route_to.weight;
                
                if (!route_relaxing || candidate_weight < route_relaxing->weight) 
                {
                    route_relaxing = { candidate_weight,
                                        route_to.prev_edge ? route_to.prev_edge : route_from.prev_edge };
                }
            }

            void RelaxRoutesInternalDataThroughVertex(size_t vertex_count, VertexId vertex_through) 
            {
                for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) 
                {
                    if (const auto& route_from = routes_internal_data_[vertex_from][vertex_through]) 
                    {
                        for (VertexId vertex_to = 0; vertex_to < vertex_count; ++vertex_to) 
                        {
                            if (const auto& route_to = routes_internal_data_[vertex_through][vertex_to]) 
                            {
                                RelaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                            }
                        }
                    }
                }
            }

            static constexpr Weight ZERO_WEIGHT{};
            const Graph& graph_;
            RoutesInternalData routes_internal_data_;
			std::map<const tc::Stop*, graph::VertexId> stop_to_vertex_id_ = {};
    };

    template <typename Weight>
    void Router<Weight>::SetVertexId(std::map<const tc::Stop*, graph::VertexId> stop_to_vertex_id)
    {
        stop_to_vertex_id_ = stop_to_vertex_id;
    }

    template <typename Weight>
    graph::VertexId Router<Weight>::GetVertexId(const tc::Stop* stop)
    {
        return stop_to_vertex_id_.at(stop);
    }
    
    template <typename Weight>
    const graph::DirectedWeightedGraph<double>& Router<Weight>::GetGraph() const 
    { 
        return graph_;
    }

    template <typename Weight>
    std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from, VertexId to) const 
    {
        const auto& route_internal_data = routes_internal_data_.at(from).at(to);
    
        if (!route_internal_data) 
        {
            return std::nullopt;
        }

        const Weight weight = route_internal_data->weight;
        std::vector<EdgeId> edges;

        for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge; edge_id;
            edge_id = routes_internal_data_[from][graph_.GetEdge(*edge_id).from]->prev_edge)
        {
            edges.push_back(*edge_id);
        }
        
        std::reverse(edges.begin(), edges.end());

        return RouteInfo{ weight, std::move(edges) };
    }
}  // end namespace graph