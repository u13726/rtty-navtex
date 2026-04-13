# rtty-navtex for ESP32

POC to decode RTTY and NAVTEX on an ESP32.2 decoding approaches included. New one is a port from the fldigi library.

Output on a web interface or LCD/TFT screen.

Uses libs:
- preferences
- audio-tools
  - Lcd
  - tFT
 
Linked to audiokit as an ESP32 platform.

changes to fldigi library:
- use Serial to log
- use float instead of double for fft
- free up object memory when swithing between decodig  or central frequency
