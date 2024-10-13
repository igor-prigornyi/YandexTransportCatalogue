#include <vector>
#include <string>
#include <sstream>
#include "json_reader.h"
#include "json.h"
#include "map_renderer.h"

using namespace std;
using namespace json;

// Пространство имён транспортного справочника
namespace transport_catalogue {

// Пространство имён для функционала, связанного с запросами к транспортному справочнику в формате JSON
namespace json_reader {

// Пространство имён для структур и функций, использующихся только для внутренней работы transport_catalogue::json_reader
namespace detail {

// Функция парсинга запроса на добавление остановки
request_handler::AddStopRequest ParseAddStopRequest(const Dict& request) {
	string_view  name      = request.at("name"s).AsString();
	const double latitude  = request.at("latitude"s).AsDouble();
	const double longitude = request.at("longitude"s).AsDouble();

	const auto& distances_map = request.at("road_distances"s).AsMap();

	vector<pair<string_view, int>> distances;

	for (const auto& [stop_to, distance] : distances_map) {
		distances.push_back({ stop_to, distance.AsInt() });
	}

	return { name, {latitude, longitude},  distances };
}
			
// Функция парсинга запроса на добавление маршрута
request_handler::AddBusRequest ParseAddBusRequest(const Dict& request) {
	string_view        name = request.at("name"s).AsString();
	const BusRouteType type = request.at("is_roundtrip"s).AsBool() ? BusRouteType::Circle : BusRouteType::Line;

	const auto& stops_arr = request.at("stops"s).AsArray();

	vector<string_view> stops;

	for (const auto& stop : stops_arr) {
		stops.push_back(stop.AsString());
	}

	return { name, type, stops };
}

// Функция парсинга цвета в формате строки/RGB/RGBa
map_renderer::Color ParseColor(const Node& color) {

	if(color.IsString()) {
		return color.AsString();
	}
	else if(color.IsArray()) {
		const auto& color_array = color.AsArray();

		if (color_array.size() == 3) {
			return map_renderer::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
		}
		else if(color_array.size() == 4) {
			return map_renderer::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
		}
		else {
			throw ParsingError("Color parsing error: color in the array format has a size not 3 (RGB) and not 4 (RGBa)"s);
		}

	}
	else {
		throw ParsingError("Color parsing error: color is not in string or array format"s);
	}
}

// Функция парсинга настроек отрисовки карты маршрутов
map_renderer::RenderSettings ParseRenderSettings(const Dict& render_settings) {

	map_renderer::RenderSettings settings;

	settings.width  = render_settings.at("width"s).AsDouble();
	settings.height = render_settings.at("height"s).AsDouble();
	
	settings.padding = render_settings.at("padding"s).AsDouble();
	
	settings.line_width  = render_settings.at("line_width"s).AsDouble();
	settings.stop_radius = render_settings.at("stop_radius"s).AsDouble();

	settings.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();
	
	const auto& bus_label_offset_array = render_settings.at("bus_label_offset"s).AsArray();
	settings.bus_label_offset = {bus_label_offset_array[0].AsDouble(),
	                             bus_label_offset_array[1].AsDouble()};

	settings.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();

	const auto& stop_label_offset_array = render_settings.at("stop_label_offset"s).AsArray();
	settings.stop_label_offset = {stop_label_offset_array[0].AsDouble(),
	                              stop_label_offset_array[1].AsDouble()};

	settings.underlayer_color = ParseColor(render_settings.at("underlayer_color"s));

	settings.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();

	for(const auto& color : render_settings.at("color_palette"s).AsArray()) {
		settings.color_palette.push_back(ParseColor(color));
	}

	return settings;
}

// Функция обработки запросов на заполнение базы данных
void BaseRequestProcessing(request_handler::RequestHandler& request_handler, const Array& base_requests) {
	using namespace request_handler;

	vector<AddStopRequest> add_stop_requests; // Запросы на добавление остановок
	vector<AddBusRequest>  add_bus_requests;  // Запросы на добавление маршрутов

	// Парсинг запросов на заполнение базы данных
	for (const auto& base_request : base_requests) {
		const auto& request = base_request.AsMap();

		// Запрос на добавление остановки
		if (request.at("type"s).AsString() == "Stop"s) {
			add_stop_requests.push_back(ParseAddStopRequest(request));
		}
		// Запрос на добавление маршрута
		else if (request.at("type"s).AsString() == "Bus"s) {
			add_bus_requests.push_back(ParseAddBusRequest(request));
		}
		// Неизвестный тип запроса на заполнение базы данных
		else {
			throw UnknownRequestType("Unknown request type \""s + request.at("type"s).AsString() + "\""s);
		}
	}

	// Обработка запросов на заполнение базы данных
	request_handler.SetData(add_stop_requests, add_bus_requests);
}

// Функция парсинга запроса на получение информации об остановке
Dict ParseGetStopInfoRequest(request_handler::RequestHandler& request_handler, const Dict& request) {
	const int   id   = request.at("id"s).AsInt();
	string_view name = request.at("name"s).AsString();

	const auto stop_info = request_handler.GetStopInfo(name);

	if (stop_info) {
		Array busses_arr;

		for (string_view bus : stop_info->buses) {
			busses_arr.push_back(Node(string(bus)));
		}
		return Dict{ { "request_id"s, id }, { "buses"s, busses_arr} };
	}
	else {
		return Dict{ { "request_id"s, id }, { "error_message"s, "not found"s} };
	}
}

// Функция парсинга запроса на получение информации о маршруте
Dict ParseGetBusInfoRequest(request_handler::RequestHandler& request_handler, const Dict& request) {
	const int   id   = request.at("id"s).AsInt();
	string_view name = request.at("name"s).AsString();

	const auto bus_info = request_handler.GetBusInfo(name);

	if (bus_info) {
		return Dict{ { "request_id"s,        id },
				     { "stop_count"s,        static_cast<int>(bus_info->stops_number) },
					 { "unique_stop_count"s, static_cast<int>(bus_info->unique_stops_number) },
					 { "route_length"s,      bus_info->route_length},
					 { "curvature"s,         bus_info->curvature} };
	}
	else {
		return Dict{ { "request_id"s, id }, { "error_message"s, "not found"s} };
	}
}

// Функция парсинга запроса на получение карты маршрутов
Dict ParseGetRouteMapRequest(request_handler::RequestHandler& request_handler, const Dict& request, const Dict& render_settings) {

	const int id = request.at("id"s).AsInt();

	stringstream route_map;

	const auto settings = ParseRenderSettings(render_settings);
	request_handler.RenderMap(settings, route_map);

	return Dict{ { "request_id"s,  id },
	             { "map"s, route_map.str() }};
}

// Функция обработки запросов к транспортному справочнику
Document StatRequestProcessing(request_handler::RequestHandler& request_handler, const Array& stat_requests, const Dict& render_settings) {
	Array response_array;

	// Обработка запросов к транспортному справочнику
	for (const auto& stat_request : stat_requests) {
		const auto& request = stat_request.AsMap();

		// Запрос на получение информации об остановке
		if (request.at("type"s).AsString() == "Stop"s) {
			response_array.push_back(Node(ParseGetStopInfoRequest(request_handler, request)));
		}
		// Запрос на получение информации о маршруте
		else if (request.at("type"s).AsString() == "Bus"s) {
			response_array.push_back(Node(ParseGetBusInfoRequest(request_handler, request)));
		}
		// Запрос на получение карты маршрутов
		else if (request.at("type"s).AsString() == "Map"s) {
			response_array.push_back(Node(ParseGetRouteMapRequest(request_handler, request, render_settings)));
		}
		// Неизвестный тип запроса к транспортному справочнику
		else {
			throw UnknownRequestType("Unknown request type \""s + request.at("type"s).AsString() + "\""s);
		}
	}

	// Запаковываем response_array в документ и возвращаем его
	return Document(response_array);
}

}

// Функция обработки запросов к транспортному справочнику в формате JSON
void RequestProcessing(request_handler::RequestHandler& request_handler, istream& input, ostream& output) {
	using namespace detail;
	using namespace request_handler;

	const Document requests(input);
	const auto& base_requests   = requests.GetRoot().AsMap().at("base_requests"s).AsArray();
	const auto& stat_requests   = requests.GetRoot().AsMap().at("stat_requests"s).AsArray();
	const auto& render_settings = requests.GetRoot().AsMap().at("render_settings"s).AsMap();

	BaseRequestProcessing(request_handler, base_requests);

	const Document stat_responses = StatRequestProcessing(request_handler, stat_requests, render_settings);
	stat_responses.Print(output);
}

}

}