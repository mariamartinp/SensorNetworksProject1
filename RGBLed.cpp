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
 
#include "RGBLed.h"
 
RGBLed::Color::Color(bool r, bool g, bool b) : _r(r), _g(g), _b(b) {
}
 
RGBLed::RGBLed(PinName redPin, PinName greenPin, PinName bluePin) : _red(redPin), _green(greenPin), _blue(bluePin) {
    this->setColor(RGBLed::BLACK); // Clear the LED output
}
 
void RGBLed::setColor(Color& color) {
    _red = color._r;
    _green = color._g;
    _blue = color._b;
}
 
RGBLed::Color RGBLed::WHITE = RGBLed::Color(1,1,1);
RGBLed::Color RGBLed::CYAN = RGBLed::Color(0,1,1);
RGBLed::Color RGBLed::YELLOW = RGBLed::Color(1,0,1);
RGBLed::Color RGBLed::MAGENTA = RGBLed::Color(1,1,0);
RGBLed::Color RGBLed::BLUE = RGBLed::Color(0,1,0);
RGBLed::Color RGBLed::RED = RGBLed::Color(1,0,0);
RGBLed::Color RGBLed::GREEN = RGBLed::Color(0,0,1);
RGBLed::Color RGBLed::BLACK = RGBLed::Color(0,0,0);