# KALIMAN: control de receptor por USB desde Windows

Conecta un cable USB de datos. Tu programa debe abrir el puerto COM del ATS
como puerto serie a 115200 baudios, 8 bits, sin paridad, 1 bit de parada, sin
control de flujo. No abras el mismo COM en dos programas a la vez. Evita activar
DTR/RTS: algunos controladores reinician el equipo al abrir el puerto. Espera a
que termine la presentación inicial antes de enviar comandos.

Envía texto ASCII, mayúsculas, un comando por línea terminado en LF (`\n`).
Espera una respuesta antes de enviar el siguiente. No es CAT de un fabricante
ni Hamlib: tu programa debe implementar este protocolo propio.

| Comando | Función |
|---|---|
| STATUS | Estado actual, sin modificar nada |
| BANDS | Lista de IDs, nombres y límites en Hz |
| BAND 27 | Seleccionar banda por ID; 27 es CB USA |
| FREQ 27205000 | Frecuencia en Hz dentro de la banda actual |
| MODE AM | AM, LSB o USB; FM solo en la banda FM; CB solo AM |
| AUTOMODE 1 | Retirado: devuelve AUTOMODE_REMOVED_USE_MODE; usar MODE |
| SYNC 1 | Habilitar sincronizacion AM en SSB manual; 0 apagar |
| BFO 120 | Ajuste SSB de -16000 a 16000 Hz; desactiva SYNC |
| VOLUME 25 | Volumen 0–63 |
| MUTE 1 | Silenciar; 0 restaura audio |
| SQL 20 | Umbral RSSI 0–80 |
| SQLMODE 1 | 0 apagado, 1 manual, 2 automático |

Cada respuesta es una línea JSON con `protocol: "KALIMAN-USB-1"`, `ok` y,
si hay rechazo, `error`. Ignora otras líneas: pueden ser diagnósticos de arranque
o WiFi. STATUS devuelve frecuencia en Hz, banda, modo, volumen, silencio,
squelch, RSSI y SNR. RSSI/SNR son la última muestra, no una medición instantánea;
pueden ser cero mientras se estabiliza la sintonía. Consulta STATUS cada 500 ms
si necesitas refrescar el panel. Las órdenes rechazadas no cambian ajustes.

Resolución: FM 10000 Hz; AM/SSB 1000 Hz, BFO ajustable en pasos de 1 Hz por USB. En CB se aceptan solo frecuencias de los 40 canales legales.
BAND conserva la lógica de modo automático del equipo; MODE en AM/SSB desactiva
ese automatismo para respetar el modo pedido. Los controles físicos siguen
activos: consulta STATUS para reflejar cambios hechos en el ATS.

Los ajustes siguen el guardado existente (10 segundos tras el último cambio).
El silencio no es persistente. No se añade transmisión, audio por USB,
control WiFi/BLE ni compatibilidad automática con programas comerciales.

Prueba: STATUS, BANDS, BAND 27, MODE AM, FREQ 27205000, VOLUME 20, STATUS.
Comprueba también que FREQ 27206000 sea rechazado en CB sin cambiar frecuencia.
Conserva tu firmware original y método de recuperación antes de desplegar.
La compilación no verifica el puerto USB ni la recepción real: probar en equipo.

STATUS distingue automatic_mode, sync_enabled (preferencia guardada),
sync_active (configuracion aplicada, no confirma bloqueo) y bfo_hz (ajuste manual).
La seleccion automatica de modo se ha eliminado. AM/LSB/USB se eligen manualmente; CB permanece exclusivamente AM.
SYNC no se restaura al reiniciar y se apaga al cambiar banda o modo. No afina voz SSB.
Para SYNC: MODE USB o MODE LSB, SYNC 1 y sintonizar una portadora AM.
Para voz SSB normal: SYNC 0 y ajustar BFO.

Desde KALIMAN 1.1.95: `AGC 1` activa AGC y `AGC 0` selecciona control
manual. `ATT 0` a `ATT 26` selecciona atenuación y desactiva AGC.
`FILTER -1` selecciona filtro automático. En manual, `FILTER 0` en
adelante sigue este orden en kHz: AM 6/4/3/2/1/1.8/2.5; SSB
1.2/2.2/3/4/0.5/1. Estos comandos no aplican a FM. STATUS informa
`agc`, `attenuation` y, fuera de FM, `filter_auto` y `filter_index` activo.
Auto VFO en el panel corresponde a `SYNC`, no a selección automática de modo.

### Idioma del receptor (1.1.99)

`STATUS` incluye `language`. `LANG <id>` cambia el idioma del ATS y
usa el guardado habitual de ajustes (espera unos 10 segundos antes de apagar).
IDs 0–9: Español, English, Português, Français, Deutsch, Italiano,
Nederlands, Polski, Svenska y Türkçe. No cambia el idioma del panel web.
