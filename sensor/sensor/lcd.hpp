#ifndef SENSOR_LCD_HPP
#define SENSOR_LCD_HPP

#include "sketch.hpp"

#include "LiquidCrystal.h"

namespace sensor
{
    extern LiquidCrystal lcd;

    const int LCD_LINE_SIZE = 16;
    const int LCD_LINE_COUNT = 2;

    extern char lcd_temperature_s[LCD_LINE_SIZE];
    extern char lcd_phase_s[LCD_LINE_SIZE];
    extern char lcd_variety_s[LCD_LINE_SIZE];
    extern char lcd_location_s[LCD_LINE_SIZE];

    void lcd_init();

    void print_startup();
    void print_location();
    void print_phase();
    void print_temperature();
    void print_variety();
}

#endif
