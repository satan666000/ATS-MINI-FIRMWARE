# KALIMAN 1.1.81

- Sintonia > VFO A/B: pulsar alterna dos posiciones de recepción; conserva banda, frecuencia, modo, paso y BFO. Se guardan tras diez segundos sin cambios. No son memorias de emisoras ni transmisión split.
- Sintonia > Frecuencia directa: girar modifica el dígito indicado; pulsar avanza por los ocho dígitos en Hz. La última pulsación confirma. Mantener cancela. Solo acepta frecuencias dentro de la banda actual y la resolución del receptor. CB acepta exclusivamente sus 40 canales AM.
- Pantalla > RDS FM: activar/desactivar lectura RDS.
- Pantalla > Reposo pantalla: siempre encendida o 30–300 segundos. Apaga únicamente la luz, sin suspender recepción ni WiFi. Botón/encoder despiertan y también ejecutan su acción normal.
- La sintonización espera STC del receptor con un límite de 250 ms; un timeout se comunica por USB. No es una garantía de recepción.
- Cambio de banda/modo mantiene el clamp de audio durante reconfiguración y conserva mute y estado previo de squelch. El squelch vuelve a evaluar la nueva frecuencia tras estabilizarse.
- Se mantiene el comportamiento manual SSB/BFO, RDS por grupos completos y escritura diferida de ajustes. No se introduce otra rejilla SSB sin validar continuidad en hardware.
- Sin Rain Map, memorias de emisoras, OTA externo ni nuevas funciones Bluetooth.

Verificación física pendiente: alternar FM/AM/SSB con mute y squelch activos; probar giro rápido, VFO A/B, reinicio tras guardar, entrada directa y reposo. No atribuir el fallo de audio al tocar la carcasa a firmware sin prueba eléctrica.
