#include "path_planner.h"

// Function to find the default end point (bottom-most, then right-most '1')
Point findDefaultEndPoint(const std::vector<std::vector<int>>& mapMatrix) {
    Point endPoint = {-1, -1};
    int max_y = -1;
    int max_x_at_max_y = -1;
    int rows = mapMatrix.size();
    if (rows == 0) return endPoint;
    int cols = mapMatrix[0].size();
    if (cols == 0) return endPoint;

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (mapMatrix[y][x] == 1) {
                if (y > max_y) {
                    max_y = y;
                    max_x_at_max_y = x;
                } else if (y == max_y && x > max_x_at_max_y) {
                    max_x_at_max_y = x;
                }
            }
        }
    }

    if (max_y != -1) {
        endPoint.x = max_x_at_max_y;
        endPoint.y = max_y;
    }
    return endPoint;
}

/**
 * @brief Converts a path in a map matrix to a sequence of actions.
 *
 * @param mapMatrix The map represented as a 2D vector (0=empty/obstacle, 1=path).
 * @param resolution_mm The size of one grid cell in millimeters.
 * @param startPointOpt Optional starting point. If nullopt or invalid, defaults to (0,0) if it's part of the path.
 * @return A vector of strings representing the action sequence (e.g., "R5", "F10"). Returns an empty vector on error or if no path exists.
 */
std::vector<std::string> generateActionSequence(
    const std::vector<std::vector<int>>& mapMatrix,
    int resolution_mm,
    std::optional<Point> startPointOpt)
{
    std::vector<std::string> actions;
    if (mapMatrix.empty() || mapMatrix[0].empty() || resolution_mm <= 0) {
        std::cerr << "Error: Invalid map matrix or resolution." << std::endl;
        return actions; // Return empty vector for invalid input
    }

    int rows = mapMatrix.size();
    int cols = mapMatrix[0].size();

    // Make a mutable copy or use a visited matrix
    std::vector<std::vector<int>> visitedMap = mapMatrix; // Copy to mark visited cells (e.g., change 1 to 2)

    Point startPoint;
    Point endPoint = findDefaultEndPoint(mapMatrix); // Find the logical end point

    // Validate or determine the start point
    if (startPointOpt.has_value()) {
        startPoint = startPointOpt.value();
        // Check if provided start point is valid and on the path
        if (startPoint.y < 0 || startPoint.y >= rows || startPoint.x < 0 || startPoint.x >= cols || mapMatrix[startPoint.y][startPoint.x] != 1) {
            std::cerr << "Error: Provided start point (" << startPoint.x << "," << startPoint.y << ") is invalid or not on the path." << std::endl;
            return actions; // Return empty if start is invalid
        }
    } else {
        // Default start: (0,0)
        startPoint = {0, 0};
        if (mapMatrix[0][0] != 1) {
             std::cerr << "Error: Default start point (0,0) is not on the path." << std::endl;
             // Optional: Could search for the first '1' as an alternative default
            return actions; // Return empty if default start is not on path
        }
    }

     // Check if an end point was found
    if (endPoint.x == -1) {
         std::cerr << "Error: No path endpoint (no '1's) found in the map." << std::endl;
        return actions;
    }

     // Check if start and end are the same (path of length 1)
     if (startPoint == endPoint) {
         std::cout << "Warning: Start point is the same as the end point." << std::endl;
         // No movement needed, return empty action list.
         return actions;
     }


    Point currentPos = startPoint;
    visitedMap[currentPos.y][currentPos.x] = 2; // Mark start as visited

    char currentDirection = '\0'; // No initial direction
    int stepsInCurrentDirection = 0;

    while (currentPos != endPoint) {
        Point nextPos = {-1, -1};
        char moveDirection = '\0';

        // Define potential moves (relative grid changes) and corresponding directions
        // Order matters: Determines path choice at branches. Let's use R, F, L, B (relative to grid axes)
        int dx[] = {1, 0, -1, 0}; // Change in x: Right, Stay, Left, Stay
        int dy[] = {0, 1, 0, -1}; // Change in y: Stay, Down(Fwd), Stay, Up(Bwd)
        char directionChars[] = {'R', 'F', 'L', 'B'}; // Commands

        bool foundNext = false;
        for (int i = 0; i < 4; ++i) {
            int checkX = currentPos.x + dx[i];
            int checkY = currentPos.y + dy[i];

            // Check bounds
            if (checkX >= 0 && checkX < cols && checkY >= 0 && checkY < rows) {
                // Check if it's an unvisited path cell
                if (visitedMap[checkY][checkX] == 1) {
                    nextPos = {checkX, checkY};
                    moveDirection = directionChars[i];
                    foundNext = true;
                    break; // Found the next step
                }
            }
        }

        if (!foundNext) {
            // If we haven't reached the end but can't find a next step, the path is broken
            // or the endpoint definition is inconsistent with the path connectivity.
             std::cerr << "Error: Path broken or end point unreachable from current position ("
                       << currentPos.x << "," << currentPos.y << ")" << std::endl;
             // Optionally return actions generated so far, or clear it. Let's clear it.
            actions.clear();
            return actions;
        }

        // Process the move
        if (stepsInCurrentDirection == 0) { // First move or direction just changed
            currentDirection = moveDirection;
            stepsInCurrentDirection = 1;
        } else if (moveDirection == currentDirection) { // Continue in the same direction
            stepsInCurrentDirection++;
        } else { // Direction changed
            // Add the completed command for the previous direction
            actions.push_back(std::string(1, currentDirection) + std::to_string(stepsInCurrentDirection * resolution_mm));
            // Reset for the new direction
            currentDirection = moveDirection;
            stepsInCurrentDirection = 1;
        }

        // Update position and mark as visited
        currentPos = nextPos;
        visitedMap[currentPos.y][currentPos.x] = 2; // Mark as visited
    }

    // Add the last command segment after reaching the end point
    if (stepsInCurrentDirection > 0) {
        actions.push_back(std::string(1, currentDirection) + std::to_string(stepsInCurrentDirection * resolution_mm));
    }

    return actions;
}

// Helper function to print the action list
void printActions(const std::vector<std::string>& actions) {
    if (actions.empty()) {
        std::cout << "No actions generated." << std::endl;
        return;
    }
    std::cout << "Generated Actions: [";
    for (size_t i = 0; i < actions.size(); ++i) {
        std::cout << "\"" << actions[i] << "\"";
        if (i < actions.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

// --- Example Usage ---
int path_planner_test_main() {
    // Example Map 1: Simple L-shape path
    // 1 1 1 0
    // 0 0 1 0
    // 0 0 1 1 <-- End point (3, 2)
    // 0 0 0 0
    std::vector<std::vector<int>> map1 = {
        {1, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 0}
    };
    int resolution1 = 5; // 5mm per grid cell

    std::cout << "--- Example 1: Start from default (0,0) ---" << std::endl;
    std::vector<std::string> actions1_default = generateActionSequence(map1, resolution1);
    // Expected: R5, R5, F5, F5, R5 (Total: R10, F10, R5)
    printActions(actions1_default);
    std::cout << std::endl;


    std::cout << "--- Example 1: Start from intermediate point (2,1) ---" << std::endl;
    Point start1_custom = {2, 1}; // This is map1[1][2]
    std::vector<std::string> actions1_custom = generateActionSequence(map1, resolution1, start1_custom);
    // Expected path from (2,1): (2,2) -> (3,2). Actions: F5, R5
    printActions(actions1_custom);
    std::cout << std::endl;

     std::cout << "--- Example 1: Start from invalid point (1,1) ---" << std::endl;
    Point start1_invalid = {1, 1}; // This is map1[1][1] which is 0
    std::vector<std::string> actions1_invalid = generateActionSequence(map1, resolution1, start1_invalid);
    printActions(actions1_invalid); // Should print error and "No actions"
    std::cout << std::endl;


    // Example Map 2: More complex path
    // 1 0 0 0 0
    // 1 1 1 0 0
    // 0 0 1 1 1
    // 0 0 0 0 1  <-- End Point (4, 3)
    std::vector<std::vector<int>> map2 = {
        {1, 0, 0, 0, 0},
        {1, 1, 1, 0, 0},
        {0, 0, 1, 1, 1},
        {0, 0, 0, 0, 1}
    };
    int resolution2 = 10; // 10mm per grid cell

    std::cout << "--- Example 2: Start from default (0,0) ---" << std::endl;
    std::vector<std::string> actions2_default = generateActionSequence(map2, resolution2);
    // Expected path: (0,0)->(0,1)->(1,1)->(2,1)->(2,2)->(3,2)->(4,2)->(4,3)
    // Expected actions: F10, R10, R10, F10, R10, R10, F10
    printActions(actions2_default);
    std::cout << std::endl;

    std::cout << "--- Example 2: Start from intermediate point (2,2) ---" << std::endl;
    Point start2_custom = {2, 2}; // This is map2[2][2]
    std::vector<std::string> actions2_custom = generateActionSequence(map2, resolution2, start2_custom);
    // Expected path from (2,2): (3,2)->(4,2)->(4,3)
    // Expected actions: R10, R10, F10
    printActions(actions2_custom);
    std::cout << std::endl;


    // Example Map 3: No path
     std::vector<std::vector<int>> map3 = {
        {1, 1, 1},
        {0, 0, 1}
    };
     std::cout << "--- Example 3: No path ---" << std::endl;
    std::vector<std::string> actions3 = generateActionSequence(map3, 5);
    printActions(actions3); // Should print error and "No actions"
    std::cout << std::endl;

    // Example Map 4: Start not at (0,0)
    std::vector<std::vector<int>> map4 = {
        {1, 0, 0},
        {0, 1, 1}, // Start should be (1,1) if default (0,0) fails
        {0, 0, 1}  // End (2,2)
    };
     std::cout << "--- Example 4: Start not at (0,0), default start ---" << std::endl;
    std::vector<std::string> actions4_default = generateActionSequence(map4, 5); // Will fail as default (0,0) is not 1
    printActions(actions4_default);
    std::cout << std::endl;

     std::cout << "--- Example 4: Start not at (0,0), specified start (1,1) ---" << std::endl;
    Point start4_custom = {1, 1}; // map4[1][1]
    std::vector<std::string> actions4_custom = generateActionSequence(map4, 5, start4_custom); // Should work
    // Path: (1,1) -> (2,1) -> (2,2)
    // Actions: R5, F5
    printActions(actions4_custom);
    std::cout << std::endl;

    return 0;
}