#include "map_renderer.h"
using namespace std;
using namespace svg;
using namespace geo;

// Пространство имён транспортного справочника
namespace transport_catalogue {

// Пространство имён для функционала, связанного с отрисовкой карты маршрутов
namespace map_renderer {

// Пространство имён для структур и функций, использующихся только для внутренней работы transport_catalogue::map_renderer
namespace detail {

// Функция проверки на ноль с точностью до EPSILON
bool IsZero(double value) {
	constexpr double EPSILON = 1e-6;
    return std::abs(value) < EPSILON;
}

// Функция проекции широты и долготы в координаты внутри SVG-изображения
Point SphereProjector::operator()(Coordinate coordinate) const {
    return {(coordinate.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coordinate.lat) * zoom_coeff_ + padding_};
}

}

MapRenderer::MapRenderer(const TransportCatalogue& catalogue) : catalogue_(catalogue) { }

// Функция отрисовки линий маршрутов на карте маршрутов
void MapRenderer::RenderRoutesPaths(Document& document, const detail::SphereProjector& projector, const RenderSettings& settings) const{

    // Счётчик для использования цветов из палитры цветов по кругу
    size_t color_counter = 0;
    
    // Проходим по всем маршрутам в алфавитном порядке
    for(const auto& [bus_name, bus] : catalogue_.GetBusnameToBusMap()) {
        
        // Если на маршруте нет остановок, то пропускаем его
        if(bus->stops.empty()) continue;

        // Формируем линию очередного маршрута
        Polyline route;

        for(const auto& stop : bus->stops) {
            route.AddPoint(projector(stop->coordinate));
        }

        // Если маршрут линейный, нужно отрисовать и обратный путь
        if(bus->type == BusRouteType::Line) {
            bool last = true;
            for(auto stop_it = bus->stops.rbegin(); stop_it != bus->stops.rend(); ++stop_it) {
                if (last) { last = false; continue; }
                route.AddPoint(projector((*stop_it)->coordinate));
            }
        }

        route.SetStrokeColor(settings.color_palette[color_counter])
             .SetFillColor(NoneColor)
             .SetStrokeWidth(settings.line_width)
             .SetStrokeLineCap(StrokeLineCap::ROUND)
             .SetStrokeLineJoin(StrokeLineJoin::ROUND);

        // Добавляем линию очередного маршрута в документ
        document.Add(route);

        // Увеличиваем счётчик цветов
        color_counter = (color_counter + 1) % settings.color_palette.size();
    }
}

// Функция отрисовки названий маршрутов на карте маршрутов
void MapRenderer::RenderRoutesNames(Document& document, const detail::SphereProjector& projector, const RenderSettings& settings) const{

    // Счётчик для использования цветов из палитры цветов по кругу
    size_t color_counter = 0;

    // Проходим по всем маршрутам в алфавитном порядке
    for(const auto& [bus_name, bus] : catalogue_.GetBusnameToBusMap()) {

        // Если на маршруте нет остановок, то пропускаем его
        if(bus->stops.empty()) continue;

        // Формируем название очередного маршрута и подложку для него
        Text route_name_text;
        Text route_name_text_underlayer;

        route_name_text.SetPosition(projector(bus->stops[0]->coordinate))
                       .SetData(string(bus_name))
                       .SetOffset(settings.bus_label_offset)
                       .SetFontSize(settings.bus_label_font_size)
                       .SetFontFamily("Verdana"s)
                       .SetFontWeight("bold"s)
                       .SetFillColor(settings.color_palette[color_counter]);
        
        route_name_text_underlayer.SetPosition(projector(bus->stops[0]->coordinate))
                                  .SetData(string(bus_name))
                                  .SetOffset(settings.bus_label_offset)
                                  .SetFontSize(settings.bus_label_font_size)
                                  .SetFontFamily("Verdana"s)
                                  .SetFontWeight("bold"s)
                                  .SetFillColor(settings.underlayer_color)
                                  .SetStrokeColor(settings.underlayer_color)
                                  .SetStrokeWidth(settings.underlayer_width)
                                  .SetStrokeLineCap(StrokeLineCap::ROUND)
                                  .SetStrokeLineJoin(StrokeLineJoin::ROUND);

        // Добавляем название и подложку очередного маршрута в документ
        document.Add(route_name_text_underlayer);
        document.Add(route_name_text);

        // Если маршрут линейный, нужно отрисовать название и подложку у конечной остановки
        if(bus->type == BusRouteType::Line) {

            route_name_text.SetPosition(projector(bus->stops.back()->coordinate));
            route_name_text_underlayer.SetPosition(projector(bus->stops.back()->coordinate));

            document.Add(route_name_text_underlayer);
            document.Add(route_name_text);
        }

        // Увеличиваем счётчик цветов
        color_counter = (color_counter + 1) % settings.color_palette.size();
    }
}

// Функция отрисовки точек остановок на карте маршрутов
void MapRenderer::RenderRoutesStopsPoints(Document& document, const detail::SphereProjector& projector, const RenderSettings& settings) const{

    // Проходим по всем остановкам в алфавитном порядке
    for(const auto& [stop_name, stop] : catalogue_.GetStopnameToStopMap()) {

        // Если через остановку не проходят маршруты, то пропускаем её
        if(!catalogue_.IfBusesOnStop(stop_name)) continue;

        // Формируем точку очередной остановки
        Circle stop_point;

        stop_point.SetCenter(projector(stop->coordinate))
                  .SetRadius(settings.stop_radius)
                  .SetFillColor("white"s);
        
        // Добавляем точку очередной остановки в документ
        document.Add(stop_point);
    }
}

// Функция отрисовки названий остановок на карте маршрутов
void MapRenderer::RenderRoutesStopsNames(Document& document, const detail::SphereProjector& projector, const RenderSettings& settings) const{

    // Проходим по всем остановкам в алфавитном порядке
    for(const auto& [stop_name, stop] : catalogue_.GetStopnameToStopMap()) {

        // Если через остановку не проходят маршруты, то пропускаем её
        if(!catalogue_.IfBusesOnStop(stop_name)) continue;

        // Формируем название очередной остановки и подложку для него
        Text stop_name_text;
        Text stop_name_text_underlayer;

        stop_name_text.SetPosition(projector(stop->coordinate))
                       .SetData(string(stop_name))
                       .SetOffset(settings.stop_label_offset)
                       .SetFontSize(settings.stop_label_font_size)
                       .SetFontFamily("Verdana"s)
                       .SetFillColor("black"s);
        
        stop_name_text_underlayer.SetPosition(projector(stop->coordinate))
                                 .SetData(string(stop_name))
                                 .SetOffset(settings.stop_label_offset)
                                 .SetFontSize(settings.stop_label_font_size)
                                 .SetFontFamily("Verdana"s)
                                 .SetFillColor(settings.underlayer_color)
                                 .SetStrokeColor(settings.underlayer_color)
                                 .SetStrokeWidth(settings.underlayer_width)
                                 .SetStrokeLineCap(StrokeLineCap::ROUND)
                                 .SetStrokeLineJoin(StrokeLineJoin::ROUND);

        // Добавляем название и подложку очередной остановки в документ
        document.Add(stop_name_text_underlayer);
        document.Add(stop_name_text);
    }
}

// Функция отрисовки карты маршрутов
void MapRenderer::RenderMap(const RenderSettings& settings, ostream& output) const {
    using namespace detail;

    // SVG-документ с картой маршрутов
    Document result;

    // Создаём проектор сферических координат на карту
    const SphereProjector projector(catalogue_.GetStops().begin(),
                                    catalogue_.GetStops().end(),
                                    settings.width, settings.height, settings.padding);

    // Отрисовываем линии маршрутов на карте маршрутов
    RenderRoutesPaths(result, projector, settings);

    // Отрисовываем названия маршрутов на карте маршрутов
    RenderRoutesNames(result, projector, settings);

    // Отрисовываем точки остановок на карте маршрутов
    RenderRoutesStopsPoints(result, projector, settings);

    // Отрисовываем названия остановок на карте маршрутов
    RenderRoutesStopsNames(result, projector, settings);

    // Отрисовываем SVG-документ с картой маршрутов
    result.Render(output);
}

}

}