#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include "transport_catalogue.h"
#include "map_renderer.h"

// Пространство имён транспортного справочника
namespace transport_catalogue {

// Пространство имён для функционала, связанного с запросами к транспортному справочнику
namespace request_handler {

// Структура запроса на добавление остановки
struct AddStopRequest {
	std::string_view name;                                   // Название остановки
	geo::Coordinate  coordinate;                               // Координаты остановки
	std::vector<std::pair<std::string_view, int>> distances; // Расстояния до соседних остановок
};

// Структура запроса на добавление маршрута
struct AddBusRequest {
	std::string_view name;               // Название маршрута
	BusRouteType     type;               // Тип маршрута (кольцевой или линейный)
	std::vector<std::string_view> stops; // Остановки на маршруте
};

// Класс обработчика запросов к транспортному справочнику
class RequestHandler {
public:
    RequestHandler(TransportCatalogue& catalogue,
                   map_renderer::MapRenderer& renderer);

    // Функция задания данных транспортного справочника
    void SetData(const std::vector<AddStopRequest>& add_stop_requests,
                 const std::vector<AddBusRequest>&  add_bus_requests);

    // Функция получения информации об остановке
    std::optional<StopInfo> GetStopInfo(std::string_view name) const;

    // Функция получения информации о маршруте
    std::optional<BusInfo>  GetBusInfo (std::string_view name) const;

    // Функция отрисовки карты маршрутов
    void RenderMap(const map_renderer::RenderSettings& settings, std::ostream& output = std::cout) const;

private:
    TransportCatalogue& catalogue_;
    map_renderer::MapRenderer& renderer_;
};

}

}