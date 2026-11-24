# Herramientas de recepción para Windows

 
Para abrir esta guía en el idioma de Windows, guarda también ambos archivos
`RECEPTION-TOOLS*.md` junto al puente y pulsa **Herramientas de recepción**
en el panel abierto desde `http://127.0.0.1:8765`.
## Panel web local
Abre `KALIMAN-DASHBOARD.html` en Chrome o Edge. Conecta el ATS con un cable USB de datos, espera a que termine la animación y cierra el monitor serie de Schematik. Pulsa **Conectar USB** y selecciona el puerto del ATS. El panel no necesita Internet ni instala un servidor en la radio. Si el puerto se reinicia al abrirlo, espera seis segundos y vuelve a conectar. Desconecta el panel antes de usar Deploy.

## Audio para CW y FT8
Conecta auriculares del ATS a una entrada de línea del PC o de una interfaz USB de audio con entrada de línea. El USB de control del ATS no lleva audio. Empieza con el volumen bajo y evita que el medidor de entrada se sature. No uses una salida de altavoz amplificada ni una entrada de micrófono con alimentación como sustituto de la entrada de línea.

- **CW (Morse):** usa USB o LSB, desactiva SYNC y ajusta el BFO hasta que el tono sea claro. En un decodificador de Morse para Windows, como fldigi en modo CW, selecciona la entrada de línea y centra el decodificador en el tono del espectrograma.
- **FT8:** sincroniza la hora de Windows. En WSJT-X elige FT8, la entrada de línea como entrada de audio y `None` como equipo (`Rig`); deja el control de transmisión sin configurar. En el ATS selecciona la banda de 20 m, modo USB, `14074000` Hz, SYNC apagado y BFO inicialmente en 0. Si hay señales, el programa intenta decodificarlas cada 15 segundos. El ATS solo recibe; no configures una salida de transmisión.

Puedes dejar el panel abierto mientras decodificas: el panel usa el puerto COM y el decodificador usa el audio. CW y FT8 se decodifican en Windows, no dentro del receptor.

## Ver texto FT8 y CW en el panel de Windows

1. Guarda `KALIMAN-DASHBOARD.html` y `KALIMAN-BRIDGE.py` juntos. Con Python 3
   instalado, ejecuta `py -3 KALIMAN-BRIDGE.py` desde PowerShell en esa carpeta.
   Abre `http://127.0.0.1:8765` en Chrome o Edge.
2. Conecta el jack de audio del ATS a la entrada de línea del PC y selecciónala
   en el decodificador. USB solo controla el ATS. El waterfall es opcional.
3. **FT8:** en WSJT-X, Reporting → UDP Server: `127.0.0.1`, puerto `2237`.
   Selecciona FT8 y activa Monitor. En el ATS: USB, SYNC apagado, por ejemplo
   14,074 MHz. Sincroniza el reloj de Windows. Se muestran mensajes nuevos
   recibidos por radio, no reproducciones WAV.
4. **CW:** abre fldigi en modo CW y centra el tono en su cascada. Usa su
   interfaz XML-RPC local en `127.0.0.1:7362` (opciones de inicio
   `--xmlrpc-server-address 127.0.0.1 --xmlrpc-server-port 7362`). No ejecutes
   otro lector de `rx.get_data` al mismo tiempo. No abras estos puertos a Internet.

PSK Reporter y Reverse Beacon Network son enlaces a reportes de terceros.
POTA muestra activaciones reportadas; HamQSL da contexto solar. Ninguno
confirma recepción en tu ATS: para eso observa RSSI/SNR y el texto local.
