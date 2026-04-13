#include "HardwareSerial.h"
#include "lwip/def.h"
#include "NavTex_rx.h"

AudioInfo info_f(F_SAMRAT, 1, 16);

//AudioBoardStream *kit_f=NULL;  // Access I2S as streamwint nread;

navtex_rx *nv=NULL;
#define BUFFER_SIZE 8192
bool only_sitor_b = false;
bool reverse = false;
int sample_rate=F_SAMRAT;
short inbuf [BUFFER_SIZE];
   
void setup_nav_fli() {
  Serial.println("================== start f =================");
  if( (kit==NULL))
  {  kit=new  AudioBoardStream(AudioKitEs8388V1);Serial.print("$");
  auto cfg = kit->defaultConfig(RXTX_MODE);Serial.print("$");
  cfg.copyFrom(info_f);Serial.print("$");
  cfg.sd_active = false;
  cfg.input_device = ADC_INPUT_LINE2;
  cfg.sample_rate = F_SAMRAT;
  cfg.bits_per_sample = 16;
  cfg.buffer_size = 8096;
  cfg.buffer_count = 1;
  kit->begin(cfg);Serial.print("$");
  }
  else {
  
  info_f.sample_rate = F_SAMRAT;
  info_f.bits_per_sample = 16;
  kit->setAudioInfo(info_f);
 
  }
  if(nv!=NULL) delete nv;
   nv=new navtex_rx(F_SAMRAT, only_sitor_b, reverse, NULL,NULL,NULL);
  Serial.println("================== start f =================");
 
}

void close_nav_f()
{  Serial.println("================== Close f =================");
 if (nv!=NULL)  delete nv ;
 nv=NULL; //}
   //kit->end();Serial.print("%");
  //  delete kit_f;Serial.print("%");
Serial.println("================== Close f =================");
 }
void res_f()
 { Serial.println("================== restart f =================");
 if(nv!=NULL) delete nv;
   nv=new navtex_rx(F_SAMRAT, only_sitor_b, reverse, NULL,NULL,NULL);
  Serial.println("================== restart f =================");
 }
unsigned int nread;
void loop_nav_fli() {   
   for(nread=0;nread==0 && nread<BUFFER_SIZE;)
      nread+=kit->readBytes((uint8_t *)inbuf+nread,BUFFER_SIZE*sizeof(uint8_t)-nread);
   nv->process_data(inbuf,  nread / sizeof(short));
   //Serial.println(nread);
}
     