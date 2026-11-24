# KALIMAN 1.1.88 — correcciones de estabilidad

- VFO A/B: comprobar que la lectura guardada esté completa antes de utilizarla. No rechazar frecuencias regionales válidas por validar contra los límites iniciales de otra región; al activar un VFO se aplican los límites actuales.
- Sintonía: confirmar y limpiar el indicador STC al terminar, para no conservar una confirmación anterior.
- Encoder: descartar la acumulación parcial si ambos contactos cambian a la vez (transición inválida).
- Squelch automático: abrir por RSSI con la misma histéresis; SNR solo sirve para evitar que una estación fuerte eleve la estimación del ruido.
- Pantalla: solicitar repintado al salir del reposo por desactivación de su ajuste.

Sin cambios de cableado, bibliotecas, funciones ni distribución de pantalla.
La compilación y carga no sustituyen las pruebas físicas de recepción, VFO,
encoder y audio. El rediseño visual permanece pendiente.
