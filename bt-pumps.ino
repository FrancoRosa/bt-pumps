#define led 13

void setup()
{

    Serial.begin(115200);
    Serial.println("...Start");
    pinMode(led, OUTPUT);
}

void loop()
{
    Serial.println("Cool");
    digitalWrite(led, digitalRead(led));
    delay(500);
}