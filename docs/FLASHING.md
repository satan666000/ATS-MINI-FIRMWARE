# Guía de flasheo para ATS Mini

Esta documentación ofrece una referencia rápida para escribir los archivos del firmware del ATS Mini en la memoria flash.

## Resumen de offsets

```text
0x00000  bootloader.bin
0x08000  partitions.bin
0x0E000  boot_app0.bin
0x10000  firmware.bin
```

## Requisitos

- Python 3.x
- esptool instalado
- Puerto serial disponible
- ATS Mini conectado en modo de programación o mediante el método indicado por tu hardware

## Instalación

```bash
python -m pip install esptool
```

```bash
python -m esptool --port <PUERTO> write-flash \
  0x00000 ATSMINI/bootloader.bin \
  0x08000 ATSMINI/partitions.bin \
  0x0E000 ATSMINI/boot_app0.bin \
  0x10000 ATSMINI/firmware.bin
```

## Consideraciones importantes

- Verifica el modelo del hardware antes de flashear.
- Una imagen incorrecta puede inutilizar el dispositivo.
- Si el proceso falla, revisa el puerto, el cable y la compatibilidad del archivo.
- Mantén una copia del firmware anterior si es posible.

## Diagnóstico rápido

Si la transferencia falla:

1. comprueba el puerto serial,
2. prueba otro cable USB,
3. verifica la versión de `esptool`,
4. revisa que el archivo sea el correcto para tu dispositivo.

## Referencias

- Archivo principal del repositorio: `README.md`
- Offsets detallados: `ATSMINI/FLASH-OFFSETS.txt`
