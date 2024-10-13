#pragma once
#include <string>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_map>
#include <optional>

#include "geo.h"
#include "domain.h"

// Пространство имён транспортного справочника
namespace transport_catalogue {

// Пространство имён для структур и функций, использующихся только для внутренней работы transport_catalogue
namespace detail {

// Хешер для ключа pair<const Stop*, const Stop*> в unordered_map
struct PairStopStopHasher {
	size_t operator() (const std::pair<const Stop*, const Stop*>& stops) const;
};

}

// Класс транспортного справочника
class TransportCatalogue {
public:
	// Функция добавления остановки в базу данных
	void AddStop(std::string_view name, const geo::Coordinate& coordinate);

	// Функция добавления маршрута в базу данных
	void AddBus(std::string_view name, BusRouteType type, const std::vector<std::string_view>& stops);

	// Функция добавления расстояния от остановки с именем stop_from до остановки с именем stop_to
	void SetDistance(std::string_view stop_from, std::string_view stop_to, int distance);

	// Функция получения константной ссылки на контейнер остановок (нужна для модуля map_renderer)
	const std::deque<Stop>& GetStops() const;

	// Функция получения словаря "Имя остановки" -> "Константный указатель на остановку в базе данных" (нужна для модуля map_renderer)
	const std::map<std::string_view, const Stop*> GetStopnameToStopMap() const;

	// Функция получения словаря "Имя маршрута" -> "Константный указатель на маршрут в базе данных" (нужна для модуля map_renderer)
	const std::map<std::string_view, const Bus*> GetBusnameToBusMap() const;

	// Функция наличия маршрутов на остановке
	bool IfBusesOnStop(std:: string_view name) const;
	
	// Функция получения информации об остановке
	std::optional<StopInfo> GetStopInfo(std::string_view name) const;

	// Функция получения информации о маршруте
	std::optional<BusInfo> GetBusInfo(std::string_view name) const;

private:
	std::deque<Stop> stops_; // Остановки
	std::deque<Bus>  buses_; // Маршруты

	std::map<std::string_view, Stop*> stopname_to_stop_; // Словарь "Имя остановки" -> "Указатель на остановку в базе данных"
	std::map<std::string_view, Bus*>  busname_to_bus_;   // Словарь "Имя маршрута"  -> "Указатель на маршрут в базе данных"

	std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::PairStopStopHasher> distances_; // Расстояния между остановками

	std::unordered_map<const Stop*, std::set<std::string_view>> buses_on_stop_; // Маршруты, проходящие через остановку (названия, упорядоченные по алфавиту)
};
}