# ATS Mini flashing guide / Guía de flasheo ATS Mini

[English](#english) | [Español](#español)

## English

This guide documents the recommended procedure for writing the ATS Mini firmware image to device flash safely and reproducibly.

### Flash offsets summary

```text
0x00000  bootloader.bin
0x08000  partitions.bin
0x0E000  boot_app0.bin
0x10000  firmware.bin
```

### Prerequisites

- Python 3.x
- [esptool](https://github.com/espressif/esptool) installed
- An available serial port
- ATS Mini connected correctly and in programming mode
- Backup of the original firmware if the device contains important configuration or user data

### Install esptool

```bash
python -m pip install esptool
```

### Flashing procedure

Run the following from the repository root:

```bash
python -m esptool --port <PORT> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Replace `<PORT>` with the actual serial port on your system (`COMx` on Windows or `/dev/tty...` on Linux/macOS).

### Important considerations

- Verify hardware compatibility before beginning.
- An incorrect or incompatible firmware image may render the device unusable.
- Do not erase the full flash unless the hardware-specific documentation explicitly allows it.
- Keep a backup of the prior state whenever possible.

### Quick troubleshooting

If the flashing process fails:

1. Check the serial port and USB cable.
2. Try another port or a different data cable.
3. Verify the installed `esptool` version.
4. Confirm that the files match your device and revision.
5. Ensure the device is entering programming mode correctly.

### References

- Main project README: `../README.md`
- Detailed offsets: `../ATSMINI/FLASH-OFFSETS.txt`
- Firmware source: `../main.cpp`

### Safety note

Firmware flashing is a sensitive operation. If you are not certain that the device and image are compatible, do not proceed until that compatibility has been verified.

## Español

Esta guía describe el procedimiento recomendado para escribir el firmware del ATS Mini en la memoria flash de forma segura y reproducible.

### Resumen de direcciones

```text
0x00000  bootloader.bin
0x08000  partitions.bin
0x0E000  boot_app0.bin
0x10000  firmware.bin
```

### Requisitos previos

- Python 3.x
- [esptool](https://github.com/espressif/esptool) instalado
- Un puerto serie disponible
- ATS Mini conectado correctamente y en modo de programación
- Copia de seguridad del firmware original si el dispositivo contiene configuraciones o datos importantes

### Instalar esptool

```bash
python -m pip install esptool
```

### Procedimiento de flasheo

Ejecuta el siguiente comando desde la raíz del repositorio:

```bash
python -m esptool --port <PORT> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

Reemplaza `<PORT>` por el puerto serie del sistema (`COMx` en Windows o `/dev/tty...` en Linux/macOS).

### Consideraciones importantes

- Verifica la compatibilidad del hardware antes de comenzar.
- Una imagen incorrecta o incompatible puede dejar el dispositivo inutilizable.
- No borres toda la memoria flash salvo que la documentación específica del hardware lo permita expresamente.
- Conserva una copia de seguridad del estado anterior siempre que sea posible.

### Solución rápida de problemas

Si falla el flasheo:

1. Comprueba el puerto serie y el cable USB.
2. Prueba otro puerto o cable de datos.
3. Verifica la versión instalada de `esptool`.
4. Confirma que los archivos corresponden al modelo y la revisión del dispositivo.
5. Asegúrate de que el dispositivo entra correctamente en modo de programación.

### Referencias

- README principal: `../README.md`
- Direcciones detalladas: `../ATSMINI/FLASH-OFFSETS.txt`
- Código fuente del firmware: `../main.cpp`

### Nota de seguridad

Flashear firmware es una operación delicada. Si no tienes certeza de que el dispositivo y la imagen sean compatibles, no continúes hasta verificarlo.
