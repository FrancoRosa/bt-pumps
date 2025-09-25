#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <RTClib.h>
#define led 13
#define sqw 2
#define volmeter A2

#define pump1 5   // 0
#define pump2 6   // 1
#define pump3 7   // 2
#define pump4 8   // 3
#define pump5 9   // 4
#define pump6 10  // 5
#define pump7 11  // 6
#define pump8 12  // 7
#define pump9 A0  // 8
#define pump10 A1 // 9

#define memoryLen 8
#define bufferLen 40

SoftwareSerial bluetoothComm(3, 4); // RX, TX
RTC_DS3231 rtc;

volatile int ibuffer = 0;
volatile char buffer[bufferLen];
volatile char outbuffer[bufferLen];

volatile int schedule[memoryLen];
volatile int address;

const char setCommand[] = "set";
const char getCommand[] = "get";
const char showCommand[] = "sho";

volatile bool readFlag = false;
volatile bool writeFlag = false;
volatile bool showFlag = false;

DateTime now;
volatile bool flagShowRTC = false;
const int pumpPins[] = {pump1, pump2, pump3, pump4, pump5, pump6, pump7, pump8, pump9, pump10};
const int numPumps = sizeof(pumpPins) / sizeof(pumpPins[0]);

void test()
{
    for (int i = 0; i < numPumps; i++)
    {
        // Turn the current pump ON
        Serial.print(i);
        Serial.println(" ON");
        digitalWrite(pumpPins[i], LOW);
        digitalWrite(led, HIGH);

        delay(2000); // Wait for 1 second

        // Turn the current pump OFF
        Serial.print(i);
        Serial.println(" OFF");
        digitalWrite(pumpPins[i], HIGH);
        digitalWrite(led, LOW);

        delay(2000); // Wait half a second before moving to the next pump
    }
}

void setup()
{
    rtc.begin();
    rtc.disable32K();
    rtc.clearAlarm(1);
    rtc.clearAlarm(2);
    rtc.disableAlarm(1);
    rtc.disableAlarm(1);
    rtc.writeSqwPinMode(DS3231_SquareWave1Hz);
    Serial.begin(115200);
    Serial.println("Hello Freakbitches!");
    bluetoothComm.begin(9600);
    bluetoothComm.println("Hello, world?");
    pinMode(led, OUTPUT);
    pinMode(sqw, INPUT_PULLUP);
    for (int i = 0; i < numPumps; i++)
    {
        pinMode(pumpPins[i], OUTPUT);
        digitalWrite(pumpPins[i], HIGH); // Start with all pumps OFF
    }
    test();
    attachInterrupt(digitalPinToInterrupt(sqw), everySecond, FALLING);
}

void everySecond()
{
    flagShowRTC = true;
    digitalWrite(led, !digitalRead(led));
}

void loop()
{
    if (Serial.available())
    {
        processingSerial(Serial.read());
    }

    if (bluetoothComm.available())
    {
        processingSerial(bluetoothComm.read());
    }

    if (flagShowRTC)
    {
        now = rtc.now();
        printDate(now);
        compareWithEEPROM();
        flagShowRTC = false;
    }
}

void compareWithEEPROM()
{
    int saved[8];
    for (int i = 0; i < 100; i++)
    {
        int offset = i * 8;
        saved[1] = EEPROM.read(offset + 1);
        if (now.dayOfTheWeek() == saved[1])
        {
            saved[0] = EEPROM.read(offset + 0);
            saved[2] = EEPROM.read(offset + 2);
            saved[3] = EEPROM.read(offset + 3);
            saved[4] = EEPROM.read(offset + 4);
            saved[5] = EEPROM.read(offset + 5);
            saved[6] = EEPROM.read(offset + 6);
            saved[7] = EEPROM.read(offset + 7);
            if ((now.hour() == saved[2]) && (now.minute() == saved[3]) && (now.second() == saved[4]))
            {
                int targetPump = saved[0];
                if (targetPump >= 0 && targetPump <= numPumps)
                {
                    digitalWrite(pumpPins[targetPump], LOW);
                }
            }

            if ((now.hour() == saved[5]) && (now.minute() == saved[6]) && (now.second() == saved[7]))
            {
                int targetPump = saved[0];
                if (targetPump >= 0 && targetPump <= numPumps)
                {
                    digitalWrite(pumpPins[targetPump], HIGH);
                }
            }
        }
    }
}
/* Format for saving schedule:
          Byte0:Pump
          Byte1:Day
          Byte2:HourBegin
          Byte3:MinuteBegin
          Byte4:SecondBegin12
          Byte5:HourEnd
          Byte6:MinuteEnd
          Byte7:SecondEnd
*/
void printDate(DateTime date)
{
    int volValue = 0;
    volValue = analogRead(volmeter);
    sprintf(outbuffer, "Tm(%d) %02d:%02d:%02d %3d\r\n", date.dayOfTheWeek(), date.hour(), date.minute(), date.second(), volValue);
    printStringSerial();
    printStringBluetooth();
}

void printProgram()
{
    sprintf(outbuffer, "Pr%01d%01d%02d%02d%02d%02d%02d%02d\r\n", schedule[0], schedule[1], schedule[2], schedule[3], schedule[4], schedule[5], schedule[6], schedule[7]);
    printStringSerial();
    printStringBluetooth();
}

void printStringSerial()
{
    for (int i = 0; i < bufferLen; i++)
    {
        if (outbuffer[i] != 0)
        {
            Serial.print(char(outbuffer[i]));
        }
        else
        {
            break;
        }
    }
}

void printStringBluetooth()
{
    for (int i = 0; i < bufferLen; i++)
    {
        if (outbuffer[i] != 0)
        {
            bluetoothComm.write(char(outbuffer[i]));
        }
        else
        {
            break;
        }
    }
}
void values2strings(void)
{
    buffer[3] = schedule[0] + 48;
    buffer[4] = schedule[1] + 48;
    buffer[5] = schedule[2] / 10 + 48;
    buffer[6] = schedule[2] % 10 + 48;
    buffer[7] = schedule[3] / 10 + 48;
    buffer[8] = schedule[3] % 10 + 48;
    buffer[9] = schedule[4] / 10 + 48;
    buffer[10] = schedule[4] % 10 + 48;
    buffer[11] = schedule[5] / 10 + 48;
    buffer[12] = schedule[5] % 10 + 48;
    buffer[14] = schedule[6] / 10 + 48;
    buffer[14] = schedule[6] % 10 + 48;
    buffer[15] = schedule[7] / 10 + 48;
    buffer[16] = schedule[7] % 10 + 48;
}
void strings2values()
{
    schedule[0] = buffer[3] - 48;
    schedule[1] = buffer[4] - 48;
    schedule[2] = (buffer[5] - 48) * 10 + buffer[6] - 48;
    schedule[3] = (buffer[7] - 48) * 10 + buffer[8] - 48;
    schedule[4] = (buffer[9] - 48) * 10 + buffer[10] - 48;
    schedule[5] = (buffer[11] - 48) * 10 + buffer[12] - 48;
    schedule[6] = (buffer[13] - 48) * 10 + buffer[14] - 48;
    schedule[7] = (buffer[15] - 48) * 10 + buffer[16] - 48;
}

void writeProgramToROM(void)
{
    // 012 3  4  56  78  90  12  34  56  78
    // set[1][1][12][30][00][12][31][20][99]
    // set[pump][day][hbegin][mbegin][sbegin][hend][mend][send][addres]
    Serial.println(">inside write to rom");
    strings2values();
    showProgram();
    address = (buffer[17] - 48) * 10 + buffer[18] - 48;
    Serial.print("address:");
    Serial.println(address);
    int offset = address * memoryLen;
    Serial.print("offset:");
    Serial.println(offset);
    for (int i = 0; i < memoryLen; i++)
    {
        EEPROM.write(offset + i, schedule[i]);
    }
}

void readProgramFromROM(void)
{
    // 012 34
    // get[99]
    Serial.println(">inside read from rom");
    address = (buffer[3] - 48) * 10 + buffer[4] - 48;
    Serial.print("address:");
    Serial.println(address);
    int offset = address * memoryLen;
    Serial.print("offset:");
    Serial.println(offset);
    for (int i = 0; i < memoryLen; i++)
    {
        schedule[i] = EEPROM.read(offset + i);
        if (schedule[i] > 100)
            schedule[i] = 99;
    }
    if (schedule[0] > 10)
        schedule[0] = 0;
    if (schedule[1] > 10)
        schedule[1] = 0;
    showProgram();
    printProgram();
}

void readAllFromROM(void)
{
    // 012 34
    // get[99]
    Serial.println(">inside read all rom");
    for (int i = 0; i < 1024; i++)
    {
        Serial.print(i);
        Serial.print(" ");
        Serial.println(EEPROM.read(i));
    }
    showProgram();
}

void showProgram(void)
{
    Serial.println("schedule values:");
    for (int i = 0; i < memoryLen; i++)
    {
        Serial.print(i);
        Serial.print(" ");
        Serial.println(schedule[i]);
    }
}

void comparator(void)
{
    Serial.println("Comparator");
    Serial.print("bufferZise:");
    Serial.println(ibuffer);
    for (int i = 0; i < ibuffer; i++)
    {
        Serial.print(i);
        Serial.print(" ");
        Serial.print(buffer[i]);
        Serial.println(" ");
    }
    if (memcmp(buffer, setCommand, 2) == 0)
    {
        writeFlag = true;
        Serial.println(">> writeFlag");
    }
    if (memcmp(buffer, getCommand, 2) == 0)
    {
        readFlag = true;
        Serial.println(">> readFlag");
    }
    if (memcmp(buffer, showCommand, 2) == 0)
    {
        showFlag = true;
        Serial.println(">> showFlag");
    }
}

void processingSerial(char c)
{
    buffer[ibuffer] = c;
    ibuffer++;

    digitalWrite(led, !digitalRead(led));
    if ((c == '\n') || (c == '\r'))
    {
        if (ibuffer > 3)
        {
            comparator();
            if (writeFlag)
            {
                writeProgramToROM();
                writeFlag = false;
            }
            if (readFlag)
            {
                readProgramFromROM();
                readFlag = false;
            }
            if (showFlag)
            {
                readAllFromROM();
                showFlag = false;
            }
        }
        ibuffer = 0;
    }
    if (ibuffer >= bufferLen)
        ibuffer = 0;
}
