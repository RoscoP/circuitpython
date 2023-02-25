USB_VID = 0x239A
USB_PID = 0x8112

USB_PRODUCT = "Adafruit QT Py ESP32S2"
USB_MANUFACTURER = "Adafruit"

IDF_TARGET = esp32s2

CIRCUITPY_ESP_FLASH_MODE = dio
CIRCUITPY_ESP_FLASH_FREQ = 40m
CIRCUITPY_ESP_FLASH_SIZE = 4MB

# Remove unneeded modules
CIRCUITPY_AESIO = 0
CIRCUITPY_ANALOGIO = 0
CIRCUITPY_AUDIOBUSIO = 0
CIRCUITPY_AUDIOCORE = 0
CIRCUITPY_AUDIOMIXER = 0
CIRCUITPY_BITBANGIO = 0
CIRCUITPY_BITMAPTOOLS = 0
CIRCUITPY_CANIO = 0
CIRCUITPY_COUNTIO = 0
CIRCUITPY_DISPLAYIO = 0
CIRCUITPY_DUALBANK = 0
CIRCUITPY_ESPCAMERA = 0
CIRCUITPY_ESP32_CAMERA = 0
CIRCUITPY_ESPIDF = 0
CIRCUITPY_ESPULP = 0
CIRCUITPY_FONTIO = 0
CIRCUITPY_FRAMEBUFFERIO = 0
CIRCUITPY_FREQUENCYIO = 0
CIRCUITPY_GETPASS = 0
CIRCUITPY_GIFIO = 0
CIRCUITPY_I2CTARGET = 0
CIRCUITPY_KEYPAD = 0
CIRCUITPY_MDNS = 0
CIRCUITPY_MEMORYMAP = 0
CIRCUITPY_MSGPACK = 0
CIRCUITPY_NVM = 0
CIRCUITPY_ONEWIREIO = 0
CIRCUITPY_PARALLELDISPLAY = 0
CIRCUITPY_PS2IO = 0
CIRCUITPY_PULSEIO = 0
CIRCUITPY_QRIO = 0
CIRCUITPY_RGBMATRIX = 0
CIRCUITPY_ROTARYIO = 0
CIRCUITPY_SDCARDIO = 0
CIRCUITPY_SHARPDISPLAY = 0
# CIRCUITPY_STORAGE = 0
CIRCUITPY_SYNTHIO = 0
CIRCUITPY_TOUCHIO = 0
CIRCUITPY_USB_HID = 0
CIRCUITPY_USB_MIDI = 0
CIRCUITPY_VECTORIO = 0
# end modules

FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_asyncio
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_datetime
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_LED_Animation
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Logging
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_NeoPixel
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Pixelbuf
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Requests
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Ticks
FROZEN_MPY_DIRS += $(TOP)/frozen/busytime
