
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 14
#define TFT_CS 15
Adafruit_ILI9341 tft = Adafruit_ILI9341(15,14,12);
 // The scrolling area must be a integral multiple of TEXT_HEIGHT
#define TEXT_HEIGHT 8 // Height of text to be printed and scrolled
#define BOT_FIXED_AREA 0 // Number of lines in bottom fixed area (lines counted from bottom of screen)
#define TOP_FIXED_AREA 8 // Number of lines in top fixed area (lines counted from top of screen)
#define YMAX 320 // Bottom of screen area
#define TFT_BLACK ILI9341_BLACK
#define TFT_WHITE ILI9341_WHITE
// The initial y coordinate of the top of the scrolling area
uint16_t yStart = TOP_FIXED_AREA;
// yArea must be a integral multiple of TEXT_HEIGHT
uint16_t yArea = YMAX-TOP_FIXED_AREA-BOT_FIXED_AREA;
// The initial y coordinate of the top of the bottom text line
uint16_t yDraw = YMAX - BOT_FIXED_AREA - TEXT_HEIGHT;

// Keep track of the drawing x coordinate
uint16_t xPos = 0;

// For the byte we read from the serial port
byte data = 0;

// A few test variables used during debugging
bool change_colour = 1;
bool selected = 1;

// We have to blank the top line each time the display is scrolled, but this takes up to 13 milliseconds
// for a full width line, meanwhile the serial buffer may be filling... and overflowing
// We can speed up scrolling of short text lines by just blanking the character we drew
int blank[38]; // We keep all the strings pixel lengths to optimise the speed of the top line blanking


short rota=0;
short txts=2;
// ##############################################################################################
// Call this function to scroll the display one text line
// ##############################################################################################
void scrollAddress(uint16_t vsp) {

  tft.scrollTo(vsp);return;
  tft.writeCommand((uint8_t)ILI9341_VSCRSADD); // Vertical scrolling pointer
//  tft.write((uint8_t)vsp>>8);
  tft.write16(vsp);
}

int scroll_line() {
   if(rota) return 0;
  int yTemp = yStart; // Store the old yStart, this is where we draw the next line
  // Use the record of line lengths to optimise the rectangle size we need to erase the top line
  tft.fillRect(0,yStart,blank[(yStart-TOP_FIXED_AREA)/TEXT_HEIGHT],TEXT_HEIGHT, TFT_BLACK);

  // Change the top of the scroll area
  yStart+=TEXT_HEIGHT;
  // The value must wrap around as the screen memory is a circular buffer
  if (yStart >= YMAX - BOT_FIXED_AREA) yStart = TOP_FIXED_AREA + (yStart - YMAX + BOT_FIXED_AREA);
  // Now we can scroll the display
  scrollAddress(yStart);
  return  yTemp;
}
void scrollOne(void) {
       xPos = 0;
      yDraw = scroll_line(); // It can take 13ms to scroll and blank 16 pixel lines
        //change_colour = 1; // Line to indicate buffer is being emptied
  }


// ##############################################################################################
// Setup a portion of the screen for vertical scrolling
// ##############################################################################################
// We are using a hardware feature of the display, so we can only scroll in portrait orientation
void setupScrollArea(uint16_t tfa, uint16_t bfa) {
  tft.setScrollMargins(0, 0);return;
  tft.writeCommand((uint8_t)ILI9341_VSCRDEF); // Vertical scroll definition
  //tft.write((uint8_t)tfa >> 8);           // Top Fixed Area line count
  tft.write16(tfa);
  //tft.write((uint8_t)(YMAX-tfa-bfa)>>8);  // Vertical Scrolling Area line count
  tft.write16(YMAX-tfa-bfa);
  ///tft.write((uint8_t)bfa >> 8);           // Bottom Fixed Area line count
  tft.write16(bfa);
}
// ##############################################################################################
// Setup the vertical scrolling start address pointer
// ##############################################################################################

void setup_tft() {
  // Setup the TFT display

  tft.setRotation(rota); // Must be setRotation(0) for this sketch to work correctly
  tft.fillScreen(TFT_BLACK);
                                                                                    
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.fillRect(0,0,240,16, TFT_BLACK);
 // tft.drawString(" Serial Terminal - 9600 baud ",120,0,2);

  // Change colour for scrolling zone text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.setTextSize(txts);

  // Setup scroll area
  setupScrollArea(TOP_FIXED_AREA, BOT_FIXED_AREA);

  // Zero the array
  for (byte i = 0; i<36; i++) blank[i]=0;
}

