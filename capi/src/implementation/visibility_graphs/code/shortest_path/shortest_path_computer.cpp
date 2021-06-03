//
// Created by James.Balajan on 3/06/2021.
//

//#include <pybind11/pybind11.h>
#include <queue>
#include <algorithm>

#include "constants/constants.hpp"
#include "shortest_path_computer.hpp"
#include "visgraph/vistree_generator.hpp"
#include "coordinate_periodicity/coordinate_periodicity.hpp"

struct AStarHeapElement {
    Coordinate node;
    double distance_to_source;
    double heuristic_distance_to_destination;
};

ShortestPathComputer::ShortestPathComputer(const Graph &graph): _graph(graph), _periodic_polygons(make_polygons_periodic(graph.get_polygons())) { }

std::vector<Coordinate> ShortestPathComputer::shortest_path(const Coordinate &source, const Coordinate &destination) const {
    auto modified_graph = Graph(_graph);

    const auto found_source = modified_graph.has_vertex(source);
    if (!found_source) {
        modified_graph.add_vertex(source);
        for (const auto &visible_vertex :
            VistreeGenerator::get_visible_vertices_from_root(source, _periodic_polygons, false)) {
            modified_graph.add_edge(source, visible_vertex.coord, visible_vertex.is_visible_across_meridian);
        }
    }

    const auto found_destination = modified_graph.has_vertex(destination);
    if (!found_destination) {
        modified_graph.add_vertex(destination);
        for (const auto &visible_vertex :
            VistreeGenerator::get_visible_vertices_from_root(destination, _periodic_polygons, false)) {
            modified_graph.add_edge(destination, visible_vertex.coord, visible_vertex.is_visible_across_meridian);
        }
    }

    const auto comparison_func = [&](const AStarHeapElement &a, const AStarHeapElement &b) {
      return (a.distance_to_source + a.heuristic_distance_to_destination*ShortestPathComputer::GREEDINESS_WEIGHTING) >
             (b.distance_to_source + b.heuristic_distance_to_destination*ShortestPathComputer::GREEDINESS_WEIGHTING);
    };

    auto pq = std::priority_queue<AStarHeapElement, std::vector<AStarHeapElement>, decltype(comparison_func)>(
        comparison_func);
    pq.push(AStarHeapElement{
        .node = source,
        .distance_to_source = 0,
        .heuristic_distance_to_destination = heuristic_distance_measurement(source, destination),
    });

    size_t num_nodes_traversed = 0; // Debugging
    auto prev_coord = std::unordered_map<Coordinate, Coordinate>();
    auto distances_to_source = std::unordered_map<Coordinate, double>();
    while (!pq.empty()) {
        const auto top = pq.top();
        pq.pop();

        ++num_nodes_traversed;
        if (top.node == destination) {
            break;
        }

        for (const auto &neighbor : modified_graph.get_neighbors(top.node)) {
            const auto meridian_spanning = modified_graph.is_edge_meridian_crossing(top.node, neighbor);

            const auto neighbor_dist_to_source =
                top.distance_to_source + distance_measurement(neighbor, top.node, meridian_spanning);

            if (distances_to_source.find(neighbor) != distances_to_source.end() &&
                distances_to_source.at(neighbor) <= neighbor_dist_to_source) {
                continue;
            }

            const auto heap_elem = AStarHeapElement{
                .node = neighbor,
                .distance_to_source = neighbor_dist_to_source,
                .heuristic_distance_to_destination = heuristic_distance_measurement(neighbor, destination),
            };
            pq.push(heap_elem);
            prev_coord[neighbor] = top.node;
            distances_to_source[neighbor] = neighbor_dist_to_source;
        }
    }

    //pybind11::print("Num nodes traversed:", num_nodes_traversed);

    auto path = std::vector<Coordinate>();
    auto curr_node = destination;

    while (curr_node != source) {
        path.push_back(curr_node);

        if (prev_coord.find(curr_node) == prev_coord.end()) {
            throw std::runtime_error("Could not find a shortest path");
        }
        curr_node = prev_coord.at(curr_node);
    }
    path.push_back(source);

    std::reverse(path.begin(), path.end());

    return path;
}

double ShortestPathComputer::distance_measurement(const Coordinate &a, const Coordinate &b, bool is_meridian_spanning) {
    if (!is_meridian_spanning) {
        return (a - b).magnitude();
    } else {
        const auto periodic_coordinates = periodic_coordinates_from_coordinate(a);
        return std::min((periodic_coordinates[1] - b).magnitude(), (periodic_coordinates[2] - b).magnitude());
    }
}

double ShortestPathComputer::heuristic_distance_measurement(const Coordinate &a, const Coordinate &b) {
    return std::min(distance_measurement(a, b, false),
                    distance_measurement(a, b, true));
}
