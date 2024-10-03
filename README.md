

<img src="./assets/系统架构.svg" alt="系统架构" style="zoom:150%;" />

```shell
- gd32e23x\									# 标准外设库
- src\
  - util\									# 工具库
    - util.h & utils.c
  - driver\									# 驱动层
    - gd32e23x_libopt.h
    - gd32e23x_it.h & gd32e23x_it.c
    - driver_impl.h & driver_impl.c     	# 设备相关实现
    - driver.h								# 设备无关接口
    - driver.c
  # 业务层
  - startup.s
  - linker.ld
  - main.c
```





```shell
- gd32e23x\									# 标准库
  - core\
  - std\
  - template\
    - startup.s								# 启动文件
    - linker.ld								# 链接脚本
    - gd32e23x_libopt.h						# 配置选项
  - gd32e23x.h
  - system_gd32e23x.c
  - system_gd32e23x.h
- build.zig									# 构建配置（通用）
- src\
  - startup.s & linker.ld & gd32e23x_libopt.h
  #== 驱动层
  - gd32e23x_it.h & gd32e23x_it.c			# 硬件相关实现：中断服务
  - gd32e23x_tool.h & gd32e23x_tool.c		# 硬件相关实现
  - tool.h & tool.c							# 硬件无关实现
  #== 业务层
  - main.h & main.c
- justfile									# 构建指令（项目）
- README.md
```

# 串口通信帧

**串口参数**：8 数据位、无校验位、1 停止位；波特率 115200

+ **下行数据**：首先加入到环形队列，队列字节容量由 `./src/bsp.c :: UART_RECEIVE_RINGQ_SIZE` 定义，当队列满时， 多余数据将会被丢弃；

**帧协议**：下行帧描述对 MCU 的控制指令；上行帧描述 MCU 上传的信息数据；

+ **基本结构**：`帧起始 帧类型 数据域 帧结束`

|        帧类型         |          数据域           | 适用 |
| :-------------------: | :-----------------------: | :--: |
| **控制帧** `!`/`0x21` | `Command ByteCount Byte+` | 下行 |
| **文本帧** `#`/`0x23` |     `Uint32_t Byte+`      | 上行 |
| **数据帧 **`$`/`0x24` |     `Uint32_t Byte+`      | 上行 |
