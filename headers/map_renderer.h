#pragma once

#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <optional>
#include "transport_catalogue.h"
#include "svg.h"

// Пространство имён транспортного справочника
namespace transport_catalogue {

// Пространство имён для функционала, связанного с отрисовкой карты маршрутов
namespace map_renderer {

// Пространство имён для структур и функций, использующихся только для внутренней работы transport_catalogue::map_renderer
namespace detail {

// Функция проверки на ноль с точностью до EPSILON
bool IsZero(double value);

// Класс проектора сферических координат на карту
class SphereProjector {
public:
    template <typename StopsInputIt>
    SphereProjector(StopsInputIt stops_begin,
                    StopsInputIt stops_end,
                    double max_width,
                    double max_height,
                    double padding) : padding_(padding) {

        // Если точки поверхности сферы не заданы, вычислять нечего
        if (stops_begin == stops_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(stops_begin, stops_end,
                                                        [](auto lhs, auto rhs) { return lhs.coordinate.lng < rhs.coordinate.lng; });
        
        min_lon_ = left_it->coordinate.lng;
        const double max_lon = right_it->coordinate.lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(stops_begin, stops_end,
                                                        [](auto lhs, auto rhs) { return lhs.coordinate.lat < rhs.coordinate.lat; });
        
        const double min_lat = bottom_it->coordinate.lat;
        max_lat_ = top_it->coordinate.lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Функция проекции широты и долготы в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinate coordinate) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

}

// Эти псевдонимы нужны модулю json_reader для парсинга настроек отрисовки карты маршрутов 
using Color = svg::Color;
using Rgb   = svg::Rgb;
using Rgba  = svg::Rgba;

// Структура настроек отрисовки карты маршрутов
struct RenderSettings {
    double width;
    double height;
    double padding;

    double line_width;
    double stop_radius;

    uint32_t bus_label_font_size;
    svg::Point bus_label_offset;

    uint32_t stop_label_font_size;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width;

    std::vector<svg::Color> color_palette;
};

// Класс отрисовщика карты маршрутов
class MapRenderer {
public:
    MapRenderer(const TransportCatalogue& catalogue);

    // Функция отрисовки карты маршрутов
    void RenderMap(const RenderSettings& settings, std::ostream& output = std::cout) const;

private:

    // Функция отрисовки линий маршрутов на карте маршрутов
    void RenderRoutesPaths(svg::Document& document, const detail::SphereProjector& projector, const RenderSettings& settings) const;

    // Функция отрисовки названий маршрутов на карте маршрутов
    void RenderRoutesNames(svg::Document& document, const detail::SphereProjector& projector, const RenderSettings& settings) const;

    // Функция отрисовки точек остановок на карте маршрутов
    void RenderRoutesStopsPoints(svg::Document& document, const detail::SphereProjector& projector, const RenderSettings& settings) const;

    // Функция отрисовки названий остановок на карте маршрутов
    void RenderRoutesStopsNames(svg::Document& document, const detail::SphereProjector& projector, const RenderSettings& settings) const;

    const TransportCatalogue& catalogue_;
};

}

}