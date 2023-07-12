#include <WiFi.h>
#include <WiFiClient.h>
#include <Servo.h>

const char *ssid = "AvA";
const char *password = "25802580";
const char *serverIP = "20.189.79.217";
const int serverPort = 2333;
const char *esp32Token = "ESP32";

const int SAMPLE_INTERVAL = 5;                             // 采样间隔，单位为毫秒
const int SAMPLE_COUNT = 79;                               // 每个数据包包含的采样数量
const int PACKET_INTERVAL = 1000;                          // 发送数据包的间隔，单位为毫秒
const int TIMER_INTERVAL = SAMPLE_INTERVAL * SAMPLE_COUNT; // 定时器间隔，单位为毫秒

const int SERVO_ANGLE = 90;

float voltage[SAMPLE_COUNT];
int sampleIndex = 0;
int packetIndex = 0;

WiFiClient client;

hw_timer_t *timer = NULL; // 定义定时器句柄变量

void IRAM_ATTR onTimer(); // 定时器中断处理函数
void controlServo(String command);

Servo servoThumb, servoIndex, servoMiddle, servoRing, servoPinky;
const uint8_t thumbPin = 2;
const uint8_t indexPin = 4;
const uint8_t middlePin = 12;
const uint8_t ringPin = 13;
const uint8_t pinkyPin = 14;

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");

    servoThumb.attach(2);   // 将舵机连接到引脚2
    servoIndex.attach(4);   // 将舵机连接到引脚4
    servoMiddle.attach(12); // 将舵机连接到引脚12
    servoRing.attach(13);   // 将舵机连接到引脚13
    servoPinky.attach(14);  // 将舵机连接到引脚14

    // 启动定时器
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, TIMER_INTERVAL * 10, true);
    timerAlarmEnable(timer);
}

void loop()
{
    // 发送数据包
    if (packetIndex > 0 && client.connected())
    {
        String dataPacket;
        for (int i = 0; i < SAMPLE_COUNT - 1; i++)
        {
            dataPacket += String(voltage[i], 6) + ",";
        }
        dataPacket += String(voltage[SAMPLE_COUNT - 1], 6);

        int rssi = WiFi.RSSI();
        dataPacket += "," + String(rssi);

        dataPacket += "\n"; // 添加换行符

        // Serial.print("Sending data packet: ");
        // Serial.println(dataPacket);

        client.print(dataPacket);
        String response = client.readStringUntil('\n');
        Serial.println(response);

        controlServo(response); // 根据响应控制舵机

        packetIndex = 0;
    }
    else if (packetIndex == 0 && !client.connected())
    {
        // 连接服务器
        if (client.connect(serverIP, serverPort))
        {
            Serial.println("Connected to server");
            // 发送身份验证消息
            client.print(esp32Token);
            client.print('\n');
        }
        else
        {
            Serial.println("Connection failed");
        }
    }
}

void IRAM_ATTR onTimer()
{
    // 采集数据
    voltage[sampleIndex] = analogRead(A0) * 3.3 / 4095.0;
    sampleIndex++;

    // 发送数据包
    if (sampleIndex == SAMPLE_COUNT)
    {
        sampleIndex = 0;
        packetIndex++;

        if (packetIndex >= PACKET_INTERVAL / TIMER_INTERVAL)
        {
            packetIndex = 0;
        }
    }
}

void controlServo(String command)
{
    static String lastCommand = ""; // 用于记录上次控制的舵机
    if (command != lastCommand)     // 判断当前要控制的舵机是否与上次相同
    {
        if (command == "食指")
        {
            for (int posDegrees = 0; posDegrees <= SERVO_ANGLE; posDegrees++)
            {
                servoIndex.write(posDegrees);
                Serial.println(posDegrees);
                delay(20);
            }
        }
        else if (command == "小指")
        {
            for (int posDegrees = 0; posDegrees <= SERVO_ANGLE; posDegrees++)
            {
                servoPinky.write(posDegrees);
                Serial.println(posDegrees);
                delay(20);
            }
        }
        else if (command == "中指")
        {
            for (int posDegrees = 0; posDegrees <= SERVO_ANGLE; posDegrees++)
            {
                servoMiddle.write(posDegrees);
                Serial.println(posDegrees);
                delay(20);
            }
        }
        else if (command == "无名指")
        {
            for (int posDegrees = 0; posDegrees <= SERVO_ANGLE; posDegrees++)
            {
                servoRing.write(posDegrees);
                Serial.println(posDegrees);
                delay(20);
            }
        }
        else if (command == "大拇指")
        {
            for (int posDegrees = 0; posDegrees <= SERVO_ANGLE; posDegrees++)
            {
                servoThumb.write(posDegrees);
                Serial.println(posDegrees);
                delay(20);
            }
        }
        else if (command == "休息")
        {
            for (int posDegrees = SERVO_ANGLE; posDegrees >= 0; posDegrees--)
            {
                servoIndex.write(posDegrees);
                servoMiddle.write(posDegrees);
                servoPinky.write(posDegrees);
                servoRing.write(posDegrees);
                servoThumb.write(posDegrees);
                Serial.println(posDegrees);
                delay(20);
            }
        }
        else if (command == "胜利手势")
        {
            for (int posDegrees = 0; posDegrees <= SERVO_ANGLE; posDegrees++)
            {
                servoIndex.write(posDegrees);
                servoMiddle.write(posDegrees);
                Serial.println(posDegrees);
                delay(20);
            }
        }

        lastCommand = command; // 更新上次控制的舵机
    }

    // // 将舵机重置到初始位置
    // servoThumb.write(0);
    // servoIndex.write(0);
    // servoMiddle.write(0);
    // servoRing.write(0);
    // servoPinky.write(0);
}
