/* 
    Copyright (c) 2014 Romain Berrada
    
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
    and associated documentation files (the "Software"), to deal in the Software without restriction, 
    including without limitation the rights to use, copy, modify, merge, publish, distribute, 
    sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:
 
    The above copyright notice and this permission notice shall be included in all copies or 
    substantial portions of the Software.
 
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
 
#ifndef INCLUDE_RGBLED_H
#define INCLUDE_RGBLED_H
 
#include "mbed.h"
 
class RGBLed {
private:
    DigitalOut _red;
    DigitalOut _green;
    DigitalOut _blue;
        
public:
    /** RGB Color class \n
    Colors have been defined and are ready to use in RGBLed class
    */
    class Color {
    private:
        bool _r; /**< Red component of the Color */
        bool _g; /**< Green component of the Color */
        bool _b; /**< Blue component of the Color */
        Color(bool r, bool g, bool b); /**< Constructor */
        friend class RGBLed;    
    };
    
    /** Create a RGBLed, containing the informations about the LED pinout.
        @param redPin the pin linked to the Red LED
        @param greenPin the pin linked to the green LED
        @param blue the pin linked to the blue LED
    */
    RGBLed(PinName redPin, PinName greenPin, PinName bluePin);    
    
    /** Change the color of the LED.
        @param color the color to display
        @see RGBLed::Color
    */
    void setColor(RGBLed::Color& color);
 
    static Color BLACK; /**< Black Color (no color) */
    static Color RED; /**< Red Color */
    static Color GREEN; /**< Green Color */
    static Color BLUE; /**< Blue Color */
    static Color MAGENTA; /**< Magenta Color (Red + Blue) */
    static Color CYAN; /**< Cyan Color (Green + Blue) */
    static Color YELLOW; /**< Yellow Color (Red + Green) */
    static Color WHITE; /**< White Color (Red + Green + Blue) */
};
 
#endif