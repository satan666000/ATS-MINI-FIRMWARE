# ATS Mini Firmware

Firmware binario para dispositivos ATS Mini, con archivos listos para flashear y una guía de instalación basada en offsets de memoria.

## Descripción general

Este repositorio reúne los archivos necesarios para actualizar o restaurar el firmware de un ATS Mini. Incluye los binarios principales y la información de offsets para escribirlos en la flash del dispositivo de forma segura.

> Advertencia: los archivos de firmware deben ser compatibles con tu modelo y revisión exacta del hardware. Un firmware incorrecto puede dejar el dispositivo inutilizable o requerir procedimientos de recuperación manual.

## Contenido del repositorio

```text
ATSMINI/
├── FLASH-OFFSETS.txt
├── boot_app0.bin
├── bootloader.bin
├── firmware.bin
├── partitions.bin
README.md
```

## Archivos principales

| Archivo | Descripción | Offset |
| --- | --- | ---: |
| `ATSMINI/bootloader.bin` | Cargador de arranque | `0x00000` |
| `ATSMINI/partitions.bin` | Tabla de particiones | `0x08000` |
| `ATSMINI/boot_app0.bin` | Imagen de arranque | `0x0E000` |
| `ATSMINI/firmware.bin` | Firmware principal | `0x10000` |
| `ATSMINI/FLASH-OFFSETS.txt` | Instrucciones rápidas y offsets | — |

## Requisitos

- ATS Mini compatible con esta imagen.
- Cable USB de datos.
- Puerto serie disponible en tu sistema.
- Python 3.x instalado.
- `esptool` instalado:

```bash
python -m pip install esptool
```

## Procedimiento de flasheo

1. Clona o descarga el repositorio.
2. Conecta el ATS Mini al equipo y identifica el puerto serial.
3. En Windows puede ser `COMx`; en Linux/macOS suele ser `/dev/ttyUSB0`, `/dev/ttyACM0` o similar.
4. Ejecuta el siguiente comando desde la raíz del proyecto:

```bash
python -m esptool --port <PUERTO> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Reemplaza `<PUERTO>` con el puerto real del dispositivo.

## Verificación

Después del flasheo:

- desconecta y vuelve a conectar el equipo,
- observa si el ATS Mini enciende correctamente,
- comprueba que la funcionalidad esperada responde como se espera,
- si aparece un problema, revisa el puerto, la compatibilidad del firmware y el procedimiento de instalación.

## Seguridad y advertencias

- No flashees un firmware que no hayas verificado como compatible.
- Asegúrate de contar con una copia de seguridad si el equipo ya tiene configuración crítica.
- No compartas información sensible, seriales ni credenciales durante el diagnóstico.
- Este repositorio no incluye documentación de hardware completa ni instrucciones oficiales del fabricante; usa la información con precaución.

## Soporte

Si deseas reportar un problema o pedir ayuda:

- incluye el modelo exacto del ATS Mini,
- indica la versión del firmware,
- menciona el sistema operativo,
- comparte el puerto serial usado,
- adjunta el mensaje de error completo.

## Nota legal

No se ha especificado una licencia para este repositorio. Antes de reutilizar, redistribuir o publicar este firmware, confirma con el propietario o con el autor original si se permite su uso y distribución.

## Estado del proyecto

El repositorio está preparado para almacenamiento y distribución de firmware, con una documentación básica para flasheo y uso. Si deseas, puede ampliarse con:

- una guía detallada de instalación,
- una sección de troubleshooting,
- un changelog,
- y un diseño visual más premium para GitHub.
