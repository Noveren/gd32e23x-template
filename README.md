串口通信协议约定

**串口参数**：8 数据位、无校验位、1 停止位；波特率 115200

+ **下行数据**：首先加入到环形队列，队列字节容量由 `./src/bsp.c :: UART_RECEIVE_RINGQ_SIZE` 定义，当队列满时， 多余数据将会被丢弃；

**帧协议**：下行帧描述对 MCU 的控制指令；上行帧描述 MCU 上传的信息数据；

+ **基本结构**：`帧起始 帧类型 数据域 帧结束`

|        帧类型         |          数据域           | 适用 |
| :-------------------: | :-----------------------: | :--: |
| **控制帧** `!`/`0x21` | `Command ByteCount Byte+` | 下行 |
| **文本帧** `"`/`0x22` |     `Uint32_t Byte+`      | 上行 |
| **数据帧 **`$`/`0x24` |     `Uint32_t Byte+`      | 上行 |

aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbccccccccccccccccccccccccccccccccddddddddddddddddddddddddddddddddeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee