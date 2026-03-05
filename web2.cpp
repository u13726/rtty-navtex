#include "WString.h"


#include <Arduino.h>
#include <stdint.h>
#include "Str.h"
 // SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2016-2026 Hristo Gochkov, Mathieu Carbou, Emil Muratov, Will Miles

//
// Shows how to serve a large HTML page from flash memory without copying it to heap in a temporary buffer
#if defined(ESP32) || defined(LIBRETINY)
#include <AsyncTCP.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#elif defined(TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
#include <RPAsyncTCP.h>
#include <WiFi.h>
#endif

#include "FS.h"
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

static AsyncWebServer serveur(80);
AsyncWebServerRequest *srequest=NULL;
extern void deleteFile2(fs::FS &fs, const char *path);

extern void handle(char);
extern char screen[40][81];
static const char *htmlContent PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Sample HTML</title>
</head>
<body>
    <h1>Hello, World!</h1>
</html>
)";

static const size_t htmlContentLength = strlen_P(htmlContent);
#include "RingB.h"

 RingB ron(512);
 bool custer=false;
 AsyncResponseStream* response ;
 #include "ssid.h"
 WiFiServer wifi;
bool pos0=false; 
void web_log(char *s){
   for(;custer && ron.availableToWrite()>1 && *s;s++) 
   {if(*s=='\n'||*s=='\r')
    {if(!pos0) 
       pos0=true;
     else 
       continue;}
    else
    {pos0=false;  /*logger.print(s);*/}
     ron.write(*s);
    }
    }
 
void web_log(char s){
   if(custer && ron.availableToWrite()>1 ) 
   {if(s=='\n'||s=='\r')
    {if(!pos0)    pos0=true; else return; 
    }
    else
    {pos0=false;  /*logger.print(s);*/}
     ron.write(s);
    }
    }
 
// parameters
extern float volume;
extern float MARK;
extern float SPACE;// = 1.0;
extern bool hsb,navtex;
extern  short shift_tab[],baud_tab[];
extern short sidx,bidx;
int Baud = 50,Shift= 85;
/*
 
*/


  static const char *para PROGMEM = R"(
    <!DOCTYPE html> <html><head><title>RTTY setup</title></head><body><h1>RTTY</h1>
    <form id='effect-form' method='post' >
                <div>
                    <input type='range' id='volume' name='volume'
                            onchange='this.form.submit()'
                            min='0' max='1' step='0.01' value='%volume%'>
                    <label for='volume'>Volume</label>
                </div>
                <div>
                    <label for='MARK'>Mark</label>
                    <input type='number' id='MARK' name='MARK' 
                            onchange='this.form.submit()'
                            min='300' max='3000' step='10' value='%MARK%'>
                </div>
                <div>
                    <label for='SPACE'>Space</label>
                    <input type='number' id='SPACE' name='SPACE' 
                            onchange='this.form.submit()'
                            min='300' max='3000' step='10' value='%SPACE%'>
                </div>
                <div>
                    <label for='Baud'>Baud</label>
                <select  name='Baud' id='Baud'  onchange='this.form.submit()'><option %BSEL45% value=45>45</option><option %BSEL50% value=50>50</option><option %BSEL100% value=100>100</option><option %BSEL200% value=200>200</option> </select>                     </div>
                <div><label for='Shift'>Shift</label>
                <select  name='Shift' id='Shift' onchange='this.form.submit()'><option %SSEL85% value=85>85</option><option %SSEL170% value=170>170</option><option %SSEL450% value=450>450</option> </select>
                     </div>
                <div>
                    <input type='checkbox' id='Hsb' name='Hsb' 
                            onchange='this.form.submit()'
                            %Hsb%>
                    <label for='Hsb'>Half stopbit</label>
                </div>
                <div>
                    <input type='checkbox' id='Nav' name='Nav' 
                            onchange='this.form.submit()'
                    %Nav%>
                    <label for='Nav'>NAVTEX</label>
                </div>
                <div><input type='text' id='cmd' name='cmd' onchange='this.form.submit()' >cmd : </div><TABLE><TR>
                <TD><div>
                    <input type='button' id='Search' value='Search' 
                            onclick='document.location.href="/handle_s"' Search>
                </div></TD>
<TD><div>
                    <input type='button' id='bSearch' value='bSearch' 
                            onclick='document.location.href="/handle_S"' Search>
                </div></td>
</TR><TR><TD><div>
                    <input type='button' id='push' value='push' 
                            onclick='document.location.href="/handle_P"' push>
                </div></td>
<TD><div>
                    <input type='button' id='invert' value='invert' 
                            onclick='document.location.href="/handle_I"' invert>
                </div></td></tr></table>
            </form> </body></html>
   )";
  static const size_t  paralen=strlen_P(para);
  static rtty::Str html(3500);
  void handleweb(AsyncWebServerRequest *request, char c)
  {char html[]="<html><body ><h1>Setup</h1><samp>Done</samp>\
<script>setTimeout(function(){window.location.href = 'para';}, 200);</script></body></html>";
  if (c=='+' ||c=='-'  || c=='<' ||c=='>' || c=='(' ||c==')' )
  {char*s;
  s=strstr(html,"para");
  *s='b';
  }
    //parameters.parse(server->client());
   handle(c);
    request->send(200,"text/html", html);
    }
void getpara(AsyncWebServerRequest *request) 

  {    html=para;
        html.replace("%volume%",volume);
    html.replace("%MARK%",MARK);
    html.replace("%SPACE%",SPACE);
    html.replace("%Baud%",baud_tab[bidx]);
    html.replace("%SSEL85%",sidx==0?"selected":"");
    html.replace("%SSEL170%",sidx==1?"selected":"");
    html.replace("%SSEL450%",sidx==2?"selected":"");
    html.replace("%BSEL45%",bidx==0?"selected":"");
    html.replace("%BSEL50%",bidx==1?"selected":"");
    html.replace("%BSEL100%",bidx==2?"selected":"");
    html.replace("%BSEL200%",bidx==3?"selected":"");
    html.replace("%Hsb%",hsb?" checked ":"");
    html.replace("%Nav%",navtex?" checked ":"");
   request->send(200,"text/html",(uint8_t *) html.c_str(),html.length());}

extern String listDirHTML();
void web_setup() { 
     if (WiFi.status() != WL_CONNECTED && ssid!=nullptr && password!=nullptr){
                WiFi.begin(ssid, password);
                short i=0;
                while (WiFi.status() != WL_CONNECTED) {        
                    delay(500);
                    Serial.print(".");
                if (++i>25  ) ESP.restart();	

              }
#ifdef ESP32
                WiFi.setSleep(false);
#endif
                Serial.println();
                Serial.print("Started Server at ");
                Serial.print(WiFi.localIP());
                Serial.print(":");
                Serial.println(80          );
            }
        
  static const char *hover PROGMEM = R"(
     <html><body><iframe title='Live data' height='50%' width='70%' src='cons'></iframe>
     <iframe title='Parameters' width='28%' height='50%' src='para'></iframe><br/> 
<iframe width='45%' height='48%' id='bara' src='bara'></iframe>
     <iframe width='45%' height='48%' id='logs' src='logs'></iframe></body></html>)";
  static const size_t  hoverlen=strlen_P(hover);

   serveur.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // need to cast to uint8_t*
    // if you do not, the const char* will be copied in a temporary String buffer
     request->send(200,"text/html",(uint8_t *) hover,hoverlen);
     //request->send(200, "text/html", (uint8_t *)htmlContent, htmlContentLength);
  });
    
      serveur.on("/para",HTTP_GET, [](AsyncWebServerRequest *request) {
    getpara(request);
  });
    serveur.on("/para",HTTP_POST,[](AsyncWebServerRequest *request) {
    if (request->hasParam("Baud", true)) 
        Baud = (request->getParam("Baud", true)->value().toInt());
    if (request->hasParam("Shift", true)) 
        Shift =(request->getParam("Shift", true)->value().toInt());
      if (request->hasParam("volume", true)) 
        volume =(request->getParam("volume", true)->value().toFloat());
   for(bidx=0;baud_tab[bidx] && baud_tab[bidx]!=Baud;bidx++) ;
        if(!baud_tab[bidx]) bidx--;
        for(sidx=0;shift_tab[sidx] && shift_tab[sidx]!=Shift;sidx++);
        if(!shift_tab[sidx]) sidx--;
        hsb = (request->hasParam("Hsb", true));  
      navtex = (request->hasParam("Nav", true));  
      handle(navtex?'N':'n');
    if (request->hasParam("cmd", true)) 
       { String cmd;
       cmd =(request->getParam("cmd", true)->value());
       for(int h=0;h<cmd.length();h++)
        handle(cmd[h]);
       }
       getpara(request);
       });
      serveur.on("/handle_s",HTTP_GET, [](AsyncWebServerRequest *request) {
      handleweb(request,'s');});
  serveur.on("/handle_pplus",HTTP_GET, [](AsyncWebServerRequest *request) {
      handleweb(request,'>');});
  serveur.on("/handle_mmin",HTTP_GET, [](AsyncWebServerRequest *request) {
      handleweb(request,'<');});
  serveur.on("/handle_plus",HTTP_GET, [](AsyncWebServerRequest *request) {
      handleweb(request,'+');});
  serveur.on("/handle_min",HTTP_GET, [](AsyncWebServerRequest *request) {
      handleweb(request,'-');});
  serveur.on("/handle_ppplus",HTTP_GET, [](AsyncWebServerRequest *request) {
      handleweb(request,')');});
  serveur.on("/handle_mmmin",HTTP_GET, [](AsyncWebServerRequest *request) {
      handleweb(request,'(');});
  
    serveur.on("/handle_I",HTTP_GET, [](AsyncWebServerRequest *request) {
    handleweb(request,'i');});
  serveur.on("/handle_P",HTTP_GET,[](AsyncWebServerRequest *request) {
    handleweb(request,'P');});
  
  serveur.on("/handle_S",HTTP_GET,[](AsyncWebServerRequest *request) {
    handleweb(request,'S');});
  
  serveur.on("/logs",HTTP_GET,[](AsyncWebServerRequest *request) {
      String pre,mid,end;

      
 pre="<html><body><script>function ab(s){pf=document.getElementById('prv');pf.data=s;}function cd(s){var req = new XMLHttpRequest();req.open(\"GET\",s,false);req.send(null);alert('File'+s+' deleted')}</script><h1>Logfiles</h1><table style=\"width:100%\" style=\"height:400\"><tr><TD style=\"width:30%\"><div>";
 end="</div></td><TD style=\"vertical-align:top\" style=\"width:68%\"><div>   <object id='prv' width='75%' height='500' ></object></div></td></tr></table> </body></html>";
 mid=listDirHTML();
 static String tot;
 tot=pre+mid+end;
 int  len=tot.length();
 uint8_t *buf;buf=(uint8_t *)malloc(len+1);
 memcpy(buf,tot.c_str(),len);
 request->send(200,"text/html",tot);
 free(buf);});
 serveur.on("/bara",HTTP_GET,[](AsyncWebServerRequest *request) {
      String pre,mid,end;
 pre="<html><meta http-equiv=\"refresh\" content=\"20\" ><body><h1>Position</h1><table><tr> \
 <TD><input type='button' id='mmmin' value='---' onclick='document.location.href=\"/handle_mmmin\"'  > </TD>\
 <TD><input type='button' id='mmin' value='--' onclick='document.location.href=\"/handle_mmin\"'  > </TD>\
 <TD><input type='button' id='min' value='-' onclick='document.location.href=\"/handle_min\"'  > </TD>\
 <TD><input type='button' id='plus' value='+' onclick='document.location.href=\"/handle_plus\"'  ></td> \
 <TD><input type='button' id='pplus' value='++' onclick='document.location.href=\"/handle_pplus\"'  ></td> \
 <TD><input type='button' id='ppplus' value='+++' onclick='document.location.href=\"/handle_ppplus\"'  ></td> \
 </tr></table><pre>";
 end="</pre><table><tr> \
 <TD><input type='button' id='mmmin' value='---' onclick='document.location.href=\"/handle_mmmin\"'  > </TD>\
 <TD><input type='button' id='mmin' value='--' onclick='document.location.href=\"/handle_mmin\"'  > </TD>\
 <TD><input type='button' id='min' value='-' onclick='document.location.href=\"/handle_min\"'  > </TD>\
 <TD><input type='button' id='plus' value='+' onclick='document.location.href=\"/handle_plus\"'  ></td> \
 <TD><input type='button' id='pplus' value='++' onclick='document.location.href=\"/handle_pplus\"'  ></td> \
 <TD><input type='button' id='ppplus' value='+++' onclick='document.location.href=\"/handle_ppplus\"'  ></td> \
 </tr></table></body></html>";
 for (int i=39;i>=0;i--)
   if (strchr(screen[i],'*') ||strchr(screen[i],'M') ||strchr(screen[i],'S'  )) mid+=String(screen[i])+"\n";
 mid += String("Mark: " )+String(MARK)+String(" Space: ")+String(SPACE);
 static String tot;
 tot=pre+mid+end;
  request->send(200,"text/html",tot);
 });

    serveur.on("/logfile",HTTP_GET, [](AsyncWebServerRequest *request) {
      if (! request->hasParam("file")) return;
      char *fn,filename[20]="/";
//if (!(fn=strchr(requestPath,'?'))) return;
 strcat(filename,request->getParam("file")->value().c_str());
 File file = LittleFS.open(filename);
 Serial.println(filename);
 if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }
  char *buf,*s;
  buf=(char *)malloc(file.size()+2);
  Serial.println("- read from file:");
  for(s=buf;file.available();s++) {
    *s=(char)file.read();
  }
  *s=0;
  file.close();
  static String resp;resp=String(buf)+"\n";
request->send(200,"text/text", resp);
  free(buf); 
  });

    serveur.on("/logfiledel",HTTP_GET, [](AsyncWebServerRequest *request) {
      if (! request->hasParam("file")) return;
      char *fn,filename[20]="/";
//if (!(fn=strchr(requestPath,'?'))) return;
 strcat(filename,request->getParam("file")->value().c_str());
 Serial.println(filename);
 deleteFile2(LittleFS,filename);
 request->send(200,"text/text", "remoev");
  });
  serveur.on("/",HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("who")) {
      Serial.printf("Who? %s\n", request->getParam("who")->value().c_str());
    }

    request->send(200, "text/html", (uint8_t *)htmlContent, htmlContentLength);
  });
  serveur.on("/cons",HTTP_GET,[](AsyncWebServerRequest *request) {
static char HH[]= "	<!DOCTYPE html><html>\
<script>function pageScroll(){window.scrollBy(0,100000);}\
function abc() {var req = new XMLHttpRequest();\
req.open(\"GET\",\"/ring\",false);\
req.onload = function () {document.getElementById('pre').innerHTML = document.getElementById('pre').innerHTML  + req.responseText;};\
req.send(null); }\
</script>\
<body  >\
<pre  id='pre' nam='pre' onclick =  'setInterval(pageScroll,1000);setInterval(abc,1000)'>Click to activate</pre></body></html>";
 AsyncResponseStream* response = request->beginResponseStream("text/html");
      response->print(HH);
      //response->print(HE);
      request->send(response);

      custer=true;
      
   });
 serveur.on("/ring",HTTP_GET,[](AsyncWebServerRequest *request) {
             AsyncResponseStream* response = request->beginResponseStream("text/html");
 while (ron.available()>1) 
        {char c=(char)ron.read();
        response->print(c);      //Serial.printf("<%d>",c);
      }
      request->send(response);
                                    custer=true;  
   });

  serveur.begin();
}
 