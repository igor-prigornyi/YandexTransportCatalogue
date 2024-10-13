#pragma once

#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <optional>

namespace svg {

// Структура цвета в формате RGB
struct Rgb {
    // Наличие конструкторов требовали тесты тренажёра
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) { }

    uint8_t red   = 0u;
    uint8_t green = 0u;
    uint8_t blue  = 0u;
};

// Структура цвета в формате RGBa
struct Rgba { 
    // Наличие конструкторов требовали тесты тренажёра
    Rgba() = default;
    Rgba(unsigned int r, unsigned int g, unsigned int b, double o) : red(r), green(g), blue(b), opacity(o) { }

    uint8_t red    = 0u;
    uint8_t green  = 0u;
    uint8_t blue   = 0u;
    double opacity = 1.0;
};

// Цвет может быть не задан, либо представлен в формате строки, формате RGB или формате RGBa
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

// Глобальная константа (singleton) для опции "цвет не задан"
inline const Color NoneColor;

// Перегрузка оператора "<<" для вывода цвета в поток
std::ostream& operator << (std::ostream& out, const Color color);

// Формы конца линии контура (возможные значения атрибута stroke-linecap)
enum class StrokeLineCap  { BUTT, ROUND, SQUARE };

// Формы соединения линии контура (возможные значения атрибута stroke-linejoin)
enum class StrokeLineJoin { ARCS, BEVEL, MITER,  MITER_CLIP, ROUND };

// Перегрузка оператора "<<" для вывода формы конца линии контура в поток
std::ostream& operator << (std::ostream& out, const StrokeLineCap  stroke_linecap);

// Перегрузка оператора "<<" для вывода формы соединения линии контура в поток
std::ostream& operator << (std::ostream& out, const StrokeLineJoin stroke_linejoin);

// Структура точки
struct Point {
    double x = 0;
    double y = 0;
};

// Вспомогательная структура для вывода содержимого с отступами
struct RenderContext {
    // Наличие конструкторов требовали тесты тренажёра
    RenderContext(std::ostream& out) : out(out) { }
    RenderContext(std::ostream& out, int indent_step, int indent = 0) : out(out), indent_step(indent_step), indent(indent) { }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) out.put(' ');
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

// Базовый класс объекта
class Object {
public:
    // Функция рендера объекта (реализует паттерн "Шаблонный метод")
    void Render(const RenderContext& ctx) const;

    // Класс предполагает полиморфное удаление наследников, поэтому имеет публичный виртуальный деструктор
    virtual ~Object() = default;

private:
    // У наследников обязательно должна быть определена функция рендера для
    // вызова в Object::Render, реализующей паттерн "Шаблонный метод"
    virtual void RenderObject(const RenderContext& ctx) const = 0;
};

// Базовый класс свойств заливки и линии контура объекта
template <typename Owner>
class PathProps {
public:
    // Функция задания цвета заливки (атрибут fill)
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    // Функция задания цвета линии контура (атрибут stroke)
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    // Функция задания толщины линии контура (атрибут stroke-width)
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }

    // Функция задания формы конца линии контура (атрибут stroke-linecap)
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_linecap_ = line_cap;
        return AsOwner();
    }

    // Функция задания формы соединения линии контура (атрибут stroke-linejoin)
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_linejoin_ = line_join;
        return AsOwner();
    }

protected:
    // Класс не предполгает полиморфного удаления, поэтому имеет защищённый невертуальный деструктор
    ~PathProps() = default;

    // Функция рендера свойств заливки и линии контура объекта
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_)      out << " fill=\""sv            << *fill_color_      << "\""sv;
        if (stroke_color_)    out << " stroke=\""sv          << *stroke_color_    << "\""sv;
        if (stroke_width_)    out << " stroke-width=\""sv    << *stroke_width_    << "\""sv;
        if (stroke_linecap_)  out << " stroke-linecap=\""sv  << *stroke_linecap_  << "\""sv;
        if (stroke_linejoin_) out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
    }

private:
    // Функция получения ссылки на текущий объект
    Owner& AsOwner() { return static_cast<Owner&>(*this); }

    // Атрибуты свойств заливки и линии контура объекта (наличие опционально)
    std::optional<Color>          fill_color_;
    std::optional<Color>          stroke_color_;
    std::optional<double>         stroke_width_;
    std::optional<StrokeLineCap>  stroke_linecap_;
    std::optional<StrokeLineJoin> stroke_linejoin_;
};

// Класс круга
class Circle final : public Object, public PathProps<Circle> {
public:
    // Функция задания центра круга (атрибуты cx и cy)
    Circle& SetCenter(Point center);

    // Функция задания радиуса круга (атрибут r)
    Circle& SetRadius(double radius);

private:
    // Функция рендера круга для вызова в Object::Render, реализующей паттерн "Шаблонный метод"
    void RenderObject(const RenderContext& ctx) const override;

    // Атрибуты круга
    Point center_ = { 0.0, 0.0 };
    double radius_ = 1.0;
};

// Класс ломаной
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Функция добавления точки в ломаную
    Polyline& AddPoint(Point point);

private:
    // Функция рендера ломаной для вызова в Object::Render, реализующей паттерн "Шаблонный метод"
    void RenderObject(const RenderContext& ctx) const override;

    // Точки ломаной
    std::vector<Point> points_;
};

// Класс текста
class Text final : public Object, public PathProps<Text> {
public:
    // Функция задания координаты опорной точки текста (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Функция задания смещения относительно опорной точки текста (атрибуты dx и dy)
    Text& SetOffset(Point offset);

    // Функция задания размера шрифта текста (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Функция задания шрифта текста (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Функция задания толщины шрифта текста (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Функция задания содержимого текста (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    // Функция рендера текста для вызова в Object::Render, реализующей паттерн "Шаблонный метод"
    void RenderObject(const RenderContext& ctx) const override;
    
    // Атрибуты текста
    Point pos_ =    { 0.0, 0.0 };
    Point offset_ = { 0.0, 0.0 };
    uint32_t size_ = 1u;

    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

// Интерфейс для контейнера объектов
class ObjectContainer {
public:
    // Функция добавления объекта в контейнер по значению
    template <typename ObjectType>
    void Add(ObjectType object) {
        AddPtr(std::make_unique<ObjectType>(std::move(object)));
    }

    // У наследников обязательно должна быть определена функция добавления объекта в контейнер по указателю
    virtual void AddPtr(std::unique_ptr<Object>&& object) = 0;
    
protected:
    // Класс не предполгает полиморфного удаления, поэтому имеет защищённый невертуальный деструктор
    ~ObjectContainer() = default;
};

// Интерфейс для объекта, который может быть нарисован
class Drawable {
public:
    // У наследников обязательно должна быть определена функция добавдения объекта в контейнер объектов
    virtual void Draw(ObjectContainer& container) const = 0;

    // Класс предполагает полиморфное удаление наследников, поэтому имеет публичный виртуальный деструктор
    virtual ~Drawable() = default;
};

// Класс svg-документа
class Document : public ObjectContainer {
public:
    // Функция добавления объекта (наследника svg::Object) в svg-документ по указателю
    void AddPtr(std::unique_ptr<Object>&& object) override;

    // Функция рендера svg-документа
    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

}