#include "request_handler.h"
using namespace std;

// Пространство имён транспортного справочника
namespace transport_catalogue {

// Пространство имён для функционала, связанного с запросами к транспортному справочнику
namespace request_handler {

RequestHandler::RequestHandler(TransportCatalogue& catalogue,
                               map_renderer::MapRenderer& renderer) : catalogue_(catalogue),
							                                          renderer_(renderer) { }

// Функция задания данных транспортного справочника
void RequestHandler::SetData(const vector<AddStopRequest>& add_stop_requests,
                             const vector<AddBusRequest>&  add_bus_requests) {

    // Добавляем в базу остановки
	for (const AddStopRequest& add_stop_request : add_stop_requests) {
		catalogue_.AddStop(add_stop_request.name, add_stop_request.coordinate);
	}

	// Добавляем в базу расстояния между остановками
	for (const AddStopRequest& add_stop_request : add_stop_requests) {
		for (const auto& [stop_to, distance] : add_stop_request.distances) {
			catalogue_.SetDistance(add_stop_request.name, stop_to, distance);
		}
	}

	// Добавляем в базу маршруты
	for (const AddBusRequest& add_bus_request : add_bus_requests) {
		catalogue_.AddBus(add_bus_request.name, add_bus_request.type, add_bus_request.stops);
	}
}

// Функция получения информации об остановке
optional<StopInfo> RequestHandler::GetStopInfo(string_view name) const {
    return catalogue_.GetStopInfo(name);
}

// Функция получения информации о маршруте
optional<BusInfo>  RequestHandler::GetBusInfo (string_view name) const {
    return catalogue_.GetBusInfo(name);
}

// Функция отрисовки карты маршрутов
void RequestHandler::RenderMap(const map_renderer::RenderSettings& settings, ostream& output) const {
	renderer_.RenderMap(settings, output);
}

}

}