//
// Created by James.Balajan on 6/04/2021.
//

#include <algorithm>
#include <fmt/format.h>

#include "vistree_generator.hpp"
#include "geom/angle_sorter/angle_sorter.hpp"
#include "constants/constants.hpp"


std::vector<Coordinate> VistreeGenerator::get_visible_vertices_from_root(const Coordinate &observer, const std::vector<Polygon>& polygons, bool half_scan) {
    const auto all_polygon_vertices_and_incident_segments = VistreeGenerator::all_vertices_and_incident_segments(
            polygons);
    if (all_polygon_vertices_and_incident_segments.empty()) {
        return {};
    }

    auto vertices_sorted_clockwise_around_observer = VistreeGenerator::all_vertices(
            all_polygon_vertices_and_incident_segments);
    AngleSorter::sort_counter_clockwise_around_observer(observer, vertices_sorted_clockwise_around_observer);

    auto open_edges = VistreeGenerator::OpenEdges();
    const auto initial_scanline_vector = Coordinate(MAX_LONGITUDE, observer.get_latitude());
    auto line_segments = VistreeGenerator::all_line_segments(all_polygon_vertices_and_incident_segments);
    for (const auto& line_segment: line_segments) {
        if (observer == line_segment.get_endpoint_1() || observer == line_segment.get_endpoint_2()) {
            continue;
        }

        const auto intersection = line_segment.intersection_with_segment(LineSegment(observer, initial_scanline_vector));
        if (intersection.has_value() && !(line_segment.get_endpoint_1() - observer).parallel(initial_scanline_vector) && !(line_segment.get_endpoint_2() - observer).parallel(initial_scanline_vector)) {
            open_edges.emplace((intersection.value() - observer).magnitude_squared(), std::make_unique<LineSegment>(line_segment));
        }
    }

    std::vector<Coordinate> visible_vertices;
    for (const auto& current_vertex: vertices_sorted_clockwise_around_observer) {
        const auto scanline_segment = LineSegment(observer, current_vertex);
        const auto& incident_segments = all_polygon_vertices_and_incident_segments.at(current_vertex);
        const auto clockwise_segments = VistreeGenerator::orientation_segments(incident_segments, scanline_segment, Orientation::CLOCKWISE);
        const auto counter_clockwise_segments = VistreeGenerator::orientation_segments(incident_segments, scanline_segment, Orientation::COUNTER_CLOCKWISE);
        const auto collinear_segments = VistreeGenerator::orientation_segments(incident_segments, scanline_segment, Orientation::COLLINEAR);

        VistreeGenerator::erase_segments_from_open_edges(clockwise_segments, open_edges);

        const auto curr_vertex_visible = VistreeGenerator::is_vertex_visible(open_edges, all_polygon_vertices_and_incident_segments, observer, current_vertex, prev_vertex, prev_vertex_visible);
        if (curr_vertex_visible) {
            visible_vertices.push_back(current_vertex);
        }

        VistreeGenerator::add_segments_to_open_edges(counter_clockwise_segments, open_edges, observer, current_vertex);

        prev_vertex = current_vertex;
        prev_vertex_visible = curr_vertex_visible;
    }

    return visible_vertices;
}

VistreeGenerator::VertexToSegmentMapping VistreeGenerator::all_vertices_and_incident_segments(const std::vector<Polygon>& polygons) {
    auto vertices_and_segments = VistreeGenerator::VertexToSegmentMapping();
    for (const auto& polygon : polygons) {
        const auto& polygon_vertices = polygon.get_vertices();
        const auto num_poly_vertices = static_cast<long>(polygon_vertices.size());

        vertices_and_segments.reserve(polygon_vertices.size());

        for (long i = 0; i < num_poly_vertices; ++i) {
            long prev_idx = (i-1) % num_poly_vertices;
            if (prev_idx < 0) prev_idx += num_poly_vertices;
            long next_idx = (i+1) % num_poly_vertices;

            const auto& curr_vertex = polygon_vertices[i];
            const auto& prev_vertex = polygon_vertices[prev_idx];
            const auto& next_vertex = polygon_vertices[next_idx];

            const auto incident_segments = std::vector<LineSegment> {
                LineSegment(prev_vertex, curr_vertex),
                LineSegment(curr_vertex, next_vertex),
            };
            vertices_and_segments[curr_vertex] = incident_segments;
        }
    }

    return vertices_and_segments;
}

std::vector<LineSegment>
VistreeGenerator::all_line_segments(const VertexToSegmentMapping &vertices_and_segments) {
    std::vector<LineSegment> segments;

    for (const auto& vertex_and_segments : vertices_and_segments) {
        segments.reserve(vertex_and_segments.second.size());
        segments.insert(segments.end(), vertex_and_segments.second.begin(), vertex_and_segments.second.end());
    }

    return segments;
}

std::vector<Coordinate>
VistreeGenerator::all_vertices(const VertexToSegmentMapping &vertices_and_segments) {
    std::vector<Coordinate> vertices;
    vertices.reserve(vertices_and_segments.size());

    for (const auto& vertex_and_segments : vertices_and_segments) {
        vertices.push_back(vertex_and_segments.first);
    }

    return vertices;
}

std::vector<LineSegment>
VistreeGenerator::orientation_segments(const std::vector<LineSegment> &segments, const LineSegment& scanline_segment, const Orientation& desired_orientation) {
    auto orientation_segs = std::vector<LineSegment>();

    for (const auto& segment: segments) {
        const auto segment_orientation = scanline_segment.orientation_of_point_to_segment(segment.get_adjacent_to(scanline_segment.get_endpoint_2()));
        if (segment_orientation == desired_orientation) {
            orientation_segs.push_back(segment);
        }
    }

    return orientation_segs;
}

bool VistreeGenerator::is_vertex_visible(const VistreeGenerator::OpenEdges &open_edges,
                                         const VistreeGenerator::VertexToSegmentMapping& vertices_and_segments,
                                         const Coordinate &observer_coordinate,
                                         const Coordinate &vertex_in_question) {
    if (vertex_in_question == observer_coordinate) {
        return false;
    } if (vertices_and_segments.find(observer_coordinate) != vertices_and_segments.end()) {
        // We perform this check to stop our observer coordinate (if it is a vertex of a polygon)
        // from seeing vertices from inside the polygon
        // Essentially we build artificial walls based on the edges we know obstruct vision (those adjacent)
        // This will be a constant time operation as each vertex only has two adjacent edges

        bool is_counter_clockwise_all_segments = true;
        for (const auto& segment : vertices_and_segments.at(observer_coordinate)) {
            is_counter_clockwise_all_segments = is_counter_clockwise_all_segments &&
                    (segment.orientation_of_point_to_segment(vertex_in_question) == Orientation::COUNTER_CLOCKWISE);
        }

        if (is_counter_clockwise_all_segments) {
            return false;
        }
    }

    if (!open_edges.empty()) {
        const auto& closest_edge = open_edges.begin()->second;
        const auto intersection = closest_edge->intersection_with_segment(LineSegment(observer_coordinate, vertex_in_question));
        if (intersection.has_value()) {
            return false;
        }
    }

    return true;
}

void VistreeGenerator::erase_segments_from_open_edges(const std::vector<LineSegment> &segments,
                                                      VistreeGenerator::OpenEdges &open_edges) {
    for (const auto& segment : segments) {
        for (auto iter = open_edges.begin(); iter != open_edges.end(); ++iter) {
            if (*(iter->second) == segment) {
                open_edges.erase(iter);
                break;
            }
        }
    }
}

void VistreeGenerator::add_segments_to_open_edges(const std::vector<LineSegment> &segments,
                                                  VistreeGenerator::OpenEdges &open_edges,
                                                  const Coordinate& observer,
                                                  const Coordinate& current_vertex) {
    for (const auto& segment : segments) {
        const auto intersection = segment.intersection_with_segment(LineSegment(observer, current_vertex));
        const auto distance_squared = (intersection.value() - observer).magnitude_squared();

        open_edges.emplace(distance_squared, std::make_unique<LineSegment>(segment));
    }
}