#include <Arduino.h>
#include <esp_system.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <Wire.h>
#include <SI4735.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "Display.h"
#include "SSBPatch.h"
#include "Languages.h"
LGFX screen;
LGFX_Sprite ui(&screen);
SI4735 radio;
Preferences prefs;
bool startupAbductCow=true;
constexpr const char* FIRMWARE_NAME="KALIMAN";
constexpr const char* FIRMWARE_VERSION="1.1.88";
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
// Keep legacy numeric IDs valid for saved preferences.
const char* hudNames[]={"Estudio","Instrumental","Digital","Barras RF","Instrumental","Minimal","Estudio","Barras RF","Estudio","Barras RF","Instrumental","Digital","Estudio","Estudio","Instrumental","Barras RF","Digital","Digital","Instrumental","Digital","Barras RF","Barras RF","Barras RF","Instrumental","Instrumental","Digital","Diales","Prisma RF","Matriz RF","Electro RF","Anillos","Termometros","Cinta RF","Diamantes","Horizonte","Orbita RF","Regla RF","Balance RF"};
constexpr int HUD_COUNT=sizeof(hudNames)/sizeof(hudNames[0]);
constexpr uint8_t selectableHuds[]={0,1,2,5,3,26,27,28,29,30,31,32,33,34,35,36,37};
constexpr int SELECTABLE_HUD_COUNT=sizeof(selectableHuds)/sizeof(selectableHuds[0]);
uint8_t canonicalHud(uint8_t id){
 switch(id){
 case 0:case 1:case 2:case 3:case 5:case 26:case 27:case 28:case 29:case 30:case 31:case 32:case 33:case 34:case 35:case 36:case 37:return id;
 case 4:case 10:case 14:case 18:case 23:case 24:return 1;
 case 11:case 16:case 17:case 19:case 25:return 2;
 case 7:case 9:case 15:case 20:case 21:case 22:return 3;
 default:return 0;
 }
}
uint8_t nextHud(uint8_t id,int direction){
 id=canonicalHud(id);
 for(int i=0;i<SELECTABLE_HUD_COUNT;i++)if(selectableHuds[i]==id)
  return selectableHuds[(i+direction%SELECTABLE_HUD_COUNT+SELECTABLE_HUD_COUNT)%SELECTABLE_HUD_COUNT];
 return 0;
}
const char* modes[]={"AM","LSB","USB"};
const uint16_t amSteps[]={1,5,9,10,50,100};
const uint16_t fmSteps[]={1,5,10,20,50,100};
const char* amFilters[]={"6","4","3","2","1","1.8","2.5"};
const char* ssbFilters[]={"1.2","2.2","3","4","0.5","1"};
const char* items[]={"Banda","Modo","Paso","Volumen","Filtro AM/SSB","AGC automatico","Atenuacion","BFO fino","HUD","Silencio","","","","Volver","Tema","Region","AMP","SYNC portadora (SSB)","","","","Squelch modo","Squelch nivel","Informacion","Idioma","Buscar senal","Temporizador audio","","FM estereo","Soft mute AM/SSB","Busqueda RSSI","Busqueda SNR","Audio FM 50/75 us","Ganancia AVC AM","Reloj: hora","Reloj: minutos","Reloj: AM / PM","Reloj: formato","WiFi TIME","TIME SYNC","AUTO CLOCK","WiFi","Clima","Propagacion","VFO A/B","Frecuencia directa","RDS FM","Reposo pantalla"};
constexpr int ITEM_COUNT=sizeof(items)/sizeof(items[0]);
// Stable setting IDs: grouping changes navigation, not stored settings.
struct MenuGroup {const char* name; const char* hint; uint8_t ids[14]; uint8_t count;};
const MenuGroup menuGroups[]={
 {"Sintonia","Banda, modo, paso y busqueda",{0,1,2,7,17,25,30,31,15,44,45,13},12},
 {"Audio y recepcion","Volumen, filtros y silencio",{3,4,5,6,21,22,9,16,28,29,32,33,13},13},
 {"Pantalla","HUD, colores e idioma",{8,14,24,46,47,13},6},
 {"Clima","Tiempo y unidades",{42,13},2},
 {"Propagacion","Indices solares y bandas HF/CB",{43,13},2},
 {"Sistema","Temporizador e informacion",{34,35,36,37,38,39,40,26,23,41,13},11}
};
constexpr int MENU_GROUP_COUNT=sizeof(menuGroups)/sizeof(menuGroups[0]);
bool menuRoot=true;
int menuGroup=0,menuPosition=0;

uint16_t frequencies[BAND_COUNT];
uint8_t band=0, mode=0, volume=12, stepIndex=2, amFilter=3, ssbFilter=1, attenuation=0, hud=0;

bool rdsEnabled=true;
uint8_t screenSleep=0, activeVfo=0;
uint32_t lastInteraction=0;
bool screenAsleep=false, directEntry=false;
uint32_t directHz=0;
uint8_t directDigit=0;
const uint32_t digitPowers[]={10000000,1000000,100000,10000,1000,100,10,1};
struct VfoState {uint16_t frequency; int16_t fine; uint8_t bandId, modeId, stepId;};
VfoState vfo[2]={{9950,0,0,0,2},{7100,0,37,1,2}};
void selectBand(bool showLoading);

const char* regionNames[]={"Global / libre","R1 Europa / Africa","R2 Americas","R3 Asia / Oceania","Japon","Brasil","OIRT historica"};
const char* themeNames[]={"Oceano","Ambar","Bosque","Claro","Violeta","Rojo nocturno","Grafito","Retro verde","Atardecer","Hielo","Rosa","Arena","Space","Neon","Lava","Selva","Dorado","Coral","Cobre","Caramelo","Aurora","Rainbow oscuro","Monitor ECG","Medianoche","Zafiro","Esmeralda","Rubi","Plasma","Nordico","Marfil","Sepia","Petroleo","Ultravioleta","Terminal ambar","Cian nocturno"};
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
 {0x0000,0x1082,0x07FF,0xFFFF,0xAD55}, // Rainbow oscuro
 {0x0000,0x0020,0x07E0,0xEFFF,0x9DF3},
 {0x0001,0x0843,0x8DFF,0xFFFF,0xBDF7},
 {0x0005,0x108A,0x4DFF,0xFFFF,0xBDF7},
 {0x0021,0x0144,0x47F2,0xFFFF,0xAEB6},
 {0x1001,0x3003,0xFB30,0xFFFF,0xDD77},
 {0x1004,0x280A,0xFB7F,0xFFFF,0xCE59},
 {0x1083,0x2947,0x8F7D,0xFFFF,0xBDF7},
 {0xFFBC,0xEF18,0x6285,0x1082,0x5269},
 {0x2081,0x3943,0xFED0,0xFFBC,0xC5B0},
 {0x0023,0x08E6,0x5FFB,0xFFFF,0xAEBB},
 {0x0804,0x2009,0xC39F,0xFFFF,0xCE59},
 {0x0000,0x1840,0xFE40,0xFFF0,0xBD68},
 {0x0001,0x0843,0x07FF,0xFFFF,0xAD55}
};
static_assert(sizeof(palettes)/sizeof(palettes[0])==THEME_COUNT,"Cada tema necesita una paleta");
uint8_t theme=0,region=0;
constexpr uint8_t RAINBOW_THEME=21;
uint16_t rainbowColor(int index){
 static constexpr uint16_t colors[]={0xFAAA,0xFD20,0xFFE0,0x47EA,0x07FF,0x6B5F,0xDB5F};
 return colors[(index%7+7)%7];
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
// Defaults preserve the receiver soft-mute default and existing search limits.
uint8_t softMuteDb=8,searchRssi=20,searchSnr=6;
// Zero preserves the chip setting captured after band initialization.
uint8_t fmDeemphasis=0;
int32_t originalFmDeemphasis=-1;
void cancelSearch();
void startSearch();
void pollSearch(uint32_t now);
void setAudioTimer(int minutes);
bool turnFine(int d);
int infoPage=0;
uint16_t step();
bool settingsAvailable=false;
constexpr size_t ECG_SAMPLES=96;
uint8_t ecgSamples[ECG_SAMPLES]={};
size_t ecgWrite=0,ecgCount=0;
int hudVisual[2]={0,0}; // fixed-point percentages (x256)
bool hudVisualInitialized=false;
void resetHud(){ecgWrite=0;ecgCount=0;hudVisual[0]=hudVisual[1]=0;hudVisualInitialized=false;dirty=true;}
uint8_t ecgSnrSamples[ECG_SAMPLES]={};
void recordEcg(uint8_t sample,uint8_t snrSample){
 if(hud!=29)return;
 bool traceChanged=ecgCount<ECG_SAMPLES;
 for(size_t i=0;i<ecgCount;i++)if(ecgSamples[i]!=sample||ecgSnrSamples[i]!=snrSample){traceChanged=true;break;}
 ecgSamples[ecgWrite]=sample;
 ecgSnrSamples[ecgWrite]=snrSample;
 ecgWrite=(ecgWrite+1)%ECG_SAMPLES;
 if(ecgCount<ECG_SAMPLES)ecgCount++;
 if(traceChanged&&!menu)dirty=true;
}
int item=0,encoderState=0,quarters=0;
bool buttonRaw=true,buttonStable=true,buttonArmed=true;
uint32_t buttonChange=0,pressedAt=0,lastSignal=0,saveAt=0;
uint8_t rssi=0,snr=0;
// Live receiver measurements, not microphone levels. Actual cadence also
// depends on I2C transactions and screen drawing; unchanged values stay steady.
constexpr uint32_t SIGNAL_POLL_MS=100; // 10 real samples/s; avoid redundant I2C traffic.
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
uint8_t rdsNameSegments=0; // Next expected PS segment (0 through 3).
char psBuffer[9]={},rtBuffer[65]={};
uint16_t rtSegments=0;
int8_t rtFlag=-1,rtVersion=-1;
uint32_t lastRdsSyncAt=0;
bool rdsSync=false,stereo=false;
bool fmForceMono=false;
int32_t fmDefaultStereo=-1,fmDefaultMono=-1;
uint32_t lastRds=0;
void resetRds(bool holdAudio=false){
 // A new frequency must not inherit a closed squelch from the old station.
 squelchClosed=false;autoSquelchThreshold=6;filterCandidate=255;lastFilterChange=millis();
 digitalWrite(MUTE,(holdAudio||muted)?HIGH:LOW);
 noiseFloorValid=false;squelchTuneAt=millis();resetHud();rssi=0;snr=0;stereo=false;rdsName="";rdsText="";rdsScrollSource="";rdsScrollOffset=0;rdsScrollWidth=0;rdsScrollAt=millis()+400;rdsNameSegments=0;memset(psBuffer,0,sizeof(psBuffer));memset(rtBuffer,0,sizeof(rtBuffer));rtSegments=0;rtFlag=-1;rtVersion=-1;lastRdsSyncAt=0;rdsSync=false;if(band==0&&ready){radio.getRdsStatus(0,1,0);radio.RdsInit();}dirty=true;}
String notice;
uint32_t noticeUntil=0;
bool fineTuning(){return band!=0&&band!=CB_BAND&&mode!=0&&stepIndex>=6;}
uint16_t fineStepHz(){return stepIndex==7?100:10;}
uint16_t step(){return band==CB_BAND?1:band==0?fmSteps[min(int(stepIndex),5)]:amSteps[min(int(stepIndex),5)];}

// Local clock: Internet UTC plus automatically detected local offset.
// Without a backed RTC, elapsed time while powered off is unknown.
bool clockSet=false,clock12h=false;
uint32_t clockBaseSeconds=0;
int64_t clockBaseUs=0;
int clockLastMinute=-1;
int64_t clockSavedAtUs=0;
bool autoClock=true;
bool internetClock=false,zoneDst=false;
int32_t zoneOffset=0;
String zoneName,netClockStatus="Esperando Internet";
uint32_t nextTimeQuery=0;
struct TimeReply { bool ok; int32_t offset; bool dst; char zone[80]; };
QueueHandle_t timeReplies=nullptr;
bool timeQueryRunning=false,forceTimeQuery=false;
bool infoRunning=false; // Shared scheduling guard; workers never access receiver/UI.
// Bound even responses without Content-Length (including chunked transfer).
class InfoStream : public Stream {
 Stream& input; size_t left;
public:
 InfoStream(Stream& s,size_t limit=65536):input(s),left(limit){setTimeout(8000);}
 int available() override {return left?input.available():0;}
 int read() override {if(!left)return -1;int c=input.read();if(c>=0)--left;return c;}
 int peek() override {return left?input.peek():-1;}
 void flush() override {}
 size_t write(uint8_t) override {return 0;}
};

void timeWorker(void*){
 TimeReply reply={};
 HTTPClient http;
 http.setConnectTimeout(4000);http.setTimeout(4000);http.useHTTP10(true);
 if(http.begin("http://worldtime.timezone.io/api/ip")){
  if(http.GET()==200 && (http.getSize()<0 || http.getSize()<=4096)){
   JsonDocument doc;
   InfoStream stream(http.getStream(),4096);
   if(!deserializeJson(doc,stream,DeserializationOption::NestingLimit(4)) && doc["raw_offset"].is<int>() &&
      doc["dst_offset"].is<int>() && doc["dst"].is<bool>() && doc["timezone"].is<const char*>()){
    int64_t offset=int64_t(doc["raw_offset"].as<int>())+doc["dst_offset"].as<int>();
    reply.offset=(offset>=-50400 && offset<=50400)?int32_t(offset):INT32_MAX;
    reply.dst=doc["dst"].as<bool>();
    strlcpy(reply.zone,doc["timezone"].as<const char*>(),sizeof(reply.zone));
    reply.ok=reply.offset>=-50400 && reply.offset<=50400 && strlen(reply.zone)>0;
   }
  }
  http.end();
 }
 xQueueSend(timeReplies,&reply,0);vTaskDelete(nullptr);
}
void internetClockTick(){
 if(!timeReplies)timeReplies=xQueueCreate(1,sizeof(TimeReply));
 TimeReply reply;
 if(timeReplies && xQueueReceive(timeReplies,&reply,0)==pdTRUE){
  timeQueryRunning=false;
  if(reply.ok){
   bool zoneChanged=zoneOffset!=reply.offset || zoneDst!=reply.dst || zoneName!=reply.zone;
   zoneOffset=reply.offset;zoneDst=reply.dst;zoneName=reply.zone;
   if(settingsAvailable && zoneChanged){prefs.putInt("tzoffset",zoneOffset);prefs.putBool("tzdst",zoneDst);prefs.putString("tzname",zoneName);}
   netClockStatus="Zona automatica / esperando NTP";nextTimeQuery=millis()+900000UL;
  }else{netClockStatus="Sin zona / reintento en 1 min";nextTimeQuery=millis()+60000UL;}
  dirty=true;
 }
 if(WiFi.status()!=WL_CONNECTED)return;
 if((autoClock||forceTimeQuery)&&!timeQueryRunning&&!infoRunning&&timeReplies&&int32_t(millis()-nextTimeQuery)>=0){
  timeQueryRunning=true;forceTimeQuery=false;netClockStatus="Consultando zona por Internet";
  if(xTaskCreate(timeWorker,"clock-http",6144,nullptr,1,nullptr)!=pdPASS){timeQueryRunning=false;nextTimeQuery=millis()+60000UL;}
  dirty=true;
 }
 time_t utc=time(nullptr);
 if(autoClock && zoneName.length() && utc>1700000000){
  uint32_t seconds=(int64_t(utc)+zoneOffset)%86400;
  clockBaseSeconds=seconds;clockBaseUs=esp_timer_get_time();clockSet=true;
  if(!internetClock){internetClock=true;netClockStatus="Hora NTP / zona automatica";dirty=true;}
 }
}
uint32_t localClockSeconds(){
 return (clockBaseSeconds+uint64_t((esp_timer_get_time()-clockBaseUs)/1000000LL))%86400UL;
}
void persistClock(){
 if(!settingsAvailable||!clockSet)return;
 prefs.putUInt("clocksec",localClockSeconds());
 prefs.putBool("clockset",true);
 clockSavedAtUs=esp_timer_get_time();
}
void setLocalClock(int hour,int minute){
 // Manual edits must not be overwritten by NTP on the next loop.
 autoClock=false;internetClock=false;forceTimeQuery=false;
 if(settingsAvailable)prefs.putBool("autoclock",false);
 clockBaseSeconds=hour*3600+minute*60;
 clockBaseUs=esp_timer_get_time();clockSet=true;
 persistClock();
}
String localClockText(){
 if(!clockSet)return "Reloj sin ajustar";
 uint32_t seconds=localClockSeconds();int hour=seconds/3600,minute=seconds/60%60;
 char text[20];
 if(clock12h)snprintf(text,sizeof(text),"%02d:%02d %s",hour%12?hour%12:12,minute,hour<12?"AM":"PM");
 else snprintf(text,sizeof(text),"%02d:%02d",hour,minute);
 return String(text);
}
void changed(){dirty=true;saveAt=millis()+10000;if(!saveAt)saveAt=1;}
void message(const char* text){notice=text;noticeUntil=millis()+2200;dirty=true;}
void captureVfo(){vfo[activeVfo]={frequencies[band],bfo,band,mode,stepIndex};}
void waitTuneComplete(){
 uint32_t start=millis();
 do {radio.getStatus(0,0);if(radio.getTuneCompleteTriggered()){radio.getStatus(1,0);return;}delay(2);} while(millis()-start<250);
 Serial.println("SI4732: timeout de confirmacion de sintonia");
}
void tuneConfirmed(uint16_t f){radio.setFrequency(f);waitTuneComplete();}
void switchVfo(){
 captureVfo();activeVfo^=1;
 const VfoState &v=vfo[activeVfo];
 band=v.bandId;mode=band==CB_BAND?0:v.modeId;stepIndex=v.stepId;bfo=v.fine;
 frequencies[band]=constrain(v.frequency,bands[band].lo,bands[band].hi);selectBand(false);changed();
 message(activeVfo?"VFO B activo":"VFO A activo");
}
void save(){
 if(!settingsAvailable){saveAt=0;return;}
 captureVfo();
 prefs.putBytes("vfoAB",vfo,sizeof(vfo));prefs.putUChar("vfoSel",activeVfo);
 prefs.putBool("rdsOn",rdsEnabled);prefs.putUChar("lcdSleep",screenSleep);
 prefs.putBool("clock12",clock12h);
 prefs.putBool("autoclock",autoClock);
 prefs.putBool("fmmono",fmForceMono);prefs.putUChar("lang",language);prefs.putUChar("band",band);prefs.putUChar("mode",mode);prefs.putUChar("vol",volume);
 prefs.putBool("afa",amFilterAuto);prefs.putBool("sfa",ssbFilterAuto);prefs.putUChar("step",stepIndex);prefs.putUChar("af",amFilter);prefs.putUChar("sf",ssbFilter);
 prefs.putUChar("fmdeemp",fmDeemphasis);
 prefs.putUChar("softmute",softMuteDb);prefs.putUChar("seekrssi",searchRssi);prefs.putUChar("seeksnr",searchSnr);
 prefs.putUChar("att",attenuation);prefs.putBool("agc",agc);prefs.putUChar("hud",hud);
 prefs.putUChar("sqlmode",squelchMode);prefs.putUChar("sql",squelch);prefs.putBool("afc",afcEnabled);prefs.putBool("amp",ampEnabled);prefs.putUChar("theme",theme);prefs.putUChar("region",region);prefs.putShort("bfo",bfo);prefs.putBytes("freq",frequencies,sizeof(frequencies));saveAt=0;
}
void updateSquelch(uint32_t now){
 // Only use settled, normal reception samples.
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
 else if(rssi>=int(threshold)+3){
  close=false;squelchOpenedAt=now;
 }else if(rssi<threshold&&now-squelchOpenedAt>=350)close=true;
 if(close!=squelchClosed){squelchClosed=close;digitalWrite(MUTE,(muted||close)?HIGH:LOW);dirty=true;}
}
// Carrier AFC remains available only in the previous SSB path.
// Native AM (including CB) never uses the SSB patch or carrier AFC.
bool autoVfoActive(){return ready&&band!=0&&band!=CB_BAND&&mode!=0&&patchLoaded&&afcEnabled;}
void audioSettings(){
 if(!ready)return;
 digitalWrite(AMP,ready&&ampEnabled?HIGH:LOW);
 digitalWrite(MUTE,(muted||squelchClosed)?HIGH:LOW);
 radio.setVolume(muted?0:volume);
 // FM must not inherit manual AM/SSB attenuation from a saved setting.
 radio.setAutomaticGainControl(band==0?0:(agc?0:1),band==0?0:attenuation);
 if(band==0&&originalFmDeemphasis>=0)
  radio.setFMDeEmphasis(fmDeemphasis?fmDeemphasis:uint8_t(originalFmDeemphasis));
 if(band==0&&fmDefaultStereo>=0&&fmDefaultMono>=0){
  radio.setFmBlendStereoThreshold(fmForceMono?127:uint8_t(fmDefaultStereo));
  radio.setFmBlendMonoThreshold(fmForceMono?127:uint8_t(fmDefaultMono));
 }
 if(band!=0){
  if(mode==0){radio.setBandwidth(activeFilter(),1);radio.setAmSoftMuteMaxAttenuation(softMuteDb);
   // Native AM AVC adjusts volume gain continuously inside the receiver.
   // 48 dB is its documented default ceiling, not a forced instantaneous gain.
   radio.setAvcAmMaxGain(48);
  }
  else {
   // SYNC tracks an AM carrier; suppressed-carrier SSB needs manual BFO.
   const bool sync=autoVfoActive();
   radio.setSSBConfig(activeFilter(),1,sync?3:0,1,0,sync?0:1);
   radio.setSSBBfo(sync?0:bfo);
   radio.setSsbSoftMuteMaxAttenuation(softMuteDb);
  }
 }
}
void updateAutoFilter(uint32_t now){
 if(band==0||!((mode)?ssbFilterAuto:amFilterAuto))return;
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
 // Keep the user's AFC preference across band/mode changes.
 // autoVfoActive() gates it to patched LSB/USB, never AM, FM or CB.
 if(band==CB_BAND){mode=0;bfo=0;frequencies[band]=cbFrequencies[cbChannel(frequencies[band])];}
 if(!ready)return;
 if((band==0||mode==0)&&stepIndex>5)stepIndex=2;
 
 const bool wasSquelchClosed=squelchClosed;
 digitalWrite(MUTE,HIGH);digitalWrite(AMP,LOW);
 if(band==CB_BAND)frequencies[band]=cbFrequencies[cbChannel(frequencies[band])];
 const Band &b=bands[band];
 if(band==0){radio.setFM(b.lo,b.hi,frequencies[band],step());patchLoaded=false;
  // Capture chip defaults after FM power-up; never guess restoration values.
  originalFmDeemphasis=radio.getProperty(FM_DEEMPHASIS);
  fmDefaultStereo=radio.getProperty(FM_BLEND_STEREO_THRESHOLD);
  fmDefaultMono=radio.getProperty(FM_BLEND_MONO_THRESHOLD);
  radio.setRdsConfig(1,2,2,2,2);radio.setFifoCount(1);}
 else if(mode==0){radio.setAM(b.lo,b.hi,frequencies[band],step());patchLoaded=false;
  // audioSettings restores the automatic AVC default after every AM tune.
 }
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
 waitTuneComplete();resetRds(true);squelchClosed=wasSquelchClosed;
 // Hold the hardware clamp through chip audio restoration.
 radio.setVolume(0);delay(50);audioSettings();changed();
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
  if(radio.getCurrentRSSI()>=max(int(searchRssi),squelchMode==1?int(squelch):0)&&radio.getCurrentSNR()>=searchSnr){
   cancelSearch();resetRds();changed();message("Senal encontrada");return;
  }
 }
 if(searchRemaining==0){
  cancelSearch();frequencies[band]=searchOriginal;tuneConfirmed(searchOriginal);
  resetRds();message("Sin senal / frecuencia restaurada");return;
 }
 int f=band==CB_BAND?cbFrequencies[(cbChannel(frequencies[band])+1)%40]:frequencies[band]+step();
 if(f>bands[band].hi)f=bands[band].lo;
 frequencies[band]=f;tuneConfirmed(f);searchRemaining--;searchAt=millis()+150;dirty=true;
}
void setAudioTimer(int minutes){
 audioTimerMinutes=constrain(minutes,0,120);
 audioTimerAt=audioTimerMinutes?millis()+uint32_t(audioTimerMinutes)*60000UL:0;
 dirty=true;
}
String value(int n){
 switch(n){
 case 34:{int hour=localClockSeconds()/3600;return String(clock12h?(hour%12?hour%12:12):hour)+(clock12h?(hour<12?" AM":" PM"):" h")+(clockSet?"":" / ajustar");}
 case 35:{char text[8];snprintf(text,sizeof(text),"%02u",unsigned(localClockSeconds()/60%60));return String(text);}
 case 36:return !clock12h?"Solo formato 12 h":localClockSeconds()/3600<12?"AM / madrugada-manana":"PM / tarde-noche";
 case 37:return clock12h?"12 horas / AM-PM":"24 horas";
 case 38:return zoneName.length()?zoneName+(zoneDst?" / verano ON":" / verano OFF")+(internetClock?" / NTP":" / pendiente"):netClockStatus;
 case 39:return timeQueryRunning?"Consultando Internet...":"Pulsar: sincronizar por WiFi";
 case 40:return autoClock?"Activado / hora por WiFi":"Desactivado / ajuste manual";
 case 32:return fmDeemphasis==0?"Original / sin cambio":fmDeemphasis==1?"50 us / Europa-Japon-Australia":"75 us / Estados Unidos";
 case 44:return activeVfo?"B / pulsar para A":"A / pulsar para B";
 case 45:return directEntry?String(directHz)+" Hz / digito "+String(directDigit+1):"Pulsar: editar 8 digitos Hz";
 case 46:return rdsEnabled?"Activado":"Desactivado";
 case 47:return screenSleep?String(screenSleep*30)+" segundos":"Siempre encendida";
 case 33:return band==0||mode!=0?"Solo AM":"Automatica";
 case 29:return band==0?"Solo AM/SSB":softMuteDb?String(softMuteDb)+" dB / atenuacion suave":"Desactivado / senal debil";
 case 30:return String(searchRssi)+" dBuV / minimo";
 case 31:return String(searchSnr)+" dB / minimo";
 case 25:return signalSearch?"Buscando / pulsar cancela":"Pulsar: buscar AM / FM";
 case 28:return fmForceMono?"Mono / menos siseo":"Automatico / estereo";
 case 26:{
  if(!audioTimerAt)return "Desactivado";
  int32_t remaining=int32_t(audioTimerAt-millis());
  return String(remaining>0?(uint32_t(remaining)+59999)/60000:0)+" min / silenciar";
 }
 case 24:return languageNames[language];
 case 23:return "Radio / Firmware / Version";
 case 21:return squelchMode==0?"Desactivado":squelchMode==1?"Manual / AM-FM":"Automatico / AM-FM";
 case 22:return squelchMode==2?String(autoSquelchThreshold)+" dBuV / auto":String(squelch)+" dBuV / manual";
 case 0:return bands[band].name;
 case 1:return band==CB_BAND?"AM / fijo":band==0?"FM":String("Manual / ")+modes[mode];
 case 2:if(band==CB_BAND)return "1 canal (1-40)";if(fineTuning())return String(fineStepHz())+" Hz / BFO";return String(band==0?step()*10:step())+" kHz";
 case 3:return String(volume)+"/63";
 case 4:return band==0?"Solo AM/SSB":String(((mode)?ssbFilterAuto:amFilterAuto)?"Auto / ":"Manual / ")+String((mode)?ssbFilters[ssbFilterAuto?autoSsbFilter:ssbFilter]:amFilters[activeFilter()])+" kHz";
 case 5:return agc?"SI":"NO";
 case 6:return agc?"Auto":String(attenuation);
 case 7:return band&&mode?(autoVfoActive()?"SYNC / girar: BFO manual":String(bfo)+" Hz"):"Solo SSB";
 case 8:return hudNames[hud];
 case 14:return themeNames[theme];
 case 15:return regionNames[region];
 case 16:return ampEnabled?"Encendido / ajustar":"Apagado / ajustar";
 case 17:if(band==0||band==CB_BAND||mode==0)return afcEnabled?"Guardado / solo LSB-USB":"Solo LSB-USB";return !afcEnabled?"NO / BFO manual":autoVfoActive()?"SYNC / sin confirmar bloqueo":"Guardado / no activo aqui";
 case 9:return muted?"SI":"NO";
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
enum MenuIcon { ANTENNA, AUDIO, MEMORY, SCREEN, GEAR, BACK, FILTER,
                STEP, ICON_MUTE, SAVE, LOAD, INFO, CLOCK, LANGUAGE, SEARCH, THEME,
                WIFI_ICON, WEATHER_ICON, SOLAR_ICON, REGION_ICON, VFO_ICON, RDS_ICON };
MenuIcon itemIcon(int id){
 // One entry per stable setting ID, including unused legacy slots.
 static const MenuIcon icons[]={
  ANTENNA, FILTER, STEP, AUDIO, FILTER, GEAR, FILTER, STEP,
  SCREEN, ICON_MUTE, MEMORY, SAVE, LOAD, BACK, THEME, REGION_ICON,
  AUDIO, VFO_ICON, GEAR, GEAR, GEAR, FILTER, FILTER, INFO,
  LANGUAGE, SEARCH, CLOCK, GEAR, AUDIO, ICON_MUTE, SEARCH, SEARCH,
  AUDIO, FILTER, CLOCK, CLOCK, CLOCK, CLOCK, WIFI_ICON, CLOCK,
  CLOCK, WIFI_ICON, WEATHER_ICON, SOLAR_ICON, VFO_ICON, STEP, RDS_ICON, SCREEN
 };
 static_assert(sizeof(icons)/sizeof(icons[0])==ITEM_COUNT,"Un icono por ajuste");
 return id>=0&&id<ITEM_COUNT?icons[id]:GEAR;
}
// Time-based gentle breathing: no catch-up frames or extra buffers.
uint8_t animationFrame=0;
uint32_t lastAnimation=0;
// Interpolate only the graphics toward measured RF values. Raw labels remain exact.
uint16_t iconPulse(uint16_t color);
void animateSelection(int y,uint16_t color){
 // Decorative travelling highlight confined to the card border, never over text.
 int x=18+(animationFrame*9)%270;
 ui.drawFastHLine(x,y+30,12,iconPulse(color));
 // A mirrored glint and breathing corner brackets stay clear of the labels.
 ui.drawFastHLine(306-x,y+1,8,iconPulse(color));
 ui.drawFastVLine(5,y+8,6+abs(16-int(animationFrame))/2,iconPulse(color));
}
constexpr uint8_t pulseLevels[32]={7,6,6,6,5,5,4,4,3,2,2,1,1,0,0,0,0,0,0,0,1,1,2,2,3,4,4,5,5,6,6,6};
uint16_t visualBlend(uint16_t a,uint16_t b,int mix){
 mix=constrain(mix,0,31);int inv=31-mix;
 return (((((a>>11)&31)*inv+((b>>11)&31)*mix)/31)<<11)|
        (((((a>>5)&63)*inv+((b>>5)&63)*mix)/31)<<5)|
        (((a&31)*inv+(b&31)*mix)/31);
}
uint16_t iconPulse(uint16_t color){
 int scale=24+pulseLevels[animationFrame];
 return visualBlend(0,color,scale);
}
void menuIcon(int x,int y,MenuIcon kind,uint16_t c){
 // Animate only the selected entry; keep every symbol inside its 16px cell.
 const bool moving=menu&&c==palettes[theme].accent;
 const int phase=moving?animationFrame/4:0;
 const int swing=moving?(phase<4?phase:7-phase)-1:0;
 if(theme==RAINBOW_THEME)c=rainbowColor(int(kind));
 else if(menu&&c==palettes[theme].accent)c=iconPulse(c);
 auto line=[&](int a,int b,int d,int e){ui.drawLine(x+a,y+b,x+d,y+e,c);};
 switch(kind){
 case ANTENNA: line(8,2,8,14);line(3,3,8,8);line(13,3,8,8);line(4,14,12,14);
  if(moving){int r=2+phase%3;line(5-r,2,4-r,5);line(11+r,2,12+r,5);}break;
 case WIFI_ICON:
  line(1,5,4,3);line(4,3,12,3);line(12,3,15,5);
  line(4,8,6,6);line(6,6,10,6);line(10,6,12,8);
  line(6,10,8,8);line(8,8,10,10);ui.fillCircle(x+8,y+13,1,c);break;
 case WEATHER_ICON:
  ui.drawCircle(x+5,y+5,3,c);line(5,0,5,1);line(0,5,1,5);line(1,1,2,2);
  ui.drawRoundRect(x+3,y+7,12,6,3,c);ui.drawCircle(x+9,y+7,3,c);
  line(5,14,4,15);line(10,14,9,15);break;
 case SOLAR_ICON:
  ui.drawCircle(x+8,y+8,3,c);
  for(int i=0;i<8;i++){float a=i*0.78539816f;
   line(8+int(roundf(cosf(a)*5)),8+int(roundf(sinf(a)*5)),
        8+int(roundf(cosf(a)*7)),8+int(roundf(sinf(a)*7)));}break;
 case REGION_ICON:
  ui.drawCircle(x+8,y+6,4,c);line(4,8,8,15);line(12,8,8,15);
  ui.fillCircle(x+8,y+6,1,c);break;
 case VFO_ICON:
  line(1,4,14,4);line(14,4,11,1);line(14,4,11,7);
  line(14,12,1,12);line(1,12,4,9);line(1,12,4,15);break;
 case RDS_ICON:
  ui.drawRoundRect(x+1,y+3,14,10,2,c);
  line(4,6,12,6);line(4,9,9,9);break;
 case AUDIO: case ICON_MUTE:
  ui.drawRect(x+1,y+6,4,5,c);line(5,6,9,3);line(9,3,9,13);line(9,13,5,10);
  if(kind==ICON_MUTE){line(11,6,15,10);line(15,6,11,10);}
  else{int r=moving?phase%3:1;line(11+r,5,13+r,7);line(13+r,7,13+r,9);line(13+r,9,11+r,11);}break;
 case MEMORY: case SAVE: case LOAD:
  ui.drawRoundRect(x+2,y+2,12,12,2,c);ui.drawRect(x+5,y+3,6,3,c);
  if(kind==MEMORY)ui.drawRect(x+5,y+9,6,4,c);
  else{int a=kind==SAVE?8:12,b=kind==SAVE?12:8;line(8,a,8,b);line(5,b+(kind==SAVE?-3:3),8,b);line(11,b+(kind==SAVE?-3:3),8,b);}break;
 case SCREEN:ui.drawRect(x+1,y+2,14,10,c);line(8,12,8,15);line(4,15,12,15);
  if(moving)line(3,4+phase%6,12,4+phase%6);break;
 case BACK:{int shift=moving?phase%3:0;line(2+shift,8,13+shift,8);line(2+shift,8,7+shift,3);line(2+shift,8,7+shift,13);break;}
 case FILTER:for(int i=0;i<3;i++){line(3+i*5,2,3+i*5,14);ui.drawRect(x+1+i*5,y+constrain(4+i*3+(i==1?-swing:swing),3,11),5,3,c);}break;
 case STEP:line(1,13,5,13);line(5,13,5,9);line(5,9,10,9);line(10,9,10,4);line(10,4,15,4);if(moving)ui.fillCircle(x+3+phase%3*5,y+11-phase%3*4,1,c);break;
 case INFO:ui.drawCircle(x+8,y+8,7,c);ui.fillCircle(x+8,y+4,1,c);line(8,7,8,12);break;
 case CLOCK:{ui.drawCircle(x+8,y+8,7,c);line(8,8,12,10);
  float a=moving?phase*0.78539816f-1.57079633f:-1.57079633f;
  line(8,8,8+int(roundf(cosf(a)*5)),8+int(roundf(sinf(a)*5)));break;}
 case LANGUAGE:ui.drawCircle(x+8,y+8,7,c);ui.drawEllipse(x+8,y+8,moving?2+phase%4:3,7,c);line(1,8,15,8);break;
 case SEARCH:ui.drawCircle(x+6,y+6,5,c);line(10,10,15,15);if(moving){float a=phase*0.78539816f;line(6,6,6+int(cosf(a)*4),6+int(sinf(a)*4));}break;
 case THEME:ui.drawCircle(x+8,y+8,7,c);ui.fillCircle(x+5,y+5,1,c);ui.fillCircle(x+11,y+5,1,c);ui.fillCircle(x+4,y+10,1,c);ui.fillCircle(x+10,y+11,moving?1+phase%2:2,c);break;
 default:ui.drawCircle(x+8,y+8,4,c);
  for(int i=0;i<8;i++){float a=i*0.78539816f+(moving?phase*0.09817477f:0);
   line(8+int(roundf(cosf(a)*5)),8+int(roundf(sinf(a)*5)),
        8+int(roundf(cosf(a)*7)),8+int(roundf(sinf(a)*7)));}break;
 }
}
// Text must remain readable on both the background and the instrument panels.
// Keep the requested hue where possible; only text is adjusted, never graphics.
bool themedText=false;
float textLuminance(uint16_t color){
 // RGB565 has only 32 red/blue and 64 green levels. Compute the exact
 // existing transfer function once, not three powf calls per candidate
 // during every text contrast search. Fixed storage: 384 bytes, no heap.
 struct LinearLevels {
  float rb[32],g[64];
  LinearLevels(){
   auto linear=[](float channel){return channel<=0.04045f?channel/12.92f:powf((channel+0.055f)/1.055f,2.4f);};
   for(int i=0;i<32;i++)rb[i]=linear(i/31.0f);
   for(int i=0;i<64;i++)g[i]=linear(i/63.0f);
  }
 };
 static const LinearLevels levels;
 return 0.2126f*levels.rb[(color>>11)&31]
       +0.7152f*levels.g[(color>>5)&63]
       +0.0722f*levels.rb[color&31];
}
uint16_t readableText(uint16_t color){
 if(!themedText)return color; // The welcome screen has its own fixed palette.
 const Palette &p=palettes[theme];
 float background=textLuminance(p.background),panel=textLuminance(p.panel);
 auto contrast=[&](uint16_t candidate){
  float light=textLuminance(candidate);
  float a=(max(light,background)+0.05f)/(min(light,background)+0.05f);
  float b=(max(light,panel)+0.05f)/(min(light,panel)+0.05f);
  return min(a,b);
 };
 if(contrast(color)>=4.5f)return color;
 uint16_t endpoint=contrast(0xFFFF)>contrast(0)?0xFFFF:0;
 // Find the smallest blend that reaches readable small-text contrast.
 int low=1,high=31;
 while(low<high){
  int middle=(low+high)/2;
  if(contrast(visualBlend(color,endpoint,middle))>=4.5f)high=middle;
  else low=middle+1;
 }
 return visualBlend(color,endpoint,low);
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
 ui.setTextColor(readableText(color));
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
 themedText=false;
 const uint16_t bg=0x0843,dim=0x2949,cyan=0x07FF,violet=0xB81F,white=0xFFFF;
 const uint32_t frame=elapsed/33;
 const float phase=elapsed*0.0035f;
 ui.fillScreen(bg);
 // Deterministic star field: no images, random allocations or extra buffers.
 for(int i=0;i<28;i++){
  int x=10+(i*73+int(frame)*(1+i%3))%300,y=9+(i*47)%148;
  uint16_t color=((frame/5+i)%7==0)?white:dim;
  ui.drawPixel(x,y,color);
 }
 // Moving perspective grid in the margins, clear of all title cells.
 for(int k=0;k<7;k++){
  int y=132+(k*5+int(frame)%5)%30;
  ui.drawFastHLine(10,y,24,visualBlend(bg,cyan,8));
  ui.drawFastHLine(286,y,24,visualBlend(bg,violet,8));
 }
 for(int k=0;k<4;k++){
  int r=8+(int(frame)+k*12)%48;
  uint16_t c=visualBlend(bg,cyan,(48-r)/3);
  ui.drawEllipse(160,38,r,max(3,r/2),c);
 }
 // Twin particle streams stay outside the text area.
 for(int k=0;k<12;k++){
  float a=phase*0.7f+k*0.5235988f;
  int px=160+int(cosf(a)*145),py=36+int(sinf(a)*22);
  ui.fillCircle(px,py,k%4==0?2:1,visualBlend(dim,k%2?cyan:violet,18));
 }
 ui.drawRoundRect(5,5,310,160,9,dim);
 ui.drawFastHLine(22,7,58,cyan);ui.drawFastHLine(240,162,58,violet);
 // Antenna with radiating waves and a softly pulsing halo.
 ui.drawCircle(160,38,26,dim);
 ui.drawCircle(160,38,28,visualBlend(dim,violet,int(15+15*sinf(phase))));
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
 // Persistent three-boot cycle: first welcome abducts, then two fly-bys.
 // Everything stays in the free space beside the antenna, with no extra delay.
 if(elapsed>=700UL && elapsed<=5200UL){
  int x=-18+int((elapsed-700UL)*356UL/4500UL);
  int y=15+int(2.0f*sinf((elapsed-700UL)*0.004f));
  if(startupAbductCow){
   if(elapsed<2100UL)x=-18+int((elapsed-700UL)*130UL/1400UL);
   else if(elapsed<3900UL)x=112;
   else x=112+int((elapsed-3900UL)*226UL/1300UL);
   y=15;
   // Narrow tractor beam: never covers titles or the central antenna.
   if(elapsed>=2100UL && elapsed<3750UL){
    uint16_t beam=visualBlend(bg,cyan,25+int(8*sinf(phase)));
    ui.fillTriangle(112,22,101,49,123,49,beam);
    ui.drawLine(112,22,101,49,cyan);
    ui.drawLine(112,22,123,49,cyan);
   }
   if(elapsed<3600UL){
    int lift=elapsed>2400UL?int((elapsed-2400UL)*27UL/1200UL):0;
    int cy=42-lift;
    // Small white cow: dark patches, muzzle, horns, legs and tail.
    ui.fillRoundRect(105,cy,13,7,2,white);
    ui.fillRect(108,cy+1,3,3,dim);
    ui.fillRect(114,cy+3,3,3,dim);
    ui.fillRect(107,cy+7,2,3,white);
    ui.fillRect(115,cy+7,2,3,white);
    ui.fillRect(117,cy-2,5,6,white);
    ui.fillRect(119,cy+2,4,2,0xFBB7);
    ui.drawPixel(120,cy-1,dim);
    ui.drawPixel(117,cy-3,white);ui.drawPixel(121,cy-3,white);
    ui.drawLine(104,cy+2,102,cy-1,white);
   }
  }
  ui.drawFastHLine(x-22,y,7,dim);
  ui.drawFastHLine(x-18,y+2,5,violet);
  ui.fillCircle(x,y-4,5,cyan);
  ui.fillCircle(x-1,y-5,2,white);
  ui.fillRoundRect(x-12,y-2,25,6,3,0xBDF7);
  ui.drawFastHLine(x-9,y-2,19,white);
  ui.drawFastHLine(x-7,y+4,15,violet);
  for(int lamp=0;lamp<3;lamp++)
   ui.fillCircle(x-7+lamp*7,y+1,1,((frame/3+lamp)%3==0)?cyan:violet);
 }
 // Symmetrical animated mini equalizers, explicitly decorative.
 for(int i=0;i<7;i++){
  int h=5+int(10.0f*(1.0f+sinf(phase+i*0.65f)));
  ui.fillRect(46+i*7,54-h,4,h,i%2?cyan:violet);
  ui.fillRect(228+i*7,54-h,4,h,i%2?violet:cyan);
 }
 // Slide-in title with a restrained neon shadow, settled after 900 ms.
 int titleY=63+int((900-min(elapsed,900UL))*12/900);
 centered(titleY+1,"ATS MINI",visualBlend(bg,violet,16),4);
 centered(titleY,"ATS MINI",white,4);
 // Symmetric reveal brackets never cross the title.
 int reveal=int(min(elapsed,1200UL)*22/1200UL);
 ui.drawFastVLine(12,69,reveal,cyan);
 ui.drawFastVLine(307,69,reveal,violet);
 centered(99,"KALIMAN  /  KO6EAI",cyan);
 centered(114,"FM / AM / HF / SSB",white);
 centered(129,"Iniciando radio...",0xBDF7);
 // Presentation timeline, not a claimed hardware initialization percentage.
 ui.drawRoundRect(40,144,240,7,3,dim);
 int fill=int(min(elapsed,6000UL)*234UL/6000UL);
 if(fill>0)ui.fillRect(43,146,fill,3,cyan);
 // Orbiting light along the bottom border.
 int comet=22+int(elapsed*100UL/1000UL)%276;
 ui.drawFastHLine(max(22,comet-12),157,min(13,comet-21),violet);
 ui.fillCircle(comet,157,1,white);
 ui.pushSprite(0,0);
}
// Lightweight simulated depth: no buffers, animation or receiver changes.
// Light comes from above-left; inset screens reverse the bevel.
void reliefFrame(int x,int y,int w,int h,uint16_t surface,bool inset=false){
 uint16_t light=visualBlend(surface,0xFFFF,11);
 uint16_t shadow=visualBlend(surface,0x0000,15);
 uint16_t top=inset?shadow:light,bottom=inset?light:shadow;
 ui.drawRoundRect(x,y,w,h,4,visualBlend(surface,0x0000,9));
 ui.drawFastHLine(x+4,y+1,w-8,top);
 ui.drawFastVLine(x+1,y+4,h-8,top);
 ui.drawFastHLine(x+4,y+h-2,w-8,bottom);
 ui.drawFastVLine(x+w-2,y+4,h-8,bottom);
}
void reliefFill(int x,int y,int w,int h,uint16_t color){
 if(w<1||h<1)return;
 ui.fillRect(x,y,w,h,color);
 ui.drawFastHLine(x,y,w,visualBlend(color,0xFFFF,10));
 if(h>2)ui.drawFastHLine(x,y+h-1,w,visualBlend(color,0x0000,12));
}
// Precision RF instruments: all geometry stays inside each 148 x 49 card.
// The thermometer is an RF scale, never a temperature measurement.
bool refinedHud(int id){
 return id!=29; // All selectable instruments share a consistent card layout.
}
void drawRefinedHud(int id,int x,int n,int reading,int level,uint16_t bg,uint16_t panel,uint16_t ink,uint16_t white,uint16_t dim){
 uint16_t track=visualBlend(panel,dim,8),soft=visualBlend(panel,ink,5);
 reliefFrame(x,79,148,49,panel);
 if(id!=5){
  ui.drawFastHLine(x+10,98,128,track);
  ui.drawFastVLine(x+4,85,8,ink);
 }
 textCell(x+9,84,76,n?"SNR / dB":"RSSI / dBuV",dim,1,false);
 textCell(x+91,82,47,String(reading),white,2,true);
 int l=x+10,r=x+137,y=113;
 if(id==1){
  // Linear analog instrument, with calibrated absolute endpoints.
  ui.drawFastHLine(l,112,128,track);
  for(int k=0;k<=16;k++)ui.drawFastVLine(l+k*7,103,k%4?4:7,k%4?track:dim);
  int px=l+level*127/100;
  ui.drawFastVLine(px,102,15,ink);
  ui.fillTriangle(px-3,118,px+3,118,px,113,white);
  textCell(l,120,28,"0",dim,1,false);
  textCell(r-28,120,28,n?"50":"80",dim,1,true);
 }else if(id==28){
  // Ordered matrix: forty cells, each representing 2.5% of scale.
  for(int k=0;k<40;k++){
   int px=l+(k%20)*6,py=104+(k/20)*10;
   bool active=level>k*100/40;
   reliefFill(px,py,4,7,active?ink:track);
   if(active)ui.drawPixel(px,py,white);
  }
 }else if(id==30||id==35){
  const int cx=x+40,cy=113;
  const float rad=3.14159265f/180;
  int rx=id==35?27:23,ry=id==35?9:10;
  ui.drawEllipse(cx,cy+1,rx+1,ry+1,visualBlend(panel,0,16));
  ui.drawEllipse(cx,cy-1,rx+1,ry+1,visualBlend(panel,0xFFFF,11));
  ui.drawEllipse(cx,cy,rx,ry,track);
  if(id==35)ui.drawEllipse(cx,cy,rx-5,ry-3,soft);
  for(int k=0;k<=20;k++){
   float a=(180+k*9)*rad;
   uint16_t c=k*5<level?ink:track;
   ui.drawLine(cx+int(cosf(a)*rx),cy+int(sinf(a)*ry),
               cx+int(cosf(a)*(rx+2)),cy+int(sinf(a)*(ry+2)),c);
  }
  float a=(180+level*1.8f)*rad;
  int px=cx+int(cosf(a)*rx),py=cy+int(sinf(a)*ry);
  ui.fillCircle(px,py,2,white);
  ui.drawFastHLine(cx-4,cy,9,dim);
  textCell(x+78,102,60,String(level)+"%",ink,2);
  textCell(x+78,119,60,"ESCALA",dim,1);
 }else if(id==32){
  // Continuous ribbon with fine, quiet scale marks.
  ui.fillRoundRect(l,104,128,10,3,bg);
  int w=level*126/100;
  reliefFrame(l,103,128,12,panel,true);
  if(w)reliefFill(l+1,106,w,6,ink);
  for(int k=0;k<=8;k++)ui.drawFastVLine(l+k*15,118,k%2?3:5,dim);
  int px=l+level*126/100;
  ui.drawFastVLine(px,103,12,white);
 }else if(id==36){
  ui.drawFastHLine(l,115,128,track);
  for(int k=0;k<=20;k++)ui.drawFastVLine(l+k*6,106,k%5?5:9,k%5?track:dim);
  int px=l+level*120/100;
  ui.fillTriangle(px-3,101,px+3,101,px,106,ink);
  textCell(l,120,28,"0",dim,1,false);
  textCell(r-28,120,28,n?"50":"80",dim,1,true);
 }else if(id==31){
  // Glass tube and reservoir, with an absolute RF level scale.
  ui.drawRoundRect(l+9,101,118,12,5,dim);
  ui.fillRoundRect(l+12,104,112,6,3,bg);
  int w=level*110/100;
  if(w)ui.fillRoundRect(l+13,105,w,4,2,ink);
  ui.fillCircle(l+7,107,7,track);ui.fillCircle(l+7,107,5,ink);
  ui.fillCircle(l+6,105,1,white);
  for(int k=0;k<=4;k++)ui.drawFastVLine(l+16+k*26,117,k%2?3:5,dim);
 }else if(id==33){
  // Eight cut gemstones, with facets rather than wire-only diamonds.
  for(int k=0;k<8;k++){
   int cx=l+7+k*16;uint16_t c=level>k*100/8?ink:track;
   ui.fillTriangle(cx-6,111,cx,103,cx+6,111,c);
   ui.fillTriangle(cx-6,111,cx,120,cx+6,111,visualBlend(panel,c,23));
   ui.drawLine(cx-4,110,cx,105,level>k*100/8?white:dim);
   ui.drawLine(cx-6,111,cx+6,111,panel);
  }
 }else if(id==34){
  // RF horizon: fixed grid, filled elevation, no artificial attitude.
  for(int k=0;k<3;k++)ui.drawFastHLine(l,103+k*9,128,track);
  for(int k=0;k<9;k++)ui.drawFastVLine(l+k*16,102,21,track);
  int h=level*19/100,hy=122-h;
  for(int row=hy+1;row<=122;row++)ui.drawFastHLine(l+1,row,126,soft);
  ui.drawFastHLine(l,hy,128,ink);
  ui.fillTriangle(x+70,hy-2,x+78,hy-2,x+74,hy+2,white);
 }else if(id==37){
  // Mirrored level meter, not a claim of measured RF balance.
  int cx=x+74;
  for(int k=0;k<9;k++){
   uint16_t c=level>k*100/9?ink:track;
   reliefFill(cx-8-k*7,105,5,13,c);
   reliefFill(cx+4+k*7,105,5,13,c);
  }
  ui.drawFastVLine(cx,101,21,white);
  ui.drawFastHLine(l,123,128,track);
 }else if(id==26){
  // Two shallow precision dials with ticks and a real level needle.
  int cx=x+74,cy=125;
  for(int k=0;k<=24;k++){
   float a=(205+k*130.0f/24)*3.14159265f/180;
   int rr=k%4?55:59;
   ui.drawLine(cx+int(cosf(a)*rr),cy+int(sinf(a)*rr*.40f),cx+int(cosf(a)*63),cy+int(sinf(a)*25),k*100/24<=level?ink:track);
  }
  float a=(205+level*1.3f)*3.14159265f/180;
  ui.drawFastHLine(x+24,126,100,visualBlend(panel,0xFFFF,8));
  ui.drawLine(cx+1,cy-1,cx+1+int(cosf(a)*54),cy+1+int(sinf(a)*21),visualBlend(panel,0,18));
  ui.drawLine(cx,cy-2,cx+int(cosf(a)*54),cy+int(sinf(a)*21),white);
  ui.fillCircle(cx,cy-2,2,ink);
 }else if(id==27){
  // Faceted prism ribbon, brightness follows the measured level.
  for(int k=0;k<10;k++){
   int px=l+k*13;uint16_t c=level>k*10?ink:track;
   ui.fillTriangle(px,120,px+6,102,px+12,120,c);
   ui.fillTriangle(px+6,102,px+6,120,px+12,120,visualBlend(panel,c,20));
   ui.drawLine(px+2,117,px+6,106,level>k*10?white:dim);
  }
 }else if(id==3){
  // Uniform broadcast-console segments, no misleading spectrum bars.
  for(int k=0;k<24;k++){
   uint16_t c=level>k*100/24?visualBlend(ink,white,k/3):track;
   reliefFill(l+k*5,103,3,14,c);
  }
  for(int k=0;k<=4;k++)ui.drawFastVLine(l+k*30,120,3,dim);
 }else if(id==2){
  // Digital meter: precise reading above a fine graduated rail.
  for(int k=0;k<32;k++)reliefFill(l+k*4,107,2,7,level>k*100/32?ink:track);
  ui.drawFastHLine(l,119,128,dim);
  int px=l+level*127/100;
  ui.fillTriangle(px-2,122,px+2,122,px,118,white);
 }else if(id==5){
  ui.drawFastHLine(l,115,128,track);
  int w=level*127/100;
  if(w)ui.drawFastHLine(l,115,w,ink);
  ui.fillCircle(l+w,115,2,white);
  textCell(l,120,20,"0",dim,1,false);
  textCell(r-22,120,22,n?"50":"80",dim,1,true);
 }else{
  // Studio: recessed rail, major divisions and crisp live cursor.
  ui.fillRoundRect(l,104,128,10,4,bg);
  int w=level*124/100;
  if(w)ui.fillRoundRect(l+2,106,w,6,2,ink);
  for(int k=1;k<8;k++)ui.drawFastVLine(l+k*16,105,8,panel);
  for(int k=0;k<=8;k++)ui.drawFastVLine(l+k*16,118,k%2?3:5,dim);
  ui.drawFastVLine(l+2+level*123/100,103,12,white);
 }
}
int wrap(int v,int count);
// Station-only WiFi; save credentials only after successful connection.
bool wifiMenu=false;
enum WifiPage { WIFI_HOME, WIFI_SCAN, WIFI_NETWORKS, WIFI_KEYBOARD, WIFI_CONNECTING };
WifiPage wifiPage=WIFI_HOME;
int wifiChoice=0,wifiCount=0,wifiKey=0,wifiKeyPage=0;
String wifiSsid,wifiPassword,wifiStatus="Apagado";
uint32_t wifiStarted=0,wifiRetryAt=0;
String wifiPendingSsid,wifiPendingKey;
bool wifiAutoEnabled=false;
void wifiBeginSaved(){
 if(!settingsAvailable||!wifiAutoEnabled)return;
 String ssid=prefs.getString("wifissid","");
 String key=prefs.getString("wifikey","");
 if(!ssid.length()){wifiStatus="Selecciona y guarda una red";return;}
 WiFi.persistent(false);WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);
 WiFi.begin(ssid.c_str(),key.c_str());wifiRetryAt=millis();
 wifiStatus="Reconectando red guardada";dirty=true;
 Serial.println("WiFi: intento con red guardada (sin mostrar clave)");
}
const char* wifiKeys[]={"abcdefghijklmnopqrstuvwxyz", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "0123456789 !@#$%^&*()-_=+[]{};:,.?/~\\|<>\"'`"};
void wifiScan(){
 WiFi.persistent(false);WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);
 WiFi.scanDelete();wifiCount=0;wifiChoice=0;
 int result=WiFi.scanNetworks(true);
 wifiPage=WIFI_SCAN;wifiStarted=millis();wifiStatus="Buscando redes...";
 if(result==WIFI_SCAN_FAILED){wifiPage=WIFI_HOME;wifiStatus="No se pudo buscar";}
 dirty=true;
}
void wifiTick(){
 if(wifiPage==WIFI_SCAN){
  int n=WiFi.scanComplete();
  if(n>=0){wifiCount=n;wifiChoice=0;wifiPage=WIFI_NETWORKS;wifiStatus=n?"Selecciona una red":"No hay redes visibles";dirty=true;}
  else if(n==WIFI_SCAN_FAILED||millis()-wifiStarted>20000){WiFi.scanDelete();wifiPage=WIFI_HOME;wifiStatus="Busqueda fallida";dirty=true;}
 }
 if(wifiPage==WIFI_CONNECTING){
  if(WiFi.status()==WL_CONNECTED){
   bool saved=false;
   if(settingsAvailable){
    prefs.putBool("wifion",false);
    prefs.putString("wifissid",wifiPendingSsid);prefs.putString("wifikey",wifiPendingKey);
    saved=prefs.getString("wifissid","")==wifiPendingSsid && prefs.getString("wifikey","")==wifiPendingKey;
    if(saved)saved=prefs.putBool("wifion",true)==1;
   }
   wifiAutoEnabled=saved;wifiPendingKey="";wifiPassword="";
   wifiPage=WIFI_HOME;wifiChoice=0;
   wifiStatus=saved?"Conectado / red guardada":"Conectado / ERROR al guardar";
   Serial.println(saved?"WiFi: credenciales guardadas; autoconexion activa":"WiFi: ERROR al guardar credenciales");dirty=true;
  }
  else if(millis()-wifiStarted>20000){WiFi.disconnect();wifiPendingKey="";wifiPassword="";wifiRetryAt=millis();wifiPage=WIFI_HOME;wifiStatus="Fallo: clave o senal / reintenta";dirty=true;}
 }
 static int previous=-1;
 int status=WiFi.status();if(status!=previous){previous=status;Serial.printf("WiFi: estado %d\n",status);if(status==WL_CONNECTED){configTime(0,0,"pool.ntp.org","time.nist.gov");nextTimeQuery=0;wifiStatus="Conectado: "+WiFi.localIP().toString();}dirty=true;}
 if(status!=WL_CONNECTED && wifiPage==WIFI_HOME && wifiAutoEnabled && millis()-wifiRetryAt>30000){wifiBeginSaved();}
}
void wifiConnect(){
 WiFi.persistent(false);WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);
 wifiPendingSsid=wifiSsid;wifiPendingKey=wifiPassword;
 WiFi.begin(wifiPendingSsid.c_str(),wifiPendingKey.c_str());wifiPage=WIFI_CONNECTING;
 wifiStarted=millis();wifiStatus="Conectando...";dirty=true;
}
void wifiTurn(int d){
 int count=wifiPage==WIFI_HOME?3:wifiPage==WIFI_NETWORKS?wifiCount+1:wifiPage==WIFI_KEYBOARD?strlen(wifiKeys[wifiKeyPage])+4:1;
 if(wifiPage==WIFI_KEYBOARD)wifiKey=wrap(wifiKey+d,count);else wifiChoice=wrap(wifiChoice+d,count);
 dirty=true;
}
void wifiClick(){
 if(wifiPage==WIFI_HOME){
  if(wifiChoice==0)wifiScan();
  else if(wifiChoice==1){WiFi.scanDelete();WiFi.disconnect(true);WiFi.mode(WIFI_OFF);wifiPassword="";wifiStatus="Apagado";wifiAutoEnabled=false;wifiPendingKey="";if(settingsAvailable)prefs.putBool("wifion",false);}
  else wifiMenu=false;
 }else if(wifiPage==WIFI_NETWORKS){
  if(wifiChoice==wifiCount){wifiPage=WIFI_HOME;wifiChoice=0;}
  else{
   wifiSsid=WiFi.SSID(wifiChoice);
   if(!wifiSsid.length()){wifiStatus="Red oculta no compatible";wifiPage=WIFI_HOME;wifiChoice=0;}
   else if(WiFi.encryptionType(wifiChoice)==WIFI_AUTH_OPEN){wifiPassword="";wifiConnect();}
   else{wifiPassword="";wifiKey=0;wifiKeyPage=0;wifiPage=WIFI_KEYBOARD;}
  }
 }else if(wifiPage==WIFI_KEYBOARD){
  int n=strlen(wifiKeys[wifiKeyPage]);
  if(wifiKey<n){if(wifiPassword.length()<63)wifiPassword+=wifiKeys[wifiKeyPage][wifiKey];}
  else if(wifiKey==n){wifiKeyPage=(wifiKeyPage+1)%3;wifiKey=0;}
  else if(wifiKey==n+1){if(wifiPassword.length())wifiPassword.remove(wifiPassword.length()-1);}
  else if(wifiKey==n+2)wifiConnect();
  else{wifiPassword="";wifiPage=WIFI_NETWORKS;}
 }else if(wifiPage==WIFI_CONNECTING){WiFi.disconnect();wifiPendingKey="";wifiPassword="";wifiRetryAt=millis();wifiPage=WIFI_HOME;wifiChoice=0;wifiStatus="Cancelado";}
 // A running asynchronous scan is not cancelled by a short click.
 dirty=true;
}
void drawWifi(uint16_t bg,uint16_t panel,uint16_t accent,uint16_t white,uint16_t dim){
 ui.fillScreen(bg);label(10,5,"WIFI / KALIMAN",accent);
 if(wifiPage==WIFI_KEYBOARD){
  label(10,23,wifiSsid.substring(0,38),white);
  String mask;for(unsigned i=0;i<wifiPassword.length()&&i<30;i++)mask+='*';
  label(10,39,"Clave: "+mask+" ("+String(wifiPassword.length())+")",dim);
  int n=strlen(wifiKeys[wifiKeyPage]);
  for(int i=0;i<n;i++){
   int x=10+(i%10)*30,y=58+(i/10)*16;
   if(i==wifiKey)ui.fillRoundRect(x-2,y-2,26,15,2,panel);
   label(x+7,y,String(wifiKeys[wifiKeyPage][i]),i==wifiKey?accent:white);
  }
  const char* actions[]={"Aa/123","Borrar","Conectar","Cancelar"};
  for(int i=0;i<4;i++){int x=8+i*78;if(wifiKey==n+i)ui.fillRect(x,128,76,17,panel);label(x+2,132,actions[i],wifiKey==n+i?accent:dim);}
 }else if(wifiPage==WIFI_NETWORKS){
  int top=wifiChoice/5*5;
  for(int i=top;i<min(top+5,wifiCount+1);i++){
   int y=25+(i-top)*24;if(i==wifiChoice)ui.fillRoundRect(8,y-2,304,23,3,panel);
   String line=i==wifiCount?"< Volver":WiFi.SSID(i).substring(0,27)+" "+String(WiFi.RSSI(i))+"dBm";
   label(12,y,line,i==wifiChoice?accent:white);
   if(i<wifiCount)label(12,y+11,WiFi.encryptionType(i)==WIFI_AUTH_OPEN?"Abierta":"Con clave",dim);
  }
 }else if(wifiPage==WIFI_HOME){
  const char* rows[]={"Buscar redes","Desconectar / apagar","< Volver"};
  for(int i=0;i<3;i++){int y=30+i*27;if(i==wifiChoice)ui.fillRoundRect(8,y-4,304,24,3,panel);label(14,y,rows[i],i==wifiChoice?accent:white);}
  label(10,114,WiFi.status()==WL_CONNECTED?WiFi.SSID().substring(0,38):"Sin conexion",white);
  label(10,132,wifiStatus.substring(0,43),dim);
 }else{centered(60,wifiStatus,white);centered(90,wifiPage==WIFI_CONNECTING?"Pulsar: cancelar":"Espera unos segundos",dim);}
 label(10,158,"Girar: elegir / pulsar: aceptar",dim);ui.pushSprite(0,0);
}
#include "InternetInfo.h"
// All HUDs share bounded header, tuning, instruments and status areas.
void draw(){
 themedText=true;
 const Palette &colors=palettes[theme];
 uint16_t bg=colors.background,panel=colors.panel,accent=colors.accent;
 uint16_t white=menuColor(1,colors.text),dim=menuColor(5,colors.dim);
 // Theme roles apply to every HUD, not just header icons.
 accent=menuColor(4,accent);
 if(internetInfoMenu){drawInternetInfo(bg,accent,white,dim);return;}
 if(wifiMenu){drawWifi(bg,panel,accent,white,dim);return;}
 ui.fillScreen(bg);ui.fillRect(0,0,320,20,panel);
 ui.drawFastHLine(8,20,304,dim);
 // Fixed header cells: clock 8..62, band 64..158, signal 164..202,
 // mode 206..236, weather 240..255 and power 258..318. All share vertical centre y=10.
 if(menu){
  textCell(8,6,150,"ATS / CONFIGURACION",accent,1,false);
 }else{
  textCell(8,6,54,clockSet?localClockText():String("--:--"),accent,1,false);
  textCell(64,6,94,String(bands[band].name),white,1,true);
 }
 // WiFi antenna: illuminated only while associated with a network.
 // Radio reception strength remains on the separate RSSI bars.
 const int antennaX=169;
 const bool wifiConnected=WiFi.status()==WL_CONNECTED;
 const uint16_t antennaColor=wifiConnected?accent:dim;
 ui.drawLine(antennaX,4,antennaX,16,antennaColor);
 ui.drawLine(antennaX-5,4,antennaX,9,antennaColor);ui.drawLine(antennaX+5,4,antennaX,9,antennaColor);
 ui.drawFastHLine(antennaX-3,16,7,antennaColor);
 if(wifiConnected){
  for(int wave=0;wave<2;wave++){
   uint16_t waveColor=(animationFrame/4)%3==wave?accent:dim;
   int offset=7+wave*3;
   ui.drawLine(antennaX-offset,4,antennaX-offset-1,7,waveColor);
   ui.drawLine(antennaX+offset,4,antennaX+offset+1,7,waveColor);
  }
 }
 int signalBars=ready?constrain(int(rssi)/20,0,4):0;
 // Recessed signal capsules: four calibrated RSSI steps, one accent.
 for(int k=0;k<4;k++){
  int h=4+k*3,bx=181+k*6;
  ui.fillRoundRect(bx,17-h,4,h,1,visualBlend(panel,dim,8));
  if(k<signalBars){
   ui.fillRoundRect(bx,17-h,4,h,1,accent);
   ui.drawFastHLine(bx+1,18-h,2,visualBlend(accent,white,12));
  }
 }
 textCell(206,6,30,band==0?(stereo?"ST":"MONO"):modes[mode],white);
 uint16_t batteryColor=!externalPower&&batteryLevel==0?iconPulse(0xF800):menuColor(2,accent);
 char volts[10];snprintf(volts,sizeof(volts),"%.2fV",((batteryMillivolts+25)/50*50)/1000.0f);
 String powerText=externalPower?String("USB"):String(volts);
 ui.setTextSize(1);
 // Reserve a separate cell for weather, without covering battery or voltage.
 drawHeaderWeather(240,2,panel,accent,white,dim);
 const int batteryX=258;
 ui.drawRoundRect(batteryX,5,18,10,2,batteryColor);
 ui.fillRoundRect(batteryX+18,8,2,4,1,batteryColor);
 ui.fillRoundRect(batteryX+2,7,14,6,1,bg);
 if(externalPower){
  // USB power is not a battery-charge estimate.
  uint16_t c=iconPulse(accent);
  ui.fillTriangle(batteryX+10,6,batteryX+6,10,batteryX+10,10,c);
  ui.fillTriangle(batteryX+9,9,batteryX+13,9,batteryX+8,14,c);
 }else if(batteryLevel>=0){
  int w=constrain((batteryLevel+1)*3,0,12);
  ui.fillRoundRect(batteryX+3,7,w,6,1,batteryColor);
  if(w>3)ui.drawFastHLine(batteryX+4,8,w-3,visualBlend(batteryColor,white,10));
 }
 textCell(batteryX+24,6,ui.textWidth(powerText),powerText,white);
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
    if(n==ampItem){ui.fillRect(8,y,4,32,iconPulse(menuColor(n,accent)));animateSelection(y,entryColor);}
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
    if(selected){
     ui.drawRoundRect(8,y,299,32,6,visualBlend(panel,entryColor,12));
     ui.fillRoundRect(8,y,4,32,2,iconPulse(menuColor(n,accent)));
     animateSelection(y,entryColor);
    }
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
     const MenuIcon groupIcons[]={ANTENNA,AUDIO,SCREEN,WEATHER_ICON,SOLAR_ICON,GEAR};
     static_assert(sizeof(groupIcons)/sizeof(groupIcons[0])==MENU_GROUP_COUNT,"Un icono por grupo");
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
   ui.fillRoundRect(313,43,3,102,1,panel);ui.fillRoundRect(313,43+pos*94/max(1,count-1),3,8,1,accent);
  }
 }else{
  String freq=frequencyText(frequencies[band]);
  String head=band==CB_BAND?"CANAL "+String(cbChannel(frequencies[band])+1)+" / 40":band==0?(rdsName.length()?rdsName:"FM / RDS"):String("RECEPCION ")+modes[mode];
  centered(25,head,menuColor(2,white));
  // Distinct layouts; only real receiver measurements.
  int font=hud==5?4:3;
  if(hud!=5){
   ui.fillRoundRect(8,38,304,36,5,panel);
   reliefFrame(8,38,304,36,panel,true);
   ui.drawFastHLine(18,71,284,visualBlend(panel,accent,5));
   ui.fillRect(8,44,3,24,accent);
  }
  ui.setTextSize(font);
  int fw=ui.textWidth(freq),fx=(320-fw-26)/2;
  textCell(fx,40,fw,freq,white,font);
  textCell(fx+fw+6,58,20,"MHz",accent);
  if(hud==29){
   // Two separate instruments: real histories, independent labelled scales.
   const size_t start=ecgCount==ECG_SAMPLES?ecgWrite:0;
   for(int n=0;n<2;n++){
    const int cardX=8+n*156;
    const int left=cardX+7,right=cardX+140,top=98,bottom=116;
    const uint16_t ink=n?menuColor(6,colors.accent):accent;
    ui.fillRoundRect(cardX,79,148,49,5,panel);
    reliefFrame(cardX,79,148,49,panel,true);
    textCell(cardX+7,82,134,(n?String("SNR ")+snr+" dB":String("RSSI ")+rssi+" dBuV"),ink,1,false);
    ui.drawFastHLine(left,94,right-left+1,visualBlend(panel,dim,8));
    int low=n?snr:rssi,high=low;
    for(size_t i=0;i<ecgCount;i++){
     const int sample=n?ecgSnrSamples[(start+i)%ECG_SAMPLES]:ecgSamples[(start+i)%ECG_SAMPLES];
     low=min(low,sample);high=max(high,sample);
    }
    const int span=max(10,high-low+4);
    const int scaleLow=max(0,(low+high-span)/2);
    const uint16_t grid=visualBlend(panel,dim,5);
    for(int x=left;x<=right;x+=19)ui.drawFastVLine(x,top,bottom-top+1,grid);
    for(int y=top;y<=bottom;y+=9)ui.drawFastHLine(left,y,right-left+1,grid);
    int previousX=0,previousY=0;
    for(size_t i=0;i<ecgCount;i++){
     const int x=left+int(ECG_SAMPLES-ecgCount+i)*(right-left)/int(ECG_SAMPLES-1);
     const int sample=n?ecgSnrSamples[(start+i)%ECG_SAMPLES]:ecgSamples[(start+i)%ECG_SAMPLES];
     const int y=bottom-constrain(sample-scaleLow,0,span)*(bottom-top)/span;
     if(i)ui.drawLine(previousX,previousY,x,y,ink);
     previousX=x;previousY=y;
    }
    if(ecgCount)ui.fillCircle(previousX,previousY,1,ink);
    // Scale labels below the trace, never covering measured data.
    textCell(left,119,134,String(scaleLow)+" - "+String(scaleLow+span)+(n?" dB":" dBuV"),dim,1,false);
   }
  }else for(int n=0;n<2;n++){
   int x=8+n*156,reading=n?snr:rssi,maximum=n?50:80;
   int level=hudVisualInitialized?(hudVisual[n]+128)/256:constrain(reading*100/maximum,0,100);
   uint16_t ink=menuColor(n?6:3,colors.accent);
   ui.fillRoundRect(x,79,148,49,5,panel);
   if(refinedHud(hud)){
    drawRefinedHud(hud,x,n,reading,level,bg,panel,ink,white,dim);
   }else if(hud>=30){
    textCell(x+6,82,86,n?"SNR / dB":"RSSI / dBuV",dim,1,false);
    textCell(x+96,82,46,String(reading),white,2);
    const int left=x+9,base=123;
    if(hud==35){
     // Calibrated orbital instrument: position follows measured RF only.
     // Left: scale and luminous marker. Right: normalized scale, not quality.
     const int cx=x+43,cy=111,rx=29,ry=10;
     const float rad=3.14159265f/180.0f;
     uint16_t track=visualBlend(panel,ink,7);
     ui.drawEllipse(cx,cy+1,rx+1,ry+1,visualBlend(panel,0,16));
  ui.drawEllipse(cx,cy-1,rx+1,ry+1,visualBlend(panel,0xFFFF,11));
  ui.drawEllipse(cx,cy,rx,ry,track);
     ui.drawEllipse(cx,cy,rx-5,ry-3,visualBlend(panel,ink,4));
     ui.drawFastHLine(cx-8,cy,17,track);
     ui.drawFastVLine(cx,cy-4,9,track);
     ui.fillCircle(cx,cy,2,ink);
     // Upper semicircle is the 0..100 scale, with quarter-scale ticks.
     for(int k=0;k<=4;k++){
      float a=(180+k*45)*rad;
      ui.drawLine(cx+int(cosf(a)*(rx+1)),cy+int(sinf(a)*(ry+1)),
                  cx+int(cosf(a)*(rx+4)),cy+int(sinf(a)*(ry+3)),dim);
     }
     // Illuminated scale segments end exactly at the current reading.
     for(int k=0;k<level;k+=3){
      float a=(180+k*1.8f)*rad;
      float b=(180+min(k+3,level)*1.8f)*rad;
      ui.drawLine(cx+int(cosf(a)*rx),cy+int(sinf(a)*ry),
                  cx+int(cosf(b)*rx),cy+int(sinf(b)*ry),ink);
     }
     float a=(180+level*1.8f)*rad;
     int px=cx+int(cosf(a)*rx),py=cy+int(sinf(a)*ry);
     ui.drawCircle(px,py,4,visualBlend(panel,ink,8));
     ui.fillCircle(px,py,2,ink);
     ui.fillCircle(px,py,1,white);
     textCell(x+82,100,60,String(level)+"%",ink,2);
     textCell(x+82,118,60,"ESCALA",dim,1);
    }else if(hud==30){
     int cx=x+35,cy=110;
     for(int k=0;k<24;k++){
      float a=(k*15-90)*3.14159265f/180;
      int radius=hud==35?12:10;
      int px=cx+int(cosf(a)*radius),py=cy+int(sinf(a)*radius);
      ui.fillCircle(px,py,1,k*100/24<level?ink:dim);
     }
     textCell(x+64,104,76,String(level)+"%",ink,2);
    }else if(hud==31){
     ui.drawRoundRect(left,102,126,10,4,dim);
     int w=level*120/100;
     if(w)ui.fillRect(left+3,105,w,4,ink);
     ui.fillCircle(left+3,108,5,ink);
     textCell(x+30,116,108,String(level)+"%",dim,1);
    }else if(hud==32){
     for(int k=0;k<30;k++)ui.fillRect(left+k*4,104,3,12,k*100/30<level?visualBlend(ink,white,k/2):bg);
     ui.drawFastVLine(left+level*119/100,101,18,white);
    }else if(hud==33){
     for(int k=0;k<12;k++){
      int cx=left+5+k*10;uint16_t c=k*100/12<level?ink:dim;
      ui.drawLine(cx-4,111,cx,105,c);ui.drawLine(cx,105,cx+4,111,c);
      ui.drawLine(cx+4,111,cx,117,c);ui.drawLine(cx,117,cx-4,111,c);
     }
    }else if(hud==34){
     // A single filled horizon, unlike the segmented RF bars.
     ui.drawRect(left,102,126,21,dim);
     int h=level*19/100;
     if(h)for(int row=0;row<h;row++)
      ui.drawFastHLine(left+1,121-row,124,visualBlend(ink,white,row*16/19));
     int y=121-h;
     ui.drawFastHLine(left+1,y,124,white);
    }else if(hud==36){
     ui.drawFastHLine(left,114,126,dim);
     for(int k=0;k<=20;k++)ui.drawFastVLine(left+k*6,110,k%5?5:9,dim);
     int px=left+level*120/100;
     ui.fillTriangle(px-3,103,px+3,103,px,109,ink);
    }else{
     // Symmetric balance wings: distance from centre follows RF level.
     int cx=x+73,w=level*59/100;
     ui.drawFastHLine(left,111,126,dim);
     for(int k=-4;k<=4;k++)ui.drawFastVLine(cx+k*15,108,7,dim);
     if(w){
      ui.fillTriangle(cx,111,cx-w,102,cx-w,120,ink);
      ui.fillTriangle(cx,111,cx+w,102,cx+w,120,ink);
     }
     ui.drawFastVLine(cx,100,23,white);
    }
   }else if(hud==26){
    // Compact dial with a real, bounded percentage arc and raw reading.
    int cx=x+35,cy=104;
    for(int k=0;k<30;k++){
     float angle=(135+k*270.0f/29)*3.14159265f/180;
     uint16_t c=k*100/30<level?ink:dim;
     ui.drawLine(cx+int(cosf(angle)*17),cy+int(sinf(angle)*17),cx+int(cosf(angle)*21),cy+int(sinf(angle)*21),c);
    }
    textCell(x+18,100,34,String(level),white);
    textCell(x+65,83,78,n?"SNR / dB":"RSSI / dBuV",dim);
    textCell(x+65,98,78,String(reading),white,3);
   }else if(hud==27){
    textCell(x+6,82,86,n?"SNR / dB":"RSSI / dBuV",dim,1,false);
    textCell(x+96,82,46,String(reading),white,2);
    // Nested chevrons form a prism instead of repeated vertical columns.
    for(int k=0;k<10;k++){
     int px=x+12+k*12;
     uint16_t c=k*100/10<level?visualBlend(ink,white,k):dim;
     ui.drawLine(px,103,px+7,112,c);
     ui.drawLine(px+7,112,px,121,c);
    }
   }else if(hud==28){
    // 40-cell matrix represents percentage of the instrument scale.
    textCell(x+6,82,82,n?"SNR / dB":"RSSI / dBuV",dim,1,false);
    textCell(x+94,82,48,String(reading),white,2);
    for(int k=0;k<40;k++){
     int col=k%20,row=k/20;
     ui.fillCircle(x+9+col*7,109+row*10,2,k*100/40<level?ink:bg);
    }
   }else if(hud==1){
    // Wide analog scale: labels and numbers sit outside the needle area.
    textCell(x+5,82,62,n?"SNR dB":"RSSI dBuV",dim,1,false);
    textCell(x+86,82,55,String(reading),white,2);
    int cx=x+74,cy=124;
    for(int k=0;k<=10;k++){
     float angle=(200+k*14)*3.14159265f/180;
     ui.drawLine(cx+int(cosf(angle)*49),cy+int(sinf(angle)*25),cx+int(cosf(angle)*54),cy+int(sinf(angle)*29),k%5?dim:ink);
    }
    float angle=(200+level*1.4f)*3.14159265f/180;
    ui.drawLine(cx,cy,cx+int(cosf(angle)*47),cy+int(sinf(angle)*24),ink);
    ui.fillCircle(cx,cy,2,white);
   }else if(hud==3){
    textCell(x+6,82,80,n?"CALIDAD / SNR":"SENAL / RSSI",dim,1,false);
    textCell(x+91,82,51,String(reading),white,2);
    for(int k=0;k<18;k++){
     int h=8+k*18/17;
     uint16_t c=theme==RAINBOW_THEME?rainbowColor(k*7/18+n*3):visualBlend(ink,white,k*10/17);
     ui.fillRoundRect(x+6+k*7,124-h,5,h,1,level>k*100/18?c:bg);
    }
   }else if(hud==2){
    ui.drawRoundRect(x,79,148,49,5,dim);
    textCell(x+7,84,40,n?"SNR":"RSSI",ink,1,false);
    textCell(x+48,82,65,String(reading),white,3);
    textCell(x+112,101,30,n?"dB":"dBuV",dim);
    for(int k=0;k<22;k++)ui.fillRect(x+8+k*6,119,4,4,level>k*100/22?ink:bg);
   }else if(hud==5){
    // Minimal: typography and endpoint only, no studio progress bars.
    textCell(x+6,84,136,n?"SNR / dB":"RSSI / dBuV",dim);
    textCell(x+6,99,136,String(reading),white,3);
    ui.drawFastHLine(x+20,125,108,dim);
    ui.fillCircle(x+20+level*108/100,125,2,ink);
   }else{
    textCell(x+6,82,84,n?"SNR / CALIDAD":"RSSI / SENAL",dim,1,false);
    textCell(x+87,82,54,String(reading),white,2);
    textCell(x+6,99,45,n?"dB":"dBuV",dim,1,false);
    ui.fillRoundRect(x+6,116,136,5,2,hud==5?panel:bg);
    int width=level*136/100;
    if(theme==RAINBOW_THEME){
     for(int k=0;k<7;k++){
      int left=k*136/7,right=min(width,(k+1)*136/7);
      if(right>left)ui.fillRect(x+6+left,116,right-left,5,rainbowColor(k+n*3));
     }
    }else if(width)ui.fillRoundRect(x+6,116,width,5,2,ink);
    if(hud==0)textCell(x+64,100,77,String(level)+"% escala",ink);
   }
  }
  if(band==0){
   if(rdsText.length()){
    label(10,131,"RDS",dim);
    ui.setTextSize(1);ui.setTextColor(readableText(dim));
    ui.setClipRect(34,131,RDS_TEXT_WIDTH,10);
    ui.setCursor(34-int(rdsScrollOffset),131);ui.print(rdsScrollSource);
    if(rdsScrollWidth){
     ui.setCursor(34+int(max(uint16_t(RDS_TEXT_WIDTH),rdsScrollWidth))+24-int(rdsScrollOffset),131);
     ui.print(rdsScrollSource);
    }
    ui.clearClipRect();
   }else label(10,131,rdsSync?"RDS: recibiendo texto...":rdsEnabled?"RDS: esperando datos de emisora":"RDS desactivado",dim);
  }else{
   label(10,132,band==CB_BAND?"CB USA / 40 canales":String("PASO ")+(fineTuning()?String(fineStepHz())+" Hz":String(band==0?step()*10:step())+" kHz")+" / "+(agc?"AGC":"MANUAL"),dim);
   textCell(10,144,230,mode?(String(ssbFilterAuto?"AUTO ":"MANUAL ")+ssbFilters[activeFilter()]+"k / "+(autoVfoActive()?String("SYNC portadora"):"BFO "+String(bfo))):String(amFilterAuto?"AUTO ":"MANUAL ")+amFilters[activeFilter()]+" kHz",dim,1,false);
  }
  if(band==0){
   String reception=signalSearch?"BUSCANDO":squelchClosed?"SQL CERRADO":squelchMode==2?"SQL AUTO":squelchMode==1?"SQL MANUAL":"SQL OFF";
   textCell(10,144,230,reception,dim,1,false);
  }
  textCell(244,144,68,muted?"MUTE":!ampEnabled?"AMP OFF":squelchClosed?"SQL":"VOL "+String(volume),accent,1,true);

 }
 ui.fillRect(0,157,320,13,panel);
 centered(160,notice.length()?notice:menu?(infoMenu?"Girar: pagina / Pulsar: volver":"Girar: elegir / Pulsar: abrir"):volumeMode?"RUEDA: volumen / Mantener: menu":"RUEDA: sintonia / Mantener: menu",dim);
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
  // A PS name is a complete ordered cycle, not a bag of segments.
  // Start again at segment zero; never mix two changing station names.
  // Keep the last complete name visible while a new cycle is collected.
  if(segment==0){rdsNameSegments=0;memset(psBuffer,0,sizeof(psBuffer));}
  if(segment!=rdsNameSegments){rdsNameSegments=0;return;}
  // Group 0 bit 4 is NOT the RadioText A/B flag.
  radio.getNext2Block(psBuffer+segment*2);
  psBuffer[8]=0;rdsNameSegments++;
  if(rdsNameSegments==4){
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
 case 45:{int64_t n=int64_t(directHz)+int64_t(d)*digitPowers[directDigit];directHz=uint32_t(constrain(n,int64_t(0),int64_t(99999990)));dirty=true;return;}
 case 46:rdsEnabled=!rdsEnabled;resetRds();break;
 case 47:screenSleep=wrap(screenSleep+d,11);break;
 case 38:case 39:return;
 case 40:autoClock=!autoClock;internetClock=false;nextTimeQuery=0;if(settingsAvailable)prefs.putBool("autoclock",autoClock);break;
 case 34:{uint32_t t=localClockSeconds();setLocalClock(wrap(int(t/3600)+d,24),t/60%60);break;}
 case 35:{uint32_t t=localClockSeconds();setLocalClock(t/3600,wrap(int(t/60%60)+d,60));break;}
 case 36:{if(!clock12h){message("Elige primero formato 12 h");return;}uint32_t t=localClockSeconds();setLocalClock((t/3600+12)%24,t/60%60);break;}
 case 37:clock12h=!clock12h;if(settingsAvailable)prefs.putBool("clock12",clock12h);break;
 case 32:fmDeemphasis=wrap(int(fmDeemphasis)+d,3);audioSettings();break;
 case 33:break; // Native AM AVC is always automatic; no manual ceiling selector.
 case 29:softMuteDb=constrain(int(softMuteDb)+d,0,8);audioSettings();break;
 case 30:searchRssi=constrain(int(searchRssi)+d,0,80);break;
 case 31:searchSnr=constrain(int(searchSnr)+d,0,50);break;
 case 26:setAudioTimer(wrap(audioTimerMinutes/5+d,25)*5);return;
 case 28:fmForceMono=!fmForceMono;audioSettings();stereo=false;break;
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
  bool ssbAudio=mode;
  int count=ssbAudio?6:7;
  bool automatic=ssbAudio?ssbFilterAuto:amFilterAuto;
  int selection=wrap((automatic?0:1+(ssbAudio?ssbFilter:amFilter))+d,count+1);
  if(ssbAudio){ssbFilterAuto=selection==0;if(selection)ssbFilter=selection-1;}
  else{amFilterAuto=selection==0;if(selection)amFilter=selection-1;}
  filterCandidate=255;lastFilterChange=millis();audioSettings();
 }break;
 case 5:agc=!agc;audioSettings();break;
 case 6:attenuation=constrain(int(attenuation)+d,0,26);audioSettings();break;
 case 7:if(autoVfoActive()){afcEnabled=false;audioSettings();message("SYNC apagado: BFO manual");}if(band&&mode){bfo=constrain(int(bfo)+d*10,-16000,16000);radio.setSSBBfo(bfo);}break;
 case 21:squelchMode=wrap(squelchMode+d,3);noiseFloorValid=false;autoSquelchThreshold=6;updateSquelch(millis());break;
 case 22:if(squelchMode==2){message("Nivel automatico");return;}squelch=constrain(int(squelch)+d,0,80);updateSquelch(millis());break;
 case 17:if(band==0||band==CB_BAND||mode==0){message("SYNC solo en LSB/USB; AM no usa AFC");break;}afcEnabled=d>0;audioSettings();message(!afcEnabled?"BFO manual restaurado":autoVfoActive()?"SYNC: requiere portadora":"Guardado / no activo aqui");break;
 case 8:hud=nextHud(hud,d);resetHud();break;
 case 14:theme=wrap(theme+d,THEME_COUNT);break;
 case 15:region=wrap(region+d,7);applyRegion();stepIndex=band==0?3:region==2?3:2;selectBand();break;
 case 9:muted=!muted;audioSettings();break;
 }
 changed();
}
void click(){
 if(menu&&!menuRoot&&item==44){switchVfo();return;}
 if(menu&&!menuRoot&&item==45){
  if(!directEntry){directHz=uint32_t(frequencies[band])*(band==0?10000UL:1000UL);directDigit=0;directEntry=true;editing=true;}
  else if(directDigit<7)directDigit++;
  else {
   uint32_t unit=band==0?10000UL:1000UL;
   if(directHz%unit||directHz/unit<bands[band].lo||directHz/unit>bands[band].hi){message("Fuera de banda o resolucion invalida");return;}
   uint16_t f=directHz/unit;
   if(band==CB_BAND&&cbFrequencies[cbChannel(f)]!=f){message("CB: usa frecuencia de canal 1-40");return;}
   frequencies[band]=f;tuneConfirmed(f);resetRds();changed();directEntry=false;editing=false;menu=false;
  }
  dirty=true;return;
 }

 if(internetInfoMenu){
  if(infoTab==2){weatherFahrenheit=!weatherFahrenheit;if(settingsAvailable)prefs.putBool("wxF",weatherFahrenheit);}
  else if(infoTab==3){weatherImperial=!weatherImperial;weatherFahrenheit=weatherImperial;if(settingsAvailable){prefs.putBool("wxUS",weatherImperial);prefs.putBool("wxF",weatherFahrenheit);}}
  else if(infoTab==1){propagationPage^=1;}
  else internetInfoMenu=false;
  dirty=true;return;
 }
 if(menu&&!menuRoot&&(item==42||item==43)){internetInfoMenu=true;infoTab=item-42;propagationPage=0;infoEnabled=true;if(!infoRunning)infoNext=0;editing=false;dirty=true;return;}
 if(wifiMenu){wifiClick();return;}
 if(menu&&!menuRoot&&item==41){wifiMenu=true;wifiPage=WIFI_HOME;wifiChoice=0;editing=false;dirty=true;return;}
 if(signalSearch){cancelSearch();resetRds();changed();message("Busqueda cancelada");return;}
 if(menu&&!menuRoot&&!ampMenu&&!infoMenu&&item==25){startSearch();editing=false;menu=false;return;}
 if(!menu){volumeMode=!volumeMode;dirty=true;return;}
 if(infoMenu){infoMenu=false;dirty=true;return;}
 if(menuRoot){
  if(menuGroup==MENU_GROUP_COUNT)menu=false;
  else{menuRoot=false;menuPosition=0;item=menuGroups[menuGroup].ids[0];}
  dirty=true;return;
 }
 if(item==38){message(netClockStatus.c_str());return;}
 if(item==39){
  if(WiFi.status()!=WL_CONNECTED){message("Conecta WiFi primero");return;}
  autoClock=true;if(settingsAvailable)prefs.putBool("autoclock",true);
  nextTimeQuery=0;forceTimeQuery=true;configTime(0,0,"pool.ntp.org","time.nist.gov");
  message("Sincronizando hora y zona por WiFi");return;
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
   band=folderBand(folder,folderPosition);
   
   if(band>=52)stepIndex=0;
   if(band==2&&region)stepIndex=region==2?3:2;
   selectBand();editing=false;menu=false;
  }
  dirty=true;return;
 }
 if(editing){editing=false;dirty=true;return;}
 if(item==13){menuRoot=true;dirty=true;return;}
 editing=true;if(item==0){locateBandFolder(band);folderOpen=false;}
 dirty=true;
}
void setup(){
 Serial.begin(115200);
 analogReadResolution(12);
 // The ESP32 Arduino ADC channel must exist before its attenuation is set.
 (void)analogRead(BATTERY_PIN);
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
 rdsName.reserve(8);rdsText.reserve(64);rdsScrollSource.reserve(64);notice.reserve(64);
 pinMode(ENC_A,INPUT_PULLUP);pinMode(ENC_B,INPUT_PULLUP);pinMode(BUTTON,INPUT_PULLUP);
 encoderState=(digitalRead(ENC_A)<<1)|digitalRead(ENC_B);
 settingsAvailable=prefs.begin("ats-custom",false);
 if(settingsAvailable){
  const uint8_t cycle=prefs.getUChar("cowcycle",0)%3;
  startupAbductCow=(cycle==0);
  if(prefs.putUChar("cowcycle",(cycle+1)%3)!=1)startupAbductCow=true;
 }
 weatherFahrenheit=prefs.getBool("wxF",false);
 weatherImperial=prefs.getBool("wxUS",false);
 clock12h=prefs.getBool("clock12",false);
 autoClock=prefs.getBool("autoclock",true);
 zoneName=prefs.getString("tzname","");zoneOffset=constrain(prefs.getInt("tzoffset",0),-50400,50400);zoneDst=prefs.getBool("tzdst",false);
 wifiAutoEnabled=settingsAvailable&&prefs.getBool("wifion",false);
 if(wifiAutoEnabled)wifiBeginSaved();
 else Serial.println("WiFi: autoconexion desactivada; conecta una red desde el menu");
 if(settingsAvailable&&prefs.getBool("clockset",false)){
  clockBaseSeconds=prefs.getUInt("clocksec",0)%86400UL;
  clockBaseUs=esp_timer_get_time();clockSavedAtUs=clockBaseUs;clockSet=!autoClock;
 }
 fmForceMono=prefs.getBool("fmmono",false);
 if(settingsAvailable&&prefs.isKey("automode"))prefs.remove("automode");
 language=constrain(prefs.getUChar("lang",0),0,LANGUAGE_COUNT-1);
 if(!settingsAvailable)Serial.println("Ajustes no disponibles: se usaran valores iniciales.");
 ampEnabled=prefs.getBool("amp",true);
 afcEnabled=prefs.getBool("afc",false); // Restore preference; activation remains SSB-only.
 rdsEnabled=prefs.getBool("rdsOn",true);screenSleep=constrain(prefs.getUChar("lcdSleep",0),0,10);
 activeVfo=constrain(prefs.getUChar("vfoSel",0),0,1);
 if(prefs.getBytesLength("vfoAB")==sizeof(vfo)){
  VfoState loaded[2]={};
  if(prefs.getBytes("vfoAB",loaded,sizeof(loaded))==sizeof(loaded)){
   // Region bounds are applied later; reject corrupt data, not valid presets.
   for(int i=0;i<2;i++){const auto &v=loaded[i];if(v.bandId<BAND_COUNT&&v.modeId<3&&v.stepId<8&&v.fine>=-16000&&v.fine<=16000&&v.frequency>0&&v.frequency<= (v.bandId==0?10800:30000))vfo[i]=v;}
  }
 }
 squelch=constrain(prefs.getUChar("sql",0),0,80);
 squelchMode=constrain(prefs.getUChar("sqlmode",squelch?1:0),0,2);
 for(int i=0;i<BAND_COUNT;i++)frequencies[i]=bands[i].initial;
 size_t savedFreqBytes=prefs.getBytesLength("freq");
 if(savedFreqBytes && savedFreqBytes<=sizeof(frequencies) && savedFreqBytes%sizeof(uint16_t)==0){
  // Preserve defaults on failed reads; older, shorter band tables remain valid.
  uint16_t pending[BAND_COUNT];memcpy(pending,frequencies,sizeof(pending));
  if(prefs.getBytes("freq",pending,savedFreqBytes)==savedFreqBytes)
   memcpy(frequencies,pending,sizeof(frequencies));
 }
 for(int i=0;i<BAND_COUNT;i++)frequencies[i]=constrain(frequencies[i],bands[i].lo,bands[i].hi);
 band=constrain(prefs.getUChar("band",0),0,BAND_COUNT-1);mode=constrain(prefs.getUChar("mode",0),0,2);
 volume=constrain(prefs.getUChar("vol",12),0,63);stepIndex=constrain(prefs.getUChar("step",2),0,7);
 amFilterAuto=prefs.getBool("afa",false);ssbFilterAuto=prefs.getBool("sfa",false);
 amFilter=constrain(prefs.getUChar("af",3),0,6);ssbFilter=constrain(prefs.getUChar("sf",1),0,5);
 fmDeemphasis=constrain(prefs.getUChar("fmdeemp",0),0,2);
 // Legacy manual amavc preference is intentionally ignored.
 softMuteDb=constrain(prefs.getUChar("softmute",8),0,8);
 searchRssi=constrain(prefs.getUChar("seekrssi",20),0,80);searchSnr=constrain(prefs.getUChar("seeksnr",6),0,50);
 attenuation=constrain(prefs.getUChar("att",0),0,26);agc=prefs.getBool("agc",true);hud=canonicalHud(constrain(prefs.getUChar("hud",0),0,HUD_COUNT-1));
 theme=constrain(prefs.getUChar("theme",0),0,THEME_COUNT-1);region=constrain(prefs.getUChar("region",0),0,6);applyRegion();
 bfo=constrain(prefs.getShort("bfo",0),-16000,16000);
 const uint32_t startupAt=millis();
 uint32_t startupFrameAt=startupAt;
 drawStartup(0);
 while(uint32_t(millis()-startupAt)<6000UL){
  const uint32_t now=millis();
  if(uint32_t(now-startupFrameAt)>=33UL){
   startupFrameAt=now;drawStartup(now-startupAt);
  }
  delay(1); // Yield to the ESP32 tasks throughout the presentation.
 }
 drawStartup(6000);
 resetHud();
 ready=radio.getDeviceI2CAddress(RESET)!=0;
 // Welcome finishes before a separate SSB loading screen is shown.
 if(ready){radio.setup(RESET,0);selectBand(true);}else Serial.println("No se detecta SI4732; amplificador desactivado.");
 // Synchronize the button after startup to avoid a phantom click.
 buttonRaw=buttonStable=digitalRead(BUTTON);
 buttonArmed=buttonStable; // Ignore the release of a button held during boot.
 buttonChange=millis();
 // Discard encoder transitions made during the blocking startup sequence.
 encoderState=(digitalRead(ENC_A)<<1)|digitalRead(ENC_B);quarters=0;
 draw();dirty=false;
}
bool turnFine(int d){
 if(!fineTuning())return false;
 if(autoVfoActive()){afcEnabled=false;audioSettings();message("SYNC apagado: BFO manual");}
 bfo=constrain(int(bfo)+d*fineStepHz(),-16000,16000);
 radio.setSSBBfo(bfo);changed();return true;
}
// USB control protocol: bounded lines, no blocking reads, main-loop radio access.
void usbReply(const char* error=nullptr){
 JsonDocument doc;doc["protocol"]="KALIMAN-USB-1";doc["ok"]=error==nullptr;
 if(error)doc["error"]=error;
 doc["ready"]=ready;doc["band"]=band;doc["band_name"]=bands[band].name;
 doc["frequency_hz"]=uint32_t(frequencies[band])*(band==0?10000UL:1000UL);
 doc["mode"]=band==0?"FM":mode==1?"LSB":mode==2?"USB":"AM";
 doc["automatic_mode"]=false;doc["sync_enabled"]=afcEnabled;doc["sync_active"]=autoVfoActive();doc["bfo_hz"]=bfo;
 doc["volume"]=volume;doc["mute"]=muted;doc["squelch"]=squelch;
 doc["squelch_mode"]=squelchMode;doc["rssi"]=rssi;doc["snr"]=snr;
 serializeJson(doc,Serial);Serial.println();
}
void usbCommand(char* line){
 char* arg=strchr(line,' ');if(arg){*arg++=0;while(*arg==' ')arg++;}
 if(!strcmp(line,"STATUS")){usbReply();return;}
 if(!strcmp(line,"BANDS")){
  JsonDocument doc;doc["protocol"]="KALIMAN-USB-1";doc["ok"]=true;
  JsonArray list=doc["bands"].to<JsonArray>();
  for(int i=0;i<BAND_COUNT;i++){JsonObject b=list.add<JsonObject>();b["id"]=i;b["name"]=bands[i].name;b["min_hz"]=uint32_t(bands[i].lo)*(i==0?10000UL:1000UL);b["max_hz"]=uint32_t(bands[i].hi)*(i==0?10000UL:1000UL);}
  serializeJson(doc,Serial);Serial.println();return;
 }
 if(!ready){usbReply("RADIO_NOT_READY");return;}
 if(!arg||!*arg){usbReply("MISSING_ARGUMENT");return;}
 if(!strcmp(line,"MODE")){
  int m=!strcmp(arg,"AM")?0:!strcmp(arg,"LSB")?1:!strcmp(arg,"USB")?2:-1;
  if(band==0){if(strcmp(arg,"FM")){usbReply("FM_BAND_ONLY");return;}usbReply();return;}
  if(m<0||(band==CB_BAND&&m!=0)){usbReply("INVALID_MODE");return;}
  mode=m;bfo=0;selectBand(false);usbReply();return;
 }
 errno=0;char* end=nullptr;long n=strtol(arg,&end,10);
 if(errno||end==arg||*end){usbReply("INVALID_NUMBER");return;}
 if(!strcmp(line,"BAND")){
  if(n<0||n>=BAND_COUNT){usbReply("INVALID_BAND");return;}
  band=n;selectBand(false);
 }else if(!strcmp(line,"FREQ")){
  long unit=band==0?10000:1000;
  if(n< long(bands[band].lo)*unit||n>long(bands[band].hi)*unit||n%unit){usbReply("FREQUENCY_OUT_OF_RANGE_OR_RESOLUTION");return;}
  uint16_t f=n/unit;
  if(band==CB_BAND&&cbFrequencies[cbChannel(f)]!=f){usbReply("INVALID_CB_CHANNEL_FREQUENCY");return;}
  cancelSearch();frequencies[band]=f;radio.setFrequency(f);resetRds();changed();
 }else if(!strcmp(line,"AUTOMODE")){
  if(n<0||n>1){usbReply("RANGE_0_1");return;}
  usbReply("AUTOMODE_REMOVED_USE_MODE");return;
 }else if(!strcmp(line,"SYNC")){
  if(n<0||n>1){usbReply("RANGE_0_1");return;}
  if(!band||!mode||band==CB_BAND){usbReply("SYNC_REQUIRES_LSB_USB_AM_CARRIER");return;}
  afcEnabled=n;audioSettings();changed();
 }else if(!strcmp(line,"BFO")){
  if(!band||!mode||band==CB_BAND){usbReply("SSB_ONLY");return;}
  if(n< -16000||n>16000){usbReply("RANGE_MINUS16000_16000");return;}
  afcEnabled=false;bfo=n;audioSettings();changed();
 }else if(!strcmp(line,"VOLUME")){
  if(n<0||n>63){usbReply("RANGE_0_63");return;}volume=n;audioSettings();changed();
 }else if(!strcmp(line,"MUTE")){
  if(n<0||n>1){usbReply("RANGE_0_1");return;}muted=n;audioSettings();changed();
 }else if(!strcmp(line,"SQL")){
  if(n<0||n>80){usbReply("RANGE_0_80");return;}squelch=n;updateSquelch(millis());changed();
 }else if(!strcmp(line,"SQLMODE")){
  if(n<0||n>2){usbReply("RANGE_0_2");return;}squelchMode=n;noiseFloorValid=false;autoSquelchThreshold=6;updateSquelch(millis());changed();
 }else{usbReply("UNKNOWN_COMMAND");return;}
 usbReply();
}
void usbControlTick(){
 static char line[96];static size_t used=0;static bool overflow=false;
 int budget=128;
 while(budget--&&Serial.available()){
  char c=Serial.read();if(c=='\r')continue;
  if(c=='\n'){
   line[used]=0;if(overflow)usbReply("LINE_TOO_LONG");else if(used)usbCommand(line);
   used=0;overflow=false;break;
  }
  if(used<sizeof(line)-1)line[used++]=c;else overflow=true;
 }
}
void loop(){
 if(!lastInteraction)lastInteraction=millis();
 bool inputWake=!digitalRead(BUTTON)||((digitalRead(ENC_A)<<1)|digitalRead(ENC_B))!=encoderState;
 if(inputWake){lastInteraction=millis();if(screenAsleep){digitalWrite(BL,HIGH);screenAsleep=false;dirty=true;}}
 if(screenSleep&&!screenAsleep&&millis()-lastInteraction>=uint32_t(screenSleep)*30000){digitalWrite(BL,LOW);screenAsleep=true;}
 if(!screenSleep&&screenAsleep){digitalWrite(BL,HIGH);screenAsleep=false;dirty=true;}

 usbControlTick();
 wifiTick();
 // Five-minute checkpoints limit flash wear; manual edits save immediately.
 internetClockTick();
 infoTick();
 if(clockSet&&esp_timer_get_time()-clockSavedAtUs>=300000000LL)persistClock();
 int currentClockMinute=clockSet?int(localClockSeconds()/60):-1;
 if(currentClockMinute!=clockLastMinute){clockLastMinute=currentClockMinute;dirty=true;}
 static const int8_t transitions[]={0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
 int state=(digitalRead(ENC_A)<<1)|digitalRead(ENC_B);
 if(state!=encoderState){
  // Both contacts changing together is a missed/invalid transition, not a turn.
  if((state^encoderState)==3)quarters=0;
  else quarters+=transitions[(encoderState<<2)|state];
  encoderState=state;
 }
 if(abs(quarters)>=4){
  int d=quarters>0?1:-1;quarters=0;
  if(ready){
   if(signalSearch){cancelSearch();resetRds();}
   if(internetInfoMenu){if(infoTab==1)propagationBand=wrap(propagationBand+d,9);else {const int pages[]={0,2,3};int pos=infoTab==0?0:infoTab==2?1:2;infoTab=pages[wrap(pos+d,3)];}dirty=true;}
   else if(wifiMenu){wifiTurn(d);}
   else if(menu){if(infoMenu){infoPage=wrap(infoPage+d,2);dirty=true;}else if(ampMenu){
    if(ampEditing){if(ampItem==0)ampEnabled=!ampEnabled;else if(ampItem==1)volume=constrain(int(volume)+d,0,63);audioSettings();changed();}
    else{ampItem=wrap(ampItem+d,3);dirty=true;}
   }else if(editing)adjust(d);else{
    if(menuRoot)menuGroup=wrap(menuGroup+d,MENU_GROUP_COUNT+1);
    else{menuPosition=wrap(menuPosition+d,menuGroups[menuGroup].count);item=menuGroups[menuGroup].ids[menuPosition];}
    dirty=true;
   }}
   else if(volumeMode){volume=constrain(int(volume)+d,0,63);audioSettings();changed();}
   else if(turnFine(d)){}
   else{int next;
    if(band==CB_BAND)next=cbFrequencies[wrap(cbChannel(frequencies[band])+d,40)];
    else{next=int(frequencies[band])+d*step();if(next>bands[band].hi)next=bands[band].lo;if(next<bands[band].lo)next=bands[band].hi;}frequencies[band]=next;tuneConfirmed(next);resetRds();changed();}
  }
 }
 uint32_t now=millis();bool raw=digitalRead(BUTTON);
 if(raw!=buttonRaw){buttonRaw=raw;buttonChange=now;}
 if(now-buttonChange>=30&&raw!=buttonStable){
  buttonStable=raw;if(!raw){pressedAt=now;buttonArmed=true;}
  else if(!buttonArmed){buttonArmed=true;}
  else if(ready){if(now-pressedAt>=700){bool childView=internetInfoMenu||wifiMenu;internetInfoMenu=false;wifiMenu=false;wifiPassword="";cancelSearch();menu=childView?true:!menu;menuRoot=true;menuGroup=0;menuPosition=0;item=0;infoMenu=false;ampMenu=false;ampEditing=false;editing=false;directEntry=false;volumeMode=false;dirty=true;}else click();}
 }
 pollSearch(now);
 now=millis(); // Never compare fresh tuning timestamps with an older clock.
 if(audioTimerAt&&int32_t(now-audioTimerAt)>=0){
  audioTimerAt=0;audioTimerMinutes=0;cancelSearch();
  muted=true;audioSettings();message("Temporizador: audio silenciado");changed();
 }
 if(ready&&!signalSearch&&now-lastSignal>=SIGNAL_POLL_MS){
  radio.getCurrentReceivedSignalQuality(0);
  lastSignal=millis(); // Timestamp the completed sample, without catch-up bursts.
  const uint8_t r=radio.getCurrentRSSI(),s=radio.getCurrentSNR();
  recordEcg(r,s); // One historical point per completed real measurement.
  const bool st=band==0&&!fmForceMono&&radio.getCurrentStereoBlend()>0;
  if(r!=rssi||s!=snr||st!=stereo){
   rssi=r;snr=s;stereo=st;
   if(!menu)dirty=true; // Redraw only when measured values change.
  }
 }
 now=millis(); // Signal-quality reads also consume time.
 if(ready&&!signalSearch){updateSquelch(now);updateAutoFilter(now);}
 if(ready&&rdsEnabled&&!signalSearch&&band==0&&now-lastRds>=20){lastRds=now;pollRds();}
 // Advance once per displayed frame, never catch up with a multi-pixel jump.
 // Keep both copies moving through the wrap without a pause at every lap.
 bool scrollFrame=false;
 now=millis(); // Radio calls above can consume time.
 if(band==0&&!menu){
  if(rdsScrollSource!=rdsText){
   // Preserve motion while the receiver appends segments of this message.
   bool continuation=rdsScrollSource.length()&&rdsText.startsWith(rdsScrollSource);
   rdsScrollSource=rdsText;
   if(!continuation){rdsScrollOffset=0;rdsScrollAt=now+700;}
   ui.setTextSize(1);rdsScrollWidth=ui.textWidth(rdsScrollSource);
   dirty=true;
  }
  if(rdsScrollWidth>RDS_TEXT_WIDTH&&int32_t(now-rdsScrollAt)>=0&&now-lastDraw>=30){
   rdsScrollOffset=(rdsScrollOffset+1)%(max(uint16_t(RDS_TEXT_WIDTH),rdsScrollWidth)+24);
   scrollFrame=true;
  }
 }else rdsScrollAt=now+400;
 // A scan frequency is provisional; persist only after stop or restoration.
 if(saveAt&&!signalSearch&&int32_t(now-saveAt)>=0)save();
 if(notice.length()&&int32_t(now-noticeUntil)>=0){notice="";dirty=true;}
 if(now-lastBattery>=2000){lastBattery=now;pollBattery();}
 // Smooth measured instruments without synthesizing RF or microphone activity.
 if(ready&&!menu&&now-lastAnimation>=66){
  for(int n=0;n<2;n++){
   int target=constrain(int(n?snr:rssi)*100/(n?50:80),0,100)*256;
   int before=(hudVisual[n]+128)/256;
   if(!hudVisualInitialized)hudVisual[n]=target;
   else{int delta=target-hudVisual[n];hudVisual[n]+=abs(delta)<32?delta:delta/4;}
   if(before!=(hudVisual[n]+128)/256)dirty=true;
  }
  hudVisualInitialized=true;
 }
 // No delays, new buffers or radio calls for animation. Never catch up
 // missed frames after a slow operation; preserve the RDS frame cadence.
 if(ready&&(!menu||ampMenu||(!infoMenu&&!editing)|| (editing&&item==0))&&now-lastAnimation>=66){
  lastAnimation=now;
  uint8_t nextFrame=uint8_t((now/33UL)%32);
  // Repaint only when the breathing brightness or antenna wave changes.
  int oldBrightness=pulseLevels[animationFrame];
  int newBrightness=pulseLevels[nextFrame];
  if(oldBrightness!=newBrightness||(animationFrame/4)%3!=(nextFrame/4)%3||
     (menu&&animationFrame!=nextFrame))dirty=true;
  animationFrame=nextFrame;
 }
 if(infoData.weather&&((internetInfoMenu&&infoTab==0)||(!internetInfoMenu&&!wifiMenu&&!menu))){
  uint32_t frame=now/180;
  if(frame!=weatherFrame){weatherFrame=frame;dirty=true;}
 }
 if(scrollFrame||(dirty&&now-lastDraw>=33)){
  draw();dirty=false;lastDraw=millis();
  if(scrollFrame)rdsScrollAt=lastDraw+30;
 }
 delay(1);
}
