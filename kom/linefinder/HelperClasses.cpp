#include "HelperClasses.h"


Point::Point(int x, int y) {
    this->x = x;
    this->y = y;
}


float Point::distanceTo(Point* p2) {
    return hypot(x-p2->x, y-p2->y);
}


bool Point::operator==(const Point& other) const {
    return x == other.x && y == other.y;
}


Line::Line(Point* p1, Point* p2) {
    vertical    = ((p2->x)-(p1->x) == 0);
    horizontal  = ((p2->y)-(p1->y) == 0);

    if (vertical) {
        m = (p1->x);
    } else {
        k = (float)(((p2->y)-(p1->y)))/((p2->x)-(p1->x));
        m = (p1->y)-(p1->x)*k;
    }
}


float Line::calculateError(Point* p) {
    if (vertical && horizontal) {
        return 0;
    } else if (vertical) {
        return abs(m - (p->x));
    } else if (horizontal) {
        return abs(m - (p->y));
    } else {
        float diffX = abs((p->x) - (((p->y)-m)/k));
        float diffY = abs((p->y) - (k*(p->x)+m));
        if (diffX == 0 && diffY == 0) return 0;

        float c = sqrt(diffX*diffX+diffY*diffY);
        return (diffX*diffY)/c;
    }
}


int vectorCirculizer(int vectorSize, int index) {
    if (vectorSize == 0) throw runtime_error("Cannot calculate modulo of 0");
    int result = index % vectorSize;
    if (result < 0) result += vectorSize;
    return result;
}

