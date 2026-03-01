# raspberrypi-ros2-car

基于树莓派4B的ROS2自动定位与导航小车，能够实现激光雷达SLAM建图、自主定位、路径规划和自主导航等功能。

****

**⭐ 欢迎提出Issues和PR，如果这个项目对你有帮助，请给个 Star！**

## 📑 目录

- [📖 项目简介](#-项目简介)
- [🏗️ 系统架构](#️-系统架构)
- [📁 项目结构](#-项目结构)
- [🛠️ 硬件配置](#️-硬件配置)
  - [1. 核心硬件](#1-核心硬件)
  - [2. 机械参数](#2-机械参数)
- [🔧 环境配置](#-环境配置)
  - [1. 树莓派环境](#1-树莓派环境)
  - [2. 虚拟机环境](#2-虚拟机环境)
  - [3. ROS2开发环境](#3-ros2开发环境)
  - [4. STM32开发环境](#4-stm32开发环境)
- [📦 编译与部署](#-编译与部署)
  - [1. STM32固件编译](#1-stm32固件编译)
  - [2. 树莓派工作空间](#2-树莓派工作空间)
  - [3. 虚拟机工作空间](#3-虚拟机工作空间)
- [🎮 使用说明](#-使用说明)
  - [1. 启动底盘和雷达](#1-启动底盘和雷达)
  - [2. 启动遥控/建图/导航](#2-启动遥控建图导航)
- [📧 联系方式](#-联系方式)

## 📖 项目简介

这是一个基于 **ROS2（机器人操作系统2）** 的自主导航智能小车项目，采用树莓派4B作为主控平台，结合STM32单片机进行底层硬件控制。小车能够实现激光雷达SLAM建图、自主定位、路径规划和自主导航等功能。   

**主要功能**：

- **STM32 底盘运动控制**

  - ✅ TB6612双路电机PWM速度控制

  - ✅ 编码器速度反馈（左右轮独立测速）

  - ✅ MPU6050 DMP姿态解算（提供角速度Z轴）

  - ✅ OLED实时显示传感器数据

  - ✅ 串口通信协议（与树莓派交互）


- **树莓派 ROS2 中继通信**

  - ✅ 差速驱动运动学模型

  - ✅ 里程计（Odometry）发布

  - ✅ TF坐标变换发布（odom → base_footprint → base_link → laser_link）

  - ✅ 速度指令处理（/cmd_vel、/key_control）

  - ✅ 多线程串口通信


- **SLAM 建图**

  - ✅ Cartographer 2D激光SLAM

  - ✅ 实时地图构建

  - ✅ 地图保存功能


- **自主导航**

  - ✅ Nav2导航栈

  - ✅ AMCL自适应蒙特卡洛定位

  - ✅ 全局路径规划

  - ✅ 局部路径规划与避障

  - ✅ RViz2可视化界面


- **手动遥控**

  - ✅ 键盘控制（WSAD按键）

  - ✅ 实时状态反馈

**视频演示**：

  - [基于ROS2的树莓派自主定位与导航小车_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1FKxTeVE5m?spm_id_from=333.788.recommend_more_video.0&trackid=web_related_0.router-related-2479604-dplt2.1771923917238.231&vd_source=b2115ccf4e995ec04004dc3600b1cecb)
  - [基于ROS2的树莓派自主定位与导航小车PLUS_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1gtuqzwESE/?spm_id_from=333.1387.upload.video_card.click&vd_source=b2115ccf4e995ec04004dc3600b1cecb) 

## 🏗️ 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                    虚拟机（Ubuntu 24.04）                   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ 键盘控制节点  │  │ Cartographer │  │    Nav2      │     │
│  │key_control   │  │    SLAM      │  │  自主导航     │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│         │                  │                   │            │
│         └──────────────────┴───────────────────┘            │
│                            │ /key_control、/cmd_vel         │
└────────────────────────────┼───────────────────────────────┘
                             │ ROS2 网络通信
┌────────────────────────────┼───────────────────────────────┐
│                  树莓派4B（Ubuntu 24.04）                   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │  car_node    │  │  lidar_node  │  │   TF树发布   │     │
│  │  底盘控制    │  │  激光雷达    │  │   里程计     │     │
│  └──────┬───────┘  └──────────────┘  └──────────────┘     │
│         │ 串口通信（115200）                                │
└─────────┼─────────────────────────────────────────────────┘
          │
┌─────────┼─────────────────────────────────────────────────┐
│    STM32F103C8T6（底层控制固件）                            │
│  ┌──────┴───────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   串口通信    │  │   电机控制   │  │   传感器读取  │     │
│  │  指令解析    │  │   TB6612     │  │  编码器+IMU   │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│         │                  │                   │            │
│         └──────────────────┴───────────────────┘            │
│              ┌─────────────┐                                │
│              │    OLED     │                                │
│              │  状态显示    │                                │
│              └─────────────┘                                │
└─────────────────────────────────────────────────────────────┘
```

## 📁 项目结构

```
raspberrypi-ros2-car/
├── stm32_base/               # STM32底层控制代码
│   └── ros2_car/
│       ├── Core/             # HAL库核心代码
│       ├── Devices/          # 外设驱动库
│       │   ├── TB6612/       # 电机驱动
│       │   ├── Encoder/      # 编码器
│       │   ├── MPU6050/      # 六轴传感器（DMP）
│       │   └── OLED/         # OLED显示
│       └── Drivers/          # STM32 HAL驱动
│
├── workspace/
│   ├── raspberrypi_ws/       # 树莓派工作空间
│   │   └── src/
│   │       ├── car_pkg/      # 小车底盘功能包
│   │       │   ├── car_pkg/
│   │       │   │   └── car_node.py         # 底盘控制节点
│   │       │   └── launch/
│   │       │       └── car_base.launch.py  # 底盘+雷达启动
│   │       └── cspc_lidar_sdk_ros2/        # 激光雷达SDK
│   │
│   └── vmware_ws/            # 虚拟机工作空间
│       └── src/
│           └── car_pkg/      # 上层控制功能包
│               ├── car_pkg/
│               │   └── key_control_node.py  # 键盘控制节点
│               ├── launch/
│               │   ├── car_display.launch.py   # 小车模型显示
│               │   ├── car_mapping.launch.py   # SLAM建图
│               │   └── car_nav.launch.py       # 自主导航
│               ├── urdf/                       # 小车模型描述
│               ├── config/                     # Cartographer配置
│               ├── params/                     # Nav2参数
│               └── maps/                       # 保存的地图
│
└── README.md
```

## 🛠️ 硬件配置

### 1. 核心硬件

- **主控板**：树莓派4B（Ubuntu 24.04 + ROS2 Jazzy）
- **底层控制器**：STM32F103C8T6
- **激光雷达**：国科光芯 COIN-D6 单线激光雷达
- **电机驱动**：TB6612FNG双路直流电机驱动模块
- **传感器**：
  - MPU6050 六轴姿态传感器（陀螺仪+加速度计）
  - 轮式编码器
- **显示屏**：0.96寸OLED显示屏
- **电源**：根据实际配置调整

### 2. 机械参数

- 轮径：65mm

- 编码器转速：12V 330转/分

- 底盘半径：96mm

![小车外观1](images/1.png)   

![小车外观2](images/2.png)



## 🔧 环境配置

### 1. 树莓派环境
- 准备好 `树莓派4B+电源适配器`、`sd卡（推荐闪迪32G以上）`、`读卡器`、`USB转串口工具`。

- 下载并安装 `Pi Imager` 工具：[Pi Imager](https://www.raspberrypi.com/software/)。

- 下载并安装 `IP Scanner` 工具：[IP Scanner](https://angryip.org/download/#windows)。

- 下载并解压 `MobaXterm` 工具：[MobaXterm](https://mobaxterm.mobatek.net/download-home-edition.html)。 

- 将 `sd卡` 插入读卡器并连接电脑，打开 `Pi Imager` 烧写镜像：

  - `Device` 选择 `Raspberry Pi 4`，点击下一步。
    ![Pi Imager设备选择](images/3.png)

  - `OS` 选择 `Other general purpose OS` -> `Ubuntu` -> `Ubuntu Server 24.04.3 LTS (64-bit)`，点击下一步。
    ![系统选择](images/4.png)

  - `存储设备` 选择 `挂载的sd卡`, 点击下一步。
    ![存储设备选择](images/5.png)

  - `Customisation` 设置 `用户名和密码` 以及 `WiFi名称和密码（必须和电脑连接同一WiFi）` 并开启 `SSH` 功能，点击下一步。
    ![自定义设置](images/6.png)

  - `Writing` 点击 `Write`，等待烧写完成。
    ![烧写镜像](images/7.png)

- 烧写完成后，弹出 `sd卡` 并插入树莓派4B，接上电源适配器并开机。

- 打开 `IP Scanner` 工具，IP地址范围设置为 `192.168.<你的WiFi网段>.1~254`（WiFi网段可通过cmd命令 `ipconfig` 查看），点击开始扫描，能ping通且主机名为你设置的用户名即为 `树莓派IP`。
  ![IP扫描](images/8.png)

- 打开 `MobaXterm` 工具，`Session` 选择 `SSH`，填写 `树莓派IP`，端口号设置为 `22`，填写 `你设置的用户名`，点击 `OK`，输入密码连接成功。
  ![SSH连接](images/9.png)

- 连接成功后，更换系统源：

  - 输入以下命令：

    ```
    wget http://fishros.com/install -O fishros && . fishros # 鱼香ROS开源的一键安装工具
    ```

  - 输入密码后选择 `[5]一键配置：系统源`。

    ![配置系统源](images/10.png)

  - 接着选择 `[2]:更换系统源并清理第三方源`。

    ![更换系统源](images/11.png)

  - 完成后选择 `[2]:不添加ROS/ROS2源`。

- 更新软件包：

  - 输入以下命令：

    ```
    sudo apt update && sudo apt upgrade -y
    ```

### 2. 虚拟机环境

- 下载并安装 `VMWare` 工具：[安装虚拟机（VMware）保姆级教程（附安装包）_vmware虚拟机-CSDN博客](https://blog.csdn.net/weixin_74195551/article/details/127288338)。

- 安装并配置 `Ubuntu24.04`：

  - 下载 `Ubuntu24.04` 镜像：[Ubuntu24.04](https://mirrors.ustc.edu.cn/ubuntu-releases/24.04/)。

    ![Ubuntu镜像下载](images/12.png)

  - 新建并配置 `Ubuntu24.04` 虚拟机：`内存8GB` `处理器2x2`。

- 配置虚拟网络：

  - 退出VMware，右键选择 `以管理员身份运行`。

  - 点击新建的 `Ubuntu24.04` 虚拟机 -> 编辑虚拟机设置 -> 网络适配器 -> 将网络连接设置为 `桥接模式` -> 确定。
    ![桥接模式设置](images/13.png)

  - 点击编辑 -> 虚拟网络编辑器 -> 设置桥接的外部网络为 `WiFi` -> 应用和确定。
    ![网络编辑器](images/14.png)

- 启动 `Ubuntu24.04` 虚拟机。

- 安装VMWare Tools：

  - 输入以下命令：

    ```
    sudo apt-get install open-vm-tools
    sudo apt-get install open-vm-tools-desktop
    ```

- 安装完成后，更换系统源：

  - 输入以下命令：

    ```
    wget http://fishros.com/install -O fishros && . fishros # 鱼香ROS开源的一键安装工具
    ```

  - 输入密码后选择 `[5]一键配置：系统源`。

    ![配置系统源](images/10.png)

  - 接着选择 `[2]:更换系统源并清理第三方源`。

    ![更换系统源](images/11.png)

  - 完成后选择 `[2]:不添加ROS/ROS2源`。

- 更新软件包：

  - 输入以下命令：

    ```
    sudo apt update && sudo apt upgrade -y
    ```

### 3. ROS2开发环境

- **虚拟机**中安装 `VSCode` 工具：[Linux环境下的Ubuntu虚拟机安装VScode超详解-CSDN博客](https://blog.csdn.net/qq_45009309/article/details/136577582)

- 打开 `VSCode`，点击左下角的 `扩展` -> 搜索 `Remote-SSH` -> 点击安装。
  ![Remote-SSH插件](images/16.png)

- 点击左下角的 `打开远程` -> 选择 `SSH`，输入 `ssh <树莓派主机名称>@<树莓派IP>` -> 点击连接 ->输入密码 -> 点击确定。

- 连接成功后可以访问树莓派的终端和文件系统。
  ![远程连接成功](images/17.png)

- **虚拟机和树莓派**中相同步骤安装相同版本的**ROS2（Jazzy）**：

  - 输入以下命令：

    ```
    wget http://fishros.com/install -O fishros && . fishros # 鱼香ROS开源的一键安装工具
    ```

  - 选择 `[1]:一键安装:ROS `。
    ![安装ROS](images/18.png)

  - 选择 `[2]:不更换系统源继续安装`。
    ![继续安装](images/19.png)

  - 选择 `[1]:中科大镜像源`。
    ![镜像源选择](images/20.png)

  - 选择 `[1]:Jazzy(ROS2)` 并选择 `桌面版`。
    ![ROS2版本选择](images/21.png)

  - 安装完成在终端输入 `ros2` 验证是否安装成功。

- **虚拟机**中安装相关依赖：

  - **虚拟机**中新建终端运行：

    ```bash
    sudo apt-get update
    
    # 安装机器人模型依赖
    sudo apt install ros-jazzy-joint-state-publisher-gui 
    sudo apt install ros-jazzy-robot-state-publisher
    
    # 安装 Cartographer 依赖
    sudo apt-get install -y ros-jazzy-cartographer-*
    
    # 安装 Nav2 依赖
    sudo apt install -y ros-jazzy-navigation2
    sudo apt install -y ros-jazzy-nav2-bringup
    ```

- **树莓派**中USB设备号固定别名：

  - 虚拟机中远程访问树莓派终端运行：

    ```bash
    # 查看USB转TTL的ID
    lsusb
    
    # 输出示例：
    # Bus 001 Device 014: ID 1a86:7523 QinHeng Electronics CH340 serial converter
    
    # 创建并编写规则文件
    # 激光雷达固定别名
    sudo touch /etc/udev/rules.d/laser_scan.rules
    sudo nano /etc/udev/rules.d/laser_scan.rules
    
    # 修改以下内容中的"1a86"和"7523"为实际的USB设备ID并复制到文件中保存
    KERNEL=="ttyUSB0", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", MODE:="0777", SYMLINK+="laser_scan"
    
    # stm32固定别名
    sudo touch /etc/udev/rules.d/stm32_dev.rules
    sudo nano /etc/udev/rules.d/stm32_dev.rules
    
    # 修改以下内容中的"1a86"和"7523"为实际的USB设备ID并复制到文件中保存
    KERNEL=="ttyUSB1", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", MODE:="0777", SYMLINK+="stm32_dev"
    
    # 重新加载和应用规则
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    
    # 检查别名是否生效
    ls -l /dev/ | grep laser_scan
    ls -l /dev/ | grep stm32_dev
    ```

  - **注意**：USB设备ID需要固定，并且要确保上电后先插入激光雷达USB（ttyUSB0），再插入STM32USB（ttyUSB1），否则每次重启树莓派都需要重新设置别名。

### 4. STM32开发环境

- 下载并安装：[STM32CubeIDE](https://www.st.com.cn/zh/development-tools/stm32cubeide.html)。
- 汉化教程：[STM32CubeIDE配置及汉化教程](https://blog.csdn.net/c858845275/article/details/144321873)。

- 新建工程并配置：
  - 选择 `文件` -> `新建` -> `STM32 Project`。

  - 选择芯片型号 `STM32F103C8T6`，点击下一步。

  - 填写项目名称 `ros2_car`，其他选项保持默认，点击完成。

  - 打开 `Pinout & Configuration` 选项卡：

    - 点击 `RCC`，将 `HSE` 设置为 `Cystal/Ceramic Resonator`。

    - 点击 `SYS`，将 `DEBUG` 设置为 `Serial Wire`。

  - 打开 `Clock Configuration` 选项卡：

    - 选择 `HSE` 频率为 `8MHz`。

    - 将 `SYSCLK` 设置为 `72MHz`。
      ![时钟配置](images/15.png)

## 📦 编译与部署

### 1. STM32固件编译
- 使用 STM32CubeIDE 打开 `stm32_base/ros2_car` 项目
- 编译项目：`Project → Build All`
- 烧录固件：连接ST-Link，点击 `Run → Debug`

### 2. 树莓派工作空间

- 虚拟机中远程访问树莓派终端运行：

    ```bash
    # 克隆项目
    cd ~
    git clone https://github.com/CaddonThaw/raspberrypi-ros2-car.git
    
    # 创建树莓派工作空间
    cp ~/raspberrypi-ros2-car/workspace/raspberrypi_ws ~/
    
    # 进入树莓派工作空间
    cd ~/raspberrypi_ws
    
    # 编译
    colcon build
    
    # 配置环境变量
    source install/setup.bash
    ```

### 3. 虚拟机工作空间

- 虚拟机中新建终端运行：

    ```bash
    # 克隆项目
    cd ~
    git clone https://github.com/CaddonThaw/raspberrypi-ros2-car.git
    
    # 创建树莓派工作空间
    cp ~/raspberrypi-ros2-car/workspace/vmware_ws ~/
    
    # 进入虚拟机工作空间
    cd ~/vmware_ws
    
    # 编译
    colcon build
    
    # 配置环境变量
    source install/setup.bash
    ```

## 🎮 使用说明

### 1. 启动底盘和雷达

- 虚拟机中远程访问树莓派终端运行：

    ```bash
    source ~/raspberrypi_ws/install/setup.bash
    ros2 launch car_pkg car_base.launch.py
    ```

### 2. 启动遥控/建图/导航

#### 模式A：键盘手动遥控

- 虚拟机中新建终端运行：

    ```bash
    cd ~/vmware_ws/
    source install/setup.bash
    ros2 run car_pkg key_control_node
    
    # 控制方式：
    # W - 前进
    # S - 后退  
    # A - 左转
    # D - 右转
    # 空格 - 停止
    ```

#### 模式B：SLAM建图

- 虚拟机中新建终端运行：

    ```bash
    cd ~/vmware_ws/
    source install/setup.bash
    ros2 launch car_pkg car_mapping.launch.py
    
    # 使用键盘控制节点遥控小车移动，构建地图
    # 建图完成后保存地图：
    ros2 run nav2_map_server map_saver_cli -f ~/vmware_ws/src/car_pkg/maps/map1
    ```

![SLAM建图效果](images/22.png)

#### 模式C：自主导航

- 虚拟机中新建终端运行：

    ```bash
    cd ~/vmware_ws/
    source install/setup.bash
    ros2 launch car_pkg car_nav.launch.py
    
    # 在RViz2中：
    # 1. 使用 "2D Pose Estimate" 设置初始位姿
    # 2. 使用 "Nav2 Goal" 设置目标点
    # 3. 小车将自主规划路径并导航
    ```

![自主导航演示](images/23.png)

## 📧 联系方式

- 🐧：2103539430

- 🛰：Ubuntu_Noble
