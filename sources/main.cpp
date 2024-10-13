#include <iostream>
#include <fstream>
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_reader.h"
using namespace std;

int main() {
	// Создаём транспортный справочник
	transport_catalogue::TransportCatalogue catalogue;

	// Создаём отрисовщик карты маршрутов
	transport_catalogue::map_renderer::MapRenderer renderer(catalogue);

	// Создаём обработчик запросов к транспортному справочнику
	transport_catalogue::request_handler::RequestHandler request_handler(catalogue, renderer);

	ifstream input("input.json");
	ofstream output("output.json");

	// Читаем и обрабатываем запросы в формате JSON
	transport_catalogue::json_reader::RequestProcessing(request_handler, input, output);

	return 0;
}