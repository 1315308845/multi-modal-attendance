# AS608指纹识别考勤系统集成指南

## 系统概述

本系统在原有IC卡考勤功能基础上，添加了AS608指纹识别考勤功能。系统支持：
- IC卡刷卡考勤
- 指纹识别考勤
- 双重验证（可选）

## 系统架构

### 任务结构
1. **RC522_Task** - IC卡读取任务（优先级：AboveNormal）
2. **Finger_Task** - 指纹识别任务（优先级：AboveNormal）
3. **Attendance_Task** - 考勤处理任务（优先级：Normal）
4. **Print_Task** - 串口打印任务（优先级：Normal）

### 队列结构
1. **xCardQueue** - IC卡UID队列（10个元素，每个4字节）
2. **xFingerQueue** - 指纹ID队列（10个元素，每个2字节）
3. **PrintQueue** - 打印消息队列（20个元素，每个80字节）

### 互斥锁
- **xDbMutex** - 数据库访问互斥锁

## 硬件连接

### AS608指纹模块连接
- **VCC** → 3.3V/5V（根据模块规格）
- **GND** → GND
- **TX** → PA10（USART1_RX）
- **RX** → PA9（USART1_TX）
- **WAK** → 不接（可选唤醒引脚）

### RC522 RFID模块连接
- **SPI1_SCK** → PA5
- **SPI1_MISO** → PA6
- **SPI1_MOSI** → PA7
- **CS** → PA4
- **RST** → PA3

## 软件配置

### 1. 数据库配置

在 `Core/Src/database.c` 中添加人员信息：

```c
static Personnel_t personnel_db[] = {
    {
        .card_uid = {{0x1B, 0x70, 0xD1, 0x06}},
        .finger_id = 1,  // 指纹ID
        .student_id = "2300110307",
        .name = "hehaoyuan"
    },
    {
        .card_uid = {{0x64, 0xC4, 0x64, 0x06}},
        .finger_id = 2,  // 指纹ID
        .student_id = "2300110308", 
        .name = "heyanzhe"
    }
    // 添加更多人员...
};
```

### 2. 指纹录入

在使用系统前，需要先录入指纹到AS608模块：

**方法1：使用上位机软件**
1. 下载AS608配套上位机软件
2. 连接AS608模块到电脑
3. 按照软件提示录入指纹
4. 记录每个人的指纹ID

**方法2：编写录入程序**
```c
// 录入指纹示例代码
uint8_t EnrollFinger(uint16_t finger_id) {
    uint8_t ensure;
    
    // 第一次采集
    printf("请按压手指...\r\n");
    while(GZ_GetImage() != 0x00);
    ensure = GZ_GenChar(CharBuffer1);
    if(ensure != 0x00) return ensure;
    
    printf("请抬起手指\r\n");
    osDelay(1000);
    
    // 第二次采集
    printf("请再次按压同一手指...\r\n");
    while(GZ_GetImage() != 0x00);
    ensure = GZ_GenChar(CharBuffer2);
    if(ensure != 0x00) return ensure;
    
    // 合并模板
    ensure = GZ_RegModel();
    if(ensure != 0x00) return ensure;
    
    // 存储模板
    ensure = GZ_StoreChar(CharBuffer1, finger_id);
    return ensure;
}
```

### 3. 串口配置

- **USART1**：57600波特率，8位数据，1位停止位，无校验（AS608通信）
- **USART2**：115200波特率，8位数据，1位停止位，无校验（调试输出）

## 工作流程

### IC卡考勤流程
1. RC522_Task检测到IC卡
2. 读取卡片UID
3. 将UID发送到xCardQueue
4. Attendance_Task从队列获取UID
5. 在数据库中查找对应人员
6. 生成考勤消息发送到PrintQueue
7. Print_Task输出考勤信息

### 指纹考勤流程
1. Finger_Task等待手指按压
2. AS608采集指纹图像
3. 生成特征文件并搜索指纹库
4. 将匹配的指纹ID发送到xFingerQueue
5. Attendance_Task从队列获取指纹ID
6. 在数据库中查找对应人员
7. 生成考勤消息发送到PrintQueue
8. Print_Task输出考勤信息

## 输出示例

### 系统启动
```
Database initialized with 2 persons
=== Personnel Database ===
[0] 2300110307 hehaoyuan - Card: 1B-70-D1-06, Finger ID: 1
[1] 2300110308 heyanzhe - Card: 64-C4-64-06, Finger ID: 2
==========================
=== Attendance System Started ===
[RC522] Task Started
[AS608] Initializing fingerprint module...
[AS608] Handshake OK, Address: 0xFFFFFFFF
[AS608] Max Fingerprints: 300
[AS608] Security Level: 3
[AS608] Module initialized successfully!
[Finger] Task Started
[Attendance] Task Started
[Print] Task Started
```

### IC卡考勤
```
[RC522] Card detected: 1B-70-D1-06
[Card] Welcome, 2300110307 hehaoyuan! (Card: 1B-70-D1-06)
```

### 指纹考勤
```
[AS608] Finger matched! ID: 1, Score: 95
[Finger] Finger ID 1 sent to queue
[Finger] Welcome, 2300110307 hehaoyuan! (Finger ID: 1)
```

### 未注册用户
```
[RC522] Card detected: AA-BB-CC-DD
[Card] Visitor, please contact administrator! (Card: AA-BB-CC-DD)

[AS608] Finger matched! ID: 99, Score: 88
[Finger] Unregistered fingerprint! (Finger ID: 99)
```

## 调试技巧

### 1. 检查AS608连接
```c
// 在main.c的USER CODE BEGIN 2中添加
if(GZ_HandShake(&AS608Addr) == 0) {
    printf("AS608 Connected!\r\n");
} else {
    printf("AS608 Connection Failed!\r\n");
}
```

### 2. 查看指纹库信息
```c
uint16_t valid_num;
GZ_ValidTempleteNum(&valid_num);
printf("Valid fingerprints: %d\r\n", valid_num);
```

### 3. 测试指纹识别
```c
SearchResult result;
uint8_t ensure = GZ_HighSpeedSearch(CharBuffer1, 0, 300, &result);
printf("Search result: %s\r\n", EnsureMessage(ensure));
if(ensure == 0x00) {
    printf("Matched ID: %d, Score: %d\r\n", result.pageID, result.mathscore);
}
```

## 常见问题

### Q1: AS608模块无响应
**解决方案：**
- 检查串口连接（TX-RX交叉连接）
- 检查波特率设置（默认57600）
- 检查供电电压（3.3V或5V）
- 检查串口中断是否使能

### Q2: 指纹识别失败率高
**解决方案：**
- 调整安全等级（GZ_WriteReg(5, level)，level=1-5）
- 重新录入指纹（每个手指录入2-3次）
- 保持手指清洁干燥
- 按压力度适中

### Q3: 系统卡死或重启
**解决方案：**
- 检查堆栈大小（Finger_Task需要384*4字节）
- 检查队列大小是否足够
- 添加看门狗保护
- 检查中断优先级配置

### Q4: IC卡和指纹同时触发
**解决方案：**
- 系统已支持同时处理
- Attendance_Task使用非阻塞方式轮询两个队列
- 可以根据需求添加优先级处理

## 性能优化

### 1. 降低功耗
```c
// 在无人时降低轮询频率
if(no_activity_count > 100) {
    osDelay(1000);  // 1秒轮询一次
} else {
    osDelay(500);   // 500ms轮询一次
}
```

### 2. 提高识别速度
```c
// 使用高速搜索
GZ_HighSpeedSearch(CharBuffer1, 0, 300, &result);

// 减小搜索范围（如果已知指纹ID范围）
GZ_HighSpeedSearch(CharBuffer1, 0, 50, &result);
```

### 3. 减少误识别
```c
// 提高匹配阈值
if(result.mathscore > 80) {  // 只接受得分>80的匹配
    // 处理考勤
}
```

## 扩展功能

### 1. 添加考勤记录存储
```c
typedef struct {
    char student_id[11];
    uint32_t timestamp;
    uint8_t method;  // 0=卡片, 1=指纹
} AttendanceRecord_t;
```

### 2. 添加LCD显示
```c
// 在考勤成功后显示信息
LCD_ShowString(0, 0, "Welcome!");
LCD_ShowString(0, 20, person->name);
```

### 3. 添加蜂鸣器提示
```c
// 考勤成功
Beep_Success();  // 短鸣一声

// 考勤失败
Beep_Error();    // 长鸣两声
```

## 注意事项

1. **指纹ID从0开始**，最大值取决于模块容量（通常300或1000）
2. **不要在中断中调用AS608函数**，它们需要较长时间
3. **定期清理指纹传感器**，保持识别准确性
4. **备份指纹数据库**，避免数据丢失
5. **测试各种环境条件**（干燥、潮湿、温度变化）

## 技术支持

如有问题，请检查：
1. 硬件连接是否正确
2. 串口配置是否匹配
3. 任务堆栈是否足够
4. 中断优先级是否合理
5. AS608模块固件版本

---
**版本**: 1.0
**日期**: 2026-01-21
**作者**: Kiro AI Assistant
