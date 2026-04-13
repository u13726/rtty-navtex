#ifndef	_CONFIG_H
#define	_CONFIG_H
#include "AudioTools.h"
#include "AudioTools/AudioLibs/AudioBoardStream.h"
#include "RingB.h"
//#define LCD 1
  #define TFT 1
  #define SERIAL 1
  #define NAVFLI 1
  #define SAVE 1
  #define WEB 1

  #define SSIDLEN 25+1
  extern  char ssid[SSIDLEN];
  extern  char password[SSIDLEN];
  
extern short baud_tab[];// = { 45, 50, 100, 102, 104, 198, 196, 200, 240, 0 };
extern short bidx;// = 1;
extern short shift_tab[];// = { 85, 170, 450, 0 };
extern  short sidx;// = 1;
  
#define G_SAMRAT (11025 * 3)
#define F_SAMRAT (11025 * 1           )

  extern uint8_t oneoutnav(uint8_t);
  extern bool navfli;

extern void close_nav_f();
extern void close_nav_g();
extern void setup_nav_fli();
extern void res_f();
extern void setup_nav_g();

extern RingB han;

#endif