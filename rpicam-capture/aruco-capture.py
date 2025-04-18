import os
import re
import time
import subprocess
import sys

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

def main(directory):
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