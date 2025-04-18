import os
import sys
import argparse
import subprocess
import signal
import time
import atexit

#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
主控程序，用于启动和管理车辆导航系统相关进程
"""

"""
# 导入aruco-capture模块
sys.path.insert(0, './rpicam-capture')
try:
    from aruco_capture import *  # 导入aruco模块
except ImportError:
    print("无法导入aruco_capture模块，请确认路径是否正确")
    sys.exit(1)
"""

# 存储已启动的进程
active_processes = []

def parse_arguments():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(description="机甲杯小分队 车辆导航系统控制程序 v1.1")
    parser.add_argument("-a", "--aruco", help="设置ARUCO图像目录", default="./")
    return parser.parse_args()

def run_command(command):
    """执行shell命令并返回进程对象"""
    print(f"执行命令: {command}")
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    active_processes.append(process)
    return process

def kill_all_processes():
    """终止所有已启动的进程"""
    for process in active_processes:
        try:
            pid = process.pid
            os.kill(pid, signal.SIGKILL)
            print(f"已终止进程 PID: {pid}")
        except (ProcessLookupError, PermissionError):
            pass
    
    # 确保所有相关进程都被终止
    process_names = ["aruco-capture.py", "navigation"]
    check_and_kill_processes(process_names)

def check_and_kill_processes(process_names):
    """检查并杀死所有相关进程"""
    for name in process_names:
        # 循环直到所有匹配进程都被终止
        while True:
            # 检查进程是否存在
            check_cmd = f"ps aux | grep '{name}' | grep -v grep"
            result = subprocess.run(check_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            
            if not result.stdout.strip():
                print(f"没有找到相关进程: {name}")
                break
            
            # 提取PID并终止进程
            lines = result.stdout.strip().split('\n')
            for line in lines:
                parts = line.split()
                if len(parts) > 1:
                    pid = parts[1]
                    print(f"发现进程: {line}")
                    
                    # 尝试普通权限终止
                    kill_cmd = f"kill -9 {pid}"
                    subprocess.run(kill_cmd, shell=True)
                    
                    # 检查是否成功终止
                    time.sleep(0.2)
                    check_result = subprocess.run(
                        f"ps -p {pid} > /dev/null 2>&1", 
                        shell=True
                    )
                    
                    # 如果仍然存在，尝试使用sudo
                    if check_result.returncode == 0:
                        print(f"普通权限终止失败，尝试使用sudo终止进程: {pid}")
                        subprocess.run(f"sudo kill -9 {pid}", shell=True)

def main():
    """主函数"""
    # 解析命令行参数
    args = parse_arguments()
    
    # 设置ARUCO_IMAGE_DIR
    aruco_image_dir = args.aruco
    os.environ["ARUCO_IMAGE_DIR"] = aruco_image_dir
    print(f"已设置ARUCO_IMAGE_DIR={aruco_image_dir}")
    
    # 注册退出处理函数
    atexit.register(kill_all_processes)
    
    try:
        # 初始化环境
        print("初始化环境...")
        run_command("uv python pin 3.9")
        
        # 启动图片捕获进程
        print("启动图片捕获进程...")
        capture_cmd = f"uv run ./rpicam-capture/aruco-capture.py {aruco_image_dir}"
        capture_process = run_command(capture_cmd)
        
        # 启动导航进程
        print("启动导航进程...")
        navigation_cmd = "./bin/navigation"
        navigation_process = run_command(navigation_cmd)
        
        print("\n系统已启动")
        print("按下ESC键退出程序...")
        
        # 等待ESC键被按下
        while True:
            try:
                # 使用系统命令读取单个键盘输入
                cmd = 'bash -c "read -n 1 key; [[ $key = $\'\\e\' ]] && exit 1 || exit 0"'
                if subprocess.call(cmd, shell=True) != 0:
                    print("检测到ESC键，开始终止所有进程...")
                    break
            except KeyboardInterrupt:
                print("\n检测到Ctrl+C，开始终止所有进程...")
                break
            
    except Exception as e:
        print(f"发生错误: {e}")
    
    finally:
        # 终止所有进程
        kill_all_processes()
        print("所有进程已终止，程序退出")

if __name__ == "__main__":
    main()