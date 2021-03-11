#ifndef HELPERCLASSES_H
#define HELPERCLASSES_H

#include <cmath>
#include <stdexcept>


using namespace std;

// Class that represents a point on the xy plane.
class Point {
public:
    int x, y;
    Point() = delete;
    Point(int x, int y);

    // Points are equal if the have the same coordinates.
    bool operator==(const Point&) const;

    // Calculates the straight distance between two points.
    float distanceTo(Point* p2);
};

// Class that represents a mathematical line.
class Line {
public:
    // Creates a line from the two points
    Line(Point* p1, Point* p2);

    // Calculates how far the point is from the closest point of the line
    float calculateError(Point* p);

private:
    bool vertical;
    bool horizontal;
    float k;

    // if the line is vertical, m is used to indicate offset from y-axis
    float m;
};

// Use this function to transform the index to a vector so that it 
// behaves as a cirular vector.
int vectorCirculizer(int vectorSize, int index);

#endif // HELPERCLASSES_H

