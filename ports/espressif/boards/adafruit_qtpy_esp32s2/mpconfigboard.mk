USB_VID = 0x239A
USB_PID = 0x8112

USB_PRODUCT = "Adafruit QT Py ESP32S2"
USB_MANUFACTURER = "Adafruit"

IDF_TARGET = esp32s2

CIRCUITPY_ESP_FLASH_MODE = dio
CIRCUITPY_ESP_FLASH_FREQ = 40m
CIRCUITPY_ESP_FLASH_SIZE = 4MB

BUSYTIME_ENABLED = 1
CIRCUITPY_ESPCAMERA = 0

# Features actively turned off
CIRCUITPY_AUDIOBUSIO = 0
CIRCUITPY_AUDIOCORE = 0
CIRCUITPY_AUDIOIO = 0
CIRCUITPY_AUDIOMIXER = 0
CIRCUITPY_DISPLAYIO = 0
CIRCUITPY_FRAMEBUFFERIO = 0
CIRCUITPY_GIFIO = 0
CIRCUITPY_PARALLELDISPLAY = 0
CIRCUITPY_RGBMATRIX = 0
CIRCUITPY_STATUS_BAR = 0
CIRCUITPY_STATUS_LED = 0
CIRCUITPY_WEB_WORKFLOW = 0
# end disabled features

FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_asyncio
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_datetime
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_LED_Animation
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Logging
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_NeoPixel
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_NTP
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Pixelbuf
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Requests
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Ticks
FROZEN_MPY_DIRS += $(TOP)/frozen/busytime
