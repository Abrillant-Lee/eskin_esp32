#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP32Servo.h>

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

    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    servoPinky.setPeriodHertz(50);
    servoIndex.setPeriodHertz(50);
    servoMiddle.setPeriodHertz(50);
    servoRing.setPeriodHertz(50);
    servoThumb.setPeriodHertz(50);

    servoThumb.attach(thumbPin, 500, 2500);   // 将舵机连接到引脚2
    servoIndex.attach(indexPin, 500, 2500);   // 将舵机连接到引脚4
    servoMiddle.attach(middlePin, 500, 2500); // 将舵机连接到引脚12
    servoRing.attach(ringPin, 500, 2500);     // 将舵机连接到引脚13
    servoPinky.attach(pinkyPin, 500, 2500);   // 将舵机连接到引脚14

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
    if (command == "食指" && indexAngle != SERVO_ANGLE)
    {
        for (int posDegrees = indexAngle; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servoIndex.write(posDegrees);
            Serial.println(posDegrees);
            delay(20);
        }
        indexAngle = SERVO_ANGLE;
    }
    else if (command == "小指" && pinkyAngle != SERVO_ANGLE)
    {
        for (int posDegrees = pinkyAngle; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servoPinky.write(posDegrees);
            Serial.println(posDegrees);
            delay(20);
        }
        pinkyAngle = SERVO_ANGLE;
    }
    else if (command == "中指" && middleAngle != SERVO_ANGLE)
    {
        for (int posDegrees = middleAngle; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servoMiddle.write(posDegrees);
            Serial.println(posDegrees);
            delay(20);
        }
        middleAngle = SERVO_ANGLE;
    }
    else if (command == "无名指" && ringAngle != SERVO_ANGLE)
    {
        for (int posDegrees = ringAngle; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servoRing.write(posDegrees);
            Serial.println(posDegrees);
            delay(20);
        }
        ringAngle = SERVO_ANGLE;
    }
    else if (command == "大拇指" && thumbAngle != SERVO_ANGLE)
    {
        for (int posDegrees = thumbAngle; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servoThumb.write(posDegrees);
            Serial.println(posDegrees);
            delay(20);
        }
        thumbAngle = SERVO_ANGLE;
    }
    else if (command == "休息")
    {
        for (int posDegrees = SERVO_ANGLE; posDegrees >= 0; posDegrees--)
        {
            if (thumbAngle != 0)
            {
                servoThumb.write(posDegrees);
                thumbAngle = posDegrees;
            }
            if (indexAngle != 0)
            {
                servoIndex.write(posDegrees);
                indexAngle = posDegrees;
            }
            if (middleAngle != 0)
            {
                servoMiddle.write(posDegrees);
                middleAngle = posDegrees;
            }
            if (ringAngle != 0)
            {
                servoRing.write(posDegrees);
                ringAngle = posDegrees;
            }
            if (pinkyAngle != 0)
            {
                servoPinky.write(posDegrees);
                pinkyAngle = posDegrees;
            }
            Serial.println(posDegrees);
            delay(20);
        }
    }
}

// #include "stdio.h"
// #include "driver/mcpwm.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #define PIN_SERVO_PWM 14 // 舵机控制信号引脚，这里假设使用GPIO14

// void servo_init()
// {
//     mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PIN_SERVO_PWM); // 初始化PWM引脚
//     mcpwm_config_t pwm_config;
//     pwm_config.frequency = 50; // PWM频率为50Hz
//     pwm_config.cmpr_a = 0;     // 初始占空比为0
//     pwm_config.counter_mode = MCPWM_UP_COUNTER;
//     pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
//     mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
// }

// void servo_set_angle(float angle)
// {
//     uint32_t duty_us = (uint32_t)(500 + angle / 180.0 * 2000);               // 计算占空比对应的脉冲宽度
//     mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us); // 设置PWM占空比
//     mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);                                // 开始PWM输出
// }

// void app_main()
// {
//     servo_init(); // 初始化舵机控制
//     while (1)
//     {
//         // 循环转动舵机
//         for (float angle = 0; angle <= 180; angle += 10)
//         {
//             servo_set_angle(angle);
//             printf("angle:%lf\n", angle);
//             vTaskDelay(pdMS_TO_TICKS(100)); // 延时100ms
//         }
//     }
// }
