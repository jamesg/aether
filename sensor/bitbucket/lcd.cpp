#include "lcd.hpp"

#include "sketch.hpp"

// HD44780 connections:
// RS - 6
// E  - 5
// D4 - 4
// D5 - 3
// D6 - 2
// D7 - 9
LiquidCrystal sensor::lcd(6, 5, 4, 3, 2, 9);

char sensor::lcd_temperature_s[LCD_LINE_SIZE];
char sensor::lcd_phase_s[LCD_LINE_SIZE];
char sensor::lcd_variety_s[LCD_LINE_SIZE];
char sensor::lcd_location_s[LCD_LINE_SIZE];

void sensor::lcd_init()
{
    lcd_temperature_s[0] = '\0';
    lcd_phase_s[0] = '\0';
    lcd_variety_s[0] = '\0';
    lcd_location_s[0] = '\0';

    lcd.begin(LCD_LINE_SIZE, LCD_LINE_COUNT);
}

void sensor::print_startup()
{
    lcd.clear();
    lcd.print(lcd_startup_s);
    callback_timer.after(4000, &print_location);
}

void sensor::print_location()
{
    lcd.clear();
    lcd.print("Location");
    lcd.setCursor(0, 1);
    lcd.print(lcd_location_s);
    callback_timer.after(4000, &print_phase);
}

void sensor::print_phase()
{
    lcd.clear();
    lcd.print("Phase");
    lcd.setCursor(0, 1);
    lcd.print(lcd_phase_s);
    callback_timer.after(4000, &print_temperature);
}

void sensor::print_temperature()
{
    if(ds18b20_temperature != NAN)
        dtostrf(ds18b20_temperature, 2, 1, lcd_temperature_s);
    lcd.clear();
    lcd.print("Temperature");
    lcd.setCursor(0, 1);
    lcd.print(lcd_temperature_s);
    callback_timer.after(4000, &print_variety);
}

void sensor::print_variety()
{
    lcd.clear();
    lcd.print("Variety");
    lcd.setCursor(0, 1);
    lcd.print(lcd_variety_s);
    callback_timer.after(4000, &print_location);
}
