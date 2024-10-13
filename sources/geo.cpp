#include <string>
#include <cmath>
#include "geo.h"
using namespace std;

// Пространство имён для географических данных и функций
namespace geo {

bool operator == (const Coordinate& lhs, const Coordinate& rhs) {
    return lhs.lat == rhs.lat && lhs.lng == rhs.lng;
}

bool operator != (const Coordinate& lhs, const Coordinate& rhs) {
    return !(lhs == rhs);
}

// Функция вычисления расстояния между координатами
double ComputeDistance(const Coordinate& from, const Coordinate& to) {
    if (from == to) return 0;
    static const double dr = 3.1415926535 / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr) + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr)) * 6371000;
}

}