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

int thumbAngle = 0;
int indexAngle = 0;
int middleAngle = 0;
int ringAngle = 0;
int pinkyAngle = 0;

WiFiClient client;

hw_timer_t *timer = NULL; // 定义定时器句柄变量

void IRAM_ATTR onTimer(); // 定时器中断处理函数
void controlServo(String command);
void resetFinger(int pin, int *angle);

Servo servo;

const uint8_t thumbPin = 13;
const uint8_t indexPin = 14;
const uint8_t middlePin = 15;
const uint8_t ringPin = 18;
const uint8_t pinkyPin = 19;

void setup()
{
    servo.write(indexPin, 0);
    servo.write(middlePin, 0);
    servo.write(pinkyPin, 0);
    servo.write(ringPin, 0);
    servo.write(thumbPin, 0);

    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");

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
    // 接收串口数据
    if (Serial.available() > 0)
    {
        String command = Serial.readStringUntil('\n');
        Serial.println(command);
        controlServo(command);
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
    if (command == "食指" && indexAngle != SERVO_ANGLE)
    {
        for (int posDegrees = indexAngle; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servo.write(indexPin, posDegrees);
            Serial.println(posDegrees);
            delay(20);
        }
        indexAngle = SERVO_ANGLE;

        resetFinger(pinkyPin, &pinkyAngle);
        resetFinger(middlePin, &middleAngle);
        resetFinger(ringPin, &ringAngle);
        resetFinger(thumbPin, &thumbAngle);
    }
    else if (command == "小指" && pinkyAngle != SERVO_ANGLE)
    {
        for (int posDegrees = pinkyAngle; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servo.write(pinkyPin, posDegrees);
            Serial.println(posDegrees);
            delay(20);
        }
        pinkyAngle = SERVO_ANGLE;

        resetFinger(indexPin, &indexAngle);
        resetFinger(middlePin, &middleAngle);
        resetFinger(ringPin, &ringAngle);
        resetFinger(thumbPin, &thumbAngle);
    }
    else if (command == "中指" && middleAngle != SERVO_ANGLE)
    {
        for (int posDegrees = middleAngle; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servo.write(middlePin, posDegrees);
            Serial.println(posDegrees);
            delay(20);
        }
        middleAngle = SERVO_ANGLE;

        resetFinger(pinkyPin, &pinkyAngle);
        resetFinger(indexPin, &indexAngle);
        resetFinger(ringPin, &ringAngle);
        resetFinger(thumbPin, &thumbAngle);
    }
    else if (command == "无名指" && ringAngle != SERVO_ANGLE)
    {
        for (int posDegrees = ringAngle; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servo.write(ringPin, posDegrees);
            Serial.println(posDegrees);
            delay(20);
        }
        ringAngle = SERVO_ANGLE;

        resetFinger(pinkyPin, &pinkyAngle);
        resetFinger(middlePin, &middleAngle);
        resetFinger(indexPin, &indexAngle);
        resetFinger(thumbPin, &thumbAngle);
    }
    else if (command == "大拇指" && thumbAngle != SERVO_ANGLE)
    {
        for (int posDegrees = thumbAngle; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servo.write(thumbPin, posDegrees);
            Serial.println(posDegrees);
            delay(20);
        }
        thumbAngle = SERVO_ANGLE;

        resetFinger(pinkyPin, &pinkyAngle);
        resetFinger(middlePin, &middleAngle);
        resetFinger(ringPin, &ringAngle);
        resetFinger(indexPin, &thumbAngle);
    }
    else if (command == "胜利手势" && (indexAngle != SERVO_ANGLE || middleAngle != SERVO_ANGLE))
    {
        for (int posDegrees = min(indexAngle, middleAngle); posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            if (indexAngle != SERVO_ANGLE)
            {
                servo.write(indexPin, posDegrees);
                indexAngle = posDegrees;
            }
            if (middleAngle != SERVO_ANGLE)
            {
                servo.write(middlePin, posDegrees);
                middleAngle = posDegrees;
            }
            Serial.println(posDegrees);
            delay(20);
        }
        indexAngle = SERVO_ANGLE;
        middleAngle = SERVO_ANGLE;

        resetFinger(pinkyPin, &pinkyAngle);
        resetFinger(ringPin, &ringAngle);
        resetFinger(thumbPin, &thumbAngle);
    }
    else if (command == "休息" && (indexAngle != 0 || pinkyAngle != 0 ||
                                   middleAngle != 0 || ringAngle != 0 || thumbAngle != 0))
    {
        for (int posDegrees = SERVO_ANGLE; posDegrees >= 0; posDegrees--)
        {
            if (thumbAngle != 0)
            {
                servo.write(thumbPin, posDegrees);
                thumbAngle = posDegrees;
            }
            if (indexAngle != 0)
            {
                servo.write(indexPin, posDegrees);
                indexAngle = posDegrees;
            }
            if (middleAngle != 0)
            {
                servo.write(middlePin, posDegrees);
                middleAngle = posDegrees;
            }
            if (ringAngle != 0)
            {
                servo.write(ringPin, posDegrees);
                ringAngle = posDegrees;
            }
            if (pinkyAngle != 0)
            {
                servo.write(pinkyPin, posDegrees);
                pinkyAngle = posDegrees;
            }
            Serial.println(posDegrees);
            delay(20);
        }
        thumbAngle = 0;
        indexAngle = 0;
        middleAngle = 0;
        ringAngle = 0;
        pinkyAngle = 0;
    }
}

void resetFinger(int pin, int *angle)
{
    for (int posDegrees = *angle; posDegrees >= 0; posDegrees--)
    {
        servo.write(pin, posDegrees);
        delay(20);
    }
    *angle = 0;
}