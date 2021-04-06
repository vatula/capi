//
// Created by James.Balajan on 31/03/2021.
//

#include <catch.hpp>

#include "types/coordinate/coordinate.hpp"

TEST_CASE("Coordinate Constructor") {
    const auto coord = Coordinate(1, 2);
    REQUIRE(coord.get_longitude() == 1);
    REQUIRE(coord.get_latitude() == 2);
}

TEST_CASE("Coordinate Equals") {
    const auto coord1 = Coordinate(1, 2);
    const auto coord2 = Coordinate(1, 2);
    const auto coord3 = Coordinate(2, 1);

    REQUIRE(coord1 == coord2);
    REQUIRE_FALSE(coord1 != coord2);

    REQUIRE_FALSE(coord1 == coord3);
    REQUIRE(coord1 != coord3);
}

TEST_CASE("Coordinate +") {
    const auto coord1 = Coordinate(1, 2);
    const auto coord2 = Coordinate(3, 4);

    REQUIRE(coord1 + coord2 == Coordinate(4, 6));
}

TEST_CASE("Coordinate unary -") {
    const auto coord = Coordinate(1, 2);

    REQUIRE(-coord == Coordinate(-1, -2));
}

TEST_CASE("Coordinate binary -") {
    const auto coord1 = Coordinate(1, 2);
    const auto coord2 = Coordinate(3, 4);

    REQUIRE(coord1 - coord2 == Coordinate(-2, -2));
}

TEST_CASE("Coordinate dot product") {
    const auto coord1 = Coordinate(1, 2);
    const auto coord2 = Coordinate(3, 4);

    REQUIRE(coord1.dot_product(coord2) == coord2.dot_product(coord1));
    REQUIRE(coord1.dot_product(coord2) == 11);
}

TEST_CASE("Coordinate magnitude of cross product") {
    const auto coord1 = Coordinate(1, 2);
    const auto coord2 = Coordinate(3, 4);

    REQUIRE(coord1.cross_product_magnitude(coord2) == -2);
}

TEST_CASE("Coordinate Magnitude squared") {
    const auto coord = Coordinate(1, 2);

    REQUIRE(coord.magnitude_squared() == 5);
}

TEST_CASE("Coordinate Multiply by scalar") {
    const auto coord = Coordinate(1, 2);

    REQUIRE((coord * 2) == Coordinate(2, 4));
}

TEST_CASE("Coordinate vector orientation") {
    const auto base = Coordinate(1, 1);
    const auto counter_clockwise = Coordinate(1, 2);
    const auto clockwise = Coordinate(2, 1);
    const auto collinear = Coordinate(2, 2);

    REQUIRE(base.vector_orientation(counter_clockwise) == Orientation::COUNTER_CLOCKWISE);
    REQUIRE(base.vector_orientation(clockwise) == Orientation::CLOCKWISE);
    REQUIRE(base.vector_orientation(collinear) == Orientation::COLLINEAR);
}

TEST_CASE("Coordinate hash") {
    const auto coord1 = Coordinate(1, 2);
    const auto coord2 = Coordinate(1.00001, 2.00001);
    const auto coord3 = Coordinate(1, 2);

    REQUIRE(std::hash<Coordinate>()(coord1) == std::hash<Coordinate>()(coord3));
    REQUIRE(std::hash<Coordinate>()(coord1) != std::hash<Coordinate>()(coord2));
}
