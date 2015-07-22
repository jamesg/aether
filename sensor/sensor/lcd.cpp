#include "lcd.hpp"

#include "sketch.hpp"

#include "LiquidCrystal.h"

void print_startup()
{
    lcd.clear();
    lcd.print(lcd_startup_s);
    callback_timer.after(4000, &print_location);
}

void print_location()
{
    lcd.clear();
    lcd.print("Location");
    lcd.setCursor(0, 1);
    lcd.print(lcd_location_s);
    callback_timer.after(4000, &print_phase);
}

void print_phase()
{
    lcd.clear();
    lcd.print("Phase");
    lcd.setCursor(0, 1);
    lcd.print(lcd_phase_s);
    callback_timer.after(4000, &print_temperature);
}

void print_temperature()
{
    if(ds18b20_temperature != NAN)
        dtostrf(ds18b20_temperature, 2, 1, lcd_temperature_s);
    lcd.clear();
    lcd.print("Temperature");
    lcd.setCursor(0, 1);
    lcd.print(lcd_temperature_s);
    callback_timer.after(4000, &print_variety);
}

void print_variety()
{
    lcd.clear();
    lcd.print("Variety");
    lcd.setCursor(0, 1);
    lcd.print(lcd_variety_s);
    callback_timer.after(4000, &print_location);
}
