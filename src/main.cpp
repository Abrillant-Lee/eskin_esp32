#include <Servo.h>

const int touchPin = T0;
const int ledPin = LED_BUILTIN;

const int threshold = 30;

int touchValue;

const int SERVO_ANGLE = 180;

void controlServo(String command);

Servo servo;

const uint8_t thumbPin = 13;
const uint8_t indexPin = 14;
const uint8_t middlePin = 15;
const uint8_t ringPin = 18;
const uint8_t pinkyPin = 19;

// 添加一个变量来记录当前状态，0表示握拳，1表示张开
int handState = 0;

void setup()
{
    servo.write(indexPin, 0);
    servo.write(middlePin, 0);
    servo.write(pinkyPin, 180);
    servo.write(ringPin, 0);
    servo.write(thumbPin, 180);

    Serial.begin(115200);

    pinMode(ledPin, OUTPUT);
}

void loop()
{
    touchValue = touchRead(touchPin);
    Serial.println(touchValue);

    if (touchValue < threshold)
    {
        digitalWrite(ledPin, HIGH);
        // 如果当前状态是握拳，才改变状态并调用controlServo函数
        if (handState == 0)
        {
            handState = 1;
            controlServo("1");
        }
    }
    else
    {
        digitalWrite(ledPin, LOW);
        // 如果当前状态是张开，才改变状态并调用controlServo函数
        if (handState == 1)
        {
            handState = 0;
            controlServo("0");
        }
    }
}

void controlServo(String command)
{
    if (command == "1")
    {
        for (int posDegrees = 0; posDegrees <= SERVO_ANGLE; posDegrees++)
        {
            servo.write(pinkyPin, 180 - posDegrees);
            servo.write(indexPin, posDegrees);
            servo.write(middlePin, posDegrees);
            servo.write(ringPin, posDegrees);
            servo.write(thumbPin, 180 - posDegrees);
            delay(20);
        }
        Serial.println("张开");
    }
    else if (command == "0")
    {
        for (int posDegrees = SERVO_ANGLE; posDegrees >= 0; posDegrees--)
        {
            servo.write(pinkyPin, 180 - posDegrees);
            servo.write(indexPin, posDegrees);
            servo.write(middlePin, posDegrees);
            servo.write(ringPin, posDegrees);
            servo.write(thumbPin, 180 - posDegrees);
            delay(20);
        }
        Serial.println("握拳");
    }
}