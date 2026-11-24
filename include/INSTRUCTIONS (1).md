# Publicación automática de KALIMAN

Preparado para https://github.com/satan666000/ATS-MINI-FIRMWARE.
No se ha subido ni activado desde Schematik.

## Archivos preparados en Schematik
Los archivos están dentro de `include/github-publishing/` para que formen
parte del proyecto entregable. Al guardarlos en GitHub, usa estas rutas:
- `release.yml.txt` → `.github/workflows/release.yml`
- `platformio-release.ini.txt` → `ci/platformio-release.ini`
- `RELEASE-README.txt` → `ci/RELEASE-README.txt`
No conserves la terminación `.txt` en los primeros dos archivos de destino.

## Activarlo una vez
1. Guarda en el repositorio el proyecto actual: `src/`, `include/`,
   `ci/` y `.github/workflows/release.yml`, conservando sus rutas.
   No basta subir el ZIP como adjunto de una Release: GitHub debe ver el código
   y el flujo dentro del repositorio. Puedes crear los archivos de texto con
   **Add file → Create new file** en GitHub.
2. En **Actions**, permite ejecutar los flujos si GitHub lo solicita.
   Este flujo usa el permiso `contents: write` y el token interno de GitHub.
   No requiere una contraseña ni un token personal. Las políticas de tu cuenta
   u organización pueden impedir ese permiso.
3. Con el código guardado, crea una etiqueta nueva que coincida exactamente
   con `FIRMWARE_VERSION` en `src/main.cpp`: actualmente `v1.1.65`.
   En la web puedes usar **Releases → Draft a new release → Choose a tag →
   Create new tag**, apuntando al último commit que contiene estos archivos,
   y pulsar **Save draft**, no Publish. La creación de la etiqueta inicia el
   flujo, que publica la Release y añade los archivos si compila correctamente.
   No reutilices una etiqueta o una Release ya publicada: usa una versión nueva.
4. Comprueba en **Actions → Publicar KALIMAN** que termina en verde y que la
   Release contiene los binarios, ZIP, posiciones de carga y sumas SHA-256.

## Cada actualización
Guarda los cambios del código en GitHub, aumenta `FIRMWARE_VERSION` y crea su
etiqueta `vX.Y.Z`. GitHub compilará y publicará esa versión. No se publican todos
los commits ni se envían cambios desde Schematik a GitHub automáticamente.

Si ya hay una Release publicada para esa etiqueta, el paso de publicación
fallará a propósito: no reemplaza archivos anteriores silenciosamente.

## Límites y seguridad
- No cambia el WiFi del ATS ni añade OTA; las cargas al radio siguen por USB.
- El compilador de GitHub tiene su configuración propia en
  `ci/platformio-release.ini`; su ejecución todavía debe verificarse en Actions.
- Antes de cargar, guarda la copia original y comprueba la variante del ATS.
- Conserva licencias y avisos de terceros. No publiques claves ni datos personales.
- No necesitas volver a flashear el ATS para activar la publicación en GitHub.

Reloj local: Sistema > Reloj: hora / minutos / AM-PM / formato.
Girar para cambiar y pulsar para aceptar. Formatos 12 h (AM/PM) y 24 h.
Sin reloj de respaldo: ajustar de nuevo tras apagar o reiniciar.
El formato se conserva; la hora no avanza con el equipo apagado.
