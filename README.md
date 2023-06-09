# 通信系统综合设计与实践-低速星型无线通信系统设计



## 设计要求



基本要求：

（1）设计包含1个主节点，2个终端节点的星型无线通信系统；

（2）PC端基于LabVIEW设计可视化界面，实现数据采集、存储，曲线拟合，实现对无线通信系统的控制功能；

（3）整个系统之间的数据传输均采用统一的通信协议（可自定义）进行。

 

提高要求：

（1）实现3个终端节点的无线组网通信；

（2）实现节点灵活入网以及网络拓扑维护，实现PC端对任意无线设备的信道参数的设置与读取，实时显示无线节点的在网状态；

（3）确保整个系统的可靠性和稳定性的前提下提高网络效率；

（4）扩展发挥，设计实用环境完成无线信息传输。



## 通信协议部分

见[报告](./report.pdf)中<u>四、通信模块设计与验证</u>章节。



## 代码提示

终端中的数据读取管脚见[终端机配置头文件](./arduino_code/termnial/terminal/config.h)。