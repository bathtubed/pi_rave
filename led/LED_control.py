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

# mosquitto_pub -h 192.168.1.75 -t channel/sub-bass -m 100 && mosquitto_pub -h 192.168.1.74 -t channel/bass -m 0 && mosquitto_pub -h 192.168.1.74 -t channel/low-mids -m 0 && mosquitto_pub -h 192.168.1.74 -t channel/high-mids -m 0 && mosquitto_pub -h 192.168.1.74 -t channel/presence -m 0 && mosquitto_pub -h 192.168.1.74 -t channel/brilliance -m 0

import time
import argparse
import paho.mqtt.client as mqtt
from rpi_ws281x import *
from collections import deque


# LED Strip Configuration
LED_COUNT      = 150     # Number of LED pixels.
# LED_PIN        = 18      # GPIO pin connected to the pixels (18 uses PWM!).
# LED_PIN        = 10      # GPIO pin connected to the pixels (10 uses SPI /dev/spidev0.0).
LED_PIN        = 21      # use PCM
LED_FREQ_HZ    = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA        = 10      # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 255     # Set to 0 for darkest and 255 for brightest
LED_INVERT     = False   # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL = 0 # set to '1' for GPIOs 13, 19, 41, 45 or 53

FFT_MAX = 255
FFT_MIN = 0

RGB_MIN = 0
RGB_MAX = 255

BPM = 120
PERIOD = 1/30.0
MODE = "simple"

def on_channel_message(client, userdata, message):
    global data_queues
    global vals
    global FFT_MAX
    global FFT_MIN
    val = int(message.payload.decode())
    vals.append(val)
    # wait until there are a few values
    if len(vals) > 5:
        FFT_MAX = max(vals)
        FFT_MIN = min(vals)

    # data_queues[message.topic[8:]].append( ( fft_map(val), time.time() ) )
    data_queues[message.topic[8:]].append( ( val, time.time() ) )

def on_input_message(client, userdate, message):
    global MODE
    global BPM
    global PERIOD
    inp = message.payload.decode()
    if message.topic == "input/mode":
        print("updating mode to {}".format(inp))
        MODE = inp
    elif message.topic == "input/bpm":
        print("updating bpm to {}".format(inp))
        BPM = int(inp)
    elif message.topic == "input/update_frequency":
        print("updating update frequncy to {}".format(inp))
        PERIOD = 1.0/int(inp)

def fft_map(x):
    global FFT_MAX
    global FFT_MIN
    if (FFT_MAX == FFT_MIN):
        FFT_MIN -= 1
    return 255*(x - FFT_MIN) / (FFT_MAX - FFT_MIN)

def scale(x, scale_min, scale_max, range_min, range_max):
    if (range_min == range_max):
        return scale_min
    return int(scale_max*(x - range_min) / (range_max - range_min) + scale_min)

def ready():
    for channel in data_queues.keys():
        if len(data_queues[channel]) == 0:
            return False
    return True

def get_data(data_queues):
    now = time.time()
    data = {}
    for channel in data_queues.keys():
        count = 0
        total = 0
        while(len(data_queues[channel]) > 0 and data_queues[channel][0][1] <= now):
            (val, _) = data_queues[channel].popleft()
            total += val
            count += 1
        avg = total/count
        data[channel] = avg
    # return(data["sub-bass"], data["bass"], data["low-mids"], data["high-mids"], data["presence"], data["brilliance"])
    return data

def colorWipe(strip, color, wait_ms=50):
    """Wipe color across display a pixel at a time."""
    for i in range(strip.numPixels()):
        strip.setPixelColor(i, color)
        strip.show()
        time.sleep(wait_ms/1000.0)

def mode_simple(strip, data_queues, output): 
    # (sub, bass, low, high, presence, brilliance) = get_data(data_queues)
    data = get_data(data_queues)

    # blue   = int((high*0.7 + presence*0.15 + brilliance*0.15)*0.9)
    # green = int((low*0.7 + presence*0.15 + brilliance*0.15)*0.9)
    # red  = int((sub*0.5 + bass*0.5)*0.7 + presence*0.15 + brilliance*0.15)
    # rgb = Color(red,          # red
    #             green,   # green
    #             blue)     # blue
    rgb = gen_color(data)
    output.insert(0,rgb)
    print("simple: {}".format(rgb))
    output.pop()
    for i,color in enumerate(output):
        strip.setPixelColor(i, color)
    strip.show()
    return output

def gen_color(data):
    d = dict(data)
    key_val_sorted = []
    while len(d) > 0:
        key = max(d, key=d.get)
        key_val_sorted.append( (key, d[key]) )
        del d[key]

    # these are instaneous min and max, switch to using FFT_MIN and FFT_MAX if this doesn't look right
    maximum = key_val_sorted[0][1]
    minimum = key_val_sorted[-1][1]

    # (255, 0, 0) (255, 255, 0) (0,255,0) (0,255,255) (0,0,255)
    # sub-bass    bass          high-mids low-mids    presence
    # brilliance adds flat values to all channels based on its amplitude

    color_map = {"sub-bass":[1,0,0],
                 "bass":[1,1,0],
                 "low-mids":[0,1,0],
                 "high-mids":[0,1,1],
                 "presence":[0,0,1],
                 "brilliance":[1,1,1],}

    # two options: 
    # 1. use all at once
    # 2. use only top x(probably 2) with brilliance added at end

    # option 1
    rgb = [0,0,0]
    for pair in key_val_sorted:
        key = pair[0]
        scaled = scale(pair[1], RGB_MIN, RGB_MAX, minimum, maximum)
        for i,_ in enumerate(rgb):
            rgb[i] += scaled*color_map[key][i]

    color_min = min(rgb)
    color_max = max(rgb)
    for i,val in enumerate(rgb):
        rgb[i] = scale(val, RGB_MIN, RGB_MAX, color_min, color_max)
    return Color(rgb[0], rgb[1], rgb[2])

    # option 2
    # color = [0,0,0]
    # keys = []
    # for pair in key_val_sorted[:2]:
    #     keys.append(pair[0])
    # if "brilliance" not in keys:
    #     keys.append("brilliance")

    # for pair in key_val_sorted[:2]:
    #     if pair[0] not in keys:
    #         continue
    #     scaled = scale(key_val_sorted[key], RGB_MIN, RGB_MAX, minimum, maximum)
    #     for i,_ in enumerate(color):
    #         color[i] += scaled*color_map[key][i]

    # color_min = min(color)
    # color_max = max(color)
    # for i,val in color:
    #     color[i] = scale(val, RGB_MIN, RGB_MAX, color_min, color_max)
    # return Color(color[0], color[1], color[2])

def mode_spectrum(strip, data_queues, output):
    (sub, bass, low, high, presence, brilliance) = get_data(data_queues)
    # sub_start = 0
    # bass_start = 25
    # low_start = 50
    # high_start = 75
    # presence_start = 100
    # brilliance_start = 125
    start = [0, 25, 50, 75, 100, 125]
    center = [x + 13 for x in start]
    # difference between adjacent channels determines center position
    

    return outptut

if __name__ == '__main__':
    print("hello chernl")
    # broker_address = "192.168.1.72"
    broker_address = "mosquitto"
    # 192.168.1.74:

    client = mqtt.Client("P1")
    client.message_callback_add("channel/#", on_channel_message)
    client.message_callback_add("input/#", on_input_message)

    print("connecting to broker")
    client.connect(broker_address)

    client.loop_start()

    client.subscribe("channel/#")
    client.subscribe("input/#")

    strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
    strip.begin()
    print("strip started")

    data_queues = {"sub-bass":deque(),
                   "bass":deque(),
                   "low-mids":deque(),
                   "high-mids":deque(),
                   "presence":deque(),
                   "brilliance":deque(),}

    vals = deque(maxlen=3000)

    output = [Color(0,0,0)] * 150

    print("loop started")
    time.sleep(PERIOD)
    while(1):
        if ready(): # if there is data available for every channel
            if MODE == "standby":
                continue
            elif MODE == "simple":
                output = mode_simple(strip, data_queues, output)
            elif MODE == "spectrum":
                output = mode_spectrum(strip, data_queues, output)
            elif MODE == "exit":
                colorWipe(strip, Color(0,0,0), 5)
                break

            
            time.sleep(PERIOD)
        if MODE == "exit":
            colorWipe(strip, Color(0,0,0), 5)
            break

    print("bye")






# mosquitto_pub -h 192.168.1.75 -t channel/sub-bass -m 0 && mosquitto_pub -h 192.168.1.75 -t channel/bass -m 0 && mosquitto_pub -h 192.168.1.75 -t channel/low-mids -m 0 && mosquitto_pub -h 192.168.1.75 -t channel/high-mids -m 0 && mosquitto_pub -h 192.168.1.75 -t channel/presence -m 0 && mosquitto_pub -h 192.168.1.75 -t channel/brilliance -m 0