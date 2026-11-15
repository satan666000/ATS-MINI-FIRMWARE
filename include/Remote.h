#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <NimBLEDevice.h>
#include <freertos/queue.h>
bool wifiEnabled=false,bleEnabled=false,bleInitialized=false;
WebServer web(80);
QueueHandle_t commands=nullptr;
NimBLECharacteristic* bleStatus=nullptr;
char remoteKey[9]={};
struct RemoteCommand {char text[80];};
const char* executeRemote(const char* text);
void statusJson(char* out,size_t size){
 if(!out||size==0)return;
 const uint8_t safeBand=band<BAND_COUNT?band:0;
 const uint8_t safeMode=(safeBand==0||safeBand==CB_BAND)?0:(mode<=2?mode:0);
 const uint16_t safeFrequency=frequencies[safeBand];
 const int safeChannel=safeBand==CB_BAND?cbChannel(safeFrequency)+1:0;
 const bool safeAutoVfo=safeBand!=0&&safeBand!=CB_BAND&&safeMode!=0&&patchLoaded&&afcEnabled;
 snprintf(out,size,"{\"band\":%u,\"frequency\":%u,\"mode\":%u,\"volume\":%u,\"mute\":%u,\"rssi\":%u,\"snr\":%u,\"channel\":%d,\"bfo\":%d,\"squelch\":%u,\"squelch_closed\":%u,\"squelch_mode\":%u,\"squelch_threshold\":%u,\"auto_vfo_enabled\":%u,\"auto_vfo_active\":%u}",safeBand,safeFrequency,safeMode,volume,muted,rssi,snr,safeChannel,bfo,squelch,squelchClosed,squelchMode,squelchMode==2?autoSquelchThreshold:squelch,afcEnabled,safeAutoVfo);
}
class RemoteWrites:public NimBLECharacteristicCallbacks{
 void onWrite(NimBLECharacteristic* c,NimBLEConnInfo&) override{
  auto v=c->getValue(); if(v.size()==0||v.size()>=80||!commands)return;
  // Reject embedded terminators instead of executing a truncated command.
  if(memchr(v.data(),0,v.size()))return;
  RemoteCommand cmd{};memcpy(cmd.text,v.data(),v.size());xQueueSend(commands,&cmd,0);
 }
} remoteWrites;
const char remotePage[] PROGMEM=R"HTML(<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>ATS MINI</title><style>body{background:#081529;color:#eef;font:18px sans-serif;max-width:650px;margin:30px auto;padding:12px}button,input,select{font:inherit;margin:6px;padding:10px}pre{white-space:pre-wrap}</style><h1>ATS MINI · Control</h1><p><button onclick="send('SEARCH 1')">Buscar señal</button><button onclick="send('SEARCH 0')">Cancelar búsqueda</button></p><p><input id="tm" type="number" min="0" max="120" placeholder="Minutos (0 cancela)"><button onclick="send('TIMER '+tm.value)">Temporizador de silencio</button></p><p>Frecuencia: FM en unidades de 10 kHz (9950 = 99,50 MHz). AM/SSB en kHz. CB usa CH 1–40.</p><select id="b"></select><button onclick="send('BAND '+b.value)">Elegir banda</button><p><input id="f" type="number" placeholder="Frecuencia"><button onclick="send('FREQ '+f.value)">Sintonizar</button></p><button onclick="send('UP')">+</button><button onclick="send('DOWN')">−</button><p><input id="v" type="number" min="0" max="63" placeholder="Volumen 0–63"><button onclick="send('VOL '+v.value)">Volumen</button><button onclick="send('MUTE 1')">Silencio</button><button onclick="send('MUTE 0')">Audio</button></p><select id="m"><option value="0">AM</option><option value="1">LSB</option><option value="2">USB</option></select><button onclick="send('MODE '+m.value)">Modo</button><p><input id="ch" type="number" min="1" max="40" placeholder="Canal CB"><button onclick="send('CH '+ch.value)">CB</button></p><input id="bf" type="number" placeholder="BFO Hz"><button onclick="send('BFO '+bf.value)">BFO</button><button onclick="send('AFC 1')">Auto VFO sí</button><button onclick="send('AFC 0')">Auto VFO no</button><p><p><select id="sm"><option value="0">Desactivado</option><option value="1">Manual</option><option value="2">Automático</option></select><button onclick="send('SQLMODE '+sm.value)">Modo squelch</button><input id="sql" type="number" min="0" max="80" placeholder="Squelch 0–80"><button onclick="send('SQL '+sql.value)">Nivel manual (0 = no)</button></p><p><input id="mem" type="number" min="1" max="10" placeholder="Memoria 1–10"><button onclick="send('STORE '+mem.value)">Guardar</button><button onclick="send('RECALL '+mem.value)">Recuperar</button></p><input id="cmd" maxlength="70" placeholder="HUD 12 / THEME 12 / AMP 1"><button onclick="send(cmd.value)">Enviar comando</button></p><pre id="s"></pre><script>async function send(c){let r=await fetch('/cmd',{method:'POST',body:new URLSearchParams({c})});s.textContent=await r.text()}fetch('/bands').then(r=>r.json()).then(a=>a.forEach((n,i)=>b.add(new Option(n,i))));setInterval(()=>fetch('/status').then(r=>r.json()).then(j=>s.textContent=JSON.stringify(j,null,2)),1500)</script>)HTML";
void remoteSetup(){
 snprintf(remoteKey,sizeof(remoteKey),"%08lX",(unsigned long)esp_random());
 commands=xQueueCreate(8,sizeof(RemoteCommand));
 web.on("/",HTTP_GET,[]{web.send_P(200,"text/html; charset=utf-8",remotePage);});
 web.on("/status",HTTP_GET,[]{char out[384];statusJson(out,sizeof(out));web.send(200,"application/json",out);});
 web.on("/bands",HTTP_GET,[]{String out;size_t capacity=3;for(int i=0;i<BAND_COUNT;i++)capacity+=strlen(bands[i].name)+3;
 if(!out.reserve(capacity)){web.send(503,"text/plain","Sin memoria");return;}out="[";for(int i=0;i<BAND_COUNT;i++){if(i)out+=',';out+='\"';out+=bands[i].name;out+='\"';}out+=']';web.send(200,"application/json",out);});
 web.on("/cmd",HTTP_POST,[]{String c=web.arg("c");if(c.length()>=80||memchr(c.c_str(),0,c.length())){web.send(400,"text/plain","Comando demasiado largo");return;}const char* result=executeRemote(c.c_str());web.send(strcmp(result,"OK")?400:200,"text/plain",result);});
}
void toggleWifi(){
 wifiEnabled=!wifiEnabled;
 if(wifiEnabled){WiFi.mode(WIFI_AP);if(!WiFi.softAP("ATS-MINI",remoteKey,1,false,2)){wifiEnabled=false;WiFi.softAPdisconnect(true);WiFi.mode(WIFI_OFF);message("Error WiFi");dirty=true;return;}web.begin();}
 else{web.stop();WiFi.softAPdisconnect(true);WiFi.mode(WIFI_OFF);}
 dirty=true;
}
void toggleBle(){
 if(!commands){message("Sin memoria para BLE");return;}
 if(!bleInitialized){
  NimBLEDevice::init("ATS-MINI");auto server=NimBLEDevice::createServer();
  auto svc=server->createService("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
  auto rx=svc->createCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e",NIMBLE_PROPERTY::WRITE,80);rx->setCallbacks(&remoteWrites);
  bleStatus=svc->createCharacteristic("6e400003-b5a3-f393-e0a9-e50e24dcca9e",NIMBLE_PROPERTY::READ|NIMBLE_PROPERTY::NOTIFY,384);
  svc->start();auto adv=NimBLEDevice::getAdvertising();adv->setName("ATS-MINI");adv->addServiceUUID(svc->getUUID());bleInitialized=true;
 }
 // Never replay commands queued before this control session was toggled.
 xQueueReset(commands);
 bleEnabled=!bleEnabled;
 if(bleEnabled)NimBLEDevice::getAdvertising()->start();else{NimBLEDevice::getAdvertising()->stop();auto server=NimBLEDevice::getServer();for(auto h:server->getPeerDevices())server->disconnect(h);}
 dirty=true;
}
void remotePoll(uint32_t now){
 if(wifiEnabled)web.handleClient();
 RemoteCommand cmd{};if(commands&&xQueueReceive(commands,&cmd,0)==pdTRUE&&bleEnabled){
  // BLE requires the session key shown on the radio: KEY COMMAND.
  if(!strncmp(cmd.text,remoteKey,8)&&cmd.text[8]==' '){const char* result=executeRemote(cmd.text+9);bleStatus->setValue(result);bleStatus->notify();}
 }
 static uint32_t last=0;
 if(bleEnabled&&now-last>=1500){last=now;char out[384];statusJson(out,sizeof(out));bleStatus->setValue(out);bleStatus->notify();if(!NimBLEDevice::getAdvertising()->isAdvertising()&&NimBLEDevice::getServer()->getConnectedCount()==0)NimBLEDevice::getAdvertising()->start();}
}
