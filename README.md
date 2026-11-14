<p align="center">
  <img src="assets/ats-mini-banner.svg" alt="ATS Mini Firmware — firmware project banner" width="100%">
</p>

<p align="center">
  <strong>Firmware binario para ATS Mini</strong><br>
  Archivos listos para flashear, offsets de memoria y guía de instalación.
</p>

---

## Descripción general

Este repositorio reúne los archivos necesarios para actualizar o restaurar el firmware de un ATS Mini. Incluye los binarios principales y la información de offsets para escribirlos en la memoria flash.

> **Advertencia:** verifica que los archivos sean compatibles con el modelo y la revisión exacta de tu hardware. Un firmware incorrecto puede dejar el dispositivo inutilizable. Conserva una copia del firmware actual si es posible.

## Contenido del repositorio

```text
.
├── ATSMINI/
│   ├── FLASH-OFFSETS.txt
│   ├── boot_app0.bin
│   ├── bootloader.bin
│   ├── firmware.bin
│   └── partitions.bin
├── assets/
│   └── ats-mini-banner.svg
├── docs/
│   └── FLASHING.md
└── README.md
```

## Archivos de firmware

| Archivo | Descripción | Offset de flash |
| --- | --- | ---: |
| `ATSMINI/bootloader.bin` | Cargador de arranque | `0x00000` |
| `ATSMINI/partitions.bin` | Tabla de particiones | `0x08000` |
| `ATSMINI/boot_app0.bin` | Imagen de arranque | `0x0E000` |
| `ATSMINI/firmware.bin` | Firmware principal | `0x10000` |

## Requisitos

- Un ATS Mini compatible con esta imagen.
- Cable USB de datos y puerto serial disponible.
- Python 3.x y [esptool](https://github.com/espressif/esptool).

Instala esptool con:

```bash
python -m pip install esptool
```

## Flasheo

1. Descarga o clona este repositorio.
2. Conecta el dispositivo e identifica el puerto serial (`COMx` en Windows o `/dev/tty...` en Linux/macOS).
3. Desde el directorio raíz del repositorio, ejecuta:

```bash
python -m esptool --port <PUERTO> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Reemplaza `<PUERTO>` por el puerto real. La sintaxis puede variar según la versión de esptool; consulta `python -m esptool --help` si es necesario. No borres la memoria flash completa salvo que las instrucciones específicas de tu dispositivo lo indiquen.

Para una guía ampliada, consulta [`docs/FLASHING.md`](docs/FLASHING.md). Los offsets también están documentados en [`ATSMINI/FLASH-OFFSETS.txt`](ATSMINI/FLASH-OFFSETS.txt).

## Seguridad y compatibilidad

- Confirma el modelo y la revisión del hardware antes de flashear.
- No instales firmware de compatibilidad desconocida.
- Si el dispositivo contiene datos o configuración importantes, respáldalos antes de comenzar.
- Este repositorio no incluye documentación completa del hardware ni instrucciones oficiales del fabricante.

## Soporte

Al reportar un problema, incluye el modelo y revisión del dispositivo, sistema operativo, versión de esptool y el mensaje de error. No publiques números de serie, credenciales ni otros datos privados.

## Licencia

Este repositorio no especifica una licencia. No se debe asumir que el firmware puede modificarse, redistribuirse o reutilizarse libremente; consulta al titular de los derechos antes de hacerlo.
