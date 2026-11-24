#pragma once
// ASCII labels match the display's built-in font. Radio data is not translated.
uint8_t language=0;
constexpr int LANGUAGE_COUNT=10;
const char* languageNames[]={"Espanol","English","Portugues","Francais","Deutsch","Italiano","Nederlands","Polski","Svenska","Turkce"};
struct Translation { const char* text[LANGUAGE_COUNT]; };
const Translation translations[]={
 {{"Pulsar: pausa/audio / Rueda: sintonia","Press: pause/audio / Wheel: tune","Pressionar: pausa/audio / Roda: sintonia","Appui: pause/audio / Roue: accord","Druck: Pause/Audio / Rad: Abstimmen","Premi: pausa/audio / Ruota: sintonia","Druk: pauze/audio / Draai: afstemmen","Nacisnij: pauza/audio / Pokretlo: strojenie","Tryck: paus/ljud / Vrid: frekvens","Bas: duraklat/ses / Cevir: frekans"}},
 {{"Interfaz: espanol / Solo recepcion","Interface: English / Receive only","Interface: portugues / So recepcao","Interface: francais / Reception seule","Sprache: Deutsch / Nur Empfang","Interfaccia: italiano / Solo ricezione","Taal: Nederlands / Alleen ontvangst","Jezyk: polski / Tylko odbior","Sprak: svenska / Endast mottagning","Dil: Turkce / Yalniz alici"}},
 {{"Pulsar: aceptar / Mantener: salir","Press: accept / Hold: exit","Pressionar: aceitar / Segurar: sair","Appui: valider / Long: quitter","Druck: OK / Halten: Ende","Premi: conferma / Tieni: esci","Druk: bevestig / Houd: sluiten","Nacisnij: OK / Przytrzymaj: wyjscie","Tryck: OK / Hall: avsluta","Bas: onay / Tut: cikis"}},
 {{"Girar: cambiar / Pulsar: aceptar","Turn: change / Press: accept","Girar: mudar / Pressionar: aceitar","Tourner: modifier / Appui: valider","Drehen: aendern / Druck: OK","Ruota: cambia / Premi: conferma","Draai: wijzigen / Druk: bevestig","Obroc: zmien / Nacisnij: OK","Vrid: andra / Tryck: OK","Cevir: degistir / Bas: onay"}},
 {{"RUEDA: sintonia / Mantener: menu","WHEEL: tune / Hold: menu","RODA: sintonia / Segurar: menu","ROUE: accord / Long: menu","RAD: Abstimmen / Halten: Menue","RUOTA: sintonia / Tieni: menu","DRAAI: afstemmen / Houd: menu","POKRETLO: strojenie / Przytrzymaj: menu","VRID: frekvens / Hall: meny","CEVIR: frekans / Tut: menu"}},
 {{"RUEDA: volumen / Mantener: menu","WHEEL: volume / Hold: menu","RODA: volume / Segurar: menu","ROUE: volume / Long: menu","RAD: Lautstaerke / Halten: Menue","RUOTA: volume / Tieni: menu","DRAAI: volume / Houd: menu","POKRETLO: glosnosc / Przytrzymaj: menu","VRID: volym / Hall: meny","CEVIR: ses / Tut: menu"}},
 {{"Girar: pagina / Pulsar: volver","Turn: page / Press: back","Girar: pagina / Pressionar: voltar","Tourner: page / Appui: retour","Drehen: Seite / Druck: zurueck","Ruota: pagina / Premi: indietro","Draai: pagina / Druk: terug","Obroc: strona / Nacisnij: wroc","Vrid: sida / Tryck: tillbaka","Cevir: sayfa / Bas: geri"}},
 {{"INFORMACION / FIRMWARE","INFORMATION / FIRMWARE","INFORMACAO / FIRMWARE","INFORMATION / FIRMWARE","INFORMATION / FIRMWARE","INFORMAZIONI / FIRMWARE","INFORMATIE / FIRMWARE","INFORMACJE / FIRMWARE","INFORMATION / FIRMWARE","BILGI / FIRMWARE"}},
 {{"AMP / SALIDA DE AUDIO","AMP / AUDIO OUTPUT","AMP / SAIDA DE AUDIO","AMP / SORTIE AUDIO","AMP / AUDIOAUSGANG","AMP / USCITA AUDIO","AMP / AUDIOUITGANG","AMP / WYJSCIE AUDIO","AMP / LJUDUTGANG","AMP / SES CIKISI"}},
 {{"Pulsar para regresar","Press to return","Pressionar para voltar","Appuyer pour revenir","Druecken fuer zurueck","Premi per tornare","Druk om terug te gaan","Nacisnij aby wrocic","Tryck for att aterga","Geri donmek icin bas"}},
 {{"ATS / CONFIGURACION","ATS / SETTINGS","ATS / AJUSTES","ATS / REGLAGES","ATS / EINSTELLUNGEN","ATS / IMPOSTAZIONI","ATS / INSTELLINGEN","ATS / USTAWIENIA","ATS / INSTALLNINGAR","ATS / AYARLAR"}},
 {{"INFORMACION / RADIO","INFORMATION / RADIO","INFORMACAO / RADIO","INFORMATION / RADIO","INFORMATION / RADIO","INFORMAZIONI / RADIO","INFORMATIE / RADIO","INFORMACJE / RADIO","INFORMATION / RADIO","BILGI / RADYO"}},
 {{"Iniciando radio...","Starting radio...","Iniciando radio...","Demarrage radio...","Radio startet...","Avvio radio...","Radio starten...","Uruchamianie radia...","Startar radio...","Radyo basliyor..."}},
 {{"Radio no detectada","Radio not detected","Radio nao detectado","Radio non detectee","Radio nicht erkannt","Radio non rilevata","Radio niet gevonden","Nie wykryto radia","Radio hittades inte","Radyo bulunamadi"}},
 {{"BANDAS / CARPETAS","BANDS / FOLDERS","BANDAS / PASTAS","BANDES / DOSSIERS","BAENDER / ORDNER","BANDE / CARTELLE","BANDEN / MAPPEN","PASMA / FOLDERY","BAND / MAPPAR","BANTLAR / KLASORLER"}},
 {{"RSSI / INTENSIDAD","RSSI / STRENGTH","RSSI / INTENSIDADE","RSSI / PUISSANCE","RSSI / STAERKE","RSSI / INTENSITA","RSSI / STERKTE","RSSI / SILA","RSSI / STYRKA","RSSI / GUC"}},
 {{"SYNC AM / AFC","SYNC AM / AFC","SYNC AM / AFC","SYNC AM / AFC","SYNC AM / AFC","SYNC AM / AFC","SYNC AM / AFC","SYNC AM / AFC","SYNC AM / AFC","SYNC AM / AFC"}},
 {{"Memoria guardada","Memory saved","Memoria salva","Memoire sauvee","Speicher gesichert","Memoria salvata","Geheugen opgeslagen","Pamiec zapisana","Minne sparat","Hafiza kaydedildi"}},
 {{"Nivel automatico","Automatic level","Nivel automatico","Niveau automatique","Automatischer Pegel","Livello automatico","Automatisch niveau","Poziom automatyczny","Automatisk niva","Otomatik seviye"}},
 {{"Guardar memoria","Save memory","Salvar memoria","Sauver memoire","Speicher sichern","Salva memoria","Geheugen opslaan","Zapisz pamiec","Spara minne","Hafiza kaydet"}},
 {{"Cargando SSB...","Loading SSB...","Carregando SSB...","Chargement SSB...","SSB wird geladen...","Caricamento SSB...","SSB laden...","Ladowanie SSB...","Laddar SSB...","SSB yukleniyor..."}},
 {{"Memoria cargada","Memory loaded","Memoria carregada","Memoire chargee","Speicher geladen","Memoria caricata","Geheugen geladen","Pamiec wczytana","Minne laddat","Hafiza yuklendi"}},
 {{"AGC automatico","Automatic AGC","AGC automatico","AGC automatique","AGC automatisch","AGC automatico","Automatische AGC","Automatyczne AGC","Automatisk AGC","Otomatik AGC"}},
 {{"Cargar memoria","Load memory","Carregar memoria","Charger memoire","Speicher laden","Carica memoria","Geheugen laden","Wczytaj pamiec","Ladda minne","Hafiza yukle"}},
 {{"BARRIDO / MUDO","SCAN / MUTED","VARREDURA / MUDO","BALAYAGE / MUET","SCAN / STUMM","SCANSIONE / MUTO","SCANNEN / STIL","SKAN / WYCISZONY","SOKNING / TYST","TARAMA / SESSIZ"}},
 {{"Filtro AM/SSB","AM/SSB filter","Filtro AM/SSB","Filtre AM/SSB","AM/SSB Filter","Filtro AM/SSB","AM/SSB filter","Filtr AM/SSB","AM/SSB filter","AM/SSB filtre"}},
 {{"Squelch nivel","Squelch level","Nivel squelch","Niveau squelch","Squelch Pegel","Livello squelch","Squelchniveau","Poziom squelch","Squelch niva","Squelch seviye"}},
 {{"CONFIGURACION","SETTINGS","AJUSTES","REGLAGES","EINSTELLUNGEN","IMPOSTAZIONI","INSTELLINGEN","USTAWIENIA","INSTALLNINGAR","AYARLAR"}},
 {{"EDITAR AJUSTE","EDIT SETTING","EDITAR AJUSTE","MODIFIER REGLAGE","EINSTELLUNG AENDERN","MODIFICA IMPOSTAZIONE","INSTELLING WIJZIGEN","EDYTUJ USTAWIENIE","ANDRA INSTALLNING","AYARI DEGISTIR"}},
 {{"RECEPCION SSB","SSB RECEPTION","RECEPCAO SSB","RECEPTION SSB","SSB EMPFANG","RICEZIONE SSB","SSB ONTVANGST","ODBIOR SSB","SSB MOTTAGNING","SSB ALIM"}},
 {{"SNR / CALIDAD","SNR / QUALITY","SNR / QUALIDADE","SNR / QUALITE","SNR / QUALITAET","SNR / QUALITA","SNR / KWALITEIT","SNR / JAKOSC","SNR / KVALITET","SNR / KALITE"}},
 {{"PAUSA / AUDIO","PAUSE / AUDIO","PAUSA / AUDIO","PAUSE / AUDIO","PAUSE / AUDIO","PAUSA / AUDIO","PAUZE / AUDIO","PAUZA / AUDIO","PAUS / LJUD","DURAKLAT / SES"}},
 {{"Squelch modo","Squelch mode","Modo squelch","Mode squelch","Squelch Modus","Modo squelch","Squelchmodus","Tryb squelch","Squelch lage","Squelch modu"}},
 {{"Amplificador","Amplifier","Amplificador","Amplificateur","Verstaerker","Amplificatore","Versterker","Wzmacniacz","Forstarkare","Amplifikator"}},
 {{"RECEPCION AM","AM RECEPTION","RECEPCAO AM","RECEPTION AM","AM EMPFANG","RICEZIONE AM","AM ONTVANGST","ODBIOR AM","AM MOTTAGNING","AM ALIM"}},
 {{"Informacion","Information","Informacao","Information","Information","Informazioni","Informatie","Informacje","Information","Bilgi"}},
 {{"Procesador:","Processor:","Processador:","Processeur:","Prozessor:","Processore:","Processor:","Procesor:","Processor:","Islemci:"}},
 {{"Desactivado","Disabled","Desativado","Desactive","Deaktiviert","Disattivato","Uitgeschakeld","Wylaczone","Avaktiverad","Devre disi"}},
 {{"Solo AM/SSB","AM/SSB only","Somente AM/SSB","AM/SSB seulement","Nur AM/SSB","Solo AM/SSB","Alleen AM/SSB","Tylko AM/SSB","Endast AM/SSB","Yalniz AM/SSB"}},
 {{"Atenuacion","Attenuation","Atenuacao","Attenuation","Daempfung","Attenuazione","Verzwakking","Tlumienie","Dampning","Zayiflatma"}},
 {{"Recepcion:","Reception:","Recepcao:","Reception:","Empfang:","Ricezione:","Ontvangst:","Odbior:","Mottagning:","Alim:"}},
 {{"Compilado:","Built:","Compilado:","Compile:","Erstellt:","Compilato:","Gebouwd:","Kompilacja:","Byggd:","Derleme:"}},
 {{"Automatico","Automatic","Automatico","Automatique","Automatisch","Automatico","Automatisch","Automatyczny","Automatisk","Otomatik"}},
 {{"Encendido","On","Ligado","Active","Ein","Acceso","Aan","Wlaczone","Pa","Acik"}},
 {{"Receptor:","Receiver:","Receptor:","Recepteur:","Empfaenger:","Ricevitore:","Ontvanger:","Odbiornik:","Mottagare:","Alici:"}},
 {{"Pantalla:","Display:","Tela:","Ecran:","Display:","Schermo:","Scherm:","Ekran:","Skarm:","Ekran:"}},
 {{"BFO fino","Fine BFO","BFO fino","BFO fin","BFO fein","BFO fine","Fijne BFO","Dokladne BFO","Fin BFO","Ince BFO"}},
 {{"Silencio","Mute","Silencio","Muet","Stumm","Muto","Stil","Wyciszenie","Tyst","Sessiz"}},
 {{"Creador:","Creator:","Criador:","Createur:","Autor:","Autore:","Maker:","Autor:","Skapare:","Yapimci:"}},
 {{"Solo SSB","SSB only","Somente SSB","SSB seulement","Nur SSB","Solo SSB","Alleen SSB","Tylko SSB","Endast SSB","Yalniz SSB"}},
 {{"Volumen","Volume","Volume","Volume","Lautstaerke","Volume","Volume","Glosnosc","Volym","Ses"}},
 {{"Memoria","Memory","Memoria","Memoire","Speicher","Memoria","Geheugen","Pamiec","Minne","Hafiza"}},
 {{"Apagado","Off","Desligado","Desactive","Aus","Spento","Uit","Wylaczone","Av","Kapali"}},
 {{"Modelo:","Model:","Modelo:","Modele:","Modell:","Modello:","Model:","Model:","Modell:","Model:"}},
 {{"ocupada","used","ocupada","occupee","belegt","usata","gebruikt","zajeta","anvant","dolu"}},
 {{"ajustar","adjust","ajustar","regler","einstellen","regola","instellen","ustaw","justera","ayarla"}},
 {{"Volver","Back","Voltar","Retour","Zurueck","Indietro","Terug","Wroc","Tillbaka","Geri"}},
 {{"Region","Region","Regiao","Region","Region","Regione","Regio","Region","Region","Bolge"}},
 {{"Idioma","Language","Idioma","Langue","Sprache","Lingua","Taal","Jezyk","Sprak","Dil"}},
 {{"Pulsar","Press","Pressionar","Appuyer","Druecken","Premi","Druk","Nacisnij","Tryck","Bas"}},
 {{"Banda","Band","Banda","Bande","Band","Banda","Band","Pasmo","Band","Bant"}},
 {{"vacia","empty","vazia","vide","leer","vuota","leeg","pusta","tomt","bos"}},
 {{"CANAL","CHANNEL","CANAL","CANAL","KANAL","CANALE","KANAAL","KANAL","KANAL","KANAL"}},
 {{"Modo","Mode","Modo","Mode","Modus","Modo","Modus","Tryb","Lage","Mod"}},
 {{"Paso","Step","Passo","Pas","Schritt","Passo","Stap","Krok","Steg","Adim"}},
 {{"Tema","Theme","Tema","Theme","Thema","Tema","Thema","Motyw","Tema","Tema"}},
 {{"SI","YES","SIM","OUI","JA","SI","JA","TAK","JA","EVET"}},
 {{"NO","NO","NAO","NON","NEIN","NO","NEE","NIE","NEJ","HAYIR"}},
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
