#include <iostream>
#include <cmath>
#include "RobotMap.h"
#include <queue>
#define CONST_PI 3.14159265358979323846

const int MAX_ANGLE_DEVIATION = 20;
const int WALL_LENGTH = 400;
const int WALL_LENGTH_DEVIATION = 100;

RobotMap::RobotMap() {
    // Sets the map to have an outside border of SquareState::OUTSIDE.
    for (int y = 0; y < MAP_DIMENTIONS; y++) {
        for (int x = 0; x < MAP_DIMENTIONS; x++) {
            
            if (map[y][x] == SquareState::OUTSIDE ||
                map[y][x] == SquareState::INSIDE ||
                map[y][x] == SquareState::START) {
                    
                    
                mapChanges += "map," + to_string(x) + "," + to_string(y) + "," + char(SquareState::UNKNOWN) + ";";
 
                    
            }
            
            // Sets all squares to SquareState::UNKNOWN first so that any
            // changes are recorded.
            map[y][x] = SquareState::UNKNOWN;
            if (x == 0 || x == MAP_DIMENTIONS -1) {
                setCellValue(x,y, SquareState::OUTSIDE);
            } else if (y == 0 || y == MAP_DIMENTIONS -1) {
                setCellValue(x,y, SquareState::OUTSIDE);
            }
        }
    }
    setCellValue(25,25, SquareState::START);
}


void RobotMap::setCellValue(const int x, const int y, SquareState state) {
    if(0 <= x && x < MAP_DIMENTIONS && 0 <= y && y < MAP_DIMENTIONS) {
        if (map[y][x] != SquareState::START) {
            if (state != map[y][x]) {
                mapChanges += "map," + to_string(x) + "," + to_string(y) + "," + char(state) + ";";
            }
            map[y][x] = state;
        }
    }
}


void RobotMap::setCellInside(const int x, const int y){
    if(0 <= x && x < MAP_DIMENTIONS && 0 <= y && y < MAP_DIMENTIONS) {
        if (map[y][x] == SquareState::UNKNOWN) {
            mapChanges += "map," + to_string(x) + "," + to_string(y) + "," + char(SquareState::INSIDE) + ";";
            map[y][x] = SquareState::INSIDE;
        }
    }
}


SquareState RobotMap::getCellValue(const int x, const int y) const {
    if(0 <= x && x < MAP_DIMENTIONS && 0 <= y && y < MAP_DIMENTIONS) {
        return map[y][x];
    } else {
        return SquareState::OUTSIDE;
    }
}


string RobotMap::getMapChanges() {
    string str(mapChanges);
    return str;
}

void RobotMap::clearMapChanges() {
    mapChanges.erase();
}

void RobotMap::filterLines(vector<pair<Point*, Point*>>* sourceLines,
                           vector<pair<Point*, Point*>>* destLines,
                           float robotX, float robotY, float robotAngle) {

    for (pair<Point*, Point*> line : *sourceLines) {
        // Discards lines that are not close enough to the correct length
        float lineLength = line.first->distanceTo(line.second);
        float lineLengthMod = fmod(lineLength, WALL_LENGTH);
        if ((WALL_LENGTH_DEVIATION < lineLengthMod && lineLengthMod < WALL_LENGTH - WALL_LENGTH_DEVIATION)
                || lineLength < WALL_LENGTH - WALL_LENGTH_DEVIATION) continue;

        // Discards lines that are not close enough to the correct angle
        float angle = atan2(line.first->y - line.second->y, line.first->x - line.second->x) * 180/CONST_PI;
        angle += robotAngle;
        if (angle < 0) angle += 360;
        angle = fmod(angle, 90);
        if (MAX_ANGLE_DEVIATION < angle && angle < 90 - MAX_ANGLE_DEVIATION) continue;

        // Rotates, scales and shifts the lines so that they align to the grid.
        // Also discards lines that are too far away from beeing a wall.
        float cosValue = cos((robotAngle - 90) * CONST_PI/180);
        float sinValue = sin((robotAngle - 90) * CONST_PI/180);

        float startX = (line.first->x*cosValue - line.first->y*sinValue)/WALL_LENGTH + robotX;
        float startY = (line.first->x*sinValue + line.first->y*cosValue)/WALL_LENGTH + robotY;
        if (hypot(abs(startX - round(startX)), abs(startY - round(startY))) > 0.4) continue;

        float endX = (line.second->x*cosValue - line.second->y*sinValue)/WALL_LENGTH + robotX;
        float endY = (line.second->x*sinValue + line.second->y*cosValue)/WALL_LENGTH + robotY;
        if (hypot(abs(endX - round(endX)), abs(endY - round(endY))) > 0.4) continue;


        // If we get here, there is definetly a wall.
        bool vertical = (round(startX) == round(endX));
        bool horizontal = (round(startY) == round(endY));
        if (!vertical && !horizontal) continue;

        // The line has passed all tests and is okay.
        destLines->push_back(line);
    }
}

void RobotMap::updateMap(vector<pair<Point*, Point*>>* lines, float robotX, float robotY, float robotAngle) {

    setCellValue(floor(robotX), floor(robotY), SquareState::INSIDE);

    setCellInside(floor(robotX +1), floor(robotY));
    setCellInside(floor(robotX -1), floor(robotY));
    setCellInside(floor(robotX), floor(robotY +1));
    setCellInside(floor(robotX), floor(robotY -1));


    auto line = lines->begin();
    while (line != lines->end()) {

        float lineLength = line->first->distanceTo(line->second);
        int numberOfWallSegments = round(lineLength/WALL_LENGTH);

        // Rotates, scales and shifts the lines so that they align to the grid.
        float cosValue = cos((robotAngle - 90) * CONST_PI/180);
        float sinValue = sin((robotAngle - 90) * CONST_PI/180);
        float startX = (line->first->x*cosValue - line->first->y*sinValue)/WALL_LENGTH + robotX;
        float startY = (line->first->x*sinValue + line->first->y*cosValue)/WALL_LENGTH + robotY;
        float endX = (line->second->x*cosValue - line->second->y*sinValue)/WALL_LENGTH + robotX;
        float endY = (line->second->x*sinValue + line->second->y*cosValue)/WALL_LENGTH + robotY;

        bool vertical = (round(startX) == round(endX));

        // Calculates the coordinate of the line endpoint that is closes to 0,0.
        int xCoord = min(round(startX), round(endX));
        int yCoord = min(round(startY), round(endY));

        // Sets INSIDE/OUTSIDE for all linesegments.
        for (int i = 0; i < numberOfWallSegments; i++) {
            int xOut;
            int yOut;
            int xIn;
            int yIn;
            if (vertical) {
                if (robotX > xCoord) {
                    xOut = xCoord -1;
                    xIn = xCoord;
                } else {
                    xOut = xCoord;
                    xIn = xCoord -1;
                }
                yOut = yCoord +i;
                yIn = yCoord +i;
            } else {
                if (robotY > yCoord) {
                    yOut = yCoord -1;
                    yIn = yCoord;
                } else {
                    yOut = yCoord;
                    yIn = yCoord -1;
                }
                xOut = xCoord +i;
                xIn = xCoord +i;
            }
            setCellValue(xOut, yOut, SquareState::OUTSIDE);
            setCellValue(xIn, yIn, SquareState::INSIDE);
        }

        // Sets all squares between the midpoint of each linesegment and the robot to inside.
        float xLine;
        float yLine;
        for (int i = 0; i < numberOfWallSegments; i++) {
            if (vertical) {
                xLine = xCoord;
                yLine = yCoord + 0.5 +i;
            } else {
                xLine = xCoord + 0.5 +i;
                yLine = yCoord;
            }

           for (float j = 0.025; j < 1; j += 0.025) {
                setCellInside(floor((1-j)*xLine + j*robotX), floor((1-j)*yLine + j*robotY) );
            }
        }
        line++;
    }
}


void RobotMap::updatePosition(vector<pair<Point *, Point *>> *lines, float *robotX, float *robotY, float *robotAngle) {
    if (lines->size() == 0) return;

    float errorX = 0;
    float errorY = 0;
    float errorAngle = 0;

    auto line = lines->begin();
    while (line != lines->end()) {

        float cosValue = cos((*robotAngle - 90) * CONST_PI/180);
        float sinValue = sin((*robotAngle - 90) * CONST_PI/180);

        float startX = (line->first->x*cosValue - line->first->y*sinValue)/WALL_LENGTH + *robotX;
        float startY = (line->first->x*sinValue + line->first->y*cosValue)/WALL_LENGTH + *robotY;
        float endX = (line->second->x*cosValue - line->second->y*sinValue)/WALL_LENGTH + *robotX;
        float endY = (line->second->x*sinValue + line->second->y*cosValue)/WALL_LENGTH + *robotY;

        errorX += startX - round(startX);
        errorX += endX - round(endX);
        errorY += startY - round(startY);
        errorY += endY - round(endY);

        float angle = atan2(line->first->y - line->second->y, line->first->x - line->second->x) * 180/CONST_PI + *robotAngle;
        angle = angle/90;
        errorAngle += angle - round(angle);

        line++;
    }

    *robotX += -errorX/(lines->size()*2);
    *robotY += -errorY/(lines->size()*2);
    *robotAngle += -(errorAngle*90)/(lines->size());
    
    *robotAngle = fmod(*robotAngle, 360);
    if (*robotAngle < 0) {
        *robotAngle += 360;
    }
}


string RobotMap::toString() {
    string str;
    for (int y = MAP_DIMENTIONS -1; y >= 0; y--) {
        for (int x = 0; x < MAP_DIMENTIONS; x++) {
            str += char(map[y][x]);
        }
        if (y != 0) {
            str += ("\n");
        }
    }
    return str;
}


bool RobotMap::findWayToSquare(float robotX, float robotY, SquareState state, int* goalX, int* goalY, bool goDirect) {
    //Breadth-first search for a way to the nearest square of state "state",
    //that doesn't run over a SquareState::OUTSIDE.

    // Represents each square on the map.
    struct Square {
    public:
        int x;
        int y;
        bool visited = false;
        Square* prev = nullptr;
        Square* next = nullptr;
    };

    Square s[MAP_DIMENTIONS][MAP_DIMENTIONS];
    //Sets the coordinates of each square to itself.
    for (int x = 0; x < MAP_DIMENTIONS; x++) {
        for (int y = 0; y < MAP_DIMENTIONS; y++) {
            s[x][y].x = x;
            s[x][y].y = y;
        }
    }
    
    int intRobX = floor(robotX);
    int intRobY = floor(robotY);

    // Sets the Square that the robot is in to be the start for the search
    s[intRobX][intRobY].visited = true;
    queue<Square*> nodes;
    nodes.push(&s[intRobX][intRobY]);

    while (!nodes.empty()) {
        Square* v = nodes.front();
        if (getCellValue(v->x, v->y) == state) break;
        nodes.pop();

        auto addNeighbour = [&](int x, int y) {
            if (getCellValue(x, y) != SquareState::OUTSIDE && !s[x][y].visited) {
                s[x][y].visited = true;
                s[x][y].prev = v;
                nodes.push(&s[x][y]);
            }
        };
        addNeighbour(v->x, v->y+1);
        addNeighbour(v->x+1, v->y);
        addNeighbour(v->x, v->y-1);
        addNeighbour(v->x-1, v->y);

    }

    // If we haven't found what we are looking for.
    if(nodes.empty()) return false;
    if (goDirect) {
        Square* v = nodes.front();
        while (true) {
            if (directPathBetween(robotX, robotY, (float)(v->x) +0.5, (float)(v->y) +0.5)) {
                *goalX = v->x;
                *goalY = v->y;
                return true;
            }
            if (v->prev == nullptr) break;
            v = v->prev;
        }
        *goalX = v->x;
        *goalY = v->y;
        return true;        

    } else {
        // Traverses backward through the path to set the next-pointers.
        Square* v = nodes.front();
        while (true) {
            if (v->prev == nullptr) break;
            v->prev->next = v;
            v = v->prev;
        }

        // Finds the coordinate of the first turn of the path.
        while (v->next != nullptr) {
            if (intRobX != v->next->x && intRobY != v->next->y) break;
            v = v->next;
        }

        *goalX = v->x;
        *goalY = v->y;
        return true;
    }
}


bool RobotMap::directPathBetween(float x1, float y1, float x2, float y2) {
    float pathLength = hypot(x2-x1, y2-y1);
    float pathAngle = atan2(y2-y1, x2-x1);

    auto checkDirectPath = [&](float offsetAngle) {
        float cosValue = cos(offsetAngle);
        float sinValue = sin(offsetAngle);
        for (float i = 0; i < 1; i += 1/(10*pathLength)) {
            SquareState state = getCellValue(floor((1-i)*(x1 +0.35*cosValue) + i*(x2 +0.35*cosValue)), 
                                             floor((1-i)*(y1 +0.35*sinValue) + i*(y2 +0.35*sinValue)));
            if (state == SquareState::OUTSIDE) {
                return false;
            }
        }
        return true;
    };

    if (!checkDirectPath(pathAngle + CONST_PI/2)) return false;
    if (!checkDirectPath(pathAngle - CONST_PI/2)) return false;

    return true;
}

