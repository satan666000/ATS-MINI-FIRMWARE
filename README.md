# ATS Mini Firmware

Firmware precompilado para el dispositivo ATS Mini. Este repositorio contiene los archivos binarios y los offsets necesarios para grabarlos en la memoria flash.

> **Importante:** Antes de instalar, confirma que esta compilación corresponde exactamente a tu modelo de ATS Mini y a la revisión de hardware. Una imagen incompatible puede dejar el dispositivo inutilizable. Conserva una copia del firmware actual si es posible.

## Contenido

| Archivo | Descripción | Offset de flash |
|---|---|---:|
| `ATSMINI/bootloader.bin` | Cargador de arranque | `0x0` |
| `ATSMINI/partitions.bin` | Tabla de particiones | `0x8000` |
| `ATSMINI/boot_app0.bin` | Aplicación de arranque | `0xE000` |
| `ATSMINI/firmware.bin` | Firmware principal | `0x10000` |
| `ATSMINI/FLASH-OFFSETS.txt` | Lista de archivos y offsets | — |

## Requisitos

- Un ATS Mini compatible con esta compilación.
- Un cable USB de datos y el puerto serie correspondiente.
- [esptool](https://github.com/espressif/esptool) instalado.
- El controlador USB/serie necesario para tu sistema operativo.

## Instalación

1. Descarga o clona este repositorio y abre una terminal en su directorio raíz.
2. Verifica la compatibilidad del firmware con tu modelo y revisión de hardware.
3. Conecta el dispositivo y determina el puerto serie (`COMx` en Windows o `/dev/tty...` en Linux/macOS).
4. Graba los archivos usando los offsets indicados. Con una versión actual de `esptool`, el comando tiene este formato:

   ```bash
   python -m esptool --port <PUERTO> write-flash \
     0x0 ATSMINI/bootloader.bin \
     0x8000 ATSMINI/partitions.bin \
     0xE000 ATSMINI/boot_app0.bin \
     0x10000 ATSMINI/firmware.bin
   ```

   Sustituye `<PUERTO>` por el puerto real. La sintaxis puede variar entre versiones de esptool; consulta `python -m esptool --help` si el comando no coincide con tu versión. No uses opciones para borrar la flash completa salvo que las instrucciones específicas de tu dispositivo lo indiquen.
5. Reinicia el dispositivo y comprueba su funcionamiento.

## Compatibilidad y versión

La información disponible en este repositorio no especifica todavía el modelo/revisión exactos de hardware, la versión del firmware ni el procedimiento de recuperación. Confirma esos datos con el autor antes de instalar en un dispositivo de uso crítico.

## Contribuciones y soporte

Para reportar un problema, incluye el modelo y revisión del dispositivo, sistema operativo, versión de esptool, puerto utilizado y el mensaje de error completo. No publiques números de serie, credenciales ni otros datos privados.

## Licencia

No se ha especificado una licencia. Hasta que se añada una, no se debe asumir que el firmware puede redistribuirse, modificarse o reutilizarse libremente.
