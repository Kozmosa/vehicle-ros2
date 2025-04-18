//
// Created by 27866 on 25-4-11.
//

#ifndef PATH_PLANNER_H
#define PATH_PLANNER_H

#include <vector>
#include <string>
#include <iostream>
#include <stdexcept> // For error handling
#include <utility> // For std::pair
#include <optional> // To represent optional start coordinates

// Structure to hold coordinates
struct Point {
    int x = -1;
    int y = -1;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

Point findDefaultEndPoint(const std::vector<std::vector<int>>& mapMatrix);

std::vector<std::string> generateActionSequence(
    const std::vector<std::vector<int>>& mapMatrix,
    int resolution_mm,
    std::optional<Point> startPointOpt = std::nullopt);

void printActions(const std::vector<std::string>& actions);

int path_planner_test_main();

#endif //PATH_PLANNER_H