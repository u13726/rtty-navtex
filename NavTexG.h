float MARK = 2125;
float SPACE = 2295;
#define SAMPLE_BUFFER 8192
AudioInfo info_g(G_SAMRAT, 1, 16);
int nPages = 22;
 GoertzelConfig gcfg;
short covers = 0, overs = 5;

  //AudioBoardStream *kit_g=NULL;
    AudioBoardStream *kit=NULL;  // Access I2S as streamw
  // Access I2S as streamw
VolumeMeter *vols;
FilteredStream<int16_t, float> *filtered;
GoertzelStream *goertzel;
StreamCopy *copier ;
extern void GoetzelCallback(float , float , void *);
extern void setBS(short);
void setup_kit_g()
{auto cfg = kit->defaultConfig(RXTX_MODE);
  cfg.copyFrom(info_g); 
  cfg.sd_active = false;
  cfg.input_device = ADC_INPUT_LINE2;
  cfg.use_apll = false;
  cfg.sample_rate = G_SAMRAT;
  cfg.bits_per_sample = 16;
  cfg.buffer_size = 8096;
  cfg.buffer_count = 2;
 
  kit->begin(cfg);
 
}

  void setup_nav_g()
{Serial.println("==================Start g =================");
 //    Wire.end();
  //Wire.begin(33, 32);
 if(kit==NULL)
 {kit=new AudioBoardStream (AudioKitEs8388V1);   // Access I2S as stream
  auto cfg = kit->defaultConfig(RXTX_MODE);
  cfg.copyFrom(info_g); 
  cfg.sd_active = false;
  cfg.input_device = ADC_INPUT_LINE2;
  cfg.use_apll = false;
  cfg.sample_rate = G_SAMRAT;
  cfg.bits_per_sample = 16;
  cfg.buffer_size = 8096;
  cfg.buffer_count =1;
  kit->begin(cfg);
 }
 else
 { info_g.bits_per_sample = 16;
 info_g.sample_rate=G_SAMRAT;
  kit->setAudioInfo(info_g);
 }
  
 vols = new VolumeMeter (*kit); 
 filtered= new FilteredStream<int16_t, float> (*vols, info_g.channels); 
 goertzel= new GoertzelStream(*filtered);  
 vols->begin(info_g);    goertzel->addFrequency(MARK); 
 goertzel->addFrequency(SPACE); 
 goertzel->setFrequencyDetectionCallback(GoetzelCallback);
 gcfg = goertzel->defaultConfig(); 
 gcfg.copyFrom(info_g); 
 gcfg.threshold = 0.0; 
 gcfg.block_size = (((1000 / baud_tab[bidx]) * (G_SAMRAT / 10))) / (overs * 100);
 goertzel->begin(gcfg); 
 setBS(1); 
 copier = new StreamCopy(*kit,*goertzel); 
  //Wire.end();
 //Wire.begin(21, 22); 
// custom i2c port on ESP
Serial.println("================== start g =================");
 } 
  
  void close_nav_g()
{Serial.println("================== Close g =================");
   copier->end();Serial.print("*");
  goertzel->end(false);Serial.print("*");
 filtered->end();Serial.print("*");
  vols->end( );Serial.print("*");
  //kit->end();Serial.print("*");
  delete copier;Serial.print("*");
  delete goertzel;Serial.print("*");
  delete filtered;Serial.print("*");
 // delete kit_g;Serial.print("*");
Serial.println("================== Close g =================");
 }