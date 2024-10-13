#include "svg.h"
using namespace std;

namespace svg {

// Объект для вывода в поток цвета, представленного в различных форматах (не задан/строка/RGB/RGBa)
struct ColorPrinter {
    ostream& out;

    // Вывод в поток незаданного цвета
    void operator() (monostate) const {
        out << "none"sv;
    }

    // Вывод в поток цвета в формате строки
    void operator() (const string& str) const {
        out << str;
    }

    // Вывод в поток цвета в формате RGB
    void operator() (const Rgb& rgb) const {
        out << "rgb("sv << unsigned(rgb.red) << ","sv << unsigned(rgb.green) << ","sv << unsigned(rgb.blue) << ")"sv;
    }
    
    // Вывод в поток цвета в формате RGBa
    void operator() (const Rgba& rgba) const {
        out << "rgba("sv << unsigned(rgba.red) << ","sv << unsigned(rgba.green) << ","sv << unsigned(rgba.blue) << ","sv << rgba.opacity << ")"sv;
    }
};

// Перегрузка оператора "<<" для вывода цвета в поток
ostream& operator << (ostream& out, const Color color) {
    visit(ColorPrinter{ out }, color);
    return out;
}

// Перегрузка оператора "<<" для вывода формы конца линии контура в поток
ostream& operator << (ostream& out, const StrokeLineCap stroke_linecap) {
    if      (stroke_linecap == StrokeLineCap::BUTT)  return out << "butt"sv;
    else if (stroke_linecap == StrokeLineCap::ROUND) return out << "round"sv;
    else                                             return out << "square"sv;
}

// Перегрузка оператора "<<" для вывода формы соединения линии контура в поток
ostream& operator << (ostream& out, const StrokeLineJoin stroke_linejoin) {
    if      (stroke_linejoin == StrokeLineJoin::ARCS)       return out << "arcs"sv;
    else if (stroke_linejoin == StrokeLineJoin::BEVEL)      return out << "bevel"sv;
    else if (stroke_linejoin == StrokeLineJoin::MITER)      return out << "miter"sv;
    else if (stroke_linejoin == StrokeLineJoin::MITER_CLIP) return out << "miter-clip"sv;
    else                                                    return out << "round"sv;
}

// Функция рендера объекта (реализует паттерн "Шаблонный метод")
void Object::Render(const RenderContext& ctx) const {
    // Выводим отступ
    ctx.RenderIndent();
    // Делегируем непосредственный рендер своим подклассам
    RenderObject(ctx);
    // Выводим перенос на новую строку
    ctx.out << endl;
}

// Функция задания центра круга (атрибуты cx и cy)
Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

// Функция задания радиуса круга (атрибут r)
Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

// Функция рендера круга для вызова в Object::Render, реализующей паттерн "Шаблонный метод"
void Circle::RenderObject(const RenderContext& ctx) const {
    ctx.out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    ctx.out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(ctx.out);
    ctx.out << "/>"sv;
}

// Функция добавления точки в ломаную
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

// Функция рендера ломаной для вызова в Object::Render, реализующей паттерн "Шаблонный метод"
void Polyline::RenderObject(const RenderContext& ctx) const {
    ctx.out << "<polyline points=\""sv;

    bool first = true;
    for (const Point& p : points_) {
        if (!first) ctx.out << " "sv;
        else        first = false;
        ctx.out << p.x << ","sv << p.y;
    }

    ctx.out << "\""sv;
    RenderAttrs(ctx.out);
    ctx.out << " />"sv;
}

// Функция задания координаты опорной точки текста (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

// Функция задания смещения относительно опорной точки текста (атрибуты dx и dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Функция задания размера шрифта текста (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

// Функция задания шрифта текста (атрибут font-family)
Text& Text::SetFontFamily(string font_family) {
    font_family_ = font_family;
    return *this;
}

// Функция задания толщины шрифта текста (атрибут font-weight)
Text& Text::SetFontWeight(string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

// Функция задания содержимого текста (отображается внутри тега text)
Text& Text::SetData(string data) {
    data_ = data;
    return *this;
}

// Функция рендера текста для вызова в Object::Render, реализующей паттерн "Шаблонный метод"
void Text::RenderObject(const RenderContext& ctx) const {
    ctx.out << "<text"sv;
    RenderAttrs(ctx.out);
    ctx.out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << size_ << "\""sv;

    if (!font_family_.empty()) ctx.out << " font-family=\""sv << font_family_ << "\""sv;
    if (!font_weight_.empty()) ctx.out << " font-weight=\""sv << font_weight_ << "\""sv;

    ctx.out << ">"sv;
    
    for (const char c : data_) {
        if      (c == '\"') { ctx.out << "&quot;"sv; }
        else if (c == '\'') { ctx.out << "&apos;"sv; }
        else if (c == '<')  { ctx.out << "&lt;"sv;   }
        else if (c == '<')  { ctx.out << "&gt;"sv;   }
        else if (c == '&')  { ctx.out << "&amp;"sv;  }
        else                { ctx.out << c;          }
    }
    
    ctx.out << "</text>"sv;
}

// Функция добавления объекта (наследника svg::Object) в svg-документ по указателю
void Document::AddPtr(unique_ptr<Object>&& object) {
    objects_.push_back(move(object));
}

// Функция рендера svg-документа
void Document::Render(ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << endl;

    RenderContext ctx{out, 2, 2};

    for (const auto& p : objects_) {
        p->Render(ctx);
    }
    
    out << "</svg>"sv;
}

} 