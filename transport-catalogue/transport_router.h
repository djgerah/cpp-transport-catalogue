#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>
 
namespace tc 
{
	struct RoutingSettings
	{
		int bus_wait_time_ = 0;
		double bus_velocity_ = 0.0;
	};

	class TransportRouter 
	{
		public:
		
			TransportRouter(const RoutingSettings& routing_settings, const TransportCatalogue& catalogue) 
				: routing_settings_ (routing_settings)
				{
					graph_ = std::move(BuildGraph(catalogue));
       				router_ = std::make_unique<graph::Router<double>>(graph_);
				}

			const std::optional<graph::Router<double>::RouteInfo> GetRoute(const tc::Stop* stop_from, const tc::Stop* stop_to) const;
			const graph::DirectedWeightedGraph<double>& GetGraph() const;

		private:

			graph::DirectedWeightedGraph<double> AddStopEdge(const TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double> graph_to_build);
			graph::DirectedWeightedGraph<double> AddBusEdge(const TransportCatalogue& catalogue, graph::DirectedWeightedGraph<double> graph_to_build);
			graph::DirectedWeightedGraph<double> BuildGraph(const TransportCatalogue& catalogue);

			graph::DirectedWeightedGraph<double> graph_;
			std::unique_ptr<graph::Router<double>> router_;
			std::map<const tc::Stop*, graph::VertexId> stop_to_vertex_id_ = {};
			RoutingSettings routing_settings_;
	};
} // end namespace tc