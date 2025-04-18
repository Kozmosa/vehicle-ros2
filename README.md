# Vehicle Project README

This project is a project developed in Python(for some utilities& useful tool scripts) and C++(core module for navigation and map building) by Yuting Yang(SSE, School of Software Engineering) and Yang Xu(SMAE, School of Mechanic and Automotive Engineering) @ SCUT(South China University of Technology).

We aimed to implement autonomous navigation, stable and robust communication with STM32F103C8T6 vehicle driver board, autonomous direction controlling for vehicle functions in this project.

The project is comprised by following parts:
- Map building module
- Auto navigation module
- UART communicating module
- Motion controller module
- ARUCO locating module

# Camera calibration

We employed aruco-capture(python + shell) module and calibration(cpp) module to implement camera calibration coefficients calculation.

- aruco-capture: capture pictures using rpicam-jpeg shell command and save pictures to file system for calibration. 
- calibration: calibration using opencv integrated calibration module, employing 14+ pictures from filesystem taken by aruco-capture module.

# Map building

# Auto navigation

# UART communicating

First we install uart library for raspi.
```bash
sudo apt install wiringpi python3-serial minicom
```

Then we enable the serial communication function for raspi.

2、串口设置
树莓派包含两个串口（树莓派3及以前版本）
1.硬件串口（/dev/ttyAMA0）,硬件串口由硬件实现，有单独的波特率时钟源，性能高，可靠。一般优先选择这个使用。
2.mini串口（/dev/ttyS0），mini串口时钟源是由CPU内核时钟提供，波特率受到内核时钟的影响，不稳定。

树莓派3及以前版本仅2个串口，4和400有4个串口，cm系列有6个串口，详见 树莓派官网Configuring UARTs）
　
树莓派4开启 ttyAMA1，可以直接配置 dtoverlay=uart2 即可（其他 ttyAMA2 -> uart3, ttyAMA3 -> uart4, ttyAMA4 -> uart5，具体引脚配置可以通过 dtoverlay -h uartN 查看）。
　
注意 CM4 使用双相机的dtb配置（ 树莓派计算模块CM4 eMMC系统烧写、配置、相机连接）时，ttyAMA1会失效。

想要通过树莓派的GPIO引脚进行稳定的串口通信，需要修改串口的映射关系。
serial0是GPIO引脚对应的串口，serial1是蓝牙对应的串口，默认未启用serial0。使用ls -l /dev/serial*查看当前的映射关系：

可以看到这里是，蓝牙串口serial1使用硬件串口ttyAMA0。

2.1 开启GPIO串口功能，并使用硬件串口
使用sudo raspi-config 进入图形界面
选择菜单 Interfacing Options -> P6 Serial,
第一个选项（would you like a login shell to be accessible over serial?）选择 NO，
第二个选项（would you like the serial port hardware to be enabled?）选择 YES

保存后重启，查看映射关系

比之前多了一个gpio的串口serial0，并且使用的ttyS0。这里已经是开启了GPIO串口功能，但是使用的cpu实现的软件串口。

如果想使用稳定可靠的硬件串口，就要将树莓派3b+的硬件串口与mini串口默认映射对换（先禁用蓝牙 sudo systemctl disable hciuart）。

在/boot/config.txt文件末尾添加一行代码 dtoverlay=pi3-miniuart-bt （树莓派4B也使用这个命令）。 还可以直接配置禁用bluetooth，代码为 dtoverlay=disable-bt，见参考链接 【树莓派 功能配置（含网络）不定期更新】。

保存后重启再查看设备对用关系，发现已经调换成功。


2.2 禁用串口的控制台功能
前面步骤已经交换了硬件串口与mini串口的映射关系，但是，现在还不能使用树莓派串口模块与电脑进行通信，因为，树莓派gpio口引出串口默认是用来做控制台使用的，即是为了用串口控制树莓派，而不是通信。所以我们要禁用此默认设置。
首先执行命令如下：
sudo systemctl stop serial-getty@ttyAMA0.service
sudo systemctl disable serial-getty@ttyAMA0.service
然后执行命令行修改文件：
sudo nano /boot/cmdline.txt
并删除语句console=serial0,115200（没有的话就不需要此步骤）

# Motion controller

# ARUCO locating

# Thanks

We have to say thanks to our teammates who have devoted a lot in the vehicle platform building work. They are:
- Yongxi Ke: help developed the mechanic structure and the electronic controller system.
- Yuxi Peng: leading the electronic controller system designing and engineering work in the team, he designed the whole electronic controller board and the embedded system on STM32F103C8T6 MCU.
- Yikai Huang: leading the mechanic structure engineering work for vehicle platform. also, he helped building the vehicle and updatig structure solution along the development timeline.

Without their devotion and dedicated work, we cannot make our vision and auto navigation project run on the real vehicle platform, while we cannot finish the Robot Contest, too.

Thanks for reading this documentation.

If you have issues or suggestions for this project, please just send us issues or fork the project, fix the problem and send us a Pull Request. I will cherish every contribution made to this small project!

Kozmosa(Yang Xu)
Apr 4th, 2025.