//
// Created by James.Balajan on 31/03/2021.
//

#include <cmath>
#include <stdexcept>
#include <fmt/format.h>

#include "coordinate.hpp"
#include "constants/constants.hpp"

double round_to_epsilon_tolerance(double val);

Coordinate::Coordinate(double longitude, double latitude) {
    _longitude = longitude;
    _latitude = latitude;
}

double Coordinate::get_latitude() const { return _latitude; }

double Coordinate::get_longitude() const { return _longitude; }

bool Coordinate::operator==(const Coordinate &other) const {
    return (other - *this).magnitude_squared() < (EPSILON_TOLERANCE * EPSILON_TOLERANCE);
}

bool Coordinate::operator!=(const Coordinate &other) const { return !(*this == other); }

Coordinate Coordinate::operator+(const Coordinate &other) const {
    return {_longitude + other._longitude, _latitude + other._latitude};
}

Coordinate Coordinate::operator-() const {
    return {-_longitude, -_latitude};
}

Coordinate Coordinate::operator-(const Coordinate &other) const {
    return (*this) + (-other);
}

Coordinate Coordinate::operator*(double scalar) const {
    return {_longitude * scalar, _latitude * scalar};
}

Coordinate Coordinate::operator/(double scalar) const {
    return (*this) * (1 / scalar);
}

double Coordinate::dot_product(const Coordinate &other) const {
    return _longitude * other._longitude + _latitude * other._latitude;
}

double Coordinate::cross_product_magnitude(const Coordinate &other) const {
    return (_longitude * other._latitude) - (other._longitude * _latitude);
}

double Coordinate::magnitude_squared() const {
    return (_longitude * _longitude) + (_latitude * _latitude);
}

double Coordinate::magnitude() const {
    return std::sqrt(magnitude_squared());
}

Orientation Coordinate::vector_orientation(const Coordinate &v2) const {
    const auto cross_prod = this->cross_product_magnitude(v2);

    if (cross_prod < -(EPSILON_TOLERANCE_SQUARED)) {
        return Orientation::CLOCKWISE;
    } else if (cross_prod > (EPSILON_TOLERANCE_SQUARED)) {
        return Orientation::COUNTER_CLOCKWISE;
    } else {
        return Orientation::COLLINEAR;
    }
}

bool Coordinate::parallel(const Coordinate& other) const {
    return this->vector_orientation(other) == Orientation::COLLINEAR;
}

std::optional<double> Coordinate::scalar_multiple_factor(const Coordinate& other) const {
    if (!this->parallel(other)) {
        return {};
    }

    if (_longitude == 0 && _latitude == 0) {
        return 0;
    } else if (_longitude == 0) {
        return other._latitude / _latitude;
    } else {
        return other._longitude / _longitude;
    }
}

Coordinate Coordinate::unit_vector() const {
    return (*this) / magnitude();
}

std::size_t std::hash<Coordinate>::operator()(const Coordinate &coord) const {
    const std::size_t long_hash = hash<double>()(round_to_epsilon_tolerance(coord.get_longitude()));
    const std::size_t lat_hash = hash<double>()(round_to_epsilon_tolerance(coord.get_latitude()));
    return lat_hash ^ (long_hash + 0x9e3779b9 + (lat_hash << 6) + (lat_hash >> 2));
}

double round_to_epsilon_tolerance(double val) {
    return (std::floor(val / EPSILON_TOLERANCE) * EPSILON_TOLERANCE) + 0.5;
}

std::ostream& operator<<(std::ostream& outs, const Coordinate& coord) {
    return outs << fmt::format("Coordinate ({}, {})", coord.get_longitude(), coord.get_latitude());
}
