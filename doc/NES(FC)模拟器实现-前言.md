# NES模拟器实现-前言

## 前言

本系列教程从零开始使用C++实现一个任天堂红白机模拟器，许多小时候的经典游戏都来自这个平台，不过阅读本教程的读者童年可能已经是智能手机的时代了，不过红白机平台上发布的《超级马里奥》、《塞尔达传说》、《银河战士》等游戏IP仍在继续推出新作，也许可以回顾游戏行业逐渐兴起的时代。

参考资料有[simpleNES](https://github.com/amhndu/SimpleNES/)，[Nesdev](https://www.nesdev.org/wiki/Nesdev_Wiki)

### 适合阅读的对象

比较适合刚学习完C/C++，对计算机课程整体有一点了解，希望对学习实际项目，不在对着DOS界面I/O的学生。

本项目整体代码量约3K行，文件不多，封装的类逻辑并不复杂，模块之间基本解耦，可以轻松读懂。

第三方库仅使用到了SFML，不作为主要内容，只用其进行图像渲染和键盘输入，使用简单。

使用到的CPP语法并不复杂，涉及一些常用STL对象，并且使用premake进行项目构建。

本项目涉及到了一些计算机体系结构的知识，并且实现了CPU和PPU的模拟，可以了解到计算机内部的运行过程，对许多计算机方向都有帮助。

## 红白机总览

### 硬件

由于需要模拟红白机，自然要对红白机的硬件组成有所了解，如果读者有了解过嵌入式或单片机，可以查看以下这张图纸，其中蓝线部分是总线：

![NES_bus](NES_bus.jpg)

去除掉外围设备，红白机主要由以下几个部分组成：

**CPU**：红白机使用了2A03芯片作为CPU，该芯片从6502芯片修改得来，这是一款8位微处理器，时钟频率1.789773MHz（约559ns每个周期，某些红白机版本频率不太一样），片上继承了一块APU（音频芯片）用于产生声音，本质上2A03即使6502芯片加APU，机上提供2K内存。

**PPU**：2C02芯片，图像处理器，和现代的显卡起类似作用，产生图像信号供显示屏显示，为其提供了2K内存。

**卡带**：游戏内容，当然对于模拟器而言并不需要将卡带插入，其内容是一个.nes文件，包括了游戏程序和游戏数据，某些卡带对红白机进行了扩展，实际上卡带成为了硬件的一部分，这部分通过mapper实现。

**总线**：红白机是冯诺依曼架构的，对于一个处理器只有一条总线，总线将数据从内存或卡带中传输给

处理器，CPU总线有16bits，可以寻址64KB的内容，PPU总线有14bits，可以寻址16KB的内容。

### 运行

红白机上并没有操作系统，也就是说其类似一个嵌入式系统，当开机或按下reset键的时候，发生reset中断，CPU开始读取指令执行，如果没有插入卡带那么没有指令执行，显示屏就会是花屏状态，当插入卡带再按下reset键，CPU读取到游戏指令开始运行，插入卡带的操作就类似于嵌入式设备拷入代码，此时CPU将一直运行游戏中的指令，每一帧里，CPU读取输入设备通过PPU像其视频内存（VRAM，也就是PPU对应的内存）写入图像数据，像APU写入音频数据，最终呈现到输出设备上。