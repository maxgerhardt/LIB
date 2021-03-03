# Examples [pico-sdk](https://github.com/raspberrypi/pico-examples)
Platform will make default main.c as template

if function **main()** exist in **other_main_file.c** 
* **delete** template main.c 
* **rename** other_main_file.c to main.c  

### Other
* [pico-examples](https://github.com/raspberrypi/pico-examples)
* [pico-playground](https://github.com/raspberrypi/pico-playground)
* [pico-extras](https://github.com/raspberrypi/pico-extras)
* [platform examples](https://github.com/Wiz-IO/wizio-pico/tree/main/examples/baremetal)

# platformio.ini ( minimal )
```ini
[env:pico]
platform = wizio-pico
board = pico
framework = baremetal

;upload_port  = PicoDrive:\
monitor_port  = COM1
monitor_speed = 115200

```
if **upload_port** is empty, the UF2 file exist in **.pio/build/** folder after **Upload**
***
### [PlatformIO Documentation](https://docs.platformio.org/en/latest/projectconf/section_env_build.html)
```ini
build_unflags = -D BOO ; remove this
build_flags   = -D FOO ; add this
lib_deps      = load_some_library
```

***
### USB
```ini
lib_deps = 
        tinyusb ; load TinyUSB library
```
**By default the tinyUSB is removed for speed**

***
### STDIO ( UART, USB, SEMIHOSTING )
```ini
lib_deps = 
        tinyusb ; load TinyUSB library

build_flags = 
        ; enable stdio 
        -D PICO_STDIO_USB
        ;-D PICO_STDIO_UART
        ;-D PICO_STDIO_SEMIHOSTING
        ;-D PICO_PRINTF_PICO
```

***
### heap
```ini
board_build.heap = 4096
```
empty key, default heap size = 2048

***
### boot
```ini
board_build.boot= w25q080
```
empty is **w25q080** or
* **generic_03h**
* **is25lp080**
* **usb_blinky**
* **w25q080**
* **w25x10cl**

[look here](https://github.com/Wiz-IO/framework-wizio-pico/tree/main/common/boot2) 

***
### bynary_type
```ini
board_build.bynary_type = copy_to_ram
```
empty key mean **default** or
* **copy_to_ram**
* * memmap_copy_to_ram.ld, application-start-address = 0x10000000
* **no_flash**
* * memmap_no_flash.ld, application-start-address = 0x20000000
* **blocked_ram**
* * TODO
* **default**
* * memmap_default.ld, application-start-address = 0x10000000

***
### linker
```ini
board_build.linker = custom.ld
```
if is empty, the linker script depend from **bynary_type** key

else script must exist in folder **framework-wizio-pico/tree/main/pico-sdk/src/rp2_common/pico_standard_link**

***
### address
```ini
board_build.address = 0x20000000
```
if is empty, the address depend from **bynary_type** key

else other Application / UF2 **start address**

***
## FreeRTOS
```ini
lib_deps = 
        freertos ; load library
```
**your-project/include/**[FreeRTOSConfig.h](https://github.com/Wiz-IO/framework-wizio-pico/blob/main/templates/Template-FreeRTOSConfig.h)

[basic example](https://github.com/Wiz-IO/wizio-pico/tree/main/examples/baremetal/pico-freertos)
***

## INI EXAMPLE
```ini
[env:pico]
platform = wizio-pico
board = pico
framework = baremetal

upload_port   = E:\
monitor_port  = COM34
monitor_speed = 115200

;board_build.use_usb = 1
;board_build.bynary_type = default
;board_build.boot = w25q080
```
