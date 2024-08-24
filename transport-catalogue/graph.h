#pragma once

#include <cstdlib>
#include <vector>

#include "ranges.h"

/*
    DirectedWeightedGraph — класс, реализующий взвешенный ориентированный граф,
    т.е. каждое его ребро обозначается числом. Это число — его вес.
    Память, нужная для хранения графа, линейна относительно суммы количеств вершин и рёбер.
*/

namespace graph 
{
    // Вершины и рёбра графа нумеруются автоинкрементно беззнаковыми целыми числами, хранящимися в типах VertexId и EdgeId: 
    // Вершины нумеруются от нуля до количества вершин минус один в соответствии с пользовательской логикой. 
    using VertexId = size_t;
    using EdgeId = size_t;

    template <typename Weight>
    struct Edge // "Набор" рёбер
    {
        std::string name; // Название остановки или номер автобуса
        size_t span_count; // Колличество перегонов между остановками
        VertexId from; // Вершина ребра "из"
        VertexId to; // Вершина ребра "до"
        Weight weight; // Вес ребра
    };

    template <typename Weight>
    class DirectedWeightedGraph 
    {
        private:

            using IncidenceList = std::vector<EdgeId>;
            using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

        public:
            // Конструктор и деструктор графа имеют линейную сложность, а остальные методы константны или амортизированно константны.
            DirectedWeightedGraph() = default;
            explicit DirectedWeightedGraph(size_t vertex_count);
            // Номер очередного ребра выдаётся методом AddEdge; 
            // Он равен нулю для первого вызова метода и при каждом следующем вызове увеличивается на единицу
            EdgeId AddEdge(const Edge<Weight>& edge);

            size_t GetVertexCount() const;
            size_t GetEdgeCount() const;
            const Edge<Weight>& GetEdge(EdgeId edge_id) const;
            IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

        private:

            std::vector<Edge<Weight>> edges_;
            std::vector<IncidenceList> incidence_lists_;
    };

    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
        : incidence_lists_(vertex_count) 
        {}

    template <typename Weight>
    EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) 
    {
        edges_.push_back(edge);
        const EdgeId id = edges_.size() - 1;
        incidence_lists_.at(edge.from).push_back(id);

        return id;
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetVertexCount() const 
    {
        return incidence_lists_.size();
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const 
    {
        return edges_.size();
    }

    template <typename Weight>
    const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const 
    {
        return edges_.at(edge_id);
    }

    template <typename Weight>
    typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
    DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const 
    {
        return ranges::AsRange(incidence_lists_.at(vertex));
    }
} // end namespace graph