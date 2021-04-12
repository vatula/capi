//
// Created by James.Balajan on 31/03/2021.
//

#include "vistree_generator.hpp"
#include "visgraph_generator.hpp"

VisgraphGenerator::VisgraphGenerator() {}

Graph VisgraphGenerator::generate(const std::vector<Polygon> &polygons) {
    const auto polygon_vertices = VisgraphGenerator::polygon_vertices(polygons);
    auto visgraph = Graph(polygon_vertices.size());

#pragma omp parallel for shared(visgraph, polygons) default(none)
    for (size_t i = 0; i < polygon_vertices.size(); ++i) { // NOLINT
        const auto visible_vertices = VistreeGenerator::get_visible_vertices_from_root(polygon_vertices[i], polygons, true);

        for (const auto& visible_vertex: visible_vertices) {
            visgraph.add_edge(polygon_vertices[i], visible_vertex);
        }
    }

    return visgraph;
}

std::vector<Coordinate> VisgraphGenerator::polygon_vertices(const std::vector<Polygon> &polygons) {
    auto vertices = std::vector<Coordinate>();

    for (const auto& polygon : polygons) {
        vertices.reserve(polygon.get_vertices().size());
        vertices.insert(vertices.end(), polygon.get_vertices().begin(), polygon.get_vertices().end());
    }

    return vertices;
}
