// Only the main loop draws or changes receiver settings; worker passes plain data.
#include <WiFiClientSecure.h>
struct InfoReply {
 bool complete=false;
 bool weather=false,kpOk=false,fluxOk=false;
 float temp=0,wind=0,kp=0,flux=0;
 int humidity=0,code=0;
 bool dayKnown=false,isDay=true;
 char weatherError[48]={};
 char city[48]={},weatherTime[24]={},kpTime[24]={},fluxTime[24]={};
};
InfoReply infoData;
QueueHandle_t infoReplies=nullptr;
bool infoEnabled=true,internetInfoMenu=false;
bool infoWasConnected=false;
int infoTab=0;
int propagationBand=0;
uint8_t propagationPage=0;
bool weatherFahrenheit=false,weatherImperial=false;
uint32_t weatherFrame=0;
uint32_t infoNext=0,infoReceived=0;
String infoStatus="Pulsar para consultar";
bool infoFetch(const String& url,JsonDocument& doc,String* error=nullptr){
 HTTPClient http; WiFiClientSecure tls; tls.setInsecure(); // Public data only, no credentials.
 http.setConnectTimeout(6000);http.setTimeout(8000);http.useHTTP10(true);
 http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
 bool begun=url.startsWith("https:")?http.begin(tls,url):http.begin(url);
 if(!begun){if(error)*error="No se pudo iniciar consulta";return false;}
 int code=http.GET();int size=http.getSize();bool ok=false;
 if(code==200 && (size<0 || size<=65536)){
  InfoStream stream(http.getStream());
  auto e=deserializeJson(doc,stream,DeserializationOption::NestingLimit(8));
  ok=!e;if(error&&!ok)*error=String("Respuesta: ")+e.c_str();
 }else if(error)*error=code>0?"HTTP "+String(code):"Conexion / DNS / tiempo agotado";
 http.end();return ok;
}
static String solarField(const String& xml,const char* tag){
 String open=String("<")+tag+">",close=String("</")+tag+">";
 int a=xml.indexOf(open);if(a<0)return "";
 a+=open.length();int b=xml.indexOf(close,a);
 String value=b<0?String():xml.substring(a,b);value.trim();return value;
}
static void fetchHamQsl(InfoReply& r){
 HTTPClient http;WiFiClientSecure tls;tls.setInsecure();
 http.setConnectTimeout(6000);http.setTimeout(8000);http.useHTTP10(true);
 if(!http.begin(tls,"https://www.hamqsl.com/solarxml.php"))return;
 if(http.GET()==200 && http.getSize()<=8192){
  String xml;xml.reserve(4096);
  auto& stream=http.getStream();
  uint32_t started=millis();int remaining=http.getSize();
  while(remaining!=0 && xml.length()<8192 && millis()-started<8000){
   if(stream.available()){int c=stream.read();if(c>=0){xml+=char(c);if(remaining>0)--remaining;}}
   else if(!http.connected())break;else delay(5);
   if(xml.endsWith("</solar>"))break;
  }
  if(xml.indexOf("</solardata>")>=0){
   String date=solarField(xml,"updated"),k=solarField(xml,"kindex"),f=solarField(xml,"solarflux");
   int d,y,hm;char mon[4]={},zone[4]={},stamp[24]={};
   const char* months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
   if(sscanf(date.c_str(),"%d %3s %d %d %3s",&d,mon,&y,&hm,zone)==5 && !strcmp(zone,"GMT")){
    for(int m=0;m<12;m++)if(!strcmp(mon,months[m])&& y>=2020&&y<=2100&&d>=1&&d<=31&&hm>=0&&hm/100<24&&hm%100<60)
     snprintf(stamp,sizeof(stamp),"%04d-%02d-%02dT%02d:%02d",y,m+1,d,hm/100,hm%100);
   }
   char* end;const char* value=k.c_str();float kv=strtof(value,&end);
   if(end!=value&&!*end&&kv>=0&&kv<=9){r.kp=kv;r.kpOk=true;strlcpy(r.kpTime,stamp,sizeof(r.kpTime));}
   value=f.c_str();float fv=strtof(value,&end);
   if(end!=value&&!*end&&fv>0&&fv<1000){r.flux=fv;r.fluxOk=true;strlcpy(r.fluxTime,stamp,sizeof(r.fluxTime));}
  }
 }
 http.end();
}
uint32_t hamQslNext=0;
void infoWorker(void* solar){
 InfoReply result; JsonDocument doc; String error;
 strlcpy(result.weatherError,"Ubicacion no disponible",sizeof(result.weatherError));
 if(infoFetch("https://ipwho.is/?fields=success,city,latitude,longitude",doc,&error)&&doc["success"]==true && doc["latitude"].is<double>() && doc["longitude"].is<double>()){
  double lat=doc["latitude"],lon=doc["longitude"];
  strlcpy(result.city,doc["city"]|"Ubicacion por IP",sizeof(result.city));
  if(isfinite(lat)&&isfinite(lon)&&abs(lat)<=90&&abs(lon)<=180){
   String url="https://api.open-meteo.com/v1/forecast?latitude="+String(lat,4)+"&longitude="+String(lon,4)+"&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m,is_day&timezone=UTC";
   doc.clear();
   if(infoFetch(url,doc,&error)){
    strlcpy(result.weatherError,"Formato de clima inesperado",sizeof(result.weatherError));
    JsonObject c=doc["current"];
    if(c["temperature_2m"].is<float>() && c["relative_humidity_2m"].is<int>() && c["weather_code"].is<int>() && c["wind_speed_10m"].is<float>() && c["time"].is<const char*>()){
     result.dayKnown=c["is_day"].is<int>() && (c["is_day"].as<int>()==0 || c["is_day"].as<int>()==1);
     result.isDay=c["is_day"].as<int>()==1;
     result.temp=c["temperature_2m"];result.humidity=c["relative_humidity_2m"];result.code=c["weather_code"];result.wind=c["wind_speed_10m"];
     strlcpy(result.weatherTime,c["time"],sizeof(result.weatherTime));
     result.weather=isfinite(result.temp)&&isfinite(result.wind)&&result.humidity>=0&&result.humidity<=100;
    }
   }else strlcpy(result.weatherError,("Clima: "+error).c_str(),sizeof(result.weatherError));
  }
 }else if(error.length())strlcpy(result.weatherError,("Ubicacion: "+error).c_str(),sizeof(result.weatherError));
 xQueueOverwrite(infoReplies,&result); // Show weather immediately; solar queries continue.
 if(solar)fetchHamQsl(result);
 result.complete=true;
 xQueueOverwrite(infoReplies,&result);vTaskDelete(nullptr);
}
void infoTick(){
 bool connected=WiFi.status()==WL_CONNECTED;
 if(connected&&!infoWasConnected)infoNext=0; // Every reconnection refreshes automatically.
 infoWasConnected=connected;
 if(!infoReplies)infoReplies=xQueueCreate(1,sizeof(InfoReply));
 InfoReply r;
 if(infoReplies&&xQueueReceive(infoReplies,&r,0)==pdTRUE){
  infoRunning=!r.complete;
  // A failed refresh must not erase the last successful observations.
  if(r.weather){
   infoData.weather=true;infoData.temp=r.temp;infoData.wind=r.wind;
   infoData.humidity=r.humidity;infoData.code=r.code;
   infoData.dayKnown=r.dayKnown;infoData.isDay=r.isDay;
   strlcpy(infoData.city,r.city,sizeof(infoData.city));
   strlcpy(infoData.weatherTime,r.weatherTime,sizeof(infoData.weatherTime));
   infoData.weatherError[0]=0;infoReceived=millis();
  }
  else {strlcpy(infoData.weatherError,r.weatherError,sizeof(infoData.weatherError));}
  if(r.kpOk){infoData.kp=r.kp;infoData.kpOk=true;strlcpy(infoData.kpTime,r.kpTime,sizeof(infoData.kpTime));}
  if(r.fluxOk){infoData.flux=r.flux;infoData.fluxOk=true;strlcpy(infoData.fluxTime,r.fluxTime,sizeof(infoData.fluxTime));}
  infoStatus=r.weather&&r.kpOk&&r.fluxOk?"Datos recibidos":"Consulta parcial / servicio no disponible";
  if(r.complete)infoNext=millis()+((r.weather&&r.kpOk&&r.fluxOk)?600000UL:300000UL);dirty=true;
 }
 if(infoEnabled&&!infoRunning&&infoReplies&&WiFi.status()==WL_CONNECTED&&int32_t(millis()-infoNext)>=0&&!timeQueryRunning){
  infoRunning=true;infoStatus="Consultando en segundo plano";
  bool solar=!hamQslNext||int32_t(millis()-hamQslNext)>=0;
  if(xTaskCreate(infoWorker,"info-http",12288,solar?static_cast<void*>(&hamQslNext):nullptr,1,nullptr)!=pdPASS){infoRunning=false;infoStatus="Sin memoria / reintento";infoNext=millis()+300000UL;}
  else if(solar)hamQslNext=millis()+3600000UL;
  dirty=true;
 }
}
const char* weatherName(int c){
 if(c==0)return "Despejado";if(c<=3)return "Nubes";if(c==45||c==48)return "Niebla";
 if(c>=95)return "Tormenta";if((c>=71&&c<=77)||c==85||c==86)return "Nieve";
 if(c>=51&&c<=82)return "Lluvia / llovizna";return "Condicion no clasificada";
}
// Check the provider timestamp too: recent downloads can contain old observations.
bool infoOld(const char* stamp,int hours){
 if(!stamp[0])return true;
 time_t now=time(nullptr);if(now<1700000000)return true;
 int y,m,d,h,mi;
 if(sscanf(stamp,"%d-%d-%dT%d:%d",&y,&m,&d,&h,&mi)!=5 &&
    sscanf(stamp,"%d-%d-%d %d:%d",&y,&m,&d,&h,&mi)!=5)return true;
 if(y<2020||y>2100||m<1||m>12||d<1||d>31||h<0||h>23||mi<0||mi>59)return true;
 // Convert the Gregorian date directly to UTC; never let a local TZ offset
 // silently change the age of HamQSL/Open-Meteo observations.
 bool leap=(y%4==0&&(y%100!=0||y%400==0));
 const int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
 if(d>days[m-1]+(m==2&&leap))return true;
 int yy=y-(m<=2),era=yy/400;
 unsigned yoe=unsigned(yy-era*400),mp=unsigned(m+(m>2?-3:9));
 unsigned doy=(153*mp+2)/5+unsigned(d)-1;
 unsigned doe=yoe*365+yoe/4-yoe/100+doy;
 int64_t observation=(int64_t(era)*146097+doe-719468)*86400+h*3600+mi*60;
 return int64_t(now)-observation>int64_t(hours)*3600 || observation>int64_t(now)+3600;
}
// WMO condition codes supplied by Open-Meteo; animation is decorative.
void drawWeatherIcon(int x,int y,int code,uint16_t bg,uint16_t accent,uint16_t white,uint16_t dim){
 int phase=weatherFrame%4;
 bool snow=(code>=71&&code<=77)||code==85||code==86;
 bool storm=code>=95;
 bool rain=(code>=51&&code<=67)||(code>=80&&code<=82)||storm;
 // Do not guess daylight from the receiver clock or an outdated observation.
 if(infoData.dayKnown&&!infoOld(infoData.weatherTime,2)){
  if(infoData.isDay){
   ui.fillCircle(x+14,y+13,7,accent);
   for(int i=0;i<8;i++){float a=(i*45+phase*5)*0.0174533f;
    ui.drawLine(x+14+int(cosf(a)*10),y+13+int(sinf(a)*10),x+14+int(cosf(a)*14),y+13+int(sinf(a)*14),accent);}
  }else{
   ui.fillCircle(x+14,y+13,9,white);ui.fillCircle(x+18,y+9,8,bg);
   ui.drawPixel(x+30,y+3,white);ui.drawPixel(x+39,y+9,white);
  }
 }else if(code==0){label(x+10,y+8,"?",dim);}
 if(code!=0){
  ui.fillCircle(x+19,y+18,7,white);ui.fillCircle(x+28,y+15,9,white);
  ui.fillCircle(x+37,y+20,6,white);ui.fillRoundRect(x+14,y+19,29,9,4,white);
 }
 if(code==45||code==48){for(int i=0;i<3;i++)ui.drawFastHLine(x+9+(phase+i)%3,y+31+i*4,34,dim);}
 if(rain||snow){for(int i=0;i<3;i++){int px=x+18+i*10,py=y+31+(phase+i)%4*2;
  if(snow){ui.drawFastHLine(px-2,py,5,white);ui.drawFastVLine(px,py-2,5,white);}
  else ui.drawLine(px,py,px-2,py+4,accent);
 }}
 if(storm){ui.fillTriangle(x+28,y+25,x+23,y+34,x+30,y+32,accent);ui.fillTriangle(x+27,y+31,x+32,y+30,x+25,y+40,accent);}
}
// Compact 16x16 header icon. Motion is decorative, not a live measurement.
void drawHeaderWeather(int x,int y,uint16_t bg,uint16_t accent,uint16_t white,uint16_t dim){
 if(!infoData.weather||infoOld(infoData.weatherTime,2)){
  ui.drawCircle(x+7,y+7,5,dim);
  ui.drawFastHLine(x+4,y+7,7,dim);
  return;
 }
 int c=infoData.code,phase=weatherFrame%4;
 bool snow=(c>=71&&c<=77)||c==85||c==86;
 bool storm=c>=95;
 bool rain=(c>=51&&c<=67)||(c>=80&&c<=82)||storm;
 uint16_t ink=WiFi.status()==WL_CONNECTED?white:dim;
 if(c==0||c==1||c==2){
  if(infoData.dayKnown&&infoData.isDay){
   ui.fillCircle(x+6,y+6,3,accent);
   for(int i=0;i<4;i++){
    float a=(i*90+phase*12)*0.0174533f;
    ui.drawLine(x+6+int(cosf(a)*5),y+6+int(sinf(a)*5),x+6+int(cosf(a)*6),y+6+int(sinf(a)*6),accent);
   }
  }else if(infoData.dayKnown){
   ui.fillCircle(x+6,y+6,4,ink);ui.fillCircle(x+8,y+4,3,bg);
   ui.drawPixel(x+13,y+3+(phase%2),ink);
  }else ui.drawCircle(x+6,y+6,3,dim);
 }
 if(c!=0){
  ui.fillCircle(x+5,y+8,3,ink);ui.fillCircle(x+9,y+6,4,ink);
  ui.fillRoundRect(x+2,y+8,13,4,2,ink);
 }
 if(c==45||c==48){ui.drawFastHLine(x+2+phase%2,y+13,11,dim);ui.drawFastHLine(x+3,y+15,10,dim);}
 if(rain||snow)for(int i=0;i<2;i++){
  int px=x+5+i*6,py=y+12+(phase+i)%3;
  if(snow){ui.drawFastHLine(px-1,py,3,ink);ui.drawFastVLine(px,py-1,3,ink);}
  else ui.drawLine(px,py,px-1,py+1,accent);
 }
 if(storm)ui.fillTriangle(x+8,y+10,x+6,y+15,x+10,y+12,accent);
}
// Band atlas is a qualitative guide, never a coverage forecast.
const char* propBands[]={"80m","40m","30m","20m","17m","15m","12m","10m","CB"};
const char* propFreq[]={"3.5 MHz","7 MHz","10 MHz","14 MHz","18 MHz","21 MHz","24 MHz","28 MHz","27 MHz AM"};
void drawPropagation(uint16_t white,uint16_t dim){
 const uint16_t cyan=ui.color565(72,210,235),panel=ui.color565(15,29,43);
 const uint16_t amber=ui.color565(245,184,80),night=ui.color565(119,145,210);
 bool kFresh=infoData.kpOk&&!infoOld(infoData.kpTime,6);
 bool fFresh=infoData.fluxOk&&!infoOld(infoData.fluxTime,72);
 ui.fillRoundRect(8,38,304,24,4,panel);
 label(15,45,"K "+(infoData.kpOk?String(infoData.kp,1):String("--")),kFresh?cyan:dim);
 label(94,45,"SFI "+(infoData.fluxOk?String(infoData.flux,0):String("--")),fFresh?cyan:dim);
 label(190,45,!kFresh?"SIN K ACTUAL":infoData.kp>=5?"PERTURBADO":infoData.kp>=4?"ACTIVO":"TRANQUILO",kFresh&&infoData.kp>=4?amber:dim);
 if(propagationPage==0){
  // Two compact rows of band tiles; selected band has a crisp cyan frame.
  for(int i=0;i<9;i++){
   int x=8+(i%5)*61,y=68+(i/5)*29;
   ui.fillRoundRect(x,y,57,25,3,panel);
   if(i==propagationBand)ui.drawRoundRect(x,y,57,25,3,cyan);
   label(x+5,y+3,propBands[i],i==propagationBand?white:dim);
   label(x+5,y+14,i<2?"NOCHE":i<4?"DIA/NOC":"DIA*",i<2?night:amber);
  }
  label(257,101,"* variable",dim);
  label(10,130,String(propBands[propagationBand])+" / "+propFreq[propagationBand],cyan);
  label(10,143,"Guia habitual; NO cobertura ni MUF",dim);
 }else{
  label(10,70,"HAMQSL / N0NBH / UTC",cyan);
  label(10,87,"K  "+(infoData.kpOk?String(infoData.kpTime).substring(0,16):String("Sin datos")),white);
  label(10,102,"SFI "+(infoData.fluxOk?String(infoData.fluxTime).substring(0,16):String("Sin datos")),white);
  label(10,117,(!kFresh||!fFresh)?"Datos antiguos / hora sin verificar":"Fechas dentro del plazo de vigencia",(!kFresh||!fFresh)?amber:dim);
  label(10,131,"Kp: actividad global; SFI: flujo solar",dim);
  label(10,144,"Sin modelo de rutas ni pronostico DX",dim);
 }
}
void drawInternetInfo(uint16_t bg,uint16_t accent,uint16_t white,uint16_t dim){
 ui.fillScreen(bg);label(10,5,infoTab==0?"CLIMA / OPEN-METEO":infoTab==1?"PROPAGACION / NOAA":"UNIDADES DEL CLIMA",accent);
 bool connected=WiFi.status()==WL_CONNECTED;
 label(10,23,!connected?"Sin Internet / conserva ultimos datos":infoRunning?"Consultando...":infoTab==1?String(propagationPage?"02 / DATOS Y LIMITES":"01 / ATLAS DE BANDAS"):infoStatus.substring(0,43),dim);
 if(infoTab==0){
  if(infoData.weather){
   label(10,42,String(infoData.city).substring(0,30)+" (IP)",white);
   drawWeatherIcon(263,39,infoData.code,bg,accent,white,dim);
   float displayedTemp=weatherFahrenheit?infoData.temp*1.8f+32:infoData.temp;
   label(10,61,String(displayedTemp,1)+(weatherFahrenheit?" F   ":" C   ")+weatherName(infoData.code),accent);
   label(10,81,"Humedad "+String(infoData.humidity)+"% / viento "+String(weatherImperial?infoData.wind/1.609344f:infoData.wind,1)+(weatherImperial?" mph":" km/h"),white);
   label(10,101,String(infoData.weatherTime)+" UTC",dim);
   label(10,119,infoOld(infoData.weatherTime,2)?"Datos antiguos / hora sin verificar":"Modelo meteorologico / no sensor local",dim);
  }else {label(10,62,"Sin datos de clima disponibles",white);
   label(10,82,String(infoData.weatherError).substring(0,47),dim);
   label(10,102,"Reintento automatico en 5 minutos",dim);}
  label(10,139,"Ubicacion aproximada; VPN puede alterarla",dim);
 }else if(infoTab==1){
  drawPropagation(white,dim);
 }

 if(infoTab==2||infoTab==3){
  label(10,48,infoTab==2?"Temperatura":"Sistema de medidas",white);
  label(10,72,infoTab==2?(weatherFahrenheit?"Fahrenheit (F)":"Centigrados (C)"):(weatherImperial?"Estadounidense: F / mph":"Metrico: C / km/h"),accent);
  label(10,100,"Pulsar: cambiar y guardar",white);
  label(10,120,"Girar: otra pagina / mantener: salir",dim);
 }
 label(10,158,infoTab==1?"Girar: banda / pulsar: pagina / largo: salir":(infoTab==2||infoTab==3)?"Unidades guardadas al apagar":"Girar: pagina / pulsar: volver",dim);ui.pushSprite(0,0);
}
