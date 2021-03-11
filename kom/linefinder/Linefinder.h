#ifndef LINEFINDER_H
#define LINEFINDER_H

#include <vector>
#include "HelperClasses.h"

using namespace std;

// Calculates the maximum distance between any of the points 
// between start and end and the line from points[start] to points[end].
float calculateError(vector<Point>* points, int start, int end);

// Searches for a straight line that starts at startPoint and ends before of at endPonit.
bool findLineOneWay(vector<Point>* points, int startPoint, int endPoint, int* retLineStart, int* retLineEnd);

// Searches for a straigt line expanding from startPoint.
bool findLineBothWays(vector<Point>* points, int startPoint, int* retLineStart, int* retLineEnd);

// Finds all straigt lines in points.
void findAllLines(vector<Point>* points, vector<pair<Point*, Point*>>* lines);

#endif // LINEFINDER_H

