#include "Arduino.h"

#include "ArduinoJson.h"
#include "LiquidCrystal.h"
#include "OneWire.h"

#define INCOMING_SIZE 200

typedef StaticJsonBuffer<100> json_buffer_type;

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
OneWire ds(10);
char incoming_buffer[INCOMING_SIZE];
int incoming_index;

void setup(void)
{
    incoming_index = 0;

    lcd.begin(16,2);
    lcd.print("Plant Monitor");
    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);
    Serial.begin(115200);
}

int decode_moisture()
{
    int moisture_val = analogRead(0);
    return moisture_val;
}

void decode_temperature(int *Whole, int *Fract)
{
    byte i;
    byte present = 0;
    byte data[12];
    byte addr[8];

    ds.reset_search();
    if(!ds.search(addr)) {
        ds.reset_search();
        return;
    }

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);         // start conversion, parasite power

    delay(1000);     // maybe 750ms is enough, maybe not

    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE);         // Read Scratchpad

    for ( i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
    }
    int HighByte, LowByte, TReading, SignBit, Tc_100;
    LowByte = data[0];
    HighByte = data[1];
    TReading = (HighByte << 8) + LowByte;
    SignBit = TReading & 0x8000;  // test most sig bit
    if(SignBit)
        TReading = (TReading ^ 0xffff) + 1;
    Tc_100 = (6 * TReading) + TReading / 4;
    *Whole = Tc_100 / 100;
    *Fract = Tc_100 % 100;
}

void log_string(const char *str)
{
    lcd.clear();
    lcd.print(str);
}

void send_status()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "status";
    JsonArray& params = root.createNestedArray("params");
    root.printTo(Serial);
    Serial.println();
}

void send_temperature()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "record_temperature";
    JsonArray& params = root.createNestedArray("params");
    int Whole, Fract;
    decode_temperature(&Whole, &Fract);
    params.add(Whole + (double)Fract/100, 1);
    root.printTo(Serial);
    Serial.println();
}

void send_moisture()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "record_moisture";
    JsonArray& params = root.createNestedArray("params");
    int moisture_val = decode_moisture();
    params.add(moisture_val);
    root.printTo(Serial);
    Serial.println();
}

void loop()
{
    log_string("send");
    send_status();
    delay(1000);

    if(Serial.available())
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "serial %d", Serial.available());
        log_string(buf);
        delay(1000);
    }

    while(Serial.available())
    {
        incoming_buffer[incoming_index] = Serial.read();
        if(incoming_buffer[incoming_index] == '\n')
        {
            incoming_buffer[incoming_index] = '\0';
            json_buffer_type buffer;
            JsonObject& result = buffer.parseObject(incoming_buffer);

            log_string("result");
            delay(1000);
            char buf[16];
            snprintf(buf, sizeof(buf), "%d s %s", incoming_index, incoming_buffer);
            log_string(buf);
            delay(1000);

            const char *status = result["result"];
            if(status)
                log_string(status);
            else
                log_string("invalid result");
            delay(1000);

            incoming_index = 0;
            while(Serial.available()) Serial.read();
        }
        else
        {
            incoming_index++;
            if(incoming_index >= INCOMING_SIZE)
            {
                incoming_index = 0;
                while(Serial.available()) Serial.read();
            }
        }
    }

    //char buf[16];

    //int moisture_val = decode_moisture();
    //int Whole, Fract;
    //decode_temperature(&Whole, &Fract);

    //snprintf(buf, sizeof(buf), "moisture = %d", moisture_val);
    //log_string(buf);

    //delay(2000);

    //snprintf(buf, sizeof(buf), "temp = %d.%d", Whole, Fract);
    //log_string(buf);

    //delay(2000);

    //send_temperature();
    //send_moisture();
}

