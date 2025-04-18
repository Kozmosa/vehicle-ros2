#include "communicator.h"

// --- 平台相关的串口名称 ---
// --- !!! 请根据你的实际情况修改此处的串口号 !!! ---

#if defined (_WIN32) || defined(_WIN64)
    #define SERIAL_PORT "COM1" // Windows 示例
#else
    #define SERIAL_PORT "/dev/ttyUSB0" // Linux/macOS 示例
#endif

const unsigned int BAUD_RATE = 115200; // 波特率，与 STM32 设置一致

int communicator_main() {
    // 1. 创建 serialib 对象
    serialib serial;

    // 2. 打开串口设备
    // 参数: 设备名称, 波特率
    // serialib 会自动处理 8N1 (8数据位, 无校验, 1停止位) 的默认设置
    char errorOpening = serial.openDevice(SERIAL_PORT, BAUD_RATE);

    // 检查串口是否成功打开
    if (errorOpening != 1) {
        std::cerr << "[Error] Cannot open serial port " << SERIAL_PORT
                  << ". Error code: " << (int)errorOpening << std::endl;
        std::cerr << "Possible reasons:\n"
                  << "- Port name is incorrect (Did you change SERIAL_PORT in the code?).\n"
                  << "- Device is not connected or powered.\n"
                  << "- Port is already in use by another application.\n"
                  << "- Insufficient permissions (on Linux, add user to 'dialout' group?)." << std::endl;
        return -1; // 退出程序，表示错误
    }
    std::cout << "[Info] Successfully connected to " << SERIAL_PORT
              << " at " << BAUD_RATE << " bps." << std::endl;

    // 3. 通信主循环
    std::cout << "Enter data to send to STM32 (type 'exit' to quit):" << std::endl;

    while (true) {
        // --- 检查并读取接收到的数据 ---
        // serialib 提供了多种读取方式，这里使用 readString / readBytes
        // readBytes 更通用，可以接收任意字节数据
        char receiveBuffer[256]; // 设定一个接收缓冲区
        int bytesRead = serial.readBytes(receiveBuffer, sizeof(receiveBuffer) - 1, 10); // 读取最多255字节，超时10ms

        if (bytesRead < 0) {
            // 读取发生错误 (非超时)
            std::cerr << "[Error] Error reading from serial port. Code: " << bytesRead << std::endl;
            // 这里可以根据需要添加错误处理逻辑，比如尝试重新连接或直接退出
             break; // 简单起见，直接退出循环
        } else if (bytesRead > 0) {
            // 成功读取到数据
            receiveBuffer[bytesRead] = '\0'; // 将缓冲区末尾置零，以便作为字符串处理（如果确定是文本）
            std::cout << "[Received] Bytes: " << bytesRead << ", Data: " << receiveBuffer << std::endl;
        }

        std::string dataToSend;
        std::cout << "> "; // 提示符
        std::getline(std::cin, dataToSend);

        if (dataToSend == "exit") {
            break; // 用户输入 exit，退出循环
        }

        if (!dataToSend.empty()) {
            // 使用 writeString 发送字符串 (会自动添加空终止符，STM32端需注意处理)
            // 或者使用 writeBytes 发送原始字节数据
            int bytesSent = serial.writeString(dataToSend.c_str());
            // int bytesSent = serial.writeBytes(dataToSend.data(), dataToSend.length()); // 发送原始字节

            if (bytesSent <= 0) {
                std::cerr << "[Error] Failed to write data to serial port." << std::endl;
                 // 这里也可以添加错误处理逻辑
                 break; // 简单起见，直接退出循环
            } else {
                std::cout << "[Sent] " << bytesSent << " bytes: " << dataToSend << std::endl;
            }
        }

        // 短暂延时，避免CPU占用过高（尤其是在没有数据收发时）
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    // 4. 关闭串口
    serial.closeDevice();
    std::cout << "[Info] Serial port closed." << std::endl;

    return 0; // 程序正常退出
}