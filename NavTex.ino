#include "config.h"
#include "web.h"
#ifdef LCD
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
TwoWire Wire_1 = TwoWire(0x27);
LiquidCrystal_PCF8574 lcd(0x27);
#endif
#ifdef TFT
#include "DISPLAY.h"
#endif
#ifdef SAVE
#include "sdd.h"
#endif

#include"NavTexG.h"

#include "NavTex_rx.h"
#include "NavTexFLI.h"
#include "NavTex.h"
short baud_tab[] = { 45, 50, 100, 102, 104, 198, 196, 200, 240, 0 };
short bidx = 1;
short shift_tab[] = { 85, 170, 450, 0 };
short sidx = 1;
  


float cf;
float mgb = 10;
int loglevel = 0;

#include <Preferences.h>
Preferences pref;

RingB navcha(256);
#define PIN_KEY1 36
#define PIN_KEY2 13
#define PIN_KEY3 19
#define PIN_KEY4 23
#define PIN_KEY5 18
#define PIN_KEY6 5
bool insync = false;
float bpf = 0, bpm = 0;
int LF = 300, HF = 3000, swm = 715, sws = 1285;
char screen[40 + 1][80 + 1];
bool zoom = false;
bool navtex = false;
bool navfli = false;
char baudot[2][33] = { "_E\rA SIU\nDRJNFCKTZLWHYPQOBG<MXV>",
                       "_3\r- *87\n$4',!:(5')2#6019?&<./;>" };
char ccir[36] = { 0x78, 0x6C, 0x5C, 0x0F, 0x33, 0x66, 0x5A, 0x36, 0X6A,
                  0x47, 0x72, 0x1D, 0x53, 0x56, 0x1B, 0x35, 0x69, 0x4D, 0x17,
                  0x1E, 0x65, 0x39, 0x59,
                  0x71, 0x2D, 0x2E, 0x55,
                  0x4B, 0x74, 0x4E, 0x3C, 0x27, 0x3A, 0x2B, 0x63, 0 };
char ccirl[2][36] = { { '\n', '\n', ' ', 'a', 'b', 'r', '>', '<', 'c',
                        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0 },
                      { '\n', '\n', ' ', 'a', 'b', 'r', '>', '<', 'c',
                        '-', '?', ':', '$', '3', '!', '&', '#', '8', '\'',
                        '(', ')', '.', ',', '9', '0', '1', '4', '*', '5', '7', ':', '2', '/', '6', '\"', 0 } };


bool valsit(uint8_t c) {
  return (strchr(ccir, c) != NULL);
}

void Search(float frequency, float magnitude, void *ref) {
  static int pf, som, somm, soms, nf;
  if (frequency != pf) {
    int x, y;
#ifdef TFT
    if (LF == frequency) tft.fillScreen(TFT_BLACK);
#endif
    if (nf) {
      if (((som) / nf) > bpm) {
        bpm = (som) / nf;
        bpf = pf;
      }
      x = map(som / nf, 0, 75, 0, 240);
      y = map(pf, LF, HF, 0, 320);
#ifdef TFT
      tft.drawFastHLine(0, y, x, pf == MARK ? ILI9341_BLUE : pf == SPACE ? ILI9341_RED
                                                                         : TFT_WHITE);
#endif
      if (custer) {
        char buf[100];
        sprintf(buf, "F: %d ", pf);
        web_log(buf);
        y = map(abs(som) / nf, 0, 75, 0, 80);
        for (int iy = 0; custer && iy < y; iy++)
          web_log("-");
        sprintf(buf, " %d \r", (som) / nf);
        web_log(buf);
        x = map((som) / nf, 0, 75, 0, 40);
        x = max(0, min(x, 39));
        if (zoom)
          y = map(pf, swm, sws, 0, 80);
        else
          y = map(pf, LF, HF, 0, 80);
        y = max(0, min(y, 79));
        for (; x > 0; x--) screen[x][y] = '*';
      }
    }
    som = magnitude;
    somm = frequency == MARK ? magnitude : 0;
    soms = frequency == SPACE ? magnitude : 0;
    pf = frequency;
    nf = 1;
  } else {
    som += magnitude;
    somm += frequency == MARK ? magnitude : 0;
    soms += frequency == SPACE ? magnitude : 0;
    nf++;
    pf = frequency;
  }
}
char resp[128] = "";
char *p2b = resp;
static short cof = 2;
float volume = 1.0;
float volumeOut = 1.0;
int sn = 0;
short noftun = 0, maxtun;
float tuncur = 0.0, tunOK;

float fat;
bool search = false;
bool fgr = false;
int MSE = 22;
unsigned long lasts, chkdoa, chktun;

String ans;
bool hsb = true;
int sdelay = 100;

uint32_t syn = 0, out = 0;
char oversv[5 + 1] = ".....";
short bitcode = 5;
int bsize = 96, sbsize = 96;
void setBS(short s) {
if(! navfli)
  {static short pbs = -1;
  if (s != pbs || s == 0) {
    if (s == 0) s = 1;
    gcfg.block_size = (((1000 / baud_tab[bidx]) * (G_SAMRAT / 10))) / (overs * s * 100) + 0;
    //Serial.println(gcfg.block_size);
    goertzel->end(false);
    goertzel->begin(gcfg);
    pbs = s;
    //Serial.printf("<<< %d %d>>>\n", baud_tab[bidx], gcfg.block_size);
  }
  }
}
bool hit() {
  if (overs == 1) return oversv[0] == 'T';
  if (overs == 2) return (oversv[0] == 'T' && oversv[1] == 'T');
  if (overs == 3) return ((oversv[0] == 'T' && oversv[1] == 'T') || (oversv[1] == 'T' && oversv[2 == 'T']));
  if (overs == 4) return (((oversv[0] == 'T') && (oversv[1] == 'T'))
                          || ((oversv[1] == 'T') && (oversv[2] == 'T'))
                          || ((oversv[3] == 'T') && (oversv[2] == 'T')));
  if (overs == 5) return (((oversv[0] == 'T') && (oversv[1] == 'T') && (oversv[2] == 'T'))
                          || ((oversv[1] == 'T') && (oversv[2] == 'T') && (oversv[3] == 'T'))
                          || ((oversv[2] == 'T') && (oversv[3] == 'T') && (oversv[4] == 'T')));
}

void oneout(uint8_t b) {
  static bool cs = false;
  static short n = 0;
  char c, *s;
  c = baudot[cs ? 1 : 0][b];
  if (c == '<') {
    c = '_';
    cs = true;
  } else if (c == '>') {
    c = '_';
    cs = false;
  } else if (c == '_')
    ;
  else if (c == ' ') cs = false;
  if (c == '_')
    ;
  else {
#ifdef SERIAL
    static bool spo = true;
    if ((c == '\n' || c == '\r') && (!spo)) {
      spo = true;
      Serial.println("");
    } else if (c != '\n' && c != '\r') {
      spo = false;
      Serial.print(c);
    }
#endif
    out++;
    lasts = millis();
#ifdef WEB
    web_log(c);
#endif
#ifdef TFT
    chrscr(c);
#endif
#ifdef LCD
    chrscr(c);
#endif
#ifdef SAVE
    chrfil(c);
#endif
  }
}
#ifdef LCD
char lcdDisplay[5][21] = { "....................",
                           "....................",
                           "....................",
                           "....................",
                           "...................." };  // 4 lines of 20 character buffer
#endif
void chrscr(char ch) {
  static short n = 0;
  if (ch == '_') return;
#ifdef LCD
  if (ch == '\n' || ch == '\r') ch = ' ';
  static short c = 0, r = 0;
  /* lcdDisplay[4][c]=ch;
 c++;if (c==20)
 {strcpy(lcdDisplay[0],lcdDisplay[1]);
  strcpy(lcdDisplay[1],lcdDisplay[2]);
  strcpy(lcdDisplay[2],lcdDisplay[3]);
  strcpy(lcdDisplay[3],lcdDisplay[4]);
  lcd.setCursor(0,0);
  lcd.print(lcdDisplay[0]);
  lcd.print(lcdDisplay[2]);
  lcd.print(lcdDisplay[1]);
  lcd.print(lcdDisplay[3]);
  c=0;
  }
  */
  lcdDisplay[r][c] = ch;
  c++;
  if (c == 20) {
    if (r == 4) {
      lcd.setCursor(0, 0);
      lcd.print(lcdDisplay[0]);
      lcd.print(lcdDisplay[2]);
      lcd.print(lcdDisplay[1]);
      lcd.print(lcdDisplay[3]);
      r = 0;
    }
    c = 0;
    r++;
  }
#endif
#ifdef TFT
  if (ch != '\n' && ch != '\r') {
    tft.drawChar(xPos, yDraw, ch, TFT_WHITE, TFT_BLACK, 1, 1);
    xPos += 8;
    blank[(0 + (yStart - TOP_FIXED_AREA) / TEXT_HEIGHT) % 37] = xPos;  // Keep a record of line lengths
  }
  n++;
  if (ch == '\n' || ch == '\r') {
    n = 0;
    scrollOne();
  } else {
    if ((n) % 33 == 0) { scrollOne(); }
  }
#endif
}

char cch(uint8_t b) {
  char c, *s;
  if (s = strchr(ccir, b)) {
    c = ccirl[0][s - ccir];  //CCIR476.Decode(b,true);
  } else {
    c = '*';
  }
  return (c);
}
#ifdef SAVE
void chrfil(char c) {
  static String ans = "";
  static int presta = 0;
  int o, n, m;
  if (c == '_') return;
  ans += c;
  n = ans.indexOf("ZCZC", presta);
  m = n >= 0 ? ans.indexOf("NNNN", presta = n) : -1;
  o = n >= 0 ? ans.indexOf("ZCZC", n + 4) : -1;
  if (o >= 0 && (m == -1 || o < m)) m = o;
  //Serial.printf("%d %d\n",n,m);
  if ((m >= 0) && (n >= 0)) {
    String msg = ans.substring(n, m);
    String hdr = "/" + ans.substring(n + 5, min(ans.indexOf("\n", n + 8), n + 10));
    hdr.replace("*", "");
    hdr.replace(")", "");
    hdr.replace(" ", "_");
    hdr.replace("?", "");
    hdr.replace("\"", "");
    hdr.replace("\'", "");
    hdr.replace("<", "");
    hdr.replace(">", "");
    hdr.replace("&", "");
    hdr = hdr.substring(0, 8);
    hdr  += ".txt";
    writeFile(LittleFS, hdr.c_str(), msg.c_str());
    ans = ans.substring(m + 5, ans.length());
    presta = 0;
  }
}
#endif
uint8_t oneoutnav(uint8_t b) {
   char c, *s;
  if (navfli)
    {c=(char)b;    
    #ifdef SERIAL
     Serial.print(c);
     #endif
    }
 else
  {static bool cs = false, csrep = false;
  static short n = 0;
  if (s = strchr(ccir, b)) {
    c = ccirl[cs ? 1 : 0][s - ccir];  //CCIR476.Decode(b,true);
    if (c == 0) c = '_';
    tuncur += 1;
  } else {
    c = '*';
  }
  if (b == 0x36) {
    cs = true;
    c = '_';
  }
  if (b == 0x5A) {
    cs = false;
    c = '_';
  }
  if (b == 0x0F || b == 0x33 || b == 0x6A) c = '_';
  if (c == '\n' || c == '\r') { cs = false; }
  insync = (c == 'r' || c == 'a');
  if (c == 'r' || c == 'a') {
    Serial.print(c);
    return (b);
  }
  static bool spo2 = true;
  if ((c == '\n' || c == '\r') && (!spo2)) {
    spo2 = true;
    Serial.println("");
  } else if (c != '\n' && c != '\r') {
    spo2 = false;
#ifdef SERIAL
    if (c != '_') Serial.print(c);
#endif
  }
  } 
  out++;
  if (out % 80 == 0) Serial.println();
  lasts = millis();
  if (c != '_') web_log(c);
#ifdef LCD
  chrscr(c);
#endif
#ifdef TFT
  chrscr(c);
#endif
#ifdef SAVE
  chrfil(c);
#endif
  return b;
}

bool validSample(float frq, float mag, float pmag) {
  if (frq != MARK && frq != SPACE) return false;
  if ((pmag < 5) && (mag < 5)) return false;
  if (search) return false;
  return true;
}
short synpos = 3;

bool navsyn(char *s) {
  for (int i = 0; i < synpos; i++) {
    short p = 0, m = 0, j;
    for (j = p = m = 0; j < 7; j++)
      if (s[i * 7 + j] == '+') p++;
      else m++;
    if (m != 3 || p != 4)
      //  if (p==5 && s[i*7+j]=='-') {s[i*7+j]='-';}
      //  else if (m==4 && s[i*7+j]=='-') {s[i*7+j]='+';}
      //  else
      return false;
  }
  return true;
}
bool bitpat = true;
static uint8_t b = 0;
static unsigned long tick = 0, tickcnt = 0;
static float pf, pm, dc;
static unsigned long pmi;
static bool nav_in_sync = false, skip = false;
static char BUB[49 + 1] = ".................................................";
static short sixsev = 0;
static short reqacc = 0;
void decode_nav(float frequency, float magnitude) {
  if (!validSample(frequency, magnitude, pm)) return;
  if (dc == 0) {
    pmi = millis();
    pf = frequency;
    pm = magnitude;
    dc = 1;
    return;
  }
  bool dodgy;
  if (dodgy = (abs(magnitude - pm) < (magnitude * 0.005))) {  //Serial.printf("dodgy %f %d %s %f %F\n",abs(magnitude-pm),covers,pf==frequency?"SAME":"diff",frequency,pf);//
  }
  if (pm > magnitude) { frequency = pf; }
  pmi = millis();
  pf = frequency;
  pm = magnitude;
  dc = 0;
  if (covers >= 0)
    oversv[covers] = /*dodgy ? 'D' :*/ (frequency == MARK) ? 'T'
                                                           : 'F';
  covers++;
  if (covers < overs) { return; }
  covers = 0;
  frequency = hit() ? SPACE : MARK;
  if (loglevel == 2)
    Serial.printf("%f %d %s\n", frequency, b, BUB);
  if (loglevel == 4) Serial.printf("%f\n", frequency);
  for (int k = 1; k < (7 * synpos); k++)
    BUB[k - 1] = BUB[k];
  BUB[(7 * synpos) - 1] = frequency == MARK ? '+' : '-';
  if ((!nav_in_sync)) {
    if (BUB[0] == '.') return;
    if (!navsyn(BUB)) return;
    nav_in_sync = true;
    sixsev = 0;
    if (loglevel == 1) Serial.print("SYN");
  }
  if (sixsev == 0) {
    static short errat = 0;
    sixsev = 6;
    b = 0;
    int k = 1;
    static char sb;
    sb = BUB[6];
    for (int l = 0; l < 7; l++, k *= 2)
      b += (BUB[l] == '+' ? k : 0);
    if (!valsit(b) && bitpat) {
      uint8_t bp = 0;
      k = 2;
      bp = (sb == '+' ? 1 : 0);
      for (int l = 0; l < 6; l++, k *= 2)
        bp += (BUB[l] == '+' ? k : 0);
      if (valsit(bp) && bitpat) {
        b = bp;
        Serial.print("R2");
        sixsev = 5;
      } else {
        k = 1;
        //  Serial.print("R?");
        for (int l = 1; l < 8; l++, k *= 2)
          bp += (BUB[l] == '+' ? k : 0);
        if (valsit(bp)) {
          b = bp;
          Serial.print("R1");
          sixsev = 7;
        }
      }
    }
    navcha.write(b);
    if (!valsit(b)) errat += 10;
    else errat = max(errat - 5, 0);
    if (errat >= 10) {
      nav_in_sync = false;
      errat = 0;
    }
  } else
    sixsev--;
}

void GoetzelCallback(float frequency, float magnitude, void *ref) {
  if (navtex) {
    decode_nav(frequency, magnitude);
    return;
  }
  static uint8_t b = 0;
  static int8_t i = 0;
  static float pf, pm, dc;
  static unsigned long pmi;
  static char RUB[6 + 1] = "123456";
  static bool rtty_in_sync = false, skip = false;
  if (!validSample(frequency, magnitude, pm)) return;
  if (dc == 0) {
    pmi = millis();
    pm = magnitude;
    pf = frequency;
    dc = 1;
    return;
  } else if (pm > magnitude) {
    frequency = pf;
  }
  pmi = millis();
  pf = frequency;
  pm = magnitude;
  dc = 0;
  oversv[covers++] = frequency == MARK ? 'T' : 'F';
  if (covers < overs) return;
  covers = 0;
  frequency = hit() ? MARK : SPACE;
  if (loglevel == 2) Serial.printf("%f %d %d <%s>\n", frequency, i, b, RUB);
  if (loglevel == 4) Serial.printf("%f\n", frequency);
  //if(skip){skip=false;i=1;b=0;return;}
  for (int ri = 0; ri < (hsb ? 4 : 2); ri++)
    RUB[ri] = RUB[ri + 1];
  RUB[hsb ? 4 : 2] = frequency == MARK ? '+' : '-';
  if ((!rtty_in_sync)) {
    static bool rcfg = false;
    if (hsb) setBS(2);
    if (hsb ? (RUB[0] == '+' && RUB[1] == '+' && RUB[2] == '+' && RUB[3] == '-' && RUB[4] == '-')
            : (RUB[0] == '+' && RUB[1] == '+' && RUB[2] == '-')) {
      rtty_in_sync = true;
      i = 0;
      RUB[0] = RUB[1] = RUB[2] = RUB[3] = RUB[4] = RUB[5] = '*';
      if (hsb) { setBS(1); }
      if (loglevel == 1) { Serial.println("<<<SYN>>>"); }
      syn++;
    } else {
      skip = true;
      delayMicroseconds(sdelay);
      return;
    }
  }
  if (loglevel == 2) Serial.printf(">>>> %f %d %d %s\n", frequency, i, b, RUB);
  if (i == 0) {
    if (frequency == SPACE)
      i = 1;
    else
      rtty_in_sync = false;
  } else if (i == 6) {
    if (frequency == MARK) {
      i = 7;
      if (hsb) setBS(2);
    } else {
      rtty_in_sync = false;
    }
  } else if (i == 7) {
    if (frequency == MARK) {
      i = 0;
      if (hsb) setBS(1);
      return;
    } else {
      rtty_in_sync = false;
    }
  } else if (i == 1) {
    b = (frequency == MARK ? 1 : 0);
    i++;
  } else if (i == 2) {
    b += (frequency == MARK ? 2 : 0);
    i++;
  } else if (i == 3) {
    b += (frequency == MARK ? 4 : 0);
    i++;
  } else if (i == 4) {
    b += (frequency == MARK ? 8 : 0);
    i++;
  } else if (i == 5) {
    b += (frequency == MARK ? 16 : 0);
    i++;
    oneout(b);
  }

  //b=0;
}
float bq = 3.605;
float hq = -2.0;
void setOutputVolume(float volume) {
#ifdef LCD
  Wire.end();
  Wire.begin(33, 32);  // custom i2c port on ESP
 #endif
  kit->setVolume(volumeOut);
 #ifdef LCD
  Wire.end();
  Wire.begin(21, 22);
#endif
}
bool hanser=true;
float gengetf()
{if (hanser)
   return(Serial.parseFloat());
  else
   return(han.parseFloat());
}

bool scan = false, tune = false;
bool filact = false;
bool filuse = false;
void handle(char ch) {
  Serial.print(ch);
  if (ch == 'i') {
    float c;
    c = MARK;
    MARK = SPACE;
    SPACE = c;
    navtex = false;
  } else if (ch == 'I') {
    SPACE = MARK;
    MARK = SPACE - shift_tab[sidx];
    rstfre(MARK, SPACE);
  } else if (ch == 'P') {
    rstfre(MARK, SPACE);
    handle('?');
  } else if (ch == '>') {
    SPACE += 10;
    MARK += 10;
    rstfre(MARK, SPACE);
  } else if (ch == '<') {
    SPACE -= 10;
    MARK -= 10;
    rstfre(MARK, SPACE);
  } else if (ch == ')') {
    SPACE += 100;
    MARK += 100;
    rstfre(MARK, SPACE);
  } else if (ch == '(') {
    SPACE -= 100;
    MARK -= 100;
    rstfre(MARK, SPACE);
  } else if (ch == '+') {
    SPACE += 1;
    MARK += 1;
    rstfre(MARK, SPACE);
  } else if (ch == '-') {
    SPACE -= 1;
    MARK -= 1;
    rstfre(MARK, SPACE);
  } else if (ch == 'M') {
    MARK = gengetf();
    Serial.printf("Set at m:%f s:%f,m:%f\n", MARK, SPACE, mgb);
  } else if (ch == 'E') {
    SPACE = gengetf();
    Serial.printf("Set at m:%f s:%f,m:%f\n", MARK, SPACE, mgb);
  } else if (ch == '|') {
    MARK = gengetf() - (shift_tab[sidx] / 2);
    SPACE = MARK + shift_tab[sidx];
    handle('P');
  } else if (ch == 'o') {
    overs = (overs > 1 ? overs - 1 : 1);
    Serial.printf("<<<%d>>>\n", overs);
    setBS(0);
  } else if (ch == '?') {
    printf("baud:%d,mark:%.0f,space:%.0f,shift:%d, %s , overs:%d SYN  %f filt :%srs\n",
           baud_tab[bidx], MARK, SPACE, shift_tab[sidx], hsb ? "HSB" : "No HSB", 
           overs, (float)((float)syn / (out > 0 ? (float)out : 1.0)), filact ? "yes" : "no");
          Serial.printf("%s heap : %d free %d \n",navfli?"fli":"gro",ESP.getHeapSize(), ESP.getFreeHeap());

  } else if (ch == 'l') {
    loglevel--;
    Serial.printf("<<<%d>>>\n", loglevel);
  } else if (ch == 'L') {
    loglevel++;
    Serial.printf("<<<%d>>>\n", loglevel);
  } else if (ch == 'v') {
    volumeOut -= 0.1;
    Serial.printf("<<<%f>>>\n", volumeOut);
    // kit.setInputVolume(volume);
    setOutputVolume(volumeOut);  // custom i2c port on ESP

  } else if (ch == 'V') {
    volumeOut += 0.5;
    Serial.printf("<<<%f>>>\n", volumeOut);
    //kit.setInputVolume(volume);
    setOutputVolume(volumeOut);
  } else if (ch == 'w') {
    volume -= 0.1;
    Serial.printf("<<<%f>>>\n", volume);
    if(kit) kit->setInputVolume(volume);
    //setOutputVolume(volumeOut);  // custom i2c port on ESP

  } else if (ch == 'W') {
    volume += 0.5;
    Serial.printf("<<<%f>>>\n", volume);
    if (kit) kit->setInputVolume(volume);
    //setOutputVolume(volumeOut);
  } else if (ch == 'R') {
    b = 0;
    tick = 0;
    tickcnt = 0;
    pf = pm = dc = 0;
    pmi = 0;
    nav_in_sync = false;
    skip = false;
    strcpy(BUB, ".................................................");
    strcpy(oversv, "DDDDD");
    sixsev = 0;
    reqacc = 0;
  } else if (ch == 'B') {
    bidx++;
    if (!baud_tab[bidx]) bidx--;
    setBS(1);
    handle('?');
  } else if (ch == 'b') {
    if (bidx > 0) bidx--;
    setBS(1);
    handle('?');
  } else if (ch == '!') {
    oer();
    ESP.restart();
  } else if (ch == 'Q') {
    synpos = min(7, synpos + 1);
    Serial.printf("SYBPOS:%d\n", synpos);

  } else if (ch == 'q') {
    synpos = max(3, synpos - 1);
    Serial.printf("SYBPOS:%d\n", synpos);
  } else if (ch == 'd') {
    bitpat = !bitpat;
    Serial.print(bitpat);
  } else if (ch == 'e') {
    swm = MARK - 150;
    sws = SPACE + 150;
    zoom = !zoom;
    Serial.print(zoom);
    putmar(MARK, SPACE);
  } else if (ch == 'G') {
    if(navfli) 
    {close_nav_f();
     navfli=false;
     setup_nav_g();
    }
  } else if (ch == 'g') {
    if(!navfli)
    {close_nav_g();
     navfli=true;
     setup_nav_fli();
    }
  } else if (ch == 'f') {
    filuse = true;
    filact = !filact;
    Serial.print(filact);
    float tmp = gengetf();
    if (tmp < 0.01 && tmp > 0.01) bq = tmp;
    setfil();
  } else if (ch == '@') {
    for (int si = 39; si > 0; si--) {
      web_log(screen[si]);
      web_log("\n");
      delay(200);
    }
  } else if ((ch == 's') && (!search)) {
    fgr = false;
    bool pf = filact;
    filact = false;
    setfil();
    filact = pf;
    search = true;
    if(navfli)
    {}
    else
      goertzel->end(true);
    LF = 400;
    HF = 3000;
    sws = SPACE - 150;
    swm = MARK + 150;
    sn = 0;
    for (int si = 0; si < 40; si++) {
      memset(screen[si], ' ', 80);
      screen[si][80] = 0;
    }
    Serial.println("search qt");
  } else if (ch == 'S') {
    bpf = fat = MARK;
    fgr = true;
    search = true;
    LF = MARK - 85;
    HF = MARK + 85;
     if(navfli)
     {}
   else
    goertzel->end(true);
      sn = 0;
  } else if (ch == 'r') {
    //tft.setRotation(rota = (rota + 1) % 5);
  } /*else if (ch == 't') {
    tft.setTextSize(txts = (txts + 1) % 4);
  } */
  else if (ch == 'p') {
    sidx = (sidx + 1) % 3;
    Serial.printf("<<<<%d>>>>", shift_tab[sidx]);
  } else if (ch == 'm') {
    MSE--;
    Serial.printf("<<<%d>>>\n", MSE);
  } else if (ch == 'M') {
    MSE++;
    Serial.printf("<<<%d>>>\n", MSE);
  } else if (ch == 'h') {
    hsb = !hsb;
    Serial.printf("<<<%s>>>\n", hsb ? "HSB" : "no HSB");
  } else if (ch == 'D') {
    ;
    //tft.setCursor(0, 304);
    //tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    //tft.setTextSize(txts);
  } else if (ch == '0') {
    bidx = 1;
    sidx = 0;
    hsb = true;
    handle('?');
  } else if (ch == '1') {
    bidx = 1;
    sidx = 2;
    hsb = true;
    handle('?');
  } else if (ch == 'n') {
    navtex = false;
    //bsize = 2000;
    setBS(0);
    handle('?');
  } else if (ch == 'N') {
    navtex = true;
    bitcode = 7;
    bidx = 2;
    overs = 5;
    sidx = 1;
    hsb = false;
    // bsize = 480;
    setBS(0);
    handle('?');
  } else if (ch == 'c' && (sdelay >= 10)) {
    sdelay -= 10;
    Serial.print(sdelay);
  } else if (ch == 'C') {
    float cen;
    cen = gengetf();
    Serial.print(cen);
    if (cen > 300) {
      MARK = cen - shift_tab[sidx] / 2;
      SPACE = cen + shift_tab[sidx] / 2;
  dflt_center_freq=cen;
   if(navfli)
   {res_f();
    oer();
   }
   else {}
      handle('P');
    }
  } else if (ch == 'Z') {
    bsize = Serial.parseInt();
    Serial.println(bsize);
    setBS(0);
  } else if (ch == '$') {
    //nPages = Serial.parseInt();
    //Serial.println(nPages);
    //setBS(0);
  } else if (ch == 'z') {
    bsize = 24;
    scan = !scan;
  } else if (ch == 't') {
    maxtun = gengetf();
    tune = true;
    MARK -= (maxtun / 2);
    SPACE -= (maxtun / 2);
    noftun = 0;
    tunOK = -99999;
    tuncur = 0;
  } else if (ch == 'T') {
    tune = true;
    MARK = 300;
    SPACE = 300 + 170;
    maxtun = 2300;
    noftun = 0;
    tunOK = -99999;
    tuncur = 0;
  }
}
void oer() {
  pref.putInt("bidx", bidx);
  pref.putInt("sidx", sidx);
  pref.putFloat("MARK", MARK);
  pref.putFloat("SPACE", SPACE);
  pref.putFloat("volume", volume);
  pref.putBool("hsb", hsb);
  pref.putBool("navtex", navtex);
  pref.putBool("navfli", navfli);
  pref.putFloat("cf",dflt_center_freq);
}
void button1(bool a, int i, void *p) {
  handle('s');
}
void button5(bool a, int i, void *p) {
  handle('<');
}
void button6(bool a, int i, void *p) {
  handle('>');
}
/*
float bb[6][3] = { { 1, 2, 1 }, { 1, 2, 1 }, { 1, 2, 1 }, { 1, 2, 1 }, { 1, 2, 1 }, { 1, 2, 1 } };
float aa[6][3] = {
  { 1.0000, -0.6599, 0.1227 },
  { 1.0000, -0.7478, 0.2722 },
  { 1.0000, -0.9720, 0.6537 },
  { 1.0000, -0.6599, 0.1227 },
  { 1.0000, -0.7478, 0.2722 },
  { 1.0000, -0.9720, 0.6537 }
};
void filcon(float cf, float Q, int i) {
  // return;
  float center_freq = cf;
  float omega = 2 * PI * cf / G_SAMRAT;
  float sn = sin(omega);
  float cs = cos(omega);
  float alpha = sn / (2 * Q);
  bb[i][0] = alpha;
  bb[i][1] = 0;
  bb[i][2] = -alpha;
  aa[i][0] = 1 + alpha;
  aa[i][1] = -2 * cs;
  aa[i][2] = 1 - alpha;
  Serial.printf("c : %f a %f %f %f b: %f %f %f\n", cf, aa[i][0], aa[i][1], aa[i][2], bb[i][0], bb[i][1], bb[i][2]);
}
*/
//BandPass    Filter<float> BQ1();
BandPassFilter<float> *F1 = NULL;
BandPassFilter<float> *F2 = NULL, *F3 = NULL;
FilterChain<float, 3> *F4=NULL;
LowPassFilter<float> *F5=NULL;
void setfil() {
  if (navfli) {

  }
  else
  {if (filuse) {  //filtered.setFilter(0, new MyFilter(filact?0.024:0.0,filact?0.5:0.0,MARK,SPACE));
    if (filact) {
      if (F1 != NULL) delete (F1);
      if (F2 != NULL) delete (F2);
      if (F3 != NULL) delete (F3);
      if (F4 != NULL) delete (F4);
      if (F4 != NULL) delete (F5);
      F1 = F2 = F3=NULL;
      F4=NULL;
      F5= NULL;
      filtered->setFilter(0, (F4=new FilterChain<float, 3 >({ F1 = new BandPassFilter<float>(cf, G_SAMRAT, filact ? 4000.0 / cf : 1000.00 / cf),
                                                         F2 = new BandPassFilter<float>(SPACE, G_SAMRAT, filact ? 6000 / cf : 1000.00 / cf),
                                                         F3 = new BandPassFilter<float>(MARK, G_SAMRAT, filact ? 6000 / cf : 1000.00 / cf) })));
                                                        
    } else
      filtered->setFilter(0,F5=( new LowPassFilter<float>(4000.0, G_SAMRAT, 1.0)));
    Serial.printf("f %f sr %d,Q  %f f2 %f q2 %f\n",
                  (MARK),
                  G_SAMRAT,
                  filact ? bq : 1.00,
                  SPACE, filact ? bq : 1.00);
  }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial.availableForWrite()) delay(50);

#ifdef LCD
  lcd.begin(20, 4);
  lcd.setBacklight(100);
  lcd.home();
  lcd.clear();
  lcd.print("NAVTEX listener");
#endif
  pref.begin("rtty", false);
  pref.getString("ssid",ssid,sizeof(ssid));
  pref.getString("password",password,sizeof(password));
  Serial.println(ssid);
  bidx = pref.getInt("bidx", 1);
  sidx = pref.getInt("sidx", 1);
  MARK = pref.getFloat("MARK", 2000);
  SPACE = pref.getFloat("SPACE", 2000 + baud_tab[bidx]);
  volume = pref.getFloat("volume", 0.8);
  hsb = pref.getBool("hsb", true);
  navtex = pref.getBool("navtex", false);
  navfli = pref.getBool("navfli", false);
   dflt_center_freq= pref.getFloat("cf",1000.00);

 Serial.println(dflt_center_freq);
  if (navtex) {
    synpos = 7;
    overs = 5;
    bitpat = false;
    bitcode = 7;
    bidx = 2;
    sidx = 1;
    hsb = false;
    bsize = 480;
  }
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);

  if (navfli)
  setup_nav_fli();
   else
  {setup_nav_g();
  }
  /*
   kit->setInputVolume(volume);
  kit->setVolume(0.8);
  setBS(0);
 */
#ifdef LCD
  Wire.end();
  Wire.begin(21, 22);  // custom i2c port on ESP
  lcd.begin(20, 4);
  lcd.setBacklight(199);
  lcd.home();
  lcd.print("REady for take off");
#endif
#ifdef TFT
  SPI.end();
  SPI.begin(22, 19, 21, 15);
  tft.begin();
  setup_tft();
#endif
#ifdef SAVE
  setup_SD();
#endif
#ifdef WEB
  web_setup();
  pref.putString("ssid",ssid);
  pref.putString("password",password);
#endif
}

void ps(float f, char c) {
  int x, y;
  if (zoom)
    y = map(f, swm, sws, 0, 80);
  else
    y = map(f, LF, HF, 0, 80);
  //Serial.printf("(%d,%d,%d)\n)",y,swm,sws);
  y = max(0, min(y, 79));
  for (x = 0; x < 40; x++)
    if (screen[x][y] != ' ') screen[x][y] = c;
}
void putmar(float f1, float f2) {
  static float pf1 = 2000, pf2 = 2000;
  ps(pf1, '*');
  ps(pf2, '*');
  ps(f1, 'M');
  ps(f2, 'S');
  pf1 = f1;
  pf2 = f2;
}

void rstfre(float f1) 
{rstfre(f1-shift_tab[sidx]/2,f1+shift_tab[sidx]/2);
}

void rstfre(float f1, float f2) 
{if (f1 == 0.0 || f2 == 0.0) {Serial.println("stop");return; }
 cf=dflt_center_freq=f1+(f2-f1)/2;
 if(navfli)
   {
   res_f();
  }
  else
  {goertzel->end(true);
  goertzel->addFrequency(f1);
  goertzel->addFrequency(f2);
  goertzel->setFrequencyDetectionCallback(GoetzelCallback);
  goertzel->begin(gcfg);
  }//ans+= "set at " +String(MARK)+"  "+String(SPACE)+"\n";
  Serial.printf("set at %.0f %.0f\n", MARK, SPACE);
  if (filact && (!search)) setfil();
  oer();
  //swm=f1-75;sws=f2+75;
  putmar(f1, f2);
}
void cpydat()
{if(navfli)
 {loop_nav_fli();
  delay(1);
 }
 else
 {int b = copier->copyMs(nPages * 1, info_g);  //copier.copyMs(7*1000/100 , info);// / baud_tab[bidx], info);
  if (b == 0) Serial.print("@");
 }
}

bool prim = true;
void handat()
{if (navfli)
 {}
 else
 {while (navcha.available() > 1) 
  {uint8_t rc;
   static bool priset = false;
   oneoutnav((!valsit(rc = navcha.read())) ? navcha.peek(5) : rc);
   navcha.read();
  }
  while (0 && navcha.available() > 5) 
  {uint8_t rc;
   static bool priset = false;
   rc = navcha.read();
   //Serial.printf(".%c.",(char)cch(rc));
   if ((!priset) && (rc == 0X0F) /*&& (navcha.peek(5) == 0X0F)*/) 
   {if (!prim) Serial.print("<swi 0F>");
    prim = true;
    priset = true;
   }
   /*    if ((!priset) && (rc == 0X66) && (navcha.peek(5) == 0X66)) {
      if (prim) Serial.print("<swi 66");
      prim = false;
     priset=true;
     }*/
   if (!valsit(rc) && prim) rc = navcha.peek(5);
   if (prim) oneoutnav(rc);
   prim = !prim;
  }
 }
}

 void hantun()
{ if (tune && ((millis() - chktun) > 5000)) 
  {static float tmark = 0, tspace = 0;
   Serial.printf("%d OK %f,cur %f,tma %f\n", noftun, tunOK, tuncur, tmark);
   if (tunOK < tuncur) 
   {tunOK = tuncur;
    tmark = MARK;
    tspace = SPACE;
   }
   tuncur = 0.0;
   chktun = millis();
   if (noftun > maxtun) 
   {tune = false;
    SPACE = tspace;
    MARK = tmark;
    if(navfli){dflt_center_freq=MARK+(SPACE-MARK)/2;res_f();} else rstfre(MARK, SPACE);
   }
   else 
   {MARK++;
    SPACE++;
    if(navfli){dflt_center_freq=MARK+(SPACE-MARK)/2;res_f();} else rstfre(MARK, SPACE);
    noftun++;
   }
  }
 }

 void hansca()
 {static unsigned long scadon;
  if (scan && ((millis() - scadon) > 15000)) 
  {Serial.printf("((((bsize %d char %d))))\n", bsize, 0);
   scadon = millis();
   bsize += 120;
   setBS(0);
  }
}

void hansrc()
{ if (search) 
  {static unsigned long sss;
   if ((millis() - sss) > (fgr ? 44 : 22)) 
   {if ((!fgr && (400 + sn * 25 > 3000)))  // ||
    {if (navtex) 
     {search = false;
      MARK = bpf - 85;
      SPACE = bpf + 85;
      rstfre(MARK, SPACE);
      if (filuse && filact) setfil();
      bpm = bpf = 0;
      return;
     }
     else 
     {fgr = true;
      fat = bpf;
      mgb = 0;
      sn = 0;
     }
    }
    else 
     if (fgr && (sn >170)) 
     {if (bpf < 2500) 
      {MARK = bpf;
       SPACE = bpf + shift_tab[sidx];
      } 
      else 
      {MARK = bpf - shift_tab[sidx];
       SPACE = bpf;
      }
      mgb = bpm / 2;
      search = false;
      sn = 0;
      bpm = bpf = 0;
      rstfre(MARK, SPACE);
      Serial.printf("Set at m:%.0f s:%.0f,m:%f\n", MARK, SPACE, mgb);
      chkdoa = millis();
#ifdef TFT
      tft.fillScreen(TFT_BLACK);
#endif
      return;
     }
     else 
     {sss = millis();
      Serial.print(fgr ? "+" : "*");
      if(navfli) {}
      else
      {goertzel->end(true);
       goertzel->addFrequency(fgr ? fat + (sn - 85) : 400 + sn * 25);
       ///goertzel.addFrequency(MARK-sn*50);
       goertzel->setFrequencyDetectionCallback(Search);
       goertzel->begin();
      }
      sn++;
    }
   }
  }
 }

unsigned long brktim;
void loop()
{cpydat();  // kit.processActions();
 handat();
 hantun();
 hansca();
 hansrc();
   if (Serial.available()) 
   {hanser=true; handle((char) Serial.read());}
  if (han.available()) 
   {hanser=false; handle((char) han.read());}
 }
