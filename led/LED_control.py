

# 16-60, 60-250, 250-2k, 2k-4k, 4k-6k, 6k-16k
# for 16 kHz need 32kHz sampling frequency
# 6 bins
# apt-get install pip3
# pip3 install rpi_ws281x
# pip3 install paho-mqtt


# (r,g,b) 0-255
# 1. sub-bass:   br
# 2. bass:       bg
# 3. low-mids:   gr
# 4. highmids:   gb
# 5. presence:   rb
# 6. brilliance: rg

# color1 = amplitude * 0.4
# color2 = amplitude * 0.1




import time
import argparse
import paho.mqtt.client as mqtt
from rpi_ws281x import *


# LED Strip Configuration
LED_COUNT      = 150     # Number of LED pixels.
# LED_PIN        = 18      # GPIO pin connected to the pixels (18 uses PWM!).
LED_PIN        = 10      # GPIO pin connected to the pixels (10 uses SPI /dev/spidev0.0).
LED_FREQ_HZ    = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA        = 10      # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 255     # Set to 0 for darkest and 255 for brightest
LED_INVERT     = False   # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL = 0 # set to '1' for GPIOs 13, 19, 41, 45 or 53

def on_message(client, userdata, message):
    frequency = message.topic


if __name__ == '__main__':
    broker_address = "192.168.1.72"

    client = mqtt.Client("P1")
    client.on_message = on_message

    client.connect(broker_address)

    client.loop_start()

    client.subscribe("sub-bass")
    client.subscribe("bass")
    client.subscribe("low-mids")
    client.subscribe("high-mids")
    client.subscribe("presence")
    client.subscribe("brilliance")

    strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
    strip.begin()

    

    colors = []



