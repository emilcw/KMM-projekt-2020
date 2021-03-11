#ifndef ROBOTMAP_H
#define ROBOTMAP_H

#include <string>
#include <vector>
#include "linefinder/HelperClasses.h"

using namespace std;

static const int MAP_DIMENTIONS = 51;

enum class SquareState: char {
    UNKNOWN = 'U',
    INSIDE = 'I',
    OUTSIDE = 'O',
    START = 'S'
};

// The map is represented by this class. Since no wall can have inside on both sides
// we can model the map as a grid of insides or outsides.
class RobotMap {
    public:

        RobotMap();

        // Sets the specified cell to the specified state.
        void setCellValue(const int x, const int y, SquareState state);

        // Sets the specified cell to inside only if it already is unknown.
        void setCellInside(const int x, const int y);

        // Gets the cell value of the specified cell.
        SquareState getCellValue(const int x, const int y) const;

        // Gets all map changes that has happend since clearMapChanges was las called.
        string getMapChanges();

        // Clears all map changes.
        void clearMapChanges();

        // Filters all lines that we cant say are walls.
        void filterLines(vector<pair<Point*, Point*>>* sourceLines,
                         vector<pair<Point*, Point*>>* destLines,
                         float robotX, float robotY, float robotAngle);

        // First filters out lines that does not fit to the grid and then updates
        // the map with the accepted lines.
        void updateMap(vector<pair<Point*, Point*>>* lines, float robotX, float robotY, float robotAngle);

        // Updates the robot position based on where the endpoints of the lines should be.
        void updatePosition(vector<pair<Point*, Point*>>* lines, float* robotX, float* robotY, float* robotAngle);

        // Pathfinding algorithm that uses BFS to search for state with robotX and robotY as starting position.
        // goalX & goalY is the suggested path for the robot to take. goDirect specifies if the robot should take
        // a direct route or follow the grid.
        bool findWayToSquare(float robotX, float robotY, SquareState state, int* goalX, int* goalY, bool goDirect);

        // Calculates if here is a direct path between two points. A direct path is one not obsucred by OUSIDE.
        bool directPathBetween(float startX, float startY, float endX, float endY);

        // Gets a string representation of a RobotMap.
        string toString();

    private:
        SquareState map[MAP_DIMENTIONS][MAP_DIMENTIONS];
        string mapChanges;
};



#endif // ROBOTMAP_H

