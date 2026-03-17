#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

// ========== 配置区域 ==========
#define WIFI_SSID "CMCC-5988"
#define WIFI_PASS "18378255988"
#define SERVER_URL "https://cloudbase-2g7cbov971d77f65-1397965697.ap-shanghai.app.tcloudbase.com/httpReceiver"

#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET 8 * 3600

// 串口定义
#define PC_SERIAL Serial      // USB串口，接电脑（用于调试查看）
#define STM32_SERIAL Serial2  // GPIO16(RX), GPIO17(TX)，接STM32

#define BAUD_RATE 115200
#define BUFFER_SIZE 256
// ==============================

// 解析结果结构
struct AttendanceData {
    String studentId;
    String name;
    bool valid = false;
};

// 函数声明
void connectWiFi();
bool syncTime();
unsigned long long getTimestamp();
AttendanceData parseLine(String line);
void uploadData(AttendanceData data);

void setup() {
    // 初始化USB串口（接电脑，用于显示调试信息）
    PC_SERIAL.begin(115200);
    delay(1000);
    
    // 初始化USART2（接STM32，GPIO16=RX, GPIO17=TX）
    STM32_SERIAL.begin(BAUD_RATE, SERIAL_8N1, 16, 17);
    
    PC_SERIAL.println("\n=== ESP32 考勤数据网关 ===");
    PC_SERIAL.println("接收STM32数据 -> 透传到PC -> 解析上传云端");
    
    // 连接网络
    connectWiFi();
    syncTime();
    
    PC_SERIAL.println("系统就绪，等待STM32数据...\n");
}

void loop() {
    static char buffer[BUFFER_SIZE];
    static int idx = 0;
    
    // 读取STM32发送的数据
    while (STM32_SERIAL.available()) {
        char c = STM32_SERIAL.read();
        
        // 实时透传到电脑串口（让你能在串口助手看到STM32的打印）
        PC_SERIAL.write(c);
        
        // 行缓冲，用于解析
        if (c == '\n' || c == '\r') {
            if (idx > 0) {
                buffer[idx] = '\0';
                String line = String(buffer);
                
                // 尝试解析考勤信息
                AttendanceData data = parseLine(line);
                if (data.valid) {
                    uploadData(data);
                }
                
                idx = 0;  // 重置缓冲区
            }
        } else {
            if (idx < BUFFER_SIZE - 1) {
                buffer[idx++] = c;
            }
        }
    }
    
    delay(5);  // 防止看门狗复位
}

// 解析STM32发来的调试文本
// 输入示例: "[Card] Welcome, 2300110307 hehaoyuan! (Card: 1B-70-D1-06)"
// 输入示例: "[Finger] Welcome, 2300110308 heyanzhe! (Finger ID: 2)"
AttendanceData parseLine(String line) {
    AttendanceData result;
    
    // 只处理包含"Welcome"的成功识别记录
    if (line.indexOf("Welcome") == -1) {
        return result;
    }
    
    // 查找"Welcome, "的位置
    int startIdx = line.indexOf("Welcome, ");
    if (startIdx == -1) return result;
    
    startIdx += 9;  // 跳过"Welcome, "
    
    // 查找"!"的位置（姓名结束标志）
    int endIdx = line.indexOf('!', startIdx);
    if (endIdx == -1) return result;
    
    // 提取中间内容: "2300110307 hehaoyuan"
    String content = line.substring(startIdx, endIdx);
    content.trim();
    
    // 查找第一个空格，分割学号和姓名
    int spaceIdx = content.indexOf(' ');
    if (spaceIdx == -1) return result;
    
    result.studentId = content.substring(0, spaceIdx);
    result.name = content.substring(spaceIdx + 1);
    result.valid = true;
    
    PC_SERIAL.print("[Parse] 学号: ");
    PC_SERIAL.print(result.studentId);
    PC_SERIAL.print(", 姓名: ");
    PC_SERIAL.println(result.name);
    
    return result;
}

// 上传到云端（只包含学号、姓名、时间戳）
void uploadData(AttendanceData data) {
    if (!data.valid) return;
    
    if (WiFi.status() != WL_CONNECTED) {
        PC_SERIAL.println("[WiFi] 未连接，跳过上传");
        return;
    }
    
    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);
    
    // 获取时间戳
    unsigned long long timestamp = getTimestamp();
    
    // 构建JSON（只包含三个字段，兼容你的微信小程序）
    String json = "{";
    json += "\"studentId\":\"" + data.studentId + "\",";
    json += "\"name\":\"" + data.name + "\",";
    json += "\"timestamp\":" + String(timestamp);
    json += "}";
    
    PC_SERIAL.print("[Upload] ");
    PC_SERIAL.println(json);
    
    int httpCode = http.POST(json);
    
    if (httpCode > 0) {
        String response = http.getString();
        PC_SERIAL.printf("[Cloud] HTTP %d: %s\n", httpCode, response.c_str());
    } else {
        PC_SERIAL.printf("[Cloud] 失败: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
}

// ========== 工具函数 ==========
void connectWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    PC_SERIAL.print("[WiFi] 连接中");
    
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
        delay(500);
        PC_SERIAL.print(".");
        timeout++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        PC_SERIAL.println();
        PC_SERIAL.print("[WiFi] 已连接, IP: ");
        PC_SERIAL.println(WiFi.localIP());
    } else {
        PC_SERIAL.println("\n[WiFi] 连接失败");
    }
}

bool syncTime() {
    configTime(GMT_OFFSET, 0, NTP_SERVER);
    
    PC_SERIAL.print("[NTP] 同步时间");
    struct tm timeinfo;
    int retry = 0;
    
    while (!getLocalTime(&timeinfo) && retry < 10) {
        delay(1000);
        PC_SERIAL.print(".");
        retry++;
    }
    
    if (retry < 10) {
        PC_SERIAL.println(" 成功");
        return true;
    } else {
        PC_SERIAL.println(" 失败，使用默认时间");
        return false;
    }
}

unsigned long long getTimestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}