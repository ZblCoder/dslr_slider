/*
 Name:		dslr_slider.ino
 Created:	5/11/2019 3:21:04 PM
 Author:	ZblCoder
*/

#include "TM1637.h"
#include <EEPROM.h>
#include "Button.h"
#include "Encoder.h"
#include "Header.h"

#define MOTOR_ENABLE      7
#define MOTOR_STEP        8
#define MOTOR_DIRECTION   9

#define ENCODER_PIN_A     4
#define ENCODER_PIN_B     5
#define ENCODER_BUTTON    3

#define TIMER_CLK         12
#define TIMER_DIO         11

#define BUTTON_BEGIN      6
#define BUTTON_END        2

#define START_VECTOR      1   // Направление движения (1 или -1)

// Стартовая скорость сканирования ( время шага = MAX_SPEED - скорость ) значение в микросекундах 1 000 000 секунды
#define MIN_SPEED 100
#define MAX_SPEED 1000 

#define SPEED_EDITSTEP 1

////////////////////////////////////////////////////////////////////////////

TM1637 tm1637(TIMER_CLK, TIMER_DIO);

int time = 0;
int timer = 0;
int position = 0;
int length = 0;
int speed = 0;

long motorLastTime = 0;
long timerLastTime = 0;

StateType state = Waiting;
DirectionType direction = Forward;

void AddTime(int value)
{
    time += value;
    if (time >= 6000)
        time -= 6000;
    else if (time < 0)
        time += 6000;
    EEPROM.put(0, time);
}

void AddSpeed(int value)
{
    speed += value;
    if (speed > MAX_SPEED)
        speed = MAX_SPEED;
    else if (speed < MIN_SPEED)
        speed = MIN_SPEED;
    EEPROM.put(4, speed);
}

void DrawTimer(int value)
{
    tm1637.point(true);
    int m = value / 60;
    int s = value % 60;
    tm1637.display(m * 100 + s);
}

void DrawSpeed(int value)
{
    tm1637.point(false);
    tm1637.display(value);
}

void SetDirection(DirectionType value)
{
    if (direction = value)
        return;
    Serial.println("SetDirection");
    direction = value;
    int vector = value == Forward ? START_VECTOR : -START_VECTOR;
    digitalWrite(MOTOR_DIRECTION, vector > 0 ? HIGH : LOW);
    Serial.println(vector);
}

void SetState(StateType value)
{
    
    if (state == value)
        return;

    StateType lastState = state;
    state = value;
    switch (state)
    {
    case Waiting:
        break;
    case Movement:
        Serial.println("Play");
        if (lastState != Paused)
        {
            SetDirection(Forward);
            speed = (time * 1000 * 1000) / length;
            motorLastTime = micros();
        }
        break;
    case Paused:
        Serial.println("Pause");
        break;
    case Scanning:
        Serial.println("Start Scaning");
        speed = MIN_SPEED;
        EEPROM.put(4, speed);
        SetDirection(Forward);
        motorLastTime = micros();
        length = 0;
        DrawSpeed(speed);
        break;
    case Reset:
        EEPROM.get(4, speed);
        SetDirection(Backwards);
        motorLastTime = micros();
        DrawSpeed(speed);
        break;
    }    
}

void OnEncoderChanged(int rotation)
{
    switch (state)
    {
    case Waiting:
        AddTime(rotation * SPEED_EDITSTEP);
        break;
    case Scanning:
        AddSpeed(rotation * SPEED_EDITSTEP);
        DrawSpeed(speed);
        break;
    case Reset:
        AddSpeed(rotation);
        DrawSpeed(speed);
        break;
    }    
}

void OnEncoderDownChanged(int rotation)
{    
    switch (state)
    {
    case Waiting:
        AddTime(rotation * 60);
        break;
    case Scanning:
        AddSpeed(rotation * SPEED_EDITSTEP * 10);
        DrawSpeed(speed);
        break;
    case Reset:
        AddSpeed(rotation * SPEED_EDITSTEP * 10);
        DrawSpeed(speed);
        break;
    }
}

void OnEncoderClick()
{
    switch (state)
    {
    case Waiting:
        SetState(Movement);
        break;
    case Movement:
        SetState(Paused);
        break;
    case Paused:
        SetState(Movement);
        break;
    case Scanning:
    case Reset:
        SetState(Waiting);
        break;
    }
}

void OnEncoderLongClick()
{
    SetState(Reset);
}

void OnEncoderVeryLongClick()
{
    SetState(Scanning);
}

Encoder PlayEncoder(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BUTTON);
Button BeginButton(BUTTON_BEGIN);
Button EndButton(BUTTON_END);

void bBegin()
{
    Serial.println("bBegin");
}

void bEnd()
{
    Serial.println("bEnd");
}

void setup() 
{
    Serial.begin(9600);
    while (!Serial){;}

    tm1637.init();
    tm1637.set(BRIGHT_DARKEST);

    pinMode(MOTOR_ENABLE, OUTPUT);
    pinMode(MOTOR_STEP, OUTPUT);
    pinMode(MOTOR_DIRECTION, OUTPUT);

    PlayEncoder.SetOnChanged(OnEncoderChanged);
    PlayEncoder.SetOnDownChanged(OnEncoderDownChanged);

    PlayEncoder.SetOnClick(OnEncoderClick);
    PlayEncoder.SetOnLongClick(OnEncoderLongClick);
    PlayEncoder.SetOnVeryLongClick(OnEncoderVeryLongClick);

    BeginButton.SetOnClick(bBegin);
    EndButton.SetOnClick(bEnd);

    EEPROM.get(0, time);
    EEPROM.get(4, speed);
    EEPROM.get(8, length);
        
    digitalWrite(MOTOR_ENABLE, LOW);

    Serial.println("GO");
}

bool MotorLoop()
{
    long t = micros() - motorLastTime;
    if (t >= 100 + (MAX_SPEED - speed) * 10)
    {
        digitalWrite(MOTOR_STEP, HIGH);
        digitalWrite(MOTOR_STEP, LOW);
        position += direction == Forward ? 1 : -1;
        motorLastTime = micros();
        return true;
    }
    return false;
}

void loop() 
{
    PlayEncoder.Update();
    BeginButton.Update();
    EndButton.Update();

    switch (state) 
    {
        case Waiting:
            DrawTimer(time);
            break;
        case Movement:
            if (EndButton.IsUp() && position < length)
                MotorLoop();
            else
                SetState(Waiting);
            
            DrawSpeed(position);
            break;
        case Scanning:
            switch (direction)
            {
            case Forward:
                if (EndButton.IsUp())
                    MotorLoop();
                else
                {
                    SetDirection(Backwards);
                }
                break;
            case Backwards:
                if (BeginButton.IsUp())
                {
                    if (MotorLoop())
                        length++;
                }
                else
                {
                    position = 0;
                    SetState(Waiting);
                    EEPROM.put(8, length);
                }
                break;
            }
            break;
        case Reset:
            if (BeginButton.IsUp())
                MotorLoop();
            else
            {
                position = 0;
                SetState(Waiting);
            }
            DrawSpeed(position);
            break;
    }
    
}
