#include "web.h"  // Uses AudioKit and I2S microphones as input
#include "DISPLAY.h"


#include "AudioTools.h"
NoFilter<float> no_fil;
#include "AudioTools/AudioLibs/AudioBoardStream.h"
#include "sdd.h"
 #define SAMRAT 48000
AudioInfo info(SAMRAT, 2, 16);            // 8kHz, mono, 16 bits
AudioBoardStream kit(AudioKitEs8388V1);  // Access I2S as stream
//F CsvOutput<int16_t> out(Serial, 1);
FilteredStream<int16_t, float> filtered(kit, info.channels);  // '1Defiles the filter as BaseConverter
GoertzelStream goertzel;           //(out);
StreamCopy copier(goertzel, filtered);  // copy kit to georzel
// define FIR filter
//StreamCopy copier2(goertzel,filtered);  // copy kit to georzel
// define FIR filter
const float b_coeff[] = { 0.00839675, -0.0, -0.00839675 };

//const float b_coeff[] = { 0., -0.0, -0.0 };
const float a_coeff[] = {1, -1.96624060, 0.98320650  };
#include <Preferences.h>
Preferences pref;


// repre
float MARK = 2125;
float SPACE = 2295;
float mgb = 10;
int loglevel = 0;
short baud_tab[] = { 45, 50, 100, 200, 240, 0 };
short bidx = 0;
short shift_tab[] = { 85, 170, 450, 0 };
short sidx = 1;
RingB navcha(4000);
#define SAMPLE_BUFFER 8912
#define PIN_KEY1 36
#define PIN_KEY2 13
#define PIN_KEY3 19
#define PIN_KEY4 23
#define PIN_KEY5 18           
#define PIN_KEY6 5
bool insync=false;
char baudot[2][33] = /*{ "_T\nO HNM\nLRGIPCVEZDBSYFXAWJ<UQK>",
                       "_9>|,.\n)4&p 80:O3Q$?T6!/X2Z<71(>"};*/
  { "_E\rA SIU\nDRJNFCKTZLWHYPQOBG<MXV>",
    "_3\r- *87\n$4',!:(5')2#6019?&<./;>" };

char ccir[36] = { 0x78, 0x6C, 0x5C, 0x0F, 0x33, 0x66,0x5A, 0x36, 0X6A,
                0x47, 0x72, 0x1D, 0x53, 0x56, 0x1B, 0x35, 0x69, 0x4D, 0x17,
                0x1E, 0x65, 0x39, 0x59,
                0x71, 0x2D, 0x2E, 0x55,
                0x4B, 0x74, 0x4E, 0x3C, 0x27, 0x3A, 0x2B, 0x63, 0 };
char ccirl[2] [36]= { {'\n', '\n', ' ', 'a', 'b', 'r', '>', '<', 'c',
                 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0 },
                  {'\n', '\n', ' ', 'a', 'b', 'r', '>', '<', 'c',
                 '-', '?', ':', '$', '3', '!', '&', '#', '8', '\'',
                 '(', ')', '.', ',', '9', '0', '1', '4', '*', '5', '7', ':', '2', '/', '6', '\"', 0 }};


bool valsit(uint8_t c) {
  return (strchr(ccir, c) != NULL);
}
// combine row and col information
float bpf = 0, bpm = 0;
int LF = 300, HF = 3000,swm=715,sws=1285;
char screen[40 + 1][80 + 1];
bool zoom=false;


bool navtex = false;

void Search(float frequency, float magnitude, void *ref) {
  //Serial.printf("%.0f %.0f %.0f %.0f\n", bpf, bpm, frequency, magnitude);
  static int pf, som,somm,soms, nf;
  ///Serial.printf("%d %d %d\n",pf,som,nf);
  if (frequency != pf) {
    int x, y;
    if (LF == frequency) tft.fillScreen(TFT_BLACK);  //tft.setCursor(0,0);}
    if (nf) {
      if (((som) / nf) > bpm) {
        bpm = (som) / nf;
        bpf = pf;
      }
      x = map(som / nf, 0, 75, 0, 240);
      y = map(pf, LF, HF, 0, 320);
      tft.drawFastHLine(0, y, x, pf == MARK ? ILI9341_BLUE : pf == SPACE ? ILI9341_RED
                                                                         : TFT_WHITE);
      if (custer) {
        char buf[80];
        sprintf(buf, "F: %d ", pf);
        web_log(buf);
        y = map(abs(som) / nf, 0, 75, 0, 80);           
        for (int iy = 0; custer && iy < y; iy++)
          web_log("-");
        sprintf(buf, " %d \r", (som) / nf);
        web_log(buf);
        x = map((som) / nf, 0, 75, 0, 40);
        x = max(0, min(x, 39));
        if(zoom)
        y = map(pf, swm,sws, 0, 80);
      else
        y = map(pf, LF, HF, 0, 80);
        y = max(0, min(y, 79));
        for (; x > 0; x--) screen[x][y] = '*';//somm>soms?'*':'.';
      }
    }
    som = magnitude;
    somm=frequency==MARK?magnitude:0;
    soms=frequency==SPACE?magnitude:0;
    pf = frequency;
    nf = 1;
  } else {
    som += magnitude; 
    somm+=frequency==MARK?magnitude:0;
    soms+=frequency==SPACE?magnitude:0;    nf++;
    pf = frequency;
  }
}
char resp[128] = "";
char *p2b = resp;
static short cof = 2;
float volume = 1.0;
int sn = 0;
short noftun = 0, maxtun;
float tuncur = 0.0, tunOK;

float fat;
bool search = false;
bool fgr = false;
int MSE = 22;
unsigned long lasts, chkdoa, chktun;

GoertzelConfig gcfg;
String ans;
bool hsb = true;
int sdelay = 100;
short covers = 0, overs = 5;
uint32_t syn = 0, out = 0;
bool oversv[5];
short bitcode = 5;
int bsize = 96, sbsize = 96;
void setBS(short s) {
  static short pbs = -1;
  if (s != pbs || s == 0) {
    if (s == 0) s = 1;
   /* if (navtex)
F



      gcfg.block_size = bsize;  //bsize;
    else*/
      gcfg.block_size = (((1000 / baud_tab[bidx]) * (SAMRAT/10))) / (overs * s * 100);
    //Serial.println(gcfg.block_size);
    goertzel.end(false);
    goertzel.begin(gcfg);
    pbs = s;
    //Serial.printf("<<< %d %d>>>\n", baud_tab[bidx], gcfg.block_size);
  }
}
bool hit() {
  if (overs == 1) return oversv[0];
  if (overs == 2) return (oversv[0] && oversv[1]);
  if (overs == 3) return ((oversv[0] && oversv[1]) || (oversv[1] && oversv[2]));
  if (overs == 4) return (((oversv[0] && oversv[1])||(oversv[1])&& oversv[2]) || (oversv[3] && oversv[2]));
  if (overs == 5) return ((oversv[0] && oversv[1] && oversv[2]) || (oversv[1] && oversv[2] && oversv[3]) || (oversv[2] && oversv[3] && oversv[4]));
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
    static bool spo = true;
    if ((c == '\n' || c == '\r') && (!spo)) {
      spo = true;
      Serial.println("");
    } else if (c != '\n' && c != '\r') {
      spo = false;
        Serial.print(c);
    } 
  //c =CCIR476.Decode(b,cs==0);
  out++;
  lasts = millis();
  ans += c;
  static char buf[2] = "x";
  buf[0] = c;
    //buf[1]=0;
  web_log(c);
  chrscr(c);
  }
}
void chrscr(char c)
{static short n=0;
if(c=='_') return;
if (c != '\n' && c != '\r') {
  tft.drawChar(xPos, yDraw, c, TFT_WHITE, TFT_BLACK, 1, 1);
  xPos += 8;
  blank[(0 + (yStart - TOP_FIXED_AREA) / TEXT_HEIGHT) % 37] = xPos;  // Keep a record of line lengths
 }
  n++;
 if (c == '\n' || c == '\r') {
      n = 0;
      scrollOne();
    } else {
       if ((n) % 33 == 0) { scrollOne(); }
    }
}
/*   if (ans.length() > 1024) {
      writeFile(LittleFS, cfn, ans.c_str());
      ans = "";
    }
 */
char cch(uint8_t  b)
{
 char c, *s;
 if (s = strchr(ccir, b)) 
 {c =ccirl[0][s - ccir] ;  //CCIR476.Decode(b,true);
 }
 else 
 {c = '*';
 }
 return(c);
}

void chrfil(char c)
{static String ans="";
 int o,n,m;
if(c=='_')return;
ans +=c;
n=ans.indexOf("ZCZC");
m=n>=0?ans.indexOf("NNNN",n):-1;
o=n>=0?ans.indexOf("ZCZC",n+4):-1;
if (o>=0 && (m==-1||o<m)) m=o;
//Serial.printf("%d %d\n",n,m);
if((m>=0) && (n>=0))
{String msg=ans.substring(n,m); 
 String hdr="/"+ans.substring(n+5,min(ans.indexOf("\n",n+8),n+10));
 hdr.replace("*","_");
hdr.replace(")","_");hdr.replace(" ","_");
hdr=hdr.substring(0,8);
  hdr+=".txt";writeFile(LittleFS, hdr.c_str(), msg.c_str());
  ans=ans.substring(m+5,ans.length());
    }

}
uint8_t oneoutnav(uint8_t b) 
{static bool cs = false,csrep=false;
 static short n = 0;
 //static bool dx = true;
 char c, *s;
 if (s = strchr(ccir, b)) 
 {c =ccirl[cs?1:0][s - ccir] ;  //CCIR476.Decode(b,true);
  if (c == 0) c = '_';
  tuncur += 1;
 }
 else 
 {c = '*';
 }
  //if (b != 0x66) dx = !dx;else return(b);
  if(b==0x36) {cs=true;c='_';}
  if(b==0x5A) {cs=false;c='_';}
  if(b==0x0F || b==0x33 || b==0x6A) c='_';
  if(c=='\n'||c=='\r') {cs=false;}
   //return(b);
 insync=(c=='r'||c=='a');
 if(c=='r'||c=='a') {Serial.print(c);return(b);}
 static bool spo2 = true;
 if ((c == '\n' || c == '\r') && (!spo2)) 
 {spo2= true;
  Serial.println("");
 }
 else if (c != '\n' && c != '\r') 
 {spo2 = false;
  if(c!='_') Serial.print(c);
 }
 out++;
 if (out%80==0) Serial.println();
 lasts = millis();
 if(c!='_') web_log(c);
 chrscr(c);
 chrfil(c);
 return b;
}

bool validSample(float frq, float mag, float pmag) {  // if (loglevel == 3) Serial.printf("%d %f %f %f %f\n        ", millis() - pmi, pf, pm, frequency, magnitude);
  if (frq != MARK && frq != SPACE) return false;
  if ((pmag < 5) && (mag < 5)) return false;
  if (search) return false;
  return true;
} 
short synpos=3;

bool navsyn(char *s) {
  for (int i = 0; i < synpos; i++) {
    short p = 0, m = 0;
    for (int j = p = m = 0; j < 7; j++)
      if (s[i * 7 + j] == '+') p++;
      else m++;
    if (m != 3 || p != 4) return false;
  }
  return true;
}
bool bitpat=true;
void decode_nav(float frequency, float magnitude) {
  static uint8_t b = 0;
  static unsigned long tick = 0, tickcnt = 0;
  static float pf, pm, dc;
  static unsigned long pmi;
  static bool sync = false, skip = false;
  static char BUB[42 + 1] = "..........................................";
  static short sixsev = 0;

  if (!validSample(frequency, magnitude, pm)) return;
  /* if ((millis()-tick)>210 &&loglevel==8)
{Serial.println(tickcnt);
tickcnt=0;
tick=millis();
} */
  if (dc == 0) {
    pmi = millis();
    pf = frequency;
    pm = magnitude;
    dc = 1;
    return;
  }
//  if(abs(magnitude-pm)<(magnitude*0.05)) 
//  {Serial.printf("dodgy %f %d %s %f %F\n",abs(magnitude-pm),covers,pf==frequency?"SAME":"diff",frequency,pf);//
//}
   if (pm > magnitude) {frequency = pf;}
  pmi = millis();
  pf = frequency;
  pm = magnitude;
  dc = 0;
  oversv[covers] = (frequency == MARK);
  covers++;
  if (covers < overs) { return; }
  covers = 0;
  frequency = hit() ? SPACE : MARK;
  if (loglevel == 2) Serial.printf("%f %d %s\n", frequency, b, BUB);
  if (loglevel == 4) Serial.printf("%f\n", frequency);
  //if(skip){skip=false;i=1;b=0;return;}
  for (int k = 1; k < (7*synpos); k++)
    BUB[k - 1] = BUB[k];
  BUB[(7*synpos)-1] = frequency == MARK ? '+' : '-';
  if ((!sync)) {insync=false;
    if (BUB[0] == '.') return;
    if (!navsyn(BUB)) {
       return;
    }
    sync = true;
    sixsev = 0;
    if (loglevel == 1) Serial.print("SYN");
  }
  if (sixsev == 0) {
    static short errat = 0;
    sixsev = 6;
    b = 0;
    int k = 1;
    static char sb;
    sb=BUB[6];
    for (int l = 0; l < 7; l++, k *= 2)
      b += (BUB[l] == '+' ? k : 0); 
    if (!valsit(b) && bitpat)
    {uint8_t bp = 0;
     k = 2;bp=(sb=='+'?1:0);
     for (int l = 0; l < 6; l++, k *= 2)
      bp += (BUB[l] == '+' ? k : 0);
     if(valsit(bp)&&bitpat)
     {b=bp;
     Serial.print("R2");
      sixsev=5;
     }
     else{   k = 1;
   //  Serial.print("R?");
     for (int l = 1; l < 8; l++, k *= 2)
      bp += (BUB[l] == '+' ? k : 0);
     if(valsit(bp))
     {b=bp;  
     Serial.print("R1");
      sixsev=7;
     }
     }
        }
    navcha.write(b);
    //b=oneoutnav(b);
    if (!valsit(b)) errat+=10; else errat=max(errat-5,0);
    if (errat >10) {
      sync = false;
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
  static char RUB[6+1] = "123456";
  static bool sync = false, skip = false;
  if (!validSample(frequency, magnitude, pm)) return;
  if (dc == 0) 
  {pmi = millis();
   pm = magnitude;
   pf=frequency;
   dc = 1;
   return;
  }
  else
   if (pm > magnitude) {frequency = pf;}
  pmi = millis();
  pf = frequency;
  pm = magnitude;
  dc = 0;
  oversv[covers++] = frequency == MARK;
  if (covers < overs) return;
  covers = 0;
  frequency = hit() ? MARK : SPACE;
  if (loglevel == 2) Serial.printf("%f %d %d <%s>\n", frequency, i, b, RUB);
  if (loglevel == 4) Serial.printf("%f\n", frequency);
  //if(skip){skip=false;i=1;b=0;return;}
  for (int ri = 0; ri < (hsb ? 4 : 2); ri++)
    RUB[ri] = RUB[ri + 1];
  RUB[hsb ? 4 : 2] = frequency == MARK ? '+' : '-';
  if ((!sync)) 
  {static bool rcfg = false;
   if (hsb) setBS(2);
   if (hsb ? (RUB[0] == '+' && RUB[1] == '+' && RUB[2] == '+' && RUB[3] == '-' && RUB[4] == '-')
            : (RUB[0] == '+' && RUB[1] == '+' && RUB[2] == '-')) 
   {sync = true;
    i = 0;
    RUB[0] = RUB[1] = RUB[2] = RUB[3] = RUB[4] = RUB[5] = '*';
    if (hsb) {setBS(1);}
    if (loglevel == 1) { Serial.println("<<<SYN>>>"); }
    syn++;
   } 
   else 
   {skip = true;
    delayMicroseconds(sdelay);
    return;
   }
  }
  if (loglevel == 2) Serial.printf(">>>> %f %d %d %s\n", frequency, i, b, RUB);
  if (i == 0) 
  {if (frequency == SPACE)
      i = 1;
   else
      sync = false;
  } else if (i == 6) {
    if (frequency == MARK) {
      i = 7;
      if (hsb) setBS(2);
    } else {
      sync = false;
    }
  } else if (i == 7) {
    if (frequency == MARK) {
      i = 0;
      if (hsb) setBS(1);
      return;
    } else {
      sync = false;
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
float bq=-1.5; float hq=-2.0;

bool scan = false, tune = false;
 bool filact=false;
void handle(char ch) {
  Serial.print(ch);
  if (ch == 'i') {
    float c;
    c = MARK;
    MARK = SPACE;
    SPACE = c;
    navtex=false;
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
  }  else if (ch == ')') {
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
    MARK = Serial.parseFloat();
    Serial.printf("Set at m:%f s:%f,m:%f\n", MARK, SPACE, mgb);
  } else if (ch == 'E') {
    SPACE = Serial.parseFloat();
    Serial.printf("Set at m:%f s:%f,m:%f\n", MARK, SPACE, mgb);
  } else if (ch == '|') {
    MARK = Serial.parseFloat() - (shift_tab[sidx]/2);
    SPACE=MARK+shift_tab[sidx];
    handle('P');
  } else if (ch == 'o') {
    overs = (overs > 1 ? overs - 1 : 1);
    Serial.printf("<<<%d>>>\n", overs);
    setBS(0);
  } else if (ch == '?') {
    printf("baud:%d,mark:%.0f,space:%.0f,shift:%d, %s ,bsize:%d overs:%d SYNra %f\n",
           baud_tab[bidx], MARK, SPACE, shift_tab[sidx], hsb ? "HSB" : "No HSB", gcfg.block_size, overs,(float)((float)syn / (out > 0 ? (float)out : 1.0)));
  } else if (ch == 'l') {
    loglevel--;
    Serial.printf("<<<%d>>>\n", loglevel);
  } else if (ch == 'L') {
    loglevel++;
    Serial.printf("<<<%d>>>\n", loglevel);
  } else if (ch == 'v') {
    volume -= 0.1;
    Serial.printf("<<<%f>>>\n", volume);
    kit.setInputVolume(volume);
  } else if (ch == 'V') {
    volume += 0.5;
    Serial.printf("<<<%f>>>\n", volume);
    kit.setInputVolume(volume);
  } else if (ch == 'B') {
    bidx++;
    if (!baud_tab[bidx]) bidx--;
    setBS(1);
  } else if (ch == 'b') {
    if (bidx > 0) bidx--;
    setBS(1); 
  }else if (ch == 'Q') {
    synpos=min(6,synpos+1);
    Serial.printf("SYBPOS:%d\n",synpos);  
    
  }  else if (ch == 'q') {
    synpos=max(3,synpos-1);
    Serial.printf("SYBPOS:%d\n",synpos);
    } else if (ch == 'd') {
    bitpat=!bitpat;
    Serial.print(bitpat);
    } else if (ch == 'e') {
      swm=MARK-150;sws=SPACE+150;
    zoom=!zoom;
    Serial.print(zoom);
    putmar(MARK,SPACE);
  }  else if (ch == 'f') {
    filact=!filact;
    Serial.print(filact);
    bq=Serial.parseFloat();
    setfil();
  } else if (ch == '@') {
    for (int si = 39; si > 0; si--) {
      web_log(screen[si]);
      web_log("\n");
      delay(200);
    }
  } else if (ch == 's') {
    fgr = false;
    search = true;
    goertzel.end(true);
    LF = 400;
    HF = 3000;
    sws=SPACE-150;swm=MARK+150;
    sn = 0;
    for (int si = 0; si < 40; si++) {
      memset(screen[si], ' ', 80);
      screen[si][80] = 0;
    }
  } else if (ch == 'S') {
    bpf = fat = MARK;
    fgr = true;
    search = true;
    LF = MARK - 30;
    HF = MARK + 30;
    goertzel.end(true);
    sn = 0;
  } else if (ch == 'r') {
    tft.setRotation(rota = (rota + 1) % 5);
  } else if (ch == 't') {
    tft.setTextSize(txts = (txts + 1) % 4);
  } else if (ch == 'p') {
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
    tft.setCursor(0, 304);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(txts);
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
    sdelay += 10;
    Serial.print(sdelay);
  } else if (ch == 'Z') {
    bsize = Serial.parseInt();
    Serial.println(bsize);
    setBS(0);
  } else if (ch == 'z') {
    bsize = 24;
    scan = !scan;
  } else if (ch == 'T') {
    maxtun = Serial.parseInt();
    tune = true;
    MARK -= (maxtun / 2);
    SPACE -= (maxtun / 2);
    noftun = 0;
    tunOK = -99999;
    tuncur = 0;
  } else if (ch == 't') {
    tune = false;
  }
}
void oer() {
  pref.putInt("bidx", bidx);
  pref.putInt("sidx", sidx);
  pref.putFloat("MARK", MARK);
  pref.putFloat("SPACE", SPACE);
  pref.putBool("hsb", hsb);
  pref.putBool("navtex", navtex);
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
 float aa[6][3] ;
 float bb[6][3] ;
void filcon(float cf,float Q,int i)
{float center_freq = cf;
  float omega = 2 * PI * cf / SAMRAT;
  float sn = sin(omega);
  float cs = cos(omega);
  float alpha = sn / (2 * Q);
   bb[i][0] = alpha;
  bb[i][1] = 0;
bb[i][2] = -alpha;
aa[i][0] = 1 + alpha;
aa[i][1]  = -2 * cs;
aa[i][2]= 1 - alpha;
}

void setfil()
{filcon(MARK+85,filact?bq:0.01,0);
filcon(MARK,filact?-0.01:0.01,1);
filcon(SPACE,filact?-0.01:0.01,2);
 filtered.setFilter(0, new FilterChain<float, 1>
  ({ new BiQuadDF2<float>(bb[0],aa[0])})); //, new BiQuadDF2<float>(bb[1],aa[1]), new BiQuadDF2<float>(bb[2],aa[2])}));///
filtered.setFilter(1, new FilterChain<float, 1>
 ({ new BiQuadDF2<float>(bb[0],aa[0])})); //, new BiQuadDF2<float>(bb[1],aa[1]), new BiQuadDF2<float>(bb[2],aa[2])}));
}
void setup() {
  Serial.begin(115200);
  while (!Serial.availableForWrite()) delay(50);
  delay(300);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);
  //const float b_coeff[] = { (1.0), -1.4542406161069188, 0.5740587002225346 };
  //const float a_coeff[] = { 0.21297064988873268, 0, -0.21297064988873268 };
// define FIR filter
float coef[] = { 0.0209967345, 0.0960112308, 0.1460005493, 0.0960112308, 0.0209967345};
 float a1[] = { 1.0,-1.90984679,0.92633439};
 float b1[] = {  0.03683280,0.00000000e+0,-0.03683280};
 float a2[] = { 1.0, -1.96510095,0.97928040};
 float b2[] = {0.01035980,0.00000000e+0,-0.01035980};
 float a3[] = {1.0,-1.95561154,0.97550159};
 float b3[] = { 0.01224920, 0.00000000e+0,-0.01224920};
 //filtered.setFilter(0, new BiQuadDF2<float>(b_coeff,a_coeff));
 //filtered.setFilter(1, new BiQuadDF2< float>(b_coeff,a_coeff));

  //filtered.setFilter(0, new BiQuadDF2<float>(b_coeff, a_coeff));
 // filtered.setFilter(1, new FIR<float>(coef));
//filtered.setFilter(0, new FIR<float>(coef));
  // start audio input from microphonesBiii
  auto cfg = kit.defaultConfig(RX_MODE);
  cfg.copyFrom(info);
  cfg.sd_active = false;
  cfg.input_device = ADC_INPUT_LINE2;
  cfg.use_apll = false;
  cfg.sample_rate = SAMRAT;
  cfg.bits_per_sample = 16;
  cfg.buffer_size = 8096;
  cfg.buffer_count = 2;
  kit.begin(cfg);
  pref.begin("rtty", false);
  // String ssid = preferences.putString("wifi_ssid", "DefaultSSID");
  bidx = pref.getInt("bidx", 1);
  sidx = pref.getInt("sidx", 1);
  MARK = pref.getFloat("MARK", 2000);
  SPACE = pref.getFloat("SPACE", 2000 + baud_tab[bidx]);
  hsb = pref.getBool("hsb", true);
  navtex = pref.getBool("navtex", false);
  if(navtex)
  {synpos=6;
   overs=5;
   bitpat=false;
    bitcode = 7;
    bidx = 2;
    sidx = 1;
    hsb = false;
    bsize = 480;
  }
  kit.setInputVolume(volume);
  // lower frequencies - with keys
  //rstfre(MARK, SPACE);
  setBS(0);

  goertzel.addFrequency(MARK);
  goertzel.addFrequency(SPACE);
  // define callback£
  goertzel.setFrequencyDetectionCallback(GoetzelCallback);

  // start goertzel
  gcfg = goertzel.defaultConfig();
 
  gcfg.copyFrom(info);
  gcfg.threshold = 0.0;
gcfg.block_size = (((1000 / baud_tab[bidx]) * (SAMRAT/10))) / (overs * 100);
  // gcfg.sample_rate=SAMPLE_BUFFER/gcfg.block_size;
  goertzel.begin(gcfg);
  setBS(1);
  SPI.end();
  SPI.begin(22, 19, 21, 15);
  tft.begin();
  setup_tft();
  setup_SD();
  web_setup();
  /*kit.addAction(PIN_KEY1, button1);
  kit.addAction(PIN_KEY5, button5);
  kit.addAction(PIN_KEY6, button6);
//*/
//  if (navtex) handle('N');
 /* for(int i=0;i<10;i++) navcha.write(i+'0');
  for(int i=0;i<5;i++) Serial.println((char)navcha.read());
   for(int i=0;i<5;i++) navcha.write(i+'0');
    for(int i=0;i<10;i++) Serial.println((char)navcha.peek(i));
*/}// "my_app" is the namespace
void ps(float f, char c) {
  int x, y;
  if(zoom)
  y = map(f, swm,sws, 0, 80);
  else
  y = map(f, LF, HF, 0, 80);
  //Serial.printf("(%d,%d,%d)\n)",y,swm,sws);
  y = max(0, min(y, 79));
  for (x = 0; x < 40; x++)
    if (screen[x][y] != ' ') screen[x][y] = c;
}
void putmar(float f1, float f2)
{   static float pf1 = 2000, pf2 = 2000;
 ps(pf1, '*');
  ps(pf2, '*');
  ps(f1, 'M');
  ps(f2, 'S');
  pf1 = f1;
  pf2 = f2;
}

void rstfre(float f1, float f2) {
  if (f1 == 0.0 || f2 == 0.0) {
    Serial.println("stop");
    return;
  }
  goertzel.end(true);
  goertzel.addFrequency(f1);
  goertzel.addFrequency(f2);
  goertzel.setFrequencyDetectionCallback(GoetzelCallback);
  goertzel.begin(gcfg);
  //ans+= "set at " +String(MARK)+"  "+String(SPACE)+"\n";
  Serial.printf("set at %.0f %.0f\n", MARK, SPACE);
  oer();
//swm=f1-75;sws=f2+75;
  putmar(f1,f2);
}
bool prim=true;
unsigned long  brktim;
void loop() {
  kit.processActions();
   static bool accbrk;
   /*
  if(navtex && insync && (millis()-brktim)>9000)
  {brktim=millis();
ze=accbrk?480:479;
   setBS(0);
   accbrk=!accbrk;
  }*/
  while (navcha.available()>5)
  {uint8_t rc;
   rc=navcha.read();
   if(rc==0X0F) prim=true;
   if(rc==0X66) prim=false;
   if(!valsit(rc)&&prim) rc=navcha.peek(5);
   if(prim) oneoutnav(rc);
   prim=!prim;
  }
  if (tune && ((millis() - chktun) > 1000)) {
    static float tmark = 0, tspace = 0;
    Serial.printf("%d OK %f,cur %f,tma %f\n", noftun, tunOK, tuncur, tmark);
    if (tunOK < tuncur) {
      tunOK = tuncur;
      tmark = MARK;
      tspace = SPACE;
    }
    tuncur = 0.0;
    chktun = millis();
    if (noftun > maxtun) {
      tune = false;
      SPACE = tspace;
      MARK = tmark;
      rstfre(MARK, SPACE);
    } else {
      MARK++;
      SPACE++;
      rstfre(MARK, SPACE);
      noftun++;
    }
  }
  static unsigned long scadon;
  if (scan && ((millis() - scadon) > 15000)) {
    Serial.printf("((((bsize %d char %d))))\n", bsize, 0);
    scadon = millis();
    bsize += 120;
    setBS(0);
  }
  if (search) {
    static unsigned long sss;
    if ((millis() - sss) > (fgr ? 100 : 50)) {
      if ((!fgr && (400 + sn * 25 > 3000)))  // ||
      {
        if (navtex) {
          search = false;
          MARK = bpf - 85;
          SPACE = bpf + 85;
          rstfre(MARK, SPACE);
          bpm = bpf = 0;
          return;
        } else {
          fgr = true;
          fat = bpf;
          mgb = 0;
          sn = 0;
        }
      } else if (fgr && (sn > 60)) {
        if (bpf < 2500) {
          MARK = bpf;
          SPACE = bpf + shift_tab[sidx];
        } else {
          MARK = bpf - shift_tab[sidx];
          SPACE = bpf;
        }
        mgb = bpm / 2;
        search = false;
        sn = 0;
        bpm = bpf = 0;
        rstfre(MARK, SPACE);
        Serial.printf("Set at m:%.0f s:%.0f,m:%f\n", MARK, SPACE, mgb);
        chkdoa = millis();
        tft.fillScreen(TFT_BLACK);
        return;
      } else {
        goertzel.end(true);
        sss = millis();
        Serial.print(fgr ? "+" : "*");
        goertzel.addFrequency(fgr ? fat + (sn - 30) : 400 + sn * 25);
        ///goertzel.addFrequency(MARK-sn*50);
        goertzel.setFrequencyDetectionCallback(Search);
        goertzel.begin();
        sn++;
      }
    }
  }
  if (Serial.available()) {
    char ch;
    ch = Serial.read();
    handle(ch);
  }
  copier.copyMs(8000 / baud_tab[bidx], info);
//copier2.copyMs(8000 / baud_tab[bidx], info);
}
