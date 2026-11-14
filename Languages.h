#pragma once
// ASCII labels match the display's built-in font. Radio data is not translated.
uint8_t language=0;
constexpr int LANGUAGE_COUNT=7;
const char* languageNames[]={"Espanol","English","Portugues","Francais","Deutsch","Italiano","Nederlands"};
struct Translation { const char* text[LANGUAGE_COUNT]; };
const Translation translations[]={
 {{"Pulsar: pausa/audio / Rueda: sintonia","Press: pause/audio / Wheel: tune","Pressionar: pausa/audio / Roda: sintonia","Appui: pause/audio / Roue: accord","Druck: Pause/Audio / Rad: Abstimmen","Premi: pausa/audio / Ruota: sintonia","Druk: pauze/audio / Draai: afstemmen"}},
 {{"Interfaz: espanol / Solo recepcion","Interface: English / Receive only","Interface: portugues / So recepcao","Interface: francais / Reception seule","Sprache: Deutsch / Nur Empfang","Interfaccia: italiano / Solo ricezione","Taal: Nederlands / Alleen ontvangst"}},
 {{"Pulsar: aceptar / Mantener: salir","Press: accept / Hold: exit","Pressionar: aceitar / Segurar: sair","Appui: valider / Long: quitter","Druck: OK / Halten: Ende","Premi: conferma / Tieni: esci","Druk: bevestig / Houd: sluiten"}},
 {{"Girar: cambiar / Pulsar: aceptar","Turn: change / Press: accept","Girar: mudar / Pressionar: aceitar","Tourner: modifier / Appui: valider","Drehen: aendern / Druck: OK","Ruota: cambia / Premi: conferma","Draai: wijzigen / Druk: bevestig"}},
 {{"RUEDA: sintonia / Mantener: menu","WHEEL: tune / Hold: menu","RODA: sintonia / Segurar: menu","ROUE: accord / Long: menu","RAD: Abstimmen / Halten: Menue","RUOTA: sintonia / Tieni: menu","DRAAI: afstemmen / Houd: menu"}},
 {{"RUEDA: volumen / Mantener: menu","WHEEL: volume / Hold: menu","RODA: volume / Segurar: menu","ROUE: volume / Long: menu","RAD: Lautstaerke / Halten: Menue","RUOTA: volume / Tieni: menu","DRAAI: volume / Houd: menu"}},
 {{"Girar: pagina / Pulsar: volver","Turn: page / Press: back","Girar: pagina / Pressionar: voltar","Tourner: page / Appui: retour","Drehen: Seite / Druck: zurueck","Ruota: pagina / Premi: indietro","Draai: pagina / Druk: terug"}},
 {{"INFORMACION / FIRMWARE","INFORMATION / FIRMWARE","INFORMACAO / FIRMWARE","INFORMATION / FIRMWARE","INFORMATION / FIRMWARE","INFORMAZIONI / FIRMWARE","INFORMATIE / FIRMWARE"}},
 {{"AMP / SALIDA DE AUDIO","AMP / AUDIO OUTPUT","AMP / SAIDA DE AUDIO","AMP / SORTIE AUDIO","AMP / AUDIOAUSGANG","AMP / USCITA AUDIO","AMP / AUDIOUITGANG"}},
 {{"Pulsar para regresar","Press to return","Pressionar para voltar","Appuyer pour revenir","Druecken fuer zurueck","Premi per tornare","Druk om terug te gaan"}},
 {{"ATS / CONFIGURACION","ATS / SETTINGS","ATS / AJUSTES","ATS / REGLAGES","ATS / EINSTELLUNGEN","ATS / IMPOSTAZIONI","ATS / INSTELLINGEN"}},
 {{"INFORMACION / RADIO","INFORMATION / RADIO","INFORMACAO / RADIO","INFORMATION / RADIO","INFORMATION / RADIO","INFORMAZIONI / RADIO","INFORMATIE / RADIO"}},
 {{"Iniciando radio...","Starting radio...","Iniciando radio...","Demarrage radio...","Radio startet...","Avvio radio...","Radio starten..."}},
 {{"Radio no detectada","Radio not detected","Radio nao detectado","Radio non detectee","Radio nicht erkannt","Radio non rilevata","Radio niet gevonden"}},
 {{"BANDAS / CARPETAS","BANDS / FOLDERS","BANDAS / PASTAS","BANDES / DOSSIERS","BAENDER / ORDNER","BANDE / CARTELLE","BANDEN / MAPPEN"}},
 {{"RSSI / INTENSIDAD","RSSI / STRENGTH","RSSI / INTENSIDADE","RSSI / PUISSANCE","RSSI / STAERKE","RSSI / INTENSITA","RSSI / STERKTE"}},
 {{"Auto VFO / AFC","Auto VFO / AFC","Auto VFO / AFC","Auto VFO / AFC","Auto VFO / AFC","Auto VFO / AFC","Auto VFO / AFC"}},
 {{"Memoria guardada","Memory saved","Memoria salva","Memoire sauvee","Speicher gesichert","Memoria salvata","Geheugen opgeslagen"}},
 {{"Nivel automatico","Automatic level","Nivel automatico","Niveau automatique","Automatischer Pegel","Livello automatico","Automatisch niveau"}},
 {{"Guardar memoria","Save memory","Salvar memoria","Sauver memoire","Speicher sichern","Salva memoria","Geheugen opslaan"}},
 {{"Cargando SSB...","Loading SSB...","Carregando SSB...","Chargement SSB...","SSB wird geladen...","Caricamento SSB...","SSB laden..."}},
 {{"Memoria cargada","Memory loaded","Memoria carregada","Memoire chargee","Speicher geladen","Memoria caricata","Geheugen geladen"}},
 {{"AGC automatico","Automatic AGC","AGC automatico","AGC automatique","AGC automatisch","AGC automatico","Automatische AGC"}},
 {{"Cargar memoria","Load memory","Carregar memoria","Charger memoire","Speicher laden","Carica memoria","Geheugen laden"}},
 {{"BARRIDO / MUDO","SCAN / MUTED","VARREDURA / MUDO","BALAYAGE / MUET","SCAN / STUMM","SCANSIONE / MUTO","SCANNEN / STIL"}},
 {{"Filtro AM/SSB","AM/SSB filter","Filtro AM/SSB","Filtre AM/SSB","AM/SSB Filter","Filtro AM/SSB","AM/SSB filter"}},
 {{"Squelch nivel","Squelch level","Nivel squelch","Niveau squelch","Squelch Pegel","Livello squelch","Squelchniveau"}},
 {{"CONFIGURACION","SETTINGS","AJUSTES","REGLAGES","EINSTELLUNGEN","IMPOSTAZIONI","INSTELLINGEN"}},
 {{"EDITAR AJUSTE","EDIT SETTING","EDITAR AJUSTE","MODIFIER REGLAGE","EINSTELLUNG AENDERN","MODIFICA IMPOSTAZIONE","INSTELLING WIJZIGEN"}},
 {{"RECEPCION SSB","SSB RECEPTION","RECEPCAO SSB","RECEPTION SSB","SSB EMPFANG","RICEZIONE SSB","SSB ONTVANGST"}},
 {{"SNR / CALIDAD","SNR / QUALITY","SNR / QUALIDADE","SNR / QUALITE","SNR / QUALITAET","SNR / QUALITA","SNR / KWALITEIT"}},
 {{"PAUSA / AUDIO","PAUSE / AUDIO","PAUSA / AUDIO","PAUSE / AUDIO","PAUSE / AUDIO","PAUSA / AUDIO","PAUZE / AUDIO"}},
 {{"WiFi control","WiFi control","Controle WiFi","Commande WiFi","WiFi Steuerung","Controllo WiFi","WiFi bediening"}},
 {{"Clave remota","Remote key","Chave remota","Cle distante","Fernzugriffsschluessel","Chiave remota","Externe sleutel"}},
 {{"Squelch modo","Squelch mode","Modo squelch","Mode squelch","Squelch Modus","Modo squelch","Squelchmodus"}},
 {{"Amplificador","Amplifier","Amplificador","Amplificateur","Verstaerker","Amplificatore","Versterker"}},
 {{"RECEPCION AM","AM RECEPTION","RECEPCAO AM","RECEPTION AM","AM EMPFANG","RICEZIONE AM","AM ONTVANGST"}},
 {{"BLE control","BLE control","Controle BLE","Commande BLE","BLE Steuerung","Controllo BLE","BLE bediening"}},
 {{"Informacion","Information","Informacao","Information","Information","Informazioni","Informatie"}},
 {{"Procesador:","Processor:","Processador:","Processeur:","Prozessor:","Processore:","Processor:"}},
 {{"Desactivado","Disabled","Desativado","Desactive","Deaktiviert","Disattivato","Uitgeschakeld"}},
 {{"Solo AM/SSB","AM/SSB only","Somente AM/SSB","AM/SSB seulement","Nur AM/SSB","Solo AM/SSB","Alleen AM/SSB"}},
 {{"Atenuacion","Attenuation","Atenuacao","Attenuation","Daempfung","Attenuazione","Verzwakking"}},
 {{"Recepcion:","Reception:","Recepcao:","Reception:","Empfang:","Ricezione:","Ontvangst:"}},
 {{"Compilado:","Built:","Compilado:","Compile:","Erstellt:","Compilato:","Gebouwd:"}},
 {{"Automatico","Automatic","Automatico","Automatique","Automatisch","Automatico","Automatisch"}},
 {{"Encendido","On","Ligado","Active","Ein","Acceso","Aan"}},
 {{"Receptor:","Receiver:","Receptor:","Recepteur:","Empfaenger:","Ricevitore:","Ontvanger:"}},
 {{"Pantalla:","Display:","Tela:","Ecran:","Display:","Schermo:","Scherm:"}},
 {{"BFO fino","Fine BFO","BFO fino","BFO fin","BFO fein","BFO fine","Fijne BFO"}},
 {{"Silencio","Mute","Silencio","Muet","Stumm","Muto","Stil"}},
 {{"Creador:","Creator:","Criador:","Createur:","Autor:","Autore:","Maker:"}},
 {{"Solo SSB","SSB only","Somente SSB","SSB seulement","Nur SSB","Solo SSB","Alleen SSB"}},
 {{"Volumen","Volume","Volume","Volume","Lautstaerke","Volume","Volume"}},
 {{"Memoria","Memory","Memoria","Memoire","Speicher","Memoria","Geheugen"}},
 {{"Apagado","Off","Desligado","Desactive","Aus","Spento","Uit"}},
 {{"Modelo:","Model:","Modelo:","Modele:","Modell:","Modello:","Model:"}},
 {{"ocupada","used","ocupada","occupee","belegt","usata","gebruikt"}},
 {{"ajustar","adjust","ajustar","regler","einstellen","regola","instellen"}},
 {{"Volver","Back","Voltar","Retour","Zurueck","Indietro","Terug"}},
 {{"Region","Region","Regiao","Region","Region","Regione","Regio"}},
 {{"Idioma","Language","Idioma","Langue","Sprache","Lingua","Taal"}},
 {{"Pulsar","Press","Pressionar","Appuyer","Druecken","Premi","Druk"}},
 {{"Banda","Band","Banda","Bande","Band","Banda","Band"}},
 {{"vacia","empty","vazia","vide","leer","vuota","leeg"}},
 {{"CANAL","CHANNEL","CANAL","CANAL","KANAL","CANALE","KANAAL"}},
 {{"Modo","Mode","Modo","Mode","Modus","Modo","Modus"}},
 {{"Paso","Step","Passo","Pas","Schritt","Passo","Stap"}},
 {{"Tema","Theme","Tema","Theme","Thema","Tema","Thema"}},
 {{"SI","YES","SIM","OUI","JA","SI","JA"}},
 {{"NO","NO","NAO","NON","NEIN","NO","NEE"}},
};
String localize(const String& original){
 if(language==0)return original;
 for(const auto& row:translations)if(original==row.text[0])return row.text[language];
 String result=original;
 // Longest matches first; boundaries protect radio acronyms and identifiers.
 for(const auto& row:translations){
  const char* key=row.text[0];const size_t keyLength=strlen(key);int start=0,pos;
  while((pos=result.indexOf(key,start))>=0){
   int end=pos+keyLength;
   if((pos==0||!isAlphaNumeric(result[pos-1]))&&(end==int(result.length())||!isAlphaNumeric(result[end]))){
    result=result.substring(0,pos)+row.text[language]+result.substring(end);
    start=pos+strlen(row.text[language]);
   }else start=end;
  }
 }
 return result;
}
