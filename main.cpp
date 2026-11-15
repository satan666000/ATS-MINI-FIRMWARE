#include <Arduino.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <Wire.h>
#include <SI4735.h>
#include <Preferences.h>
#include "Display.h"
#include "SSBPatch.h"
#include "Languages.h"
LGFX screen;
LGFX_Sprite ui(&screen);
SI4735 radio;
Preferences prefs;
constexpr const char* FIRMWARE_NAME="KALIMAN";
constexpr const char* FIRMWARE_VERSION="1.1.17";
constexpr const char* FIRMWARE_CREATOR="KO6EAI";
constexpr int POWER=15, AMP=10, MUTE=3, RESET=16, SDA_PIN=18, SCL_PIN=17;
constexpr int BL=38, ENC_A=2, ENC_B=1, BUTTON=21, BATTERY_PIN=4;
struct Band {const char* name; uint16_t lo,hi,initial;};
Band bands[]={
 {"FM completa",6400,10800,9950},{"Onda larga",150,519,198},
 {"Onda media",520,1710,1000},{"OC completa",1711,30000,7100},
 {"OC 120m",2300,2495,2400},{"OC 90m",3200,3400,3300},
 {"OC 75m",3900,4000,3950},{"OC 60m",4750,5060,5000},
 {"OC 49m",5900,6200,6000},{"OC 41m",7200,7450,7300},
 {"OC 31m",9400,9900,9600},{"OC 25m",11600,12100,11800},
 {"OC 22m",13570,13870,13600},{"OC 19m",15100,15830,15300},
 {"OC 16m",17480,17900,17600},{"OC 15m",18900,19020,18950},
 {"OC 13m",21450,21850,21600},{"Radio OC 11m",25670,26100,25800},
 {"Afic. 160m",1800,2000,1900},{"Afic. 80m",3500,4000,3700},
 {"Afic. 60m",5250,5450,5357},{"Afic. 40m",7000,7300,7100},
 {"Afic. 30m",10100,10150,10120},{"Afic. 20m",14000,14350,14200},
 {"Afic. 17m",18068,18168,18100},{"Afic. 15m",21000,21450,21200},
 {"Afic. 12m",24890,24990,24920},{"CB USA",26965,27405,27205},
 {"Afic. 10m",28000,29700,28500},{"11m amplia",25000,30000,27000},{"AM total",150,30000,1000},
 {"Hora 2.5 MHz",2495,2505,2500},{"Hora 5 MHz",4995,5005,5000},
 {"Hora 10 MHz",9995,10005,10000},{"Hora 15 MHz",14995,15005,15000},
 {"Hora 20 MHz",19995,20005,20000},{"Hora 25 MHz",24995,25005,25000},
 // Append only: existing saved band IDs must never change.
 {"HF completa",3000,30000,7100},
 {"HF 3-5 MHz",3000,5000,3500},{"HF 5-7 MHz",5000,7000,5500},
 {"HF 7-9 MHz",7000,9000,7500},{"HF 9-11 MHz",9000,11000,10000},
 {"HF 11-13 MHz",11000,13000,12000},{"HF 13-15 MHz",13000,15000,14000},
 {"HF 15-17 MHz",15000,17000,16000},{"HF 17-19 MHz",17000,19000,18000},
 {"HF 19-21 MHz",19000,21000,20000},{"HF 21-23 MHz",21000,23000,22000},
 {"HF 23-25 MHz",23000,25000,24000},{"HF 25-27 MHz",25000,27000,26000},
 {"HF 27-30 MHz",27000,30000,28000},
 {"Afic. 75m / amplio",3800,4000,3900},
 {"160m CW",1800,2000,1810},{"160m Digital",1800,1810,1805},
 {"160m LSB / SSTV",1843,2000,1910},{"160m QRP CW",1810,1810,1810},
 {"160m QRP LSB",1910,1910,1910},{"160m Balizas",1995,2000,1998},
 {"80m RTTY / Datos",3570,3600,3580},{"80m DX LSB",3790,3800,3795},
 {"80m SSTV LSB",3845,3845,3845},{"80m Llamada AM",3885,3885,3885},
 {"60m USB / continuo",5250,5450,5357},
 {"40m RTTY / Datos",7080,7125,7100},{"40m SSTV LSB",7171,7171,7171},
 {"40m Llamada AM",7290,7290,7290},
 {"30m RTTY",10130,10140,10135},{"30m Packet",10140,10150,10145},
 {"20m RTTY",14070,14095,14080},{"20m Packet",14095,14112,14105},
 {"20m Balizas",14100,14100,14100},{"20m SSTV USB",14230,14230,14230},
 {"20m Llamada AM",14286,14286,14286},
 {"17m RTTY",18100,18105,18102},{"17m Packet",18105,18110,18108},
 {"15m RTTY / Datos",21070,21110,21080},{"15m SSTV USB",21340,21340,21340},
 {"12m RTTY",24920,24925,24922},{"12m Packet",24925,24930,24928},
 {"10m CW",28000,28070,28030},{"10m RTTY",28070,28150,28100},
 {"10m Balizas",28200,28300,28250},{"10m Voz USB",28300,29300,28500},
 {"10m AM",29000,29200,29100}
};
constexpr int BAND_COUNT=sizeof(bands)/sizeof(bands[0]);
constexpr uint8_t CB_BAND=27;
// FCC 47 CFR 95.963: channel order is not strictly frequency order.
const uint16_t cbFrequencies[]={
 26965,26975,26985,27005,27015,27025,27035,27055,27065,27075,
 27085,27105,27115,27125,27135,27155,27165,27175,27185,27205,
 27215,27225,27255,27235,27245,27265,27275,27285,27295,27305,
 27315,27325,27335,27345,27355,27365,27375,27385,27395,27405
};
int cbChannel(uint16_t frequency){
 int best=0;
 for(int i=1;i<40;i++)
  if(abs(int(frequency)-int(cbFrequencies[i]))<abs(int(frequency)-int(cbFrequencies[best])))best=i;
 return best;
}

// Menu folders use original IDs so saved frequencies and memories stay valid.
struct BandFolder {const char* name; const char* ids;};
const BandFolder folders[]={
 {"FM", "0"}, {"Onda larga", "1"}, {"Onda media", "2"},
 {"OC / Radiodifusion", "4,5,6,7,8,9,10,11,12,13,14,15,16,17"},
 {"CB / 11 metros", "27,29"},
 {"Aficionados / 160m", "18,52,53,54,55,56,57"},
 {"Aficionados / 80-75m", "19,51,58,59,60,61"},
 {"Aficionados / 60m", "20,62"},
 {"Aficionados / 40m", "21,63,64,65"},
 {"Aficionados / 30m", "22,66,67"},
 {"Aficionados / 20m", "23,68,69,70,71,72"},
 {"Aficionados / 17m", "24,73,74"},
 {"Aficionados / 15m", "25,75,76"},
 {"Aficionados / 12m", "26,77,78"},
 {"Aficionados / 10m", "28,79,80,81,82,83"},
 {"Senales horarias", "31,32,33,34,35,36"},
 {"HF / Tramos continuos", "37,38,39,40,41,42,43,44,45,46,47,48,49,50"},
 {"Cobertura completa", "3,30"}
};
constexpr int FOLDER_COUNT=sizeof(folders)/sizeof(folders[0]);
int folder=0,folderPosition=0;
bool folderOpen=false;
int folderBand(int f,int position){
 const char* p=folders[f].ids;
 for(int i=0;i<position;i++){p=strchr(p,',');if(!p)return -1;p++;}
 return atoi(p);
}
int folderSize(int f){int n=1;for(const char* p=folders[f].ids;*p;p++)if(*p==',')n++;return n;}
void locateBandFolder(int id){
 for(int f=0;f<FOLDER_COUNT;f++)for(int i=0;i<folderSize(f);i++)
 if(folderBand(f,i)==id){folder=f;folderPosition=i;return;}
}
const char* hudNames[]={"Panel","Instrumentos","Contraste","Barras LED","Doble dial","Minimal","Consola","Escaner","Tarjetas","Ecualizador","Agujas","Telemetria","Waterfall","Semaforo","Radar","Termometros","Mosaico","Vu multicolor","Orbita","Diamantes","Escalera","Doble pista"};
constexpr int HUD_COUNT=sizeof(hudNames)/sizeof(hudNames[0]);
const char* modes[]={"AM","LSB","USB"};
const uint16_t amSteps[]={1,5,9,10,50,100};
const uint16_t fmSteps[]={1,5,10,20,50,100};
const char* amFilters[]={"6","4","3","2","1","1.8","2.5"};
const char* ssbFilters[]={"1.2","2.2","3","4","0.5","1"};
const char* items[]={"Banda","Modo","Paso","Volumen","Filtro AM/SSB","AGC automatico","Atenuacion","BFO fino","HUD","Silencio","Memoria","Guardar memoria","Cargar memoria","Volver","Tema","Region","AMP","Auto VFO / AFC","WiFi control","BLE control","Clave remota","Squelch modo","Squelch nivel","Informacion","Idioma","Buscar senal","Temporizador audio"};
constexpr int ITEM_COUNT=sizeof(items)/sizeof(items[0]);
// Stable setting IDs: grouping changes navigation, not stored settings.
struct MenuGroup {const char* name; const char* hint; uint8_t ids[9]; uint8_t count;};
const MenuGroup menuGroups[]={
 {"Sintonia","Banda, modo, paso y busqueda",{0,1,2,7,17,25,15,13},8},
 {"Audio y recepcion","Volumen, filtros y silencio",{3,4,5,6,21,22,9,16,13},9},
 {"Memorias","Elegir, guardar o recuperar",{10,11,12,13},4},
 {"Pantalla","HUD, colores e idioma",{8,14,24,13},4},
 {"Conexiones","Control WiFi y Bluetooth BLE",{18,19,20,13},4},
 {"Sistema","Temporizador e informacion",{26,23,13},3}
};
constexpr int MENU_GROUP_COUNT=sizeof(menuGroups)/sizeof(menuGroups[0]);
bool menuRoot=true;
int menuGroup=0,menuPosition=0;

struct Memory {uint16_t frequency; uint8_t band,mode;};
Memory memories[10]={};
uint16_t frequencies[BAND_COUNT];
uint8_t band=0, mode=0, volume=12, stepIndex=2, amFilter=3, ssbFilter=1, attenuation=0, hud=0, slot=0;

const char* regionNames[]={"Global / libre","R1 Europa / Africa","R2 Americas","R3 Asia / Oceania","Japon","Brasil","OIRT historica"};
const char* themeNames[]={"Oceano","Ambar","Bosque","Claro","Violeta","Rojo nocturno","Grafito","Retro verde","Atardecer","Hielo","Rosa","Arena","Space","Neon","Lava","Selva","Dorado","Coral","Cobre","Caramelo","Aurora","Rainbow oscuro"};
constexpr int THEME_COUNT=sizeof(themeNames)/sizeof(themeNames[0]);
struct Palette {uint16_t background,panel,accent,text,dim;};
const Palette palettes[]={
 {0x0841,0x10E3,0x07FF,0xFFFF,0xAD55},
 {0x0800,0x18C1,0xFD20,0xFFFF,0xAD55},
 {0x0020,0x0922,0x07E8,0xFFFF,0xAD55},
 {0xEF7D,0xD69A,0x025F,0x0000,0x4208},
 {0x1003,0x2086,0xB3FF,0xFFFF,0xBD77},
 {0x0800,0x2000,0xF800,0xFC10,0xB208},
 {0x1082,0x2945,0xFFE0,0xFFFF,0xBDF7},
 {0x0000,0x0120,0x07E0,0x87F0,0x4D69},
 {0x1803,0x3086,0xFC60,0xFF7B,0xCD36},
 {0xEFFF,0xC71F,0x0396,0x0028,0x52CA},
 {0x1808,0x308E,0xFB5F,0xFFFF,0xCD79},
 {0xF735,0xDE50,0x8300,0x2080,0x6B08},
 // Space: official ATS Mini colors, adapted to the existing HUD roles.
 {0x0004,0x105B,0x3FE0,0xBEDF,0xD69A},
 {0x0803,0x2008,0xF81F,0x07FF,0xC618}, // Neon: magenta and cyan
 {0x1000,0x3000,0xFD20,0xFFE0,0xFC10}, // Lava: orange, yellow, red
 {0x0020,0x0162,0xAFE0,0xFFE0,0x76AD}, // Selva: lime and yellow
 {0x1040,0x2940,0xFEA0,0xFFF0,0xBD68}, // Dorado
 {0x2002,0x4005,0xFAEA,0xFFD2,0xCCB3}, // Coral and cream
 {0x1080,0x2901,0xCC06,0xFF37,0xAC4C}, // Cobre
 {0x2805,0x480A,0xFD20,0xFF5B,0xDD36}, // Caramelo: orange and pink
 {0x0023,0x0167,0x07F0,0xE59F,0x969B}, // Aurora: emerald and lavender
 {0x0000,0x1082,0x07FF,0xFFFF,0xAD55}  // Rainbow oscuro
};
static_assert(sizeof(palettes)/sizeof(palettes[0])==THEME_COUNT,"Cada tema necesita una paleta");
uint8_t theme=0,region=0;
constexpr uint8_t RAINBOW_THEME=THEME_COUNT-1;
uint16_t rainbowColor(int index){
 static constexpr uint16_t colors[]={0xFAAA,0xFD20,0xFFE0,0x47EA,0x07FF,0x6B5F,0xDB5F};
 return colors[index%7];
}
uint16_t menuColor(int index,uint16_t fallback){
 return theme==RAINBOW_THEME?rainbowColor(index):fallback;
}
void applyRegion(){
 const uint16_t fmLo[]={6400,8750,8800,8750,7600,7600,6590};
 const uint16_t fmHi[]={10800,10800,10800,10800,9500,10800,7400};
 bands[0].lo=fmLo[region];bands[0].hi=fmHi[region];
 bands[2].lo=region==2?530:region==0?520:531;
 bands[2].hi=region==2||region==0?1710:1602;
 // Region presets are listening guides, not national transmit permissions.
 bands[18].lo=region==1?1810:1800;
 bands[19].hi=region==1?3800:(region==3||region==4)?3900:4000;
 bands[6].lo=region==1?3950:3900;
 bands[9].lo=region==2?7300:7200;
 bands[9].hi=region==2?7400:7450;
 bands[21].hi=region==1||region==3?7200:7300;
 // Expanded segments follow the US listening plan supplied by the user.
 // Keep national authorization separate from receiver coverage.
 for(int i=0;i<BAND_COUNT;i++)frequencies[i]=constrain(frequencies[i],bands[i].lo,bands[i].hi);
}
// Manual widths are preserved while automatic reception selects its own width.
bool amFilterAuto=false,ssbFilterAuto=false;
uint8_t autoAmFilter=0,autoSsbFilter=2,filterCandidate=255;
uint32_t filterCandidateAt=0,lastFilterChange=0;
uint8_t activeFilter(){return mode?(ssbFilterAuto?autoSsbFilter:ssbFilter):(amFilterAuto?autoAmFilter:amFilter);}
int16_t bfo=0;
// AFC follows a carrier (SYNC); it is not voice-based SSB auto-tuning.
bool afcEnabled=false;
uint8_t squelch=0;
// 0: off, 1: manual, 2: adaptive RSSI noise estimate (not audio DSP).
uint8_t squelchMode=0,autoSquelchThreshold=6;
int noiseFloorQ8=0;
bool noiseFloorValid=false;
uint32_t squelchTuneAt=0,squelchSampleAt=0;
bool squelchClosed=false;
uint32_t squelchOpenedAt=0;
bool ready=false,dirty=true,menu=false,editing=false,volumeMode=false,agc=true,muted=false,patchLoaded=false;
bool ampEnabled=true,ampMenu=false,ampEditing=false;
int ampItem=0;
bool infoMenu=false;
bool signalSearch=false;
uint16_t searchRemaining=0,searchOriginal=0;
uint32_t searchAt=0,audioTimerAt=0;
uint8_t audioTimerMinutes=0;
void cancelSearch();
void startSearch();
void pollSearch(uint32_t now);
void setAudioTimer(int minutes);
bool turnFine(int d);
int infoPage=0;
uint16_t step();
// Keep every CB channel; fewer sampled points make other bands faster.
// Interpolate only for display: intermediate pixels are not extra measurements.
constexpr int WF_BINS=40, WF_SCAN_BINS=16, WF_ROWS=44;
uint8_t waterfall[WF_ROWS][WF_BINS]={};
uint8_t sweepLine[WF_BINS]={};
int sweepBin=0;
bool waterfallPaused=false,sweepActive=false,sweepWaiting=false;
uint8_t waterfallHead=0;
uint16_t waterfallColors[81];
bool settingsAvailable=false;
uint32_t sweepAt=0;
uint16_t sweepLow=0,sweepHigh=0;
int sweepCount(){return band==CB_BAND?40:min(WF_SCAN_BINS,int(bands[band].hi-bands[band].lo)+1);}
bool waterfallView(){return hud==12;}
void stopSweep(){
 if(sweepActive&&ready){radio.setFrequency(frequencies[band]);digitalWrite(AMP,ampEnabled?HIGH:LOW);}
 sweepActive=false;sweepWaiting=false;sweepBin=0;
}
void clearWaterfall(){stopSweep();memset(waterfall,0,sizeof(waterfall));memset(sweepLine,0,sizeof(sweepLine));waterfallHead=0;sweepLow=bands[band].lo;sweepHigh=bands[band].hi;dirty=true;}
uint16_t sweepFrequency(int bin){
 if(band==CB_BAND)return cbFrequencies[bin];
 int count=sweepCount();
 return count==1?sweepLow:sweepLow+(uint32_t(sweepHigh-sweepLow)*bin)/(count-1);
}
void pollWaterfall(uint32_t now){
 if(!ready||!waterfallView()||menu||waterfallPaused){stopSweep();return;}
 if(!sweepActive){
  // Overview of the ENTIRE selected band; tuning no longer zooms the plot.
  // Select a narrower band in the menu when looking for narrow signals.
  sweepLow=bands[band].lo;sweepHigh=bands[band].hi;sweepActive=true;
  digitalWrite(AMP,LOW);
 }
 if(!sweepWaiting){radio.setFrequency(sweepFrequency(sweepBin));sweepAt=millis();sweepWaiting=true;return;}
 if(uint32_t(millis()-sweepAt)<(band==0?10u:12u))return;
 radio.getCurrentReceivedSignalQuality(0);
 sweepLine[sweepBin]=radio.getCurrentRSSI();
 sweepWaiting=false;
 int count=sweepCount();
 if(++sweepBin==count){
  waterfallHead=(waterfallHead+WF_ROWS-1)%WF_ROWS;
  memcpy(waterfall[waterfallHead],sweepLine,WF_BINS);sweepBin=0;dirty=true;
 }
}
int item=0,encoderState=0,quarters=0;
bool buttonRaw=true,buttonStable=true,buttonArmed=true;
uint32_t buttonChange=0,pressedAt=0,lastSignal=0,saveAt=0;
uint8_t rssi=0,snr=0;
uint32_t lastBattery=0,lastDraw=0;
int batteryMillivolts=0,batteryLevel=-1;
bool externalPower=false;
// ATS Mini reference Battery.cpp: GPIO4, raw ADC correction 1.702.
// The monitor reads the shared supply: USB voltage is NOT battery charge.
void pollBattery(){
 uint32_t sum=0;
 for(int i=0;i<10;i++)sum+=analogRead(BATTERY_PIN);
 int measured=int(sum*1.702f/10.0f);
 batteryMillivolts=batteryMillivolts?(batteryMillivolts*3+measured)/4:measured;
 bool usb=externalPower?batteryMillivolts>4250:batteryMillivolts>4350;
 const int thresholds[]={3680,3780,3880};
 int level=batteryLevel;
 if(level<0){level=0;while(level<3&&batteryMillivolts>=thresholds[level])level++;}
 else {while(level<3&&batteryMillivolts>thresholds[level]+20)level++;
       while(level>0&&batteryMillivolts<thresholds[level-1]-20)level--;}
 static int shownVoltage=-1;
 int rounded=(batteryMillivolts+25)/50*50;
 if(level!=batteryLevel||usb!=externalPower||rounded!=shownVoltage)dirty=true;
 batteryLevel=level;externalPower=usb;shownVoltage=rounded;
}
String rdsName, rdsText;
String rdsScrollSource;
uint16_t rdsScrollOffset=0;
uint32_t rdsScrollAt=0;
constexpr uint16_t RDS_TEXT_WIDTH=276;
uint16_t rdsScrollWidth=0;
uint8_t rdsNameSegments=0;
char psBuffer[9]={},rtBuffer[65]={};
uint16_t rtSegments=0;
int8_t rtFlag=-1,rtVersion=-1;
uint32_t lastRdsSyncAt=0;
bool rdsSync=false,stereo=false;
uint32_t lastRds=0;
void resetRds(){
 // A new frequency must not inherit a closed squelch from the old station.
 squelchClosed=false;autoSquelchThreshold=6;filterCandidate=255;lastFilterChange=millis();
 digitalWrite(MUTE,muted?HIGH:LOW);
 noiseFloorValid=false;squelchTuneAt=millis();clearWaterfall();rssi=0;snr=0;stereo=false;rdsName="";rdsText="";rdsScrollSource="";rdsScrollOffset=0;rdsScrollWidth=0;rdsScrollAt=millis()+400;rdsNameSegments=0;memset(psBuffer,0,sizeof(psBuffer));memset(rtBuffer,0,sizeof(rtBuffer));rtSegments=0;rtFlag=-1;rtVersion=-1;lastRdsSyncAt=0;rdsSync=false;if(band==0&&ready){radio.getRdsStatus(0,1,0);radio.RdsInit();}dirty=true;}
String notice;
uint32_t noticeUntil=0;
bool fineTuning(){return band!=0&&band!=CB_BAND&&mode!=0&&stepIndex>=6;}
uint16_t fineStepHz(){return stepIndex==7?100:10;}
uint16_t step(){return band==CB_BAND?1:band==0?fmSteps[min(int(stepIndex),5)]:amSteps[min(int(stepIndex),5)];}

void changed(){dirty=true;saveAt=millis()+10000;if(!saveAt)saveAt=1;}
void message(const char* text){notice=text;noticeUntil=millis()+2200;dirty=true;}
void save(){
 if(!settingsAvailable){saveAt=0;return;}
 prefs.putUChar("lang",language);prefs.putUChar("band",band);prefs.putUChar("mode",mode);prefs.putUChar("vol",volume);
 prefs.putBool("afa",amFilterAuto);prefs.putBool("sfa",ssbFilterAuto);prefs.putUChar("step",stepIndex);prefs.putUChar("af",amFilter);prefs.putUChar("sf",ssbFilter);
 prefs.putUChar("att",attenuation);prefs.putBool("agc",agc);prefs.putUChar("hud",hud);
 prefs.putUChar("sqlmode",squelchMode);prefs.putUChar("sql",squelch);prefs.putBool("afc",afcEnabled);prefs.putBool("amp",ampEnabled);prefs.putUChar("theme",theme);prefs.putUChar("region",region);prefs.putShort("bfo",bfo);prefs.putBytes("freq",frequencies,sizeof(frequencies));saveAt=0;
}
void updateSquelch(uint32_t now){
 // Only use settled, normal reception samples, never waterfall measurements.
 bool supported=band==0||mode==0;
 bool settled=now-squelchTuneAt>=500;
 if(squelchMode==2&&supported&&settled&&now-squelchSampleAt>=100){
  squelchSampleAt=now;
  // A strong station must not train the noise floor upward.
  if(snr<=3){
   int sample=int(rssi)*256;
   if(!noiseFloorValid){noiseFloorQ8=sample;noiseFloorValid=true;}
   else noiseFloorQ8+=(sample-noiseFloorQ8)/(sample<noiseFloorQ8?4:128);
   autoSquelchThreshold=constrain((noiseFloorQ8+128)/256+6,6,80);
  }
 }
 uint8_t threshold=squelchMode==2?autoSquelchThreshold:squelch;
 bool close=squelchClosed;
 if(!squelchMode||!supported||!settled||!threshold)close=false;
 else if(rssi>=int(threshold)+3||(squelchMode==2&&snr>=6)){
  close=false;squelchOpenedAt=now;
 }else if(rssi<threshold&&now-squelchOpenedAt>=350)close=true;
 if(close!=squelchClosed){squelchClosed=close;digitalWrite(MUTE,(muted||close)?HIGH:LOW);dirty=true;}
}
bool autoVfoActive(){return ready&&band!=0&&band!=CB_BAND&&mode!=0&&patchLoaded&&afcEnabled;}
void audioSettings(){
 if(!ready)return;
 digitalWrite(AMP,ready&&ampEnabled&&!sweepActive?HIGH:LOW);
 digitalWrite(MUTE,(muted||squelchClosed)?HIGH:LOW);
 radio.setVolume(muted?0:volume);
 radio.setAutomaticGainControl(agc?0:1,attenuation);
 if(band!=0){
  if(mode==0)radio.setBandwidth(activeFilter(),1);
  else {
   // SYNC tracks an AM carrier; suppressed-carrier SSB needs manual BFO.
   const bool sync=autoVfoActive();
   radio.setSSBConfig(activeFilter(),1,sync?3:0,1,0,sync?0:1);
   radio.setSSBBfo(sync?0:bfo);
  }
 }
}
void updateAutoFilter(uint32_t now){
 if(band==0||!(mode?ssbFilterAuto:amFilterAuto))return;
 if(now-squelchTuneAt<1200)return;
 // Conservative SNR heuristic, not an adjacent-channel or audio analyser.
 uint8_t current=activeFilter(),target=current;
 if(mode){
  if(snr<=5)target=0; else if(snr<=10)target=1;
  else if(snr>=15)target=2;
 }else{
  if(snr<=5)target=3; else if(snr<=10)target=2;
  else if(snr>=15)target=0;
 }
 if(target==current){filterCandidate=255;return;}
 if(filterCandidate!=target){filterCandidate=target;filterCandidateAt=now;return;}
 // Require stable conditions and at least eight seconds between changes.
 if(now-filterCandidateAt<3000||now-lastFilterChange<8000)return;
 if(mode)autoSsbFilter=target;else autoAmFilter=target;
 filterCandidate=255;lastFilterChange=now;audioSettings();dirty=true;
}
void selectBand(bool showLoading=true){
 cancelSearch();
 if(band==CB_BAND){mode=0;bfo=0;frequencies[band]=cbFrequencies[cbChannel(frequencies[band])];}
 if(!ready)return;
 if((band==0||mode==0)&&stepIndex>5)stepIndex=2;
 stopSweep();
 digitalWrite(AMP,LOW);
 if(band==CB_BAND)frequencies[band]=cbFrequencies[cbChannel(frequencies[band])];
 const Band &b=bands[band];
 if(band==0){radio.setFM(b.lo,b.hi,frequencies[band],step());patchLoaded=false;radio.setRdsConfig(1,2,2,2,2);radio.setFifoCount(1);}
 else if(mode==0){radio.setAM(b.lo,b.hi,frequencies[band],step());patchLoaded=false;}
 else{
  if(!patchLoaded){
   if(showLoading){
    screen.fillScreen(0);screen.setTextColor(0xFFFF,0);screen.setTextSize(2);screen.setCursor(12,65);screen.print("Cargando SSB...");
   }
   radio.loadPatch(ssbPatch,sizeof(ssbPatch),ssbFilter);patchLoaded=true;
   radio.setSSBConfig(activeFilter(),1,0,1,0,1);
  }
  radio.setSSB(b.lo,b.hi,frequencies[band],step(),mode);
 }
 resetRds();audioSettings();delay(50);digitalWrite(AMP,ampEnabled?HIGH:LOW);changed();
}
// Cooperative search: one settled RSSI/SNR measurement per tuning step.
// This is a signal detector, not a station identification or voice detector.
void cancelSearch(){
 if(signalSearch)changed(); // Preserve the current tuning when a search is cancelled.
 signalSearch=false;
}
void startSearch(){
 if(!ready)return;
 if(band&&mode){message("Busqueda solo AM / FM");return;}
 stopSweep();waterfallPaused=true;
 // Search must not inherit an old station's closed squelch.
 squelchClosed=false;digitalWrite(MUTE,muted?HIGH:LOW);
 searchOriginal=frequencies[band];
 searchRemaining=band==CB_BAND?40:(bands[band].hi-bands[band].lo)/step()+1;
 signalSearch=true;searchAt=millis();message("Buscando / pulsar cancela");
}
void pollSearch(uint32_t now){
 if(!signalSearch||int32_t(now-searchAt)<0)return;
 // First call tunes; subsequent calls evaluate after 150 ms settling.
 if(searchRemaining!=(band==CB_BAND?40:(bands[band].hi-bands[band].lo)/step()+1)){
  radio.getCurrentReceivedSignalQuality(0);
  if(radio.getCurrentRSSI()>=max(20,int(squelch))&&radio.getCurrentSNR()>=6){
   cancelSearch();resetRds();changed();message("Senal encontrada");return;
  }
 }
 if(searchRemaining==0){
  cancelSearch();frequencies[band]=searchOriginal;radio.setFrequency(searchOriginal);
  resetRds();message("Sin senal / frecuencia restaurada");return;
 }
 int f=band==CB_BAND?cbFrequencies[(cbChannel(frequencies[band])+1)%40]:frequencies[band]+step();
 if(f>bands[band].hi)f=bands[band].lo;
 frequencies[band]=f;radio.setFrequency(f);searchRemaining--;searchAt=now+150;dirty=true;
}
void setAudioTimer(int minutes){
 audioTimerMinutes=constrain(minutes,0,120);
 audioTimerAt=audioTimerMinutes?millis()+uint32_t(audioTimerMinutes)*60000UL:0;
 dirty=true;
}
#include "Remote.h"
String value(int n){
 switch(n){
 case 25:return signalSearch?"Buscando / pulsar cancela":"Pulsar: buscar AM / FM";
 case 26:return audioTimerAt?String((uint32_t(audioTimerAt-millis())+59999)/60000)+" min / silenciar":"Desactivado";
 case 24:return languageNames[language];
 case 23:return "Radio / Firmware / Version";
 case 18:return wifiEnabled?"SI / 192.168.4.1":"NO";
 case 19:return bleEnabled?"SI / ATS-MINI":"NO";
 case 21:return squelchMode==0?"Desactivado":squelchMode==1?"Manual / AM-FM":"Automatico / AM-FM";
 case 22:return squelchMode==2?String(autoSquelchThreshold)+" dBuV / auto":String(squelch)+" dBuV / manual";
 case 20:return remoteKey;
 case 0:return bands[band].name;
 case 1:return band==CB_BAND?"AM / fijo":band==0?"FM":modes[mode];
 case 2:if(band==CB_BAND)return "1 canal (1-40)";if(fineTuning())return String(fineStepHz())+" Hz / BFO";return String(band==0?step()*10:step())+" kHz";
 case 3:return String(volume)+"/63";
 case 4:return band==0?"Solo AM/SSB":String((mode?ssbFilterAuto:amFilterAuto)?"Auto / ":"Manual / ")+String(mode?ssbFilters[activeFilter()]:amFilters[activeFilter()])+" kHz";
 case 5:return agc?"SI":"NO";
 case 6:return agc?"Auto":String(attenuation);
 case 7:return band&&mode?(afcEnabled?"AFC: BFO manual pausado":String(bfo)+" Hz"):"Solo SSB";
 case 8:return hudNames[hud];
 case 14:return themeNames[theme];
 case 15:return regionNames[region];
 case 16:return ampEnabled?"Encendido / ajustar":"Apagado / ajustar";
 case 17:return !afcEnabled?"NO / BFO manual":autoVfoActive()?"SI / SYNC portadora":"SI / pendiente en SSB";
 case 9:return muted?"SI":"NO";
 case 10:return String(slot+1)+(memories[slot].frequency?" ocupada":" vacia");
 default:return "Pulsar";
 }
}
String frequencyText(uint16_t f){
 char text[20];
 if(band==0)snprintf(text,sizeof(text),"%u.%02u",f/100,f%100);
 else snprintf(text,sizeof(text),"%u.%03u",f/1000,f%1000);
 return String(text);
}
// Small vector symbols: no font glyph dependency or image buffers.
enum MenuIcon { ANTENNA, AUDIO, MEMORY, SCREEN, WIFI, GEAR, BACK, FILTER,
                STEP, ICON_MUTE, SAVE, LOAD, KEY, INFO, CLOCK, LANGUAGE, SEARCH, THEME, BLE };
MenuIcon itemIcon(int id){
 static const MenuIcon icons[]={ANTENNA,ANTENNA,STEP,AUDIO,FILTER,GEAR,FILTER,
 STEP,SCREEN,ICON_MUTE,MEMORY,SAVE,LOAD,BACK,THEME,LANGUAGE,AUDIO,ANTENNA,
 WIFI,BLE,KEY,FILTER,FILTER,INFO,LANGUAGE,SEARCH,CLOCK};
 return id>=0&&id<ITEM_COUNT?icons[id]:GEAR;
}
// Six lightweight animation frames, shared by all vector icons.
uint8_t animationFrame=0;
uint32_t lastAnimation=0;
uint16_t iconPulse(uint16_t color){
 if(animationFrame==0||animationFrame==5) return color;
 int scale=animationFrame==1||animationFrame==4?26:21;
 return ((((color>>11)&31)*scale/31)<<11)|((((color>>5)&63)*scale/31)<<5)|((color&31)*scale/31);
}
void menuIcon(int x,int y,MenuIcon kind,uint16_t c){
 // Only the selected icon pulses; other entries stay quiet and readable.
 if(theme==RAINBOW_THEME)c=rainbowColor(int(kind));
 else if(menu&&c==palettes[theme].accent)c=iconPulse(c);
 auto line=[&](int a,int b,int d,int e){ui.drawLine(x+a,y+b,x+d,y+e,c);};
 switch(kind){
 case ANTENNA: line(8,2,8,14);line(3,3,8,8);line(13,3,8,8);line(4,14,12,14);break;
 case AUDIO: case ICON_MUTE:
  ui.drawRect(x+1,y+6,4,5,c);line(5,6,9,3);line(9,3,9,13);line(9,13,5,10);
  if(kind==ICON_MUTE){line(11,6,15,10);line(15,6,11,10);}
  else{line(12,5,14,7);line(14,7,14,9);line(14,9,12,11);}break;
 case MEMORY: case SAVE: case LOAD:
  ui.drawRoundRect(x+2,y+2,12,12,2,c);ui.drawRect(x+5,y+3,6,3,c);
  if(kind==MEMORY)ui.drawRect(x+5,y+9,6,4,c);
  else{int a=kind==SAVE?8:12,b=kind==SAVE?12:8;line(8,a,8,b);line(5,b+(kind==SAVE?-3:3),8,b);line(11,b+(kind==SAVE?-3:3),8,b);}break;
 case SCREEN:ui.drawRect(x+1,y+2,14,10,c);line(8,12,8,15);line(4,15,12,15);break;
 case WIFI:line(1,5,4,2);line(4,2,12,2);line(12,2,15,5);line(4,8,6,6);line(6,6,10,6);line(10,6,12,8);ui.fillCircle(x+8,y+12,1,c);break;
 case BLE:line(7,1,7,15);line(7,1,12,5);line(12,5,3,12);line(3,4,12,11);line(12,11,7,15);break;
 case BACK:line(2,8,14,8);line(2,8,7,3);line(2,8,7,13);break;
 case FILTER:for(int i=0;i<3;i++){line(3+i*5,2,3+i*5,14);ui.drawRect(x+1+i*5,y+4+i*3,5,3,c);}break;
 case STEP:line(1,13,5,13);line(5,13,5,9);line(5,9,10,9);line(10,9,10,4);line(10,4,15,4);break;
 case KEY:ui.drawCircle(x+5,y+5,3,c);line(7,7,14,14);line(10,10,12,8);line(12,12,14,10);break;
 case INFO:ui.drawCircle(x+8,y+8,7,c);ui.fillCircle(x+8,y+4,1,c);line(8,7,8,12);break;
 case CLOCK:ui.drawCircle(x+8,y+8,7,c);line(8,3,8,8);line(8,8,12,10);break;
 case LANGUAGE:ui.drawCircle(x+8,y+8,7,c);ui.drawEllipse(x+8,y+8,3,7,c);line(1,8,15,8);break;
 case SEARCH:ui.drawCircle(x+6,y+6,5,c);line(10,10,15,15);break;
 case THEME:ui.drawCircle(x+8,y+8,7,c);ui.fillCircle(x+5,y+5,1,c);ui.fillCircle(x+11,y+5,1,c);ui.fillCircle(x+4,y+10,1,c);ui.fillCircle(x+10,y+11,2,c);break;
 default:ui.drawCircle(x+8,y+8,4,c);for(int i=2;i<=14;i+=6){line(i,1,i,3);line(i,13,i,15);line(1,i,3,i);line(13,i,15,i);}break;
 }
}
// Fit each text into its own cell, using actual font metrics.
void textCell(int x,int y,int width,const String &original,uint16_t color,uint8_t size=1,bool center=true){
 String text=(original==rdsName||original==rdsText)?original:localize(original);
 ui.setTextSize(size);
 while(size>1&&ui.textWidth(text)>width)ui.setTextSize(--size);
 String fitted;fitted.reserve(text.length()+3);fitted=text;
 if(ui.textWidth(fitted)>width){
  while(fitted.length()&&ui.textWidth(fitted+"...")>width)fitted.remove(fitted.length()-1);
  fitted+="...";
 }
 ui.setTextColor(color);
 ui.setCursor(x+(center?max(0,(width-int(ui.textWidth(fitted)))/2):0),y);
 ui.print(fitted);
}
void label(int x,int y,const String &text,uint16_t color,uint8_t size=1){
 textCell(x,y,320-x-8,text,color,size,false);
}
void centered(int y,const String &text,uint16_t color,uint8_t size=1){
 textCell(8,y,304,text,color,size);
}
// Six-second presentation, rendered in the existing sprite. All effects
// are decorative: neither the bars nor the timeline report RF or load progress.
void drawStartup(uint32_t elapsed){
 const uint16_t bg=0x0843,dim=0x2949,cyan=0x07FF,violet=0xB81F,white=0xFFFF;
 const uint32_t frame=elapsed/40;
 ui.fillScreen(bg);
 // Deterministic star field: no images, random allocations or extra buffers.
 for(int i=0;i<28;i++){
  int x=10+(i*73)%300,y=9+(i*47)%148;
  uint16_t color=((frame/5+i)%7==0)?white:dim;
  ui.drawPixel(x,y,color);
 }
 ui.drawRoundRect(5,5,310,160,9,dim);
 ui.drawFastHLine(22,7,58,cyan);ui.drawFastHLine(240,162,58,violet);
 // Antenna with radiating waves and a softly pulsing halo.
 ui.drawCircle(160,38,26,dim);
 ui.drawCircle(160,38,28,(frame%16<8)?dim:violet);
 ui.drawLine(160,23,160,53,cyan);
 ui.drawLine(150,24,160,34,cyan);ui.drawLine(170,24,160,34,cyan);
 ui.drawFastHLine(151,54,19,cyan);
 ui.fillCircle(160,22,2,white);
 for(int wave=0;wave<3;wave++){
  int r=12+wave*6;
  uint16_t color=(frame/4%3==uint32_t(wave))?cyan:dim;
  ui.drawLine(160-r,23,157-r,30,color);
  ui.drawLine(157-r,30,157-r,39,color);
  ui.drawLine(160+r,23,163+r,30,color);
  ui.drawLine(163+r,30,163+r,39,color);
 }
 // Symmetrical animated mini equalizers, explicitly decorative.
 for(int i=0;i<7;i++){
  int h=5+((frame+i*3)%10)*2;
  ui.fillRect(46+i*7,54-h,4,h,i%2?cyan:violet);
  ui.fillRect(228+i*7,54-h,4,h,i%2?violet:cyan);
 }
 centered(63,"ATS MINI",white,4);
 centered(99,"KALIMAN  /  KO6EAI",cyan);
 centered(114,"FM / AM / HF / SSB",white);
 centered(129,"Iniciando radio...",0xBDF7);
 // Presentation timeline, not a claimed hardware initialization percentage.
 ui.drawRoundRect(40,144,240,7,3,dim);
 int fill=int(min(elapsed,6000UL)*234UL/6000UL);
 if(fill>0)ui.fillRect(43,146,fill,3,cyan);
 // Orbiting light along the bottom border.
 int comet=22+(frame*4)%276;
 ui.drawFastHLine(max(22,comet-12),157,min(13,comet-21),violet);
 ui.fillCircle(comet,157,1,white);
 ui.pushSprite(0,0);
}
// All HUDs share bounded header, tuning, instruments and status areas.
void draw(){
 const Palette &colors=palettes[theme];
 uint16_t bg=colors.background,panel=colors.panel,accent=colors.accent;
 uint16_t white=colors.text,dim=colors.dim;
 ui.fillScreen(bg);ui.fillRect(0,0,320,20,panel);
 ui.drawFastHLine(8,20,304,dim);
 // Fixed header cells: band 8..158, signal 164..202,
 // mode 206..236 and power 240..312. All share vertical centre y=10.
 textCell(8,6,150,menu?"ATS / CONFIGURACION":String(bands[band].name).substring(0,22),accent,1,false);
 // Antenna stays beside the right-hand status indicators, not at screen centre.
 const int antennaX=169;
 ui.drawLine(antennaX,4,antennaX,16,menuColor(3,accent));
 ui.drawLine(antennaX-5,4,antennaX,9,menuColor(3,accent));ui.drawLine(antennaX+5,4,antennaX,9,menuColor(3,accent));
 ui.drawFastHLine(antennaX-3,16,7,menuColor(3,accent));
 // Decorative receive waves, not a transmit or signal-strength indicator.
 for(int wave=0;wave<2;wave++){
  uint16_t waveColor=animationFrame%3==wave?accent:dim;
  int offset=7+wave*3;
  ui.drawLine(antennaX-offset,4,antennaX-offset-1,7,waveColor);
  ui.drawLine(antennaX+offset,4,antennaX+offset+1,7,waveColor);
 }
 int signalBars=ready?constrain(int(rssi)/20,0,4):0;
 for(int k=0;k<4;k++){int h=4+k*3;ui.fillRect(180+k*6,17-h,4,h,k<signalBars?menuColor(k,accent):dim);}
 textCell(206,6,30,band==0?(stereo?"ST":"MONO"):modes[mode],white);
 uint16_t batteryColor=!externalPower&&batteryLevel==0?iconPulse(0xF800):menuColor(2,accent);
 char volts[10];snprintf(volts,sizeof(volts),"%.2fV",((batteryMillivolts+25)/50*50)/1000.0f);
 String powerText=externalPower?String("USB"):String(volts);
 ui.setTextSize(1);
 // Centre the complete icon + text group in the rightmost 72-pixel cell.
 const int powerWidth=28+4+ui.textWidth(powerText);
 const int batteryX=240+(72-powerWidth)/2;
 ui.drawRoundRect(batteryX,3,26,14,2,batteryColor);
 ui.fillRect(batteryX+26,7,2,6,batteryColor);
 if(externalPower){
  uint16_t usbPulse=iconPulse(accent);
  ui.drawLine(batteryX+15,5,batteryX+8,10,usbPulse);
  ui.drawLine(batteryX+8,10,batteryX+16,10,usbPulse);
  ui.drawLine(batteryX+16,10,batteryX+10,15,usbPulse);
 }else{
  for(int k=0;k<=batteryLevel;k++)ui.fillRect(batteryX+3+k*5,6,4,8,batteryColor);
 }
 textCell(batteryX+32,6,ui.textWidth(powerText),powerText,white);
 if(!ready){label(12,60,"Radio no detectada",white);ui.pushSprite(0,0);return;}
 if(menu){
  if(infoMenu){
   label(12,27,infoPage==0?"INFORMACION / RADIO":"INFORMACION / FIRMWARE",accent);
   label(280,27,String(infoPage+1)+"/2",dim);
   ui.fillRoundRect(8,43,304,106,6,panel);
   if(infoPage==0){
    label(18,51,"Modelo: ATS Mini",white);
    label(18,68,"Receptor: SI4732",white);
    label(18,85,"Procesador: ESP32-S3",white);
    label(18,102,"Pantalla: 320 x 170",white);
    label(18,119,"Recepcion: FM / AM / HF / SSB",dim);
   }else{
    label(18,51,FIRMWARE_NAME,white);
    label(18,68,String("Version: ")+FIRMWARE_VERSION,accent);
    label(18,85,String("Creador: ")+FIRMWARE_CREATOR,white);
    label(18,102,String("Compilado: ")+__DATE__,white);
    label(18,119,"Interfaz: espanol / Solo recepcion",dim);
   }
  }else if(ampMenu){
   label(12,27,"AMP / SALIDA DE AUDIO",accent);
   const char* names[]={"Amplificador","Volumen","Volver"};
   for(int n=0;n<3;n++){
    int y=43+n*35;
    uint16_t entryColor=menuColor(n,n==ampItem?accent:white);
    ui.fillRoundRect(8,y,299,32,6,panel);
    if(n==ampItem)ui.fillRect(8,y,4,32,iconPulse(menuColor(n,accent)));
    menuIcon(19,y+8,n==2?BACK:AUDIO,n==ampItem?accent:dim);
    label(42,y+5,String(ampEditing&&n==ampItem?"> ":"")+names[n],entryColor);
    label(42,y+19,n==0?(ampEnabled?"Encendido":"Apagado"):n==1?String(volume)+" / 63 (nivel de audio)":"Pulsar para regresar",dim);
   }
  }else if(editing&&item!=0){
   menuIcon(18,27,itemIcon(item),accent);
   centered(29,"EDITAR AJUSTE",dim);
   centered(52,items[item],white,2);
   ui.fillRoundRect(10,80,300,48,8,panel);
   textCell(20,95,280,value(item),accent,2);
   centered(139,"Girar: cambiar / Pulsar: aceptar",dim);
  }else{
   bool list=editing&&item==0;
   int pos=list?(folderOpen?folderPosition:folder):menuRoot?menuGroup:menuPosition;
   int count=list?(folderOpen?folderSize(folder)+1:FOLDER_COUNT+1):menuRoot?MENU_GROUP_COUNT+1:menuGroups[menuGroup].count;
   label(10,27,list?(folderOpen?folders[folder].name:"BANDAS / CARPETAS"):menuRoot?"CONFIGURACION":menuGroups[menuGroup].name,dim);
   label(264,27,String(pos+1)+"/"+String(count),accent);
   int start=constrain(pos-1,0,max(0,count-3));
   for(int row=0;row<min(3,count);row++){
    int n=start+row,y=43+row*35;
    bool selected=n==pos;
    uint16_t entryColor=menuColor(n,selected?accent:white);
    ui.fillRoundRect(8,y,299,32,6,selected?bg:panel);
    if(selected)ui.fillRoundRect(8,y,4,32,2,iconPulse(menuColor(n,accent)));
    if(list){
     if(!folderOpen || n==folderSize(folder)){
      bool back=n==(folderOpen?folderSize(folder):FOLDER_COUNT);
      menuIcon(19,y+8,back?BACK:ANTENNA,selected?accent:dim);
      label(42,y+5,back?"< Volver":folders[n].name,entryColor);
      label(42,y+19,back?"Pulsar para regresar":String(folderSize(n))+" bandas / segmentos",dim);
      continue;
     }
     menuIcon(19,y+8,ANTENNA,selected?accent:dim);
     int idx=folderBand(folder,n);label(42,y+5,bands[idx].name,entryColor);
     char range[40];if(idx==0)snprintf(range,sizeof(range),"%.2f - %.2f MHz",bands[idx].lo/100.0,bands[idx].hi/100.0);
     else snprintf(range,sizeof(range),"%.3f - %.3f MHz",bands[idx].lo/1000.0,bands[idx].hi/1000.0);
     label(42,y+19,range,dim);
    }else if(menuRoot){
     const MenuIcon groupIcons[]={ANTENNA,AUDIO,MEMORY,SCREEN,WIFI,GEAR};
     menuIcon(19,y+8,n==MENU_GROUP_COUNT?BACK:groupIcons[n],selected?accent:dim);
     label(42,y+5,n==MENU_GROUP_COUNT?"< Volver":menuGroups[n].name,entryColor);
     label(42,y+19,n==MENU_GROUP_COUNT?"Pulsar para regresar":menuGroups[n].hint,dim);
    }else{
     int id=menuGroups[menuGroup].ids[n];
     menuIcon(19,y+8,itemIcon(id),selected?accent:dim);
     label(42,y+5,id==13?"< Volver":items[id],entryColor);
     label(42,y+19,id==13?"Pulsar para regresar":value(id),dim);
    }
   }
   ui.fillRect(313,43,3,102,panel);ui.fillRect(313,43+pos*94/(count-1),3,8,accent);
  }
 }else if(waterfallView()){
  textCell(8,24,186,frequencyText(frequencies[band])+" MHz",accent,2);
  label(202,27,waterfallPaused?"PAUSA / AUDIO":"BARRIDO / MUDO",white);
  int count=sweepCount();
  label(8,44,band==CB_BAND?"CH 1 / 26.965":frequencyText(sweepLow),dim);
  label(222,44,band==CB_BAND?"40 / 27.405":frequencyText(sweepHigh),dim);
  for(int row=0;row<WF_ROWS;row++)for(int x=0;x<304;x++){
   int v;
   if(band==CB_BAND){v=waterfall[(waterfallHead+row)%WF_ROWS][x*count/304];}
   else{
    // Smooth narrow colour pixels instead of magnified 9-pixel blocks.
    int position=x*(count-1),bin=position/303,fraction=position%303;
    int next=min(bin+1,count-1);
    v=(int(waterfall[(waterfallHead+row)%WF_ROWS][bin])*(303-fraction)+int(waterfall[(waterfallHead+row)%WF_ROWS][next])*fraction)/303;
   }
   v=constrain(v,0,80);
   uint16_t c=waterfallColors[v];
   ui.drawFastVLine(8+x,56+row*2,2,c);
  }
  int marker=band==CB_BAND?cbChannel(frequencies[band])*304/40:
   sweepHigh>sweepLow?int(uint32_t(frequencies[band]-sweepLow)*303/(sweepHigh-sweepLow)):152;
  ui.drawFastVLine(8+constrain(marker,0,303),54,90,white);
  centered(147,band==CB_BAND?"CANAL "+String(cbChannel(frequencies[band])+1):"RSSI por frecuencia / tiempo abajo",accent);
 }else{
  String freq=frequencyText(frequencies[band]);
  String head=band==CB_BAND?"CANAL "+String(cbChannel(frequencies[band])+1)+" / 40":band==0?(rdsName.length()?rdsName:"FM / RDS"):String(mode?"RECEPCION SSB":"RECEPCION AM");
  centered(25,head,white);
  int font=(hud==2||hud==5)?4:3;
  // Stable tuning card across all styles; no decorative lines through text.
  if(hud!=5){
   ui.fillRoundRect(8,37,304,38,6,panel);
   if(hud==2||hud==6||hud==11)ui.drawRoundRect(8,37,304,38,6,accent);
  }
  ui.setTextSize(font);
  int fw=ui.textWidth(freq), unitWidth=18;
  int fx=(320-fw-unitWidth-8)/2;
  textCell(fx,40,fw,freq,accent,font);
  textCell(fx+fw+8,58,unitWidth,"MHz",dim);
  for(int n=0;n<2;n++){
   int x=8+n*156;int reading=n?snr:rssi,maximum=n?50:80;
   int level=constrain(reading*100/maximum,0,100);
   if(hud!=2&&hud!=5&&hud!=6)ui.fillRoundRect(x,78,148,49,6,panel);
   if(hud==6||hud==11)ui.drawRect(x,78,148,49,dim);
   textCell(x+7,82,134,n?"SNR / CALIDAD":"RSSI / INTENSIDAD",dim);
   label(x+7,96,String(reading),white,2);
   textCell(x+48,104,36,n?"dB":"dBuV",dim,1,false);
   if(hud==1){for(int k=0;k<10;k++)ui.fillRect(x+89+(k%5)*10,94+(k/5)*11,7,8,level>k*10?accent:bg);}
   if(hud==4||hud==10){
    int cx=x+114,cy=116;
    for(int k=0;k<=10;k++){
     float angle=3.14159265f+k*3.14159265f/10;
     ui.drawLine(cx+int(cosf(angle)*18),cy+int(sinf(angle)*18),cx+int(cosf(angle)*21),cy+int(sinf(angle)*21),dim);
    }
    float angle=3.14159265f+level*3.14159265f/100;
    ui.drawLine(cx,cy,cx+int(cosf(angle)*18),cy+int(sinf(angle)*18),accent);
   }
   if(hud==9){for(int k=0;k<8;k++){int h=3+k*3;ui.fillRect(x+85+k*7,117-h,5,h,level>k*12?accent:bg);}}
   if(hud==11)label(x+96,101,String(level)+"%",accent);
   // Additional instruments use the same live RSSI/SNR, not invented audio data.
   if(hud==13){ // Traffic-light quality indicators
    const uint16_t lamps[]={0xF800,0xFFE0,0x07E0};
    int active=level<33?0:level<66?1:2;
    for(int k=0;k<3;k++){
     ui.fillCircle(x+94+k*18,103,7,k==active?lamps[k]:bg);
     ui.drawCircle(x+94+k*18,103,7,dim);
    }
   }
   if(hud==14){ // Circular signal gauge (not a scanning radar)
    int cx=x+116,cy=105;
    ui.drawCircle(cx,cy,12,dim);ui.drawCircle(cx,cy,6,dim);
    ui.drawFastHLine(cx-12,cy,25,dim);ui.drawFastVLine(cx,cy-12,25,dim);
    float a=(225.0f+level*2.7f)*3.14159265f/180.0f;
    ui.drawLine(cx,cy,cx+int(cosf(a)*11),cy+int(sinf(a)*11),accent);
    ui.fillCircle(cx,cy,2,white);
   }
   if(hud==15){
    for(int k=0;k<3;k++){
     int bx=x+92+k*16,h=level*21/100;
     uint16_t c=k==0?0x07E0:k==1?0xFFE0:0xFAEA;
     ui.drawRoundRect(bx,93,11,24,3,dim);
     ui.fillRect(bx+3,115-h,5,h,c);
    }
   }
   if(hud==16){
    for(int k=0;k<12;k++){
     uint16_t c=k<4?0x07E0:k<8?0xFFE0:0xF81F;
     ui.fillRoundRect(x+88+(k%6)*9,94+(k/6)*12,7,9,2,level*12/100>k?c:bg);
    }
   }
   if(hud==17){
    label(x+95,96,String(level)+"%",accent);
    for(int k=0;k<10;k++)ui.fillRect(x+88+k*5,112,3,5,level>k*10?(k<6?0x07E0:k<8?0xFFE0:0xF800):bg);
   }
   if(hud==18){ // Orbit: bounded ring segments driven by signal level
    int cx=x+115,cy=105;
    for(int k=0;k<16;k++){
     float a=(k*22.5f-90)*3.14159265f/180.0f;
     ui.fillCircle(cx+int(cosf(a)*12),cy+int(sinf(a)*12),2,level*16/100>k?accent:dim);
    }
    ui.fillCircle(cx,cy,3,white);
   }
   if(hud==19){
    for(int k=0;k<5;k++){
     int cx=x+92+k*10,cy=105;
     uint16_t c=level>k*20?accent:dim;
     ui.fillTriangle(cx,cy-6,cx-4,cy,cx+4,cy,c);
     ui.fillTriangle(cx,cy+6,cx-4,cy,cx+4,cy,c);
    }
   }
   if(hud==20){
    for(int k=0;k<7;k++){
     int h=4+k*3;
     ui.fillRect(x+88+k*7,117-h,5,h,level>k*100/7?accent:dim);
    }
   }
   if(hud==21){
    for(int k=0;k<2;k++){
     int y=96+k*13;
     ui.drawRoundRect(x+88,y,50,8,3,dim);
     ui.fillRect(x+90,y+2,level*46/100,4,k?white:accent);
    }
   }
   ui.fillRect(x+7,120,134,5,bg);
   if(hud>=13){
    for(int k=0;k<26;k++){
     uint16_t c=k<15?0x07E0:k<21?0xFFE0:0xF800;
     ui.fillRect(x+7+k*5,120,4,5,level>k*100/26?c:dim);
    }
   }else if(hud==3||hud==7||hud==9){
    for(int k=0;k<20;k++)ui.fillRect(x+7+k*6,120,5,5,level>k*5?accent:dim);
   }else if(hud==10){
    ui.drawFastHLine(x+7,122,134,dim);ui.fillCircle(x+7+level*132/100,122,2,accent);
   }else ui.fillRect(x+7,120,level*134/100,5,accent);
  }
  if(band==0){
   if(rdsText.length()){
    label(10,131,"RDS",dim);
    ui.setTextSize(1);ui.setTextColor(dim);
    ui.setClipRect(34,131,RDS_TEXT_WIDTH,10);
    ui.setCursor(34-int(rdsScrollOffset),131);ui.print(rdsScrollSource);
    if(rdsScrollWidth){
     ui.setCursor(34+int(max(uint16_t(RDS_TEXT_WIDTH),rdsScrollWidth))+24-int(rdsScrollOffset),131);
     ui.print(rdsScrollSource);
    }
    ui.clearClipRect();
   }else label(10,131,rdsSync?"RDS: recibiendo texto...":"RDS: esperando datos de emisora",dim);
  }else{
   label(10,132,band==CB_BAND?"CB USA / 40 canales":String("PASO ")+(fineTuning()?String(fineStepHz())+" Hz":String(band==0?step()*10:step())+" kHz")+" / "+(agc?"AGC":"MANUAL"),dim);
   textCell(10,144,230,mode?(String(ssbFilterAuto?"AUTO ":"MANUAL ")+ssbFilters[activeFilter()]+"k / "+(afcEnabled?String("AFC"):"BFO "+String(bfo))):String(amFilterAuto?"AUTO ":"MANUAL ")+amFilters[activeFilter()]+" kHz",dim,1,false);
  }
  if(band==0){
   String reception=signalSearch?"BUSCANDO":squelchClosed?"SQL CERRADO":squelchMode==2?"SQL AUTO":squelchMode==1?"SQL MANUAL":"SQL OFF";
   textCell(10,144,230,reception,dim,1,false);
  }
  textCell(244,144,68,muted?"MUTE":!ampEnabled?"AMP OFF":squelchClosed?"SQL":"VOL "+String(volume),accent,1,true);

 }
 ui.fillRect(0,157,320,13,panel);
 centered(160,notice.length()?notice:menu?(infoMenu?"Girar: pagina / Pulsar: volver":"Girar: elegir / Pulsar: abrir"):waterfallView()?"Pulsar: pausa/audio / Rueda: sintonia":volumeMode?"RUEDA: volumen / Mantener: menu":"RUEDA: sintonia / Mantener: menu",dim);
 ui.pushSprite(0,0);
}
String cleanRds(const char* text,size_t maxLen){
 String out;if(!text)return out;out.reserve(maxLen);
 for(size_t i=0;i<maxLen&&text[i]&&text[i]!='\r';i++){unsigned char c=text[i];out+=(c>=32&&c<127)?char(c):' ';}
 out.trim();return out;
}
void pollRds(){
 // Read one queued group, acknowledge its interrupt, never discard the FIFO.
 radio.getRdsStatus(1,0,0);
 const uint32_t now=millis();
 bool sync=radio.getRdsSync();
 if(sync)lastRdsSyncAt=now;
 bool visibleSync=sync||(rdsSync&&now-lastRdsSyncAt<3000);
 if(visibleSync!=rdsSync){rdsSync=visibleSync;if(!menu)dirty=true;}
 if(!radio.getRdsReceived())return;
 uint8_t group=radio.getRdsGroupType();
 uint8_t address=radio.getRdsTextSegmentAddress();
 if(group==0){
  uint8_t segment=address&3;
  // Own PS buffer: group 0 bit 4 is NOT the RadioText A/B flag.
  radio.getNext2Block(psBuffer+segment*2);
  psBuffer[8]=0;rdsNameSegments|=uint8_t(1U<<segment);
  if(rdsNameSegments==15){
   String complete=cleanRds(psBuffer,8);
   if(complete.length()&&complete!=rdsName){rdsName=complete;if(!menu)dirty=true;}
   rdsNameSegments=0;
  }
 }else if(group==2){
  uint8_t version=radio.getRdsVersionCode(),flag=radio.getRdsFlagAB();
  if(flag!=rtFlag||version!=rtVersion){
   memset(rtBuffer,0,sizeof(rtBuffer));rtSegments=0;
   rtFlag=flag;rtVersion=version;
   rdsText="";rdsScrollSource="";rdsScrollWidth=0;rdsScrollOffset=0;
   if(!menu)dirty=true;
  }
  uint8_t segment=address&15,chars=version?2:4;
  if(version)radio.getNext2Block(rtBuffer+segment*chars);
  else radio.getNext4Block(rtBuffer+segment*chars);
  rtSegments|=uint16_t(1U<<segment);
  // Publish only the contiguous received prefix, never unreceived holes.
  size_t length=0;
  for(uint8_t i=0;i<16&&(rtSegments&(1U<<i));i++)length+=chars;
  String complete=cleanRds(rtBuffer,length);
  if(complete.length()&&complete!=rdsText){rdsText=complete;if(!menu)dirty=true;}
 }
}

int wrap(int v,int count){return (v%count+count)%count;}
void adjust(int d){
 switch(item){
 case 26:setAudioTimer(wrap(audioTimerMinutes/5+d,25)*5);return;
 case 24:language=wrap(language+d,LANGUAGE_COUNT);break;
 case 0:
 if(folderOpen)folderPosition=wrap(folderPosition+d,folderSize(folder)+1);
 else folder=wrap(folder+d,FOLDER_COUNT+1);
 dirty=true;return;
 case 1:if(band==CB_BAND){message("CB: AM fijo en 40 canales");break;}if(band){mode=wrap(mode+d,3);selectBand();}else message("FM: modo fijo");break;
 case 2:if(band==CB_BAND)message("CB: paso fijo por canal");else {stepIndex=wrap(stepIndex+d,(band&&mode)?8:6);radio.setFrequencyStep(step());}break;
 case 3:volume=constrain(int(volume)+d,0,63);audioSettings();break;
 case 4:if(band){
  // First selection is Auto; remaining selections are explicit manual widths.
  int count=mode?6:7;
  bool automatic=mode?ssbFilterAuto:amFilterAuto;
  int selection=wrap((automatic?0:1+(mode?ssbFilter:amFilter))+d,count+1);
  if(mode){ssbFilterAuto=selection==0;if(selection)ssbFilter=selection-1;}
  else{amFilterAuto=selection==0;if(selection)amFilter=selection-1;}
  filterCandidate=255;lastFilterChange=millis();audioSettings();
 }break;
 case 5:agc=!agc;audioSettings();break;
 case 6:attenuation=constrain(int(attenuation)+d,0,26);audioSettings();break;
 case 7:if(autoVfoActive()){message("Desactiva AFC para BFO");break;}if(band&&mode){bfo=constrain(int(bfo)+d*10,-16000,16000);radio.setSSBBfo(bfo);}break;
 case 18:toggleWifi();return;
 case 19:toggleBle();return;
 case 21:squelchMode=wrap(squelchMode+d,3);noiseFloorValid=false;autoSquelchThreshold=6;updateSquelch(millis());break;
 case 22:if(squelchMode==2){message("Nivel automatico");return;}squelch=constrain(int(squelch)+d,0,80);updateSquelch(millis());break;
 case 20:return;
 case 17:afcEnabled=d>0;audioSettings();message(!afcEnabled?"BFO manual restaurado":autoVfoActive()?"SYNC: requiere portadora":"Guardado / activo en SSB");break;
 case 8:stopSweep();hud=wrap(hud+d,HUD_COUNT);clearWaterfall();waterfallPaused=false;break;
 case 14:theme=wrap(theme+d,THEME_COUNT);break;
 case 15:region=wrap(region+d,7);applyRegion();stepIndex=band==0?3:region==2?3:2;selectBand();break;
 case 9:muted=!muted;audioSettings();break;
 case 10:slot=wrap(slot+d,10);break;
 }
 changed();
}
void click(){
 if(signalSearch){cancelSearch();resetRds();changed();message("Busqueda cancelada");return;}
 if(menu&&!menuRoot&&!ampMenu&&!infoMenu&&item==25){startSearch();editing=false;menu=false;return;}
 if(!menu&&waterfallView()){waterfallPaused=!waterfallPaused;stopSweep();dirty=true;return;}
 if(!menu){volumeMode=!volumeMode;dirty=true;return;}
 if(infoMenu){infoMenu=false;dirty=true;return;}
 if(menuRoot){
  if(menuGroup==MENU_GROUP_COUNT)menu=false;
  else{menuRoot=false;menuPosition=0;item=menuGroups[menuGroup].ids[0];}
  dirty=true;return;
 }
 if(item==23){infoMenu=true;infoPage=0;editing=false;dirty=true;return;}
 if(ampMenu){
  if(ampItem==2){ampMenu=false;ampEditing=false;}
  else ampEditing=!ampEditing;
  dirty=true;return;
 }
 if(item==16){ampMenu=true;ampItem=0;ampEditing=false;dirty=true;return;}
 if(editing&&item==0){
  if(!folderOpen){
   if(folder==FOLDER_COUNT){editing=false;locateBandFolder(band);}
   else{folderOpen=true;folderPosition=0;}
  }else if(folderPosition==folderSize(folder)){folderOpen=false;}
  else{
   stopSweep();band=folderBand(folder,folderPosition);
   if(band>=52){const char* name=bands[band].name;mode=strstr(name,"AM")?0:strstr(name,"LSB")?1:2;bfo=0;stepIndex=0;}
   if(band==2&&region)stepIndex=region==2?3:2;
   selectBand();editing=false;menu=false;
  }
  dirty=true;return;
 }
 if(editing){editing=false;dirty=true;return;}
 if(item==13){menuRoot=true;dirty=true;return;}
 if(item==11){memories[slot]={frequencies[band],band,mode};if(settingsAvailable&&prefs.putBytes("mem",memories,sizeof(memories))==sizeof(memories))message("Memoria guardada");else message("Error al guardar memoria");}
 else if(item==12){
  Memory m=memories[slot];
  if(!m.frequency||m.band>=BAND_COUNT||m.mode>2)message("Memoria vacia o invalida");
  else {stopSweep();band=m.band;mode=m.mode;bfo=0;frequencies[band]=constrain(m.frequency,bands[band].lo,bands[band].hi);selectBand();message("Memoria cargada");}
 }else{editing=true;if(item==0){locateBandFolder(band);folderOpen=false;}}
 dirty=true;
}
const char* executeRemote(const char* text){
 if(!ready)return "Radio no detectada";
 // Parse within explicit bounds; reject overflow and trailing garbage.
 if(!text)return "Formato invalido";
 while(isspace((unsigned char)*text))++text;
 char op[12]={};size_t opLen=0;
 while(*text&&!isspace((unsigned char)*text)){
  if(opLen>=sizeof(op)-1)return "Formato invalido";
  op[opLen++]=*text++;
 }
 if(!opLen)return "Formato invalido";
 while(isspace((unsigned char)*text))++text;
 int fields=1;long n=0;
 if(*text){
  errno=0;char* end=nullptr;n=strtol(text,&end,10);
  if(end==text||errno==ERANGE)return "Formato invalido";
  while(isspace((unsigned char)*end))++end;
  if(*end)return "Formato invalido";
  fields=2;
 }
 if(fields==1&&(!strcmp(op,"UP")||!strcmp(op,"DOWN"))){
  cancelSearch();stopSweep();int d=!strcmp(op,"UP")?1:-1;
  if(turnFine(d))return autoVfoActive()?"Desactiva AFC para BFO":"OK";
  int f=band==CB_BAND?cbFrequencies[wrap(cbChannel(frequencies[band])+d,40)]:int(frequencies[band])+d*step();
  if(f>bands[band].hi)f=bands[band].lo;if(f<bands[band].lo)f=bands[band].hi;
  frequencies[band]=f;radio.setFrequency(f);resetRds();changed();return "OK";
 }
 if(fields!=2)return "Formato invalido";
 if(!strcmp(op,"SEARCH")&&(n==0||n==1)){
  if(n){if(band&&mode)return "Busqueda solo AM / FM";startSearch();}
  else{cancelSearch();resetRds();changed();}return "OK";
 }
 if(!strcmp(op,"TIMER")&&n>=0&&n<=120){setAudioTimer(n);return "OK";}
 cancelSearch();
 if(!strcmp(op,"BAND")&&n>=0&&n<BAND_COUNT){stopSweep();band=n;selectBand();}
 else if(!strcmp(op,"FREQ")&&n>=bands[band].lo&&n<=bands[band].hi){
  if(band==CB_BAND&&cbFrequencies[cbChannel(n)]!=n)return "Usa CH 1..40";
  stopSweep();frequencies[band]=n;radio.setFrequency(n);resetRds();
 }
 else if(!strcmp(op,"CH")&&n>=1&&n<=40){stopSweep();band=CB_BAND;mode=0;frequencies[band]=cbFrequencies[n-1];selectBand();}
 else if(!strcmp(op,"MODE")&&n>=0&&n<=2&&band&&band!=CB_BAND){mode=n;selectBand();}
 else if(!strcmp(op,"VOL")&&n>=0&&n<=63){volume=n;audioSettings();}
 else if(!strcmp(op,"MUTE")&&(n==0||n==1)){muted=n;audioSettings();}
 else if(!strcmp(op,"BFO")&&n>=-16000&&n<=16000&&band&&mode&&!autoVfoActive()){bfo=n;radio.setSSBBfo(bfo);}
 else if((!strcmp(op,"AFC")||!strcmp(op,"AUTOVFO"))&&(n==0||n==1)){afcEnabled=n;audioSettings();}
 else if(!strcmp(op,"HUD")&&n>=0&&n<HUD_COUNT){stopSweep();hud=n;clearWaterfall();waterfallPaused=false;}
 else if(!strcmp(op,"THEME")&&n>=0&&n<THEME_COUNT)theme=n;
 else if(!strcmp(op,"SQLMODE")&&n>=0&&n<=2){squelchMode=n;noiseFloorValid=false;autoSquelchThreshold=6;updateSquelch(millis());}
 else if(!strcmp(op,"SQL")&&n>=0&&n<=80){squelch=n;squelchMode=n?1:0;updateSquelch(millis());}
 else if(!strcmp(op,"STORE")&&n>=1&&n<=10){memories[n-1]={frequencies[band],band,mode};if(!settingsAvailable||prefs.putBytes("mem",memories,sizeof(memories))!=sizeof(memories))return "Error al guardar";}
 else if(!strcmp(op,"RECALL")&&n>=1&&n<=10){Memory m=memories[n-1];if(!m.frequency||m.band>=BAND_COUNT||m.mode>2)return "Memoria vacia o invalida";stopSweep();band=m.band;mode=m.mode;bfo=0;frequencies[band]=constrain(m.frequency,bands[band].lo,bands[band].hi);selectBand();}
 else if(!strcmp(op,"AMP")&&(n==0||n==1)){ampEnabled=n;audioSettings();}
 else return "Valor o modo no valido";
 changed();return "OK";
}
void setup(){
 Serial.begin(115200);
 analogReadResolution(12);
 analogSetPinAttenuation(BATTERY_PIN,ADC_11db);
 pollBattery();
 pinMode(AMP,OUTPUT);digitalWrite(AMP,LOW);
 pinMode(MUTE,OUTPUT);digitalWrite(MUTE,LOW);
 pinMode(POWER,OUTPUT);digitalWrite(POWER,HIGH);delay(100);
 Wire.begin(SDA_PIN,SCL_PIN);
 pinMode(BL,OUTPUT);digitalWrite(BL,LOW);
 screen.init();screen.setRotation(3);screen.fillScreen(0);digitalWrite(BL,HIGH);
 ui.setColorDepth(16);
 if(!ui.createSprite(320,170)){
  screen.setTextColor(0xFFFF,0);screen.setCursor(8,50);screen.print("Error: memoria de pantalla");
  Serial.println("No se pudo reservar el buffer de pantalla; radio desactivada.");
  while(true)delay(1000);
 }
 ui.setTextWrap(false);
 for(int v=0;v<=80;v++)waterfallColors[v]=v<20?screen.color565(0,0,v*10):v<40?screen.color565(0,(v-20)*12,200):v<60?screen.color565((v-40)*12,240,200-(v-40)*10):screen.color565(240,240-(v-60)*10,0);
 rdsName.reserve(8);rdsText.reserve(64);rdsScrollSource.reserve(64);notice.reserve(64);
 pinMode(ENC_A,INPUT_PULLUP);pinMode(ENC_B,INPUT_PULLUP);pinMode(BUTTON,INPUT_PULLUP);
 encoderState=(digitalRead(ENC_A)<<1)|digitalRead(ENC_B);
 settingsAvailable=prefs.begin("ats-custom",false);
 language=constrain(prefs.getUChar("lang",0),0,LANGUAGE_COUNT-1);
 if(!settingsAvailable)Serial.println("Ajustes no disponibles: se usaran valores iniciales.");
 ampEnabled=prefs.getBool("amp",true);
 afcEnabled=prefs.getBool("afc",false);
 squelch=constrain(prefs.getUChar("sql",0),0,80);
 squelchMode=constrain(prefs.getUChar("sqlmode",squelch?1:0),0,2);
 for(int i=0;i<BAND_COUNT;i++)frequencies[i]=bands[i].initial;
 size_t savedFreqBytes=prefs.getBytesLength("freq");
 if(savedFreqBytes && savedFreqBytes<=sizeof(frequencies) && savedFreqBytes%sizeof(uint16_t)==0)
  prefs.getBytes("freq",frequencies,savedFreqBytes);
 for(int i=0;i<BAND_COUNT;i++)frequencies[i]=constrain(frequencies[i],bands[i].lo,bands[i].hi);
 band=constrain(prefs.getUChar("band",0),0,BAND_COUNT-1);mode=constrain(prefs.getUChar("mode",0),0,2);
 volume=constrain(prefs.getUChar("vol",12),0,63);stepIndex=constrain(prefs.getUChar("step",2),0,7);
 amFilterAuto=prefs.getBool("afa",false);ssbFilterAuto=prefs.getBool("sfa",false);
 amFilter=constrain(prefs.getUChar("af",3),0,6);ssbFilter=constrain(prefs.getUChar("sf",1),0,5);
 attenuation=constrain(prefs.getUChar("att",0),0,26);agc=prefs.getBool("agc",true);hud=constrain(prefs.getUChar("hud",0),0,HUD_COUNT-1);
 theme=constrain(prefs.getUChar("theme",0),0,THEME_COUNT-1);region=constrain(prefs.getUChar("region",0),0,6);applyRegion();
 bfo=constrain(prefs.getShort("bfo",0),-16000,16000);
 if(prefs.getBytesLength("mem")==sizeof(memories))prefs.getBytes("mem",memories,sizeof(memories));
 for(auto &m:memories){
  if(m.band>=BAND_COUNT||m.mode>2||m.frequency<bands[m.band].lo||m.frequency>bands[m.band].hi)m={0,0,0};
  else if(m.band==CB_BAND){m.mode=0;m.frequency=cbFrequencies[cbChannel(m.frequency)];}
 }
 const uint32_t startupAt=millis();
 uint32_t startupFrameAt=startupAt;
 drawStartup(0);
 while(uint32_t(millis()-startupAt)<6000UL){
  const uint32_t now=millis();
  if(uint32_t(now-startupFrameAt)>=40UL){
   startupFrameAt=now;drawStartup(now-startupAt);
  }
  delay(1); // Yield to the ESP32 tasks throughout the presentation.
 }
 drawStartup(6000);
 clearWaterfall();
 ready=radio.getDeviceI2CAddress(RESET)!=0;
 // Welcome finishes before a separate SSB loading screen is shown.
 if(ready){radio.setup(RESET,0);selectBand(true);}else Serial.println("No se detecta SI4732; amplificador desactivado.");
 // Synchronize the button after startup to avoid a phantom click.
 buttonRaw=buttonStable=digitalRead(BUTTON);
 buttonArmed=buttonStable; // Ignore the release of a button held during boot.
 buttonChange=millis();
 // Discard encoder transitions made during the blocking startup sequence.
 encoderState=(digitalRead(ENC_A)<<1)|digitalRead(ENC_B);quarters=0;
 remoteSetup();
 draw();dirty=false;
}
bool turnFine(int d){
 if(!fineTuning())return false;
 if(autoVfoActive()){message("Desactiva AFC para BFO");return true;}
 stopSweep();bfo=constrain(int(bfo)+d*fineStepHz(),-16000,16000);
 radio.setSSBBfo(bfo);changed();return true;
}
void loop(){
 static const int8_t transitions[]={0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
 int state=(digitalRead(ENC_A)<<1)|digitalRead(ENC_B);
 if(state!=encoderState){quarters+=transitions[(encoderState<<2)|state];encoderState=state;}
 if(abs(quarters)>=4){
  int d=quarters>0?1:-1;quarters=0;
  if(ready){
   if(signalSearch){cancelSearch();resetRds();}
   if(menu){if(infoMenu){infoPage=wrap(infoPage+d,2);dirty=true;}else if(ampMenu){
    if(ampEditing){if(ampItem==0)ampEnabled=!ampEnabled;else if(ampItem==1)volume=constrain(int(volume)+d,0,63);audioSettings();changed();}
    else{ampItem=wrap(ampItem+d,3);dirty=true;}
   }else if(editing)adjust(d);else{
    if(menuRoot)menuGroup=wrap(menuGroup+d,MENU_GROUP_COUNT+1);
    else{menuPosition=wrap(menuPosition+d,menuGroups[menuGroup].count);item=menuGroups[menuGroup].ids[menuPosition];}
    dirty=true;
   }}
   else if(volumeMode){volume=constrain(int(volume)+d,0,63);audioSettings();changed();}
   else if(turnFine(d)){}
   else{stopSweep();int next;
    if(band==CB_BAND)next=cbFrequencies[wrap(cbChannel(frequencies[band])+d,40)];
    else{next=int(frequencies[band])+d*step();if(next>bands[band].hi)next=bands[band].lo;if(next<bands[band].lo)next=bands[band].hi;}frequencies[band]=next;radio.setFrequency(next);resetRds();changed();}
  }
 }
 uint32_t now=millis();bool raw=digitalRead(BUTTON);
 if(raw!=buttonRaw){buttonRaw=raw;buttonChange=now;}
 if(now-buttonChange>=30&&raw!=buttonStable){
  buttonStable=raw;if(!raw){pressedAt=now;buttonArmed=true;}
  else if(!buttonArmed){buttonArmed=true;}
  else if(ready){if(now-pressedAt>=700){cancelSearch();stopSweep();menu=!menu;menuRoot=true;menuGroup=0;menuPosition=0;item=0;infoMenu=false;ampMenu=false;ampEditing=false;editing=false;volumeMode=false;dirty=true;}else click();}
 }
 remotePoll(now);
 now=millis(); // Remote commands can retune or load the SSB patch.
 pollSearch(now);
 if(!signalSearch)pollWaterfall(millis());
 now=millis(); // Never compare fresh tuning timestamps with an older clock.
 if(audioTimerAt&&int32_t(now-audioTimerAt)>=0){
  audioTimerAt=0;audioTimerMinutes=0;cancelSearch();stopSweep();waterfallPaused=true;
  muted=true;audioSettings();message("Temporizador: audio silenciado");changed();
 }
 if(ready&&!sweepActive&&!signalSearch&&now-lastSignal>=100){lastSignal=now;radio.getCurrentReceivedSignalQuality(0);uint8_t r=radio.getCurrentRSSI(),s=radio.getCurrentSNR();bool st=band==0&&radio.getCurrentPilot();if(r!=rssi||s!=snr||st!=stereo){rssi=r;snr=s;stereo=st;if(!menu)dirty=true;}}
 now=millis(); // Signal-quality reads also consume time.
 if(ready&&!sweepActive&&!signalSearch){updateSquelch(now);updateAutoFilter(now);}
 if(ready&&!sweepActive&&!signalSearch&&band==0&&now-lastRds>=20){lastRds=now;pollRds();}
 // Advance once per displayed frame, never catch up with a multi-pixel jump.
 // Keep both copies moving through the wrap without a pause at every lap.
 bool scrollFrame=false;
 now=millis(); // Radio and remote calls above can consume time.
 if(band==0&&!menu&&!waterfallView()){
  if(rdsScrollSource!=rdsText){
   // Preserve motion while the receiver appends segments of this message.
   bool continuation=rdsScrollSource.length()&&rdsText.startsWith(rdsScrollSource);
   rdsScrollSource=rdsText;
   if(!continuation){rdsScrollOffset=0;rdsScrollAt=now+700;}
   ui.setTextSize(1);rdsScrollWidth=ui.textWidth(rdsScrollSource);
   dirty=true;
  }
  if(rdsScrollWidth&&int32_t(now-rdsScrollAt)>=0&&now-lastDraw>=30){
   rdsScrollOffset=(rdsScrollOffset+1)%(max(uint16_t(RDS_TEXT_WIDTH),rdsScrollWidth)+24);
   scrollFrame=true;
  }
 }else rdsScrollAt=now+400;
 if(saveAt&&int32_t(now-saveAt)>=0)save();
 if(notice.length()&&int32_t(now-noticeUntil)>=0){notice="";dirty=true;}
 if(now-lastBattery>=2000){lastBattery=now;pollBattery();}
 // No delays, new buffers or radio calls for animation. Never catch up
 // missed frames after a slow operation; preserve the RDS frame cadence.
 if(ready&&(!menu||ampMenu||(!infoMenu&&!editing)|| (editing&&item==0))&&now-lastAnimation>=160){
  lastAnimation=now;animationFrame=(animationFrame+1)%6;dirty=true;
 }
 if(scrollFrame||(dirty&&now-lastDraw>=50)){
  draw();dirty=false;lastDraw=millis();
  if(scrollFrame)rdsScrollAt=lastDraw+30;
 }
 delay(1);
}
