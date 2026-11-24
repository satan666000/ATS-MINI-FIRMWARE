// Read-only POTA reports. Network work stays off the receiver/UI loop.
struct HfSpot {
 char utc[20],call[17],mode[5];
 float khz;
};
struct HfReport {
 bool ok=false;
 uint8_t count=0;
 HfSpot rows[12]={};
};
static HfReport hfData;
static QueueHandle_t hfQueue=nullptr;
static bool hfBusy=false,hfFailed=false,hfConnected=false,hfOld=false;
static uint32_t hfNext=0,hfReceived=0;
static int hfOffset=0;

static void hfWorker(void*){
 HfReport report;
 {
 HTTPClient http; WiFiClientSecure tls; tls.setInsecure(); // Public, read-only data.
 http.setConnectTimeout(5000);http.setTimeout(5000);http.useHTTP10(true);
 if(http.begin(tls,"https://api.pota.app/spot/activator") && http.GET()==200 && http.getSize()<=131072){
  JsonDocument filter,doc;
  auto fields=filter[0];
  fields["activator"]=true;fields["frequency"]=true;fields["mode"]=true;fields["spotTime"]=true;
  InfoStream stream(http.getStream(),131072);
  if(!deserializeJson(doc,stream,DeserializationOption::Filter(filter),DeserializationOption::NestingLimit(5)) && doc.is<JsonArray>()){
   report.ok=true;
   for(JsonObject row:doc.as<JsonArray>()){
    const char* mode=row["mode"]|"";
    if(strcmp(mode,"SSB")&&strcmp(mode,"USB")&&strcmp(mode,"LSB")&&strcmp(mode,"AM"))continue;
    const char* freq=row["frequency"].as<const char*>();
    char* end=nullptr;
    float khz=freq?strtof(freq,&end):row["frequency"].as<float>();
    if((freq&&(end==freq||*end))||!isfinite(khz)||khz<3000||khz>=30000)continue;
    const char* utc=row["spotTime"]|"";const char* call=row["activator"]|"";
    if(strlen(utc)<19||strlen(call)==0||strspn(call,"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/")!=strlen(call))continue;
    char stamp[20];strlcpy(stamp,utc,sizeof(stamp));if(stamp[10]==' ')stamp[10]='T';
    unsigned pos=0;
    while(pos<report.count && strcmp(report.rows[pos].utc,stamp)>=0)++pos;
    if(pos>=12)continue;
    if(report.count<12)++report.count;
    for(unsigned i=report.count-1;i>pos;--i)report.rows[i]=report.rows[i-1];
    auto& spot=report.rows[pos];
    strlcpy(spot.utc,stamp,sizeof(spot.utc));strlcpy(spot.call,call,sizeof(spot.call));
    strlcpy(spot.mode,mode,sizeof(spot.mode));spot.khz=khz;
   }
  }
 }
 http.end();
 }
 xQueueOverwrite(hfQueue,&report);vTaskDelete(nullptr);
}
static void hfTick(){
 bool visible=internetInfoMenu&&infoTab==4;
 if(!visible&&!hfBusy)return;
 bool connected=WiFi.status()==WL_CONNECTED;
 bool old=hfData.ok && millis()-hfReceived>=600000;
 if(connected!=hfConnected||old!=hfOld){hfConnected=connected;hfOld=old;if(visible)dirty=true;}
 HfReport incoming;
 if(hfQueue&&xQueueReceive(hfQueue,&incoming,0)==pdTRUE){
  hfBusy=false;hfFailed=!incoming.ok;
  if(incoming.ok){hfData=incoming;hfReceived=millis();hfOld=false;hfOffset=0;}
  if(visible)dirty=true;
 }
 if(!visible||!connected||hfBusy||(hfNext&&int32_t(millis()-hfNext)<0))return;
 if(!hfQueue)hfQueue=xQueueCreate(1,sizeof(HfReport));
 hfBusy=hfQueue&&xTaskCreate(hfWorker,"hf-pota",10240,nullptr,1,nullptr)==pdPASS;
 hfFailed=!hfBusy;hfNext=millis()+120000;dirty=true;
}
static void drawHfActivity(uint16_t bg,uint16_t accent,uint16_t white,uint16_t dim){
 ui.fillScreen(bg);label(10,5,"ACTIVIDAD HF / POTA",accent);
 const char* status=!hfConnected?(hfData.ok?"Sin Internet / ultima consulta":"Sin Internet"):
  hfBusy?"Consultando reportes...":hfFailed?(hfData.ok?"POTA no disponible / ultima consulta":"POTA no disponible"):
  hfOld?"Datos antiguos":"Reportes de parques, no recepcion local";
 label(10,23,status,dim);
 ui.drawFastHLine(10,42,300,dim);
 if(hfData.ok&&!hfData.count)label(10,64,"Sin reportes HF de voz",white);
 if(hfData.count){
  label(10,47,"UTC",dim);label(91,47,"INDICATIVO",dim);label(191,47,"kHz",dim);label(271,47,"MODO",dim);
  for(int i=0;i<5&&hfOffset+i<hfData.count;++i){
   const auto& s=hfData.rows[hfOffset+i];int y=65+17*i;
   String stamp=String(s.utc+5).substring(0,11);stamp[5]=' ';
   label(10,y,stamp,dim);label(91,y,String(s.call).substring(0,15),white);
   label(191,y,String(s.khz,2),accent);label(271,y,s.mode,white);
  }
 }
 label(10,155,"Girar: lista / pulsar: volver",dim);ui.pushSprite(0,0);
}
