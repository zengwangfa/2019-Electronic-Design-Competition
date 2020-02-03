# 纸张计数显示装置😄

<p align="center">
  <a href="https://zengwangfa.top/">个人主页🕺|</a>
  <a href="/docs/论文/纸张计数显示器论文.md">电赛论文📚|</a>
  <a href="/software/">软件说明💻|</a>
  <a href="/mechanical_structure/">机械说明🎮|</a>
  <a href="/hmi/">交互界面说明🖼</a>
</p>

---

<p align="center">
  <img src="/docs/pictures/logo.jpg"/>
</p>


<p align="center">
  <a href="https://www.stmcu.com.cn"><img src="https://img.shields.io/badge/Device-STM32F407-brigreen.svg?style=flat-square" alt="STM32"></a>
  <a href="https://www.rt-thread.org/"><img src="https://img.shields.io/badge/OS-RT--Thread-brightgreen" ></a>
  <a href="https://img.shields.io"><img src="https://img.shields.io/github/repo-size/zengwangfa/2019-Electronic-Design-Competition?style=flat-square" alt="Size"></a>
</p>

## 1、简介:pencil2:
>纸张计数显示装置基于**RT-Thread**实时操作系统，硬件平台采用**STM32F407**单片机为主控制器，以具有抗电磁干扰（**EMI**）架构的FDC2214模块作为电容采集传感器，通过**屏蔽双绞线**连接至两铜极板，读取采集的数据并进行相应判断，应用触摸屏和语音模块进行状态显示与播报。FDC2214模块将采集到的数据通过IIC协议传输给主控制器，主控制器对原始数据进行**卡尔曼滤波**，抑制噪声对数据采集的影响。校准模式下，本系统利用最大隶属度法，确定电容模拟值及纸张数的论域，定义模糊子集和隶属函数，建立模糊规则控制表，求得模糊控制查询表。其根据采集到的两极板的数据，做出短路判断并读取多组实时数据，将其与模拟区间进行归类，选取**最大可能性区间**作为期望值，减小最终判断的差错率。

- [x] 成果：在校准好后，50张以下`100%`正确

---

| 配置参数 | 主控制器 | 配置参数 | 主传感器 |
| :--: | :--: | :--: | :--: |
|芯片型号| STM32F407ZGT6 |芯片型号| FCD2214 | 
|CPU| 32位 Cortex-M4(ARMv7) |分辨率| 28位 | 
|主频| 168MHz |通道数| 4 | 
|RAM| 192KB |接口| I2C | 
|外存| 1MB(FLASH) |最高输出速率| 4.08ksps |  
|单元| FPU、DSP | 特性 |抗电磁干扰(EMI) |

---

- 系统结构简图：
![系统结构简图](/docs/pictures/系统结构简图.png "系统结构简图")

## 2、目录说明:bookmark:

#### STM32F407控制程序在：
```
+——2019-Electronic-Design-Competition
|---+ software:【软件设计】
|       ├──README.md
|       ├──rt-thread-master
|            └──bsp
|               └──stm32f407【控制程序】
|                  └──project.uvprojx【Keil5工程】
|---- ...

```
| 目录名 | 描述  |
| :---: | :---: |
| 2019电赛题目| 2019年电赛A-H题题目及器件清单 |
| [docs](./docs/) | 数据手册、设计图片、参考文献及设计文档 |
| [hardware](./hardware/) | FDC2214、核心板原理图及PCB |
| hmi| 串口屏幕设计UI及交互程序 |
| matlab| matlab函数拟合及Kalman  |
| mechanical | 机械结构设计 |
| others | 一些关于电赛前的准备 |
| [**software**](./software/) | **纸张计数显示装置STM32程序** |
| [综合测评](./综合测评/) | **综合测评题目及Multisim电路仿真** | 

## 3、机械结构:hammer:
- 3维模型
![3D效果图](/docs/pictures/3D_structure.jpg "3D效果图")
- 实物图
![铰链结构实物图](/docs/pictures/metal_struture.png)

## 4、[软件说明💻](/software/) 

应用线程总体分为：
- 1.简单设备运行线程
    - LED
	- OLED
	- Buzzer
	- Flash
	
- 2.主测量应用线程
	- HMI屏幕交互
	- FD2214数据获取与转换
	- 语音模块交互
	
- 3.系统监控线程
	- Watch Dog:dog2:


## 5、硬件结构:artificial_satellite:

![Controller PCB 3D](/docs/pictures/Controller_3D.jpg "Controller 3D")

- 核心板(Core Controller)拥有外设：

| 外设名称 | RGB LED | Key | Buzzer | Dial Switch | **FCD2214** | USART HMI |OLED | W25Q128 | Voltage Detection | Current Detection | Zigbee |  JY901 | USR-C216 | CP2102 | 
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |  :---: | 
| 控制(通讯)方式 | GPIO | GPIO | GPIO | GPIO | **I2C** | USART | SPI | SPI | ADC | ADC | USART | USART | USART  | USART | 
| 功能描述 | 指示灯 | 按键 | 蜂鸣器 | 拨码开关 | **电容传感器** | 串口触摸屏 | 显示屏 | 闪存芯片 | 电压检测 | 电流检测 | 2.4G无线通信 | 九轴 | WiFi模块 | 串口转USB | 

- 整体实物图
![整体实物图](/docs/pictures/all_structure.png)

---
## 6、[交互界面:framed_picture:](/hmi/) 

- [x] 电脑调试时的交互界面展示：

<p align="center">
  <img src="https://zengwangfa.oss-cn-shanghai.aliyuncs.com/github/paper_hmi.gif"/>
</p>


## 7、测试数据分析:bar_chart:

(1)
保持测试环境不变，采集50个样本，通过MATLAB拟合出纸张页数与传感器原始数据的曲线关系，如图4-2所示。

![拟合曲线](/docs/pictures/fitting.png "拟合曲线")

**图4-2 纸张页数与模拟量的拟合函数关系图**

- (2) 根据(1)中采集的50个样本，确定电容模拟值及纸张数的论域，划分模糊子集，如表4-1
所示。

**表4-1页数与模拟量区间对应表**

| 页数/张 | 对应模拟量区间    | 页数/张 | 对应模拟量区间  |
|---------|-------------------|---------|-----------------|
| 1       | [1680.00,1345.34] | 6       | [682.48,633.84] |
| 2       | [1345.34,1013.26] | 7       | [633.84,598.03] |
| 3       | [1013.26,845.12]  | 8       | [598.03,569.46] |
| 4       | [845.12,748.79]   | 9       | [569.46,546.28] |
| 5       | [748.79,682.48]   | 10      | [546.28,529.16] |

- (3)根据（2）中划分的模糊子集，保持测试环境不变，固定测量35张纸张，采集数据如表4-2所示，采集到的原始数据90%落入35张页数所对应的区间[401.35,399.50]，如图4-35

![35张纸区间图](/docs/pictures/35page.png)

**表4-2 纸张数为35的实时数据**

| 测试组别 | 原始数据 | 测试组别 | 原始数据 |
|----------|----------|----------|----------|
| 1        | 400.896  | 6        | 400.824  |
| 2        | 401.049  | 7        | 400.591  |
| 3        | 400.157  | 8        | 401.111  |
| 4        | 399.912  | 9        | 400.443  |
| 5        | 401.125  | 10       | 398.529  |


- 测试结果

**表4-4 工作模式下纸张测试结果表**

| 范围   | 测试次数 | 正确次数 | 正确率 |
|--------|----------|----------|--------|
| 1\~10  | 25       | 25       | 100%   |
| 11\~20 | 25       | 25       | 100%   |
| 20\~30 | 25       | 25       | 100%   |
| 30\~40 | 25       | 25       | 100%   |
| 40\~50 | 25       | 25       | 100%   |
| 50\~60 | 25       | 23       | 92%    |
| 60\~70 | 25       | 20       | 80%    |
| 70\~80 | 25       | 16       | 64%    |


## 8、综合测评演练:triangular_ruler:
- 选定2017综合测评题目【符合信号发生器】，进行综合测评训练
![2017年全国大学生电子设计竞赛综合测评题](/docs/pictures/2017年全国大学生电子设计竞赛综合测评题.jpg "2017年全国大学生电子设计竞赛综合测评题")

虽然测评训练的时候做的还行，但是实际当天综测比赛的时候，还是会手忙脚乱，现在想起那天焊接电路颤抖的双手都有点好笑:rofl::rofl:。
- 综测训练测试结果：
![2017综测训练](/docs/pictures/2017综测训练.jpg "2017综测训练")

## 9、分工:family:
- [X] 电路设计
    - [X] 硬件搭建 by [@Ian](https://github.com/zengwangfa)	
	
- [X] 控制程序
    - [X] 整体框架 by [@Ian](https://github.com/zengwangfa)
	- [x] 交互界面及通信 by [@Ian](https://github.com/zengwangfa)
	- [X] 核心分割算法 by [@Okurarisona](https://github.com/Okurarisona)

- [X] 机械结构
	- [X] 结构方案设计 by [@JackyLin](https://github.com/JackyLin1205) 
	- [X] 固定铰链式抗干扰结构 by [@JackyLin](https://github.com/JackyLin1205) 

- [X] 论文
	- [X] 论文 by [@Hyf338](https://github.com/Hyf338)


### 赞赏
![赞赏](/docs/pictures/给赞.png "赞赏")


#### 整理纪念
- [点击联系我](Mailto:zengwangfa@outlook.com)




