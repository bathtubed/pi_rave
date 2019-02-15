#!/usr/bin/env python3

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

# apt-get install pip3
# pip3 install rpi_ws281x paho-mqtt

# x axis is frequency spectrum
# 

import time
import argparse
import paho.mqtt.client as mqtt
from rpi_ws281x import *
from queue import Queue


# LED Strip Configuration
LED_COUNT      = 150     # Number of LED pixels.
# LED_PIN        = 18      # GPIO pin connected to the pixels (18 uses PWM!).
# LED_PIN        = 10      # GPIO pin connected to the pixels (10 uses SPI /dev/spidev0.0).
LED_PIN = 21             # use PCM
LED_FREQ_HZ    = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA        = 10      # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 255     # Set to 0 for darkest and 255 for brightest
LED_INVERT     = False   # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL = 0 # set to '1' for GPIOs 13, 19, 41, 45 or 53

MAX_FFT = 255
MIN_FFT = 0

BPM = 60

def fft_map(x):
    return 255*(x-FFT_MIN)/(FFT_MAX - FFT_MIN)

def ready():
    for key in data_queues.keys():
        if data_queues[channel].empty():
            return False
    return True

def on_channel_message(client, userdata, message):
    global data_queues
    data_queues[message.topic].put(fft_map(message.payload))

def on_input_message(client, userdate, message):
    pass

def colorWipe(strip, color, wait_ms=50):
    """Wipe color across display a pixel at a time."""
    for i in range(strip.numPixels()):
        strip.setPixelColor(i, color)
        strip.show()
        time.sleep(wait_ms/1000.0)

if __name__ == '__main__':
    print("hello chernl")
    # broker_address = "192.168.1.72"

    # client = mqtt.Client("P1")
    # client.message_callback_add("channel/#", on_channel_message)
    # client.message_callback_add("input/#", on_input_message)

    # client.connect(broker_address)

    # client.loop_start()

    # topics = ["channel/sub-bass", "channel/bass", "channel/low-mids", "channel/high-mids", "channel/presence", "channel/brilliance", "input/mode", "input/bpm"]

    # client.subscribe("channel/#")
    # client.subscribe("input/#")

    strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
    strip.begin()

    print("red")
    colorWipe(strip, Color(255, 0, 0))  # Red wipe
    print("blue")
    colorWipe(strip, Color(0, 255, 0))  # Blue wipe
    print("green")
    colorWipe(strip, Color(0, 0, 255)) # Green wipe
    print("off")
    colorWipe(strip, Color(0,0,0), 10) #turn off
    print("finish")

    # data_queues = {"sub-bass":Queue(),
    #                "bass":Queue(),
    #                "low-mids":Queue(),
    #                "high-mids":Queue(),
    #                "presence":Queue(),
    #                "brilliance":Queue(),}

    # output = [Color(0,0,0)] * 150
    # while(1):
    #     if ready():
    #         su = data_queues["sub-bass"].get()
    #         ba = data_queues["bass"].get()
    #         lo = data_queues["low-mids"].get()
    #         hi = data_queues["high-mids"].get()
    #         pr = data_queues["presence"].get()
    #         br = data_queues["brilliance"].get()
    #         rgb = Color(br*0.4+pr*0.4+lo*0.1+su*0.1, hi*0.4+lo*0.4+ba*0.1+br*0.1, su*0.4+ba*0.4+hi*0.1+br*0.1)
    #         ouput.insert(0,rgb)
    #         output.pop()
    #         for i,color in enumerate(output):
    #             strip.setPixelColor(i, color)
    #         strip.show()
    #     # 1 / beats per minute / 60 seconds per minute
    #     time.sleep(1/(BPM/60.0))








