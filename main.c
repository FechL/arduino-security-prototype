#include <Servo.h>

const int trigPin = 11;
const int echoPin = 12;
const int soundPin = A0;
const int ldrPin = 2;
const int ledPin = 3;
const int buzzerPin = 4;
const int buttonPin = 5;

Servo myServo;

long distance = 0;
long sound;
unsigned long startTime = 0;
const unsigned long WARNING_TIMEOUT = 300000;
bool isTimerActive = false;
const int LED_INTERVAL = 100;
const int MIN_FREQ = 500;
const int MAX_FREQ = 2000;
int freqStep = 100;

void setup()
{
    Serial.begin(9600);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(ldrPin, INPUT);
    pinMode(soundPin, INPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);
    myServo.attach(13);
    startTime = millis();
    isTimerActive = true;
}

void loop()
{
    for (int i = 15; i <= 115; i++)
    {
        checkTimer();
        if (digitalRead(ldrPin) == HIGH)
            moveServoCheck(i);
        else
            off();
    }
    for (int i = 115; i > 15; i--)
    {
        checkTimer();
        if (digitalRead(ldrPin) == HIGH)
            moveServoCheck(i);
        else
            off();
    }
}

void checkTimer()
{
    if (isTimerActive)
    {
        unsigned long currentTime = millis();
        unsigned long elapsedTime = currentTime - startTime;
        if (elapsedTime >= WARNING_TIMEOUT)
            triggerTimeoutWarning(true);
    }
}

void off()
{
    Serial.println("OFF");
    myServo.detach();
    digitalWrite(ledPin, LOW);
    noTone(buzzerPin);
}

void moveServoCheck(int angle)
{
    myServo.attach(13);
    myServo.write(angle);
    delay(25);
    distance = calculateDistance();
    sound = analogRead(soundPin);
    Serial.print("ON | Angle: ");
    Serial.print(angle);
    Serial.print(" | Distance: ");
    Serial.print(distance);
    Serial.print(" | Sound: ");
    Serial.println(sound);
    if ((distance <= 10 && distance > 0) || sound >= 1023)
        triggerWarning(false);
}

int calculateDistance()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    return pulseIn(echoPin, HIGH) * 0.034 / 2;
}

void triggerWarning(bool timer)
{
    myServo.detach();
    if (timer)
        Serial.println("TIMEOUT WARNING");
    else
        Serial.println("WARNING");
    unsigned long lastLEDChange = 0;
    unsigned long lastBuzzerChange = 0;
    int currentFreq = MIN_FREQ;
    bool ledState = false;
    while (1)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - lastLEDChange >= LED_INTERVAL)
        {
            lastLEDChange = currentMillis;
            ledState = !ledState;
            digitalWrite(ledPin, ledState);
        }
        if (currentMillis - lastBuzzerChange >= 50)
        {
            lastBuzzerChange = currentMillis;
            tone(buzzerPin, currentFreq);
            if (timer)
                currentFreq += freqStep * 2;
            else
                currentFreq += freqStep;
            if (currentFreq >= MAX_FREQ || currentFreq <= MIN_FREQ)
                freqStep = -freqStep;
        }
        if (digitalRead(buttonPin) == LOW)
        {
            if (timer)
                startTime = millis();
            break;
        }
    }
    digitalWrite(ledPin, LOW);
    noTone(buzzerPin);
}