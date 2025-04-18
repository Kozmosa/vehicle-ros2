from collections import deque

def find_path(matrix, start, end):
    queue = deque([(start, [start])])
    visited = set([start])

    while queue:
        current, path = queue.popleft()
        if current == end:
            return path
        x, y = current
        for dx, dy in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
            nx, ny = x + dx, y + dy
            if 0 <= nx < len(matrix) and 0 <= ny < len(matrix[0]) and matrix[nx][ny] == 1 and (nx, ny) not in visited:
                visited.add((nx, ny))
                queue.append(((nx, ny), path + [(nx, ny)]))
    return None


def path_to_actions(path, resolution):
    actions = []
    directions = {
        (-1, 0): 'F',
        (1, 0): 'B',
        (0, -1): 'L',
        (0, 1): 'R'
    }
    for i in range(1, len(path)):
        prev = path[i - 1]
        curr = path[i]
        dx = curr[0] - prev[0]
        dy = curr[1] - prev[1]
        direction = directions.get((dx, dy), '')
        if direction:
            actions.append(f"{direction}{resolution}")
    return actions


# 示例使用
matrix = [
    [1, 1, 0, 1],
    [1, 0, 1, 0],
    [1, 1, 1, 1],
    [0, 1, 0, 1]
]

start = (0, 0)
end = (3, 3)
resolution = 5

path = find_path(matrix, start, end)
actions = path_to_actions(path, resolution)
print(actions)