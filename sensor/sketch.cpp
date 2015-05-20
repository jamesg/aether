#include "Arduino.h"

#include "LiquidCrystal.h"
#include "OneWire.h"

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
OneWire ds(10);

void setup(void)
{
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
    Serial.print(str);
    lcd.clear();
    lcd.print(str);
}

void loop()
{
    char buf[16];

    int moisture_val = decode_moisture();
    snprintf(buf, sizeof(buf), "moisture = %d\n", moisture_val);
    log_string(buf);

    int Whole, Fract;
    decode_temperature(&Whole, &Fract);

    snprintf(buf, sizeof(buf), "temp = %d.%d\n", Whole, Fract);
    log_string(buf);

    delay(1000);
}

