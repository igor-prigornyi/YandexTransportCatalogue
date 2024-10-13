#pragma once

// Пространство имён для географических данных и функций
namespace geo {

// Структура геограчифеских координат
struct Coordinate {
    double lat; // Широта
    double lng; // Долгота
};

bool operator == (const Coordinate& lhs, const Coordinate& rhs);

bool operator != (const Coordinate& lhs, const Coordinate& rhs);

// Функция вычисления расстояния между координатами
double ComputeDistance(const Coordinate& from, const Coordinate& to);

}