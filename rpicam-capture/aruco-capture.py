import os
import re
import time
import subprocess
import sys
import cv2
import numpy as np


def find_max_index(directory):
    """
    Find the maximum numeric index from JPEG files named with pure numbers in the given directory.
    Returns 0 if no such files exist.
    """
    max_index = 0
    pattern = re.compile(r'^(\d+)\.jpg$')

    try:
        for filename in os.listdir(directory):
            match = pattern.match(filename)
            if match:
                index = int(match.group(1))
                max_index = max(max_index, index)
    except FileNotFoundError:
        # Directory doesn't exist
        pass

    return max_index


def undistort_image(image_path, cameraMatrix, distCoeffs):
    """
    对指定路径的图像进行畸变校正并覆盖原图像
    """
    image = cv2.imread(image_path)
    if image is not None:
        undistorted_image = cv2.undistort(image, cameraMatrix, distCoeffs)
        cv2.imwrite(image_path, undistorted_image)


def main(directory):
    # 相机内参矩阵
    cameraMatrix = np.array([
        [1.34402339e+04, 0.00000000e+00, 1.26623344e+03],
        [0.00000000e+00, 1.37063304e+04, 1.34757003e+03],
        [0.00000000e+00, 0.00000000e+00, 1.00000000e+00]
    ])

    # 畸变系数矩阵
    distCoeffs = np.array([
        -4.84008189e+00,
        -6.29809057e+02,
        -2.35117925e-01,
        -7.46205746e-02,
        -2.09795524e+00
    ])

    # Change working directory to the given path
    try:
        os.chdir(directory)
        print(f"Working directory changed to: {directory}")
    except FileNotFoundError:
        print(f"Directory not found: {directory}")
        return

    # Initialize counter based on existing files
    curr_index = find_max_index(directory)
    print(f"Starting with index: {curr_index}")

    try:
        while True:
            # Increment counter for new image
            curr_index += 1

            # Execute command to capture image
            capture_cmd = f"rpicam-jpeg -o {curr_index}.jpg --timeout 1"
            print(f"Executing: {capture_cmd}")

            try:
                subprocess.run(capture_cmd, shell=True, check=True)
            except subprocess.CalledProcessError as e:
                print(f"Error executing command: {e}")

            # 对拍摄的图像进行畸变校正
            image_path = f"{curr_index}.jpg"
            undistort_image(image_path, cameraMatrix, distCoeffs)

            # Wait for 500ms
            time.sleep(0.5)

    except KeyboardInterrupt:
        print("\nProgram terminated by user.")


if __name__ == "__main__":

    if len(sys.argv) > 1:
        directory_path = sys.argv[1]
    else:
        directory_path = os.getcwd()  # Use current directory if none specified

    main(directory_path)
    