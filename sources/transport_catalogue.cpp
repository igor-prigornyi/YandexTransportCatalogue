#include <stdexcept>
#include "transport_catalogue.h"
using namespace std;

// Пространство имён транспортного справочника
namespace transport_catalogue {

// Пространство имён для структур и функций, использующихся только для внутренней работы класса transport_catalogue
namespace detail {

// Хешер для ключа pair<const Stop*, const Stop*> в unordered_map
size_t PairStopStopHasher::operator() (const pair<const Stop*, const Stop*>& stops) const {
	constexpr size_t simp_mult = 37u;

	// size_t first_hash  = hash<Stop*>{}(stops.first);
	// size_t second_hash = hash<Stop*>{}(stops.second);

	size_t first_hash  = reinterpret_cast<size_t>(stops.first);
	size_t second_hash = reinterpret_cast<size_t>(stops.second);

	return first_hash + simp_mult * second_hash;
}

}

// Функция добавления остановки в базу данных
void TransportCatalogue::AddStop(string_view name, const geo::Coordinate& coordinate) {
	using namespace detail;

	stops_.push_back({ string(name), coordinate });
	stopname_to_stop_[stops_.back().name] = &stops_.back();
}

// Функция добавления маршрута в базу данных
void TransportCatalogue::AddBus(string_view name, BusRouteType type, const vector<string_view>& stops) {
	using namespace detail;

	vector<Stop*> stops_ptrs;

	for (string_view stop : stops) {
		stops_ptrs.push_back(stopname_to_stop_[stop]);
	}

	buses_.push_back({ string(name), stops_ptrs, type });
	busname_to_bus_[buses_.back().name] = &buses_.back();

	for (string_view stop : stops) {
		buses_on_stop_[stopname_to_stop_[stop]].insert(buses_.back().name);
	}
}

// Функция добавления расстояния от остановки с именем stop_from до остановки с именем stop_to
void TransportCatalogue::SetDistance(std::string_view stop_from, std::string_view stop_to, int distance) {
	using namespace detail;

	distances_[{stopname_to_stop_[stop_from], stopname_to_stop_[stop_to]}] = distance;
}

// Функция получения константной ссылки на контейнер остановок (нужна для модуля map_renderer)
const deque<Stop>& TransportCatalogue::GetStops() const {
	return stops_;
}

// Функция получения словаря "Имя остановки" -> "Константный указатель на остановку в базе данных" (нужна для модуля map_renderer)
const map<string_view, const Stop*> TransportCatalogue::GetStopnameToStopMap() const {
	map<string_view, const Stop*> result;

	for(const auto& [stop_name, stop_ptr] : stopname_to_stop_) {
		result[stop_name] = stop_ptr;
	}

	return result;
}

// Функция получения словаря "Имя маршрута" -> "Константный указатель на маршрут в базе данных" (нужна для модуля map_renderer)
const map<string_view, const Bus*> TransportCatalogue::GetBusnameToBusMap() const {
	map<string_view, const Bus*> result;

	for(const auto& [bus_name, bus_ptr] : busname_to_bus_) {
		result[bus_name] = bus_ptr;
	}

	return result;
}

// Функция наличия маршрутов на остановке
bool TransportCatalogue::IfBusesOnStop(string_view name) const {
	return !buses_on_stop_.at(stopname_to_stop_.at(name)).empty();
}

// Функция получения информации об остановке
optional<StopInfo> TransportCatalogue::GetStopInfo(string_view name) const {
	using namespace detail;

	// Если остановки нету в базе данных
	if(!stopname_to_stop_.count(name)) {
		return nullopt;
	}

	// Указатель на остановку в базе данных
	const Stop* stop_ptr = stopname_to_stop_.at(name);

	// Если через остановку не проходит ни один маршрут
	if (!buses_on_stop_.count(stop_ptr)) {
		return StopInfo{ stop_ptr->name, vector<string_view>{} };
	}

	const auto& buses_on_stop_set = buses_on_stop_.at(stop_ptr);

	// Формирование ответа на запрос
	return StopInfo{ stop_ptr->name, vector<string_view>(buses_on_stop_set.begin(), buses_on_stop_set.end()) };
}

// Функция получения информации о маршруте
optional<BusInfo> TransportCatalogue::GetBusInfo(string_view name) const {
	using namespace detail;

	// Если маршрут не найден
	if (!busname_to_bus_.count(name)) {
		return nullopt;
	}

	// Ссылка на маршрут в базе данных
	const Bus& bus_ref = *busname_to_bus_.at(name);

	// Вычисление географической и фактической длины маршрута
	double length_geographic = 0;
	double length_actual = 0;

	for (size_t n = 0; n + 1 < bus_ref.stops.size(); ++n) {
		length_geographic += ComputeDistance(bus_ref.stops[n]->coordinate, bus_ref.stops[n + 1]->coordinate);

		if (distances_.count({ bus_ref.stops[n],  bus_ref.stops[n + 1] })) {
			length_actual += distances_.at({ bus_ref.stops[n], bus_ref.stops[n + 1] });
		}
		else {
			length_actual += distances_.at({ bus_ref.stops[n + 1], bus_ref.stops[n] });
		}
	}

	// Если маршрут линейный, нужно посчитать и обратный путь
	if(bus_ref.type == BusRouteType::Line) {
		length_geographic *= 2;
		
		for (size_t n = 0; n + 1 < bus_ref.stops.size(); ++n) {

			if (distances_.count({ bus_ref.stops[n + 1],  bus_ref.stops[n] })) {
				length_actual += distances_.at({ bus_ref.stops[n + 1], bus_ref.stops[n] });
			}
			else {
				length_actual += distances_.at({ bus_ref.stops[n], bus_ref.stops[n + 1] });
			}
		}
	}

	// Вычисление извилистости маршрута
	const double curvature = length_actual / length_geographic;

	// Вычисление количества остановок и уникальных остановок маршрута
	set<string_view> unique_stops;

	for (const Stop* stop : bus_ref.stops) {
		unique_stops.insert(stop->name);
	}

	const size_t stops_num = (bus_ref.type == BusRouteType::Line && bus_ref.stops.size() != 0) ? 2 * bus_ref.stops.size() - 1 : bus_ref.stops.size();
	const size_t unique_stops_num = unique_stops.size();

	// Формирование ответа на запрос
	return BusInfo{ bus_ref.name, stops_num, unique_stops_num, length_actual, curvature };
}

}