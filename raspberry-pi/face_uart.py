import cv2
import serial
import time
import os

# ========== 配置 ==========
SERIAL_PORT = '/dev/ttyUSB0'      # USB转串口，接STM32
BAUDRATE = 115200
TIMEOUT = 10                      # 识别超时10秒
UNKNOWN_ID = 100                  # 识别失败ID

# 加载模型
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')
recognizer = cv2.face.LBPHFaceRecognizer_create()
recognizer.read('trainer.yml')

# 人员表（本地显示用）
names = {1: "何浩源", 2: "何岩哲", 100: "未知"}

# 打开串口
try:
    ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
    print(f"串口 {SERIAL_PORT} 已连接")
except Exception as e:
    print(f"串口失败: {e}")
    ser = None

def send_id(face_id):
    """直接发一个字节"""
    if ser and ser.is_open:
        ser.write(bytes([face_id]))  # 发 0x01, 0x02 或 0x64(100)
        print(f"[UART发送] 字节: {face_id}")

def recognize_with_timeout():
    """识别，10秒超时"""
    cap = cv2.VideoCapture(0)
    start_time = time.time()
    result_id = UNKNOWN_ID

    print(f"开始识别，{TIMEOUT}秒超时...")

    while time.time() - start_time < TIMEOUT:
        ret, frame = cap.read()
        if not ret:
            continue

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        faces = face_cascade.detectMultiScale(gray, 1.3, 5)

        for (x, y, w, h) in faces:
            face_roi = gray[y:y+h, x:x+w]
            id_, confidence = recognizer.predict(face_roi)

            print(f"检测到人脸，ID={id_}, 置信度={confidence:.1f}")

            # 置信度<60认为识别成功
            if confidence < 80:
                result_id = id_
                name = names.get(id_, "未知")
                print(f"识别成功: {name}")
                cap.release()
                return result_id

    print("识别超时")
    cap.release()
    return result_id

# ========== 主循环 ==========
print("人脸识别考勤系统启动")
print("按 Ctrl+C 退出")

try:
    while True:
        print("\n" + "="*30)
        input("按回车开始识别，或Ctrl+C退出...")

        face_id = recognize_with_timeout()
        send_id(face_id)

        name = names.get(face_id, "未知")
        if face_id == UNKNOWN_ID:
            print("结果: 未识别，发送ID=100")
        else:
            print(f"结果: {name}，发送ID={face_id}")

        print("等待2秒后继续...")
        time.sleep(2)

except KeyboardInterrupt:
    print("\n系统退出")
    if ser:
        ser.close()
