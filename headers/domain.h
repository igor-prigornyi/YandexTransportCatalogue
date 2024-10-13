#pragma once
#include <string>
#include <vector>
#include "geo.h"

// Пространство имён транспортного справочника
namespace transport_catalogue {

// Тип маршрута
enum class BusRouteType {
	Circle, // Кольцевой
	Line    // Линейный
};

// Структура остановки
struct Stop {
	std::string name;
	geo::Coordinate coordinate;
};

// Структура маршрута
struct Bus {
	std::string name;
	std::vector<Stop*> stops;
	BusRouteType type;
};

// Структура с информацией об остановке (её возвращает метод GetStopInfo)
struct StopInfo {
	std::string_view name;               // Название
	std::vector<std::string_view> buses; // Маршруты, проходящие через остановку
};

// Структура с информацией о маршруте (её возвращает метод GetBusInfo)
struct BusInfo {
	std::string_view name;      // Название
	size_t stops_number;        // Число остановок
	size_t unique_stops_number; // Число уникальных остановок
	double route_length;        // Длина маршрута
	double curvature;           // Извилистость
};

}