#include "Linefinder.h"
#include <iostream>

const int MIN_LINE_LENGTH = 5;
const int MAX_LINE_ERROR = 20;
const int MAX_LINE_GAP = 250;

float calculateError(vector<Point>* points, int start, int end) {
    int n = points->size();
    if (n == 0) throw runtime_error("Cannot calculate error on empty list");

    float error = 0;
    Line line = Line(
        &(points->at(vectorCirculizer(n, start))),
        &(points->at(vectorCirculizer(n, end))));

    if (end < start) end += n;

    // Calculates the error for every point and saves the maximum in error
    for (int i = start; i < end; i++)  {
        error = max(error, line.calculateError(&(points->at(vectorCirculizer(n, i)))));
    }
    return error;
}


bool findLineOneWay(vector<Point>* points, int startPoint, int endPoint, int* retLineStart, int* retLineEnd) {
    int n = points->size();
    int lineStart = startPoint;
    int lineEnd = startPoint;

    while (true) {
        // Calculates the error for the line. If the error is to much we make the line shorter again and break.
        float error = calculateError(points, lineStart, vectorCirculizer(n, lineEnd));
        if (error > MAX_LINE_ERROR) {
            lineEnd--;
            break;
        }

        // If the distance to the next point is too great we break.
        float distance = points->at(vectorCirculizer(n, lineEnd)).distanceTo(&(points->at(vectorCirculizer(n, lineEnd +1))));
        if (distance > MAX_LINE_GAP) break;

        // If we have reached our stop point we break.
        if (vectorCirculizer(n, lineEnd) == vectorCirculizer(n, endPoint)) break;

        // Otherwise we make the line 1 longer.
        lineEnd++;
    }

    *retLineStart = vectorCirculizer(n, lineStart);
    *retLineEnd = vectorCirculizer(n, lineEnd);
    int lineLength = lineEnd - lineStart;
    if (lineLength < 0) lineLength += n;
    return (lineLength > MIN_LINE_LENGTH);
}


bool findLineBothWays(vector<Point>* points, int startPoint, int* retLineStart, int* retLineEnd) {
    int n = points->size();
    int lineStart = startPoint;
    int lineEnd = startPoint;

    // If we should keep extending the start- and endpoint of the line.
    bool expandStart = true;
    bool expandEnd = true;

    while (expandStart || expandEnd) {
        // Calculates the error of the line. If error is too big we decrease the line lenght and stop extending it.
        if (expandStart) {
            float error = calculateError(points, vectorCirculizer(n, lineStart), vectorCirculizer(n, lineEnd));
            if (error > MAX_LINE_ERROR) {
                expandStart = false;
                lineStart += 10;
            } else {
                // Calculate the distance to the next point and stop if it is too large.
                float distance = points->at(vectorCirculizer(n, lineStart)).distanceTo(&(points->at(vectorCirculizer(n, lineStart -1))));
                if (distance > MAX_LINE_GAP) expandStart = false;
                else lineStart--;
            }
        }

        // Exactly the same for the end of the line.
        if (expandEnd) {
            float error = calculateError(points, vectorCirculizer(n, lineStart), vectorCirculizer(n, lineEnd));
            if (error > MAX_LINE_ERROR) {
                expandEnd = false;
                lineEnd -= 10;
            } else {
                float distance = points->at(vectorCirculizer(n, lineEnd)).distanceTo(&(points->at(vectorCirculizer(n, lineEnd +1))));
                if (distance > MAX_LINE_GAP) expandEnd = false;
                else lineEnd++;
            }
        }
    }
    *retLineStart = vectorCirculizer(n, lineStart);
    *retLineEnd = vectorCirculizer(n, lineEnd);
    int lineLength = lineEnd - lineStart;
    if (lineLength < 0) lineLength += n;
    return (lineLength > MIN_LINE_LENGTH);
}


void findAllLines(vector<Point>* points, vector<pair<Point*, Point*>>* lines) {
    int n = points->size();
    int searchStart = 0;
    int searchEnd = n-1;
    bool lineFound = false;

    int lineStart;
    int lineEnd;

    // Starts searching clockwise for new lines, starting of on the end point of the last found line.
    while (searchStart < searchEnd) {
        if (findLineOneWay(points, searchStart, searchEnd, &lineStart, &lineEnd)) {
            if (!lineFound) {
                lineFound = true;
                searchStart = lineEnd;
                searchEnd = lineEnd + n-1;
            } else {
                lines->push_back(make_pair(&(points->at(lineStart)), &(points->at(lineEnd))));
                if (lineEnd < searchStart) {
                    searchStart = lineEnd +n;
                } else {
                    searchStart = lineEnd;
                }

            }
        } else searchStart++;
        if (!lineFound) searchEnd++;
        if (vectorCirculizer(n, searchStart) == 0) break;
    }
}


