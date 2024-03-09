import board
import time
import analogio
import neopixel
import array
import microcontroller

from analogio import AnalogIn
from adafruit_led_animation.animation.solid      import Solid
from adafruit_led_animation.animation.colorcycle import ColorCycle
from adafruit_led_animation.animation.blink      import Blink
from adafruit_led_animation.animation.comet      import Comet
from adafruit_led_animation.animation.chase      import Chase
from adafruit_led_animation.animation.pulse      import Pulse
from adafruit_led_animation.sequence             import AnimationSequence
from adafruit_led_animation.color                import PURPLE, WHITE, AMBER, JADE, TEAL, PINK, MAGENTA, ORANGE
from teaandtechtime_fft import fft
from math import sin, pi

##########################################################
##							FFT							##
##########################################################
# assign the fft size we want to use, must be power of 2
n_samples = 64
# fs ~= 22,4 kHz
# res = fs / n_samples ~= 87.5 Hz

mic_pin = board.GP26_A0
#mic_pin = board.GP27_A1 # To test the fft with the Function Generator

# Create basic data structure to hold samples
samples = array.array('f', [0] * n_samples)

# Initialize the analog object
samples = analogio.AnalogIn(mic_pin)

##########################################################
##						NEOPIXEL						##
##########################################################
# Resources:
# https://learn.adafruit.com/getting-started-with-raspberry-pi-pico-circuitpython/neopixel-leds
# https://learn.adafruit.com/circuitpython-led-animations/colors
# https://docs.circuitpython.org/projects/neopixel/en/latest/
pixel_pin  = board.GP2
pixel_num  = 16
brightness = 0.05
auto_write = False # if False, to have the pixels turn on, pixels.show() must be used

# Create the LED strip object. The order of colors will be RGB
strip = neopixel.NeoPixel(pixel_pin, pixel_num, brightness=brightness, auto_write=auto_write)

solid      = Solid(strip, color=PINK)
blink      = Blink(strip, speed=0.5, color=JADE)
colorcycle = ColorCycle(strip, speed=0.4, colors=[MAGENTA, ORANGE, TEAL])
chase      = Chase(strip, speed=0.1, color=WHITE, size=3, spacing=6)
comet      = Comet(strip, speed=0.01, color=PURPLE, tail_length=10, bounce=True)
pulse      = Pulse(strip, speed=0.1, color=AMBER, period=3)
animations = AnimationSequence(solid, blink, colorcycle, chase, comet, pulse, advance_interval=5, auto_clear=True)

def Wheel(WheelPos):
    WheelPos = 255 - WheelPos % 255
    if WheelPos < 85:
        return (255 - WheelPos * 3, 0, WheelPos * 3)
    elif WheelPos < 170:
        WheelPos -= 85
        return (0, WheelPos * 3, 255 - WheelPos * 3)
    else:
        WheelPos -= 170
        return (WheelPos * 3, 255 - WheelPos * 3, 0)

def sampling(samples, n_samples):
    complex_samples = []
    for n in range(n_samples):
        sample = samples.value / 32768.0
        complex_samples.append(sample + 0.0j)
    return complex_samples
        
def do_fft(complex_samples):
    fft_values = fft(complex_samples) # Obtain fft values
    magnitudes = [abs(f) for f in fft_values] # Calculate amplitude of each bin
    return magnitudes

bass_th = 1
pix = int(pixel_num/2)

def hori_loop(samples, n_samples, complex_samples, magnitudes):
    global pix
    
    for j in range(25600):
        wheel_color = Wheel(j)
        max_bin = magnitudes[1]; # Not counting component 0
        for i in range(2, 3):
            if magnitudes[i] > max_bin:
                max_bin = magnitudes[i]
    #     for i in range(1, 12):  # Iterar sobre el rango de posiciones deseado
    #         print(f"{43.75 + i*87.5} - {magnitudes[i]}")

        strip.fill(wheel_color)
        # changing the direccion of the 3 LED if a beat was heard
        if max_bin > bass_th:
            pix1 = pix % pixel_num
            pix2 = (pix + 1) % pixel_num
            pix3 = (pix + 2) % pixel_num
        else:
            pix1 = pix % pixel_num
            pix2 = (pix - 1) % pixel_num
            pix3 = (pix - 2) % pixel_num
        strip[pix1] = (255, 255, 255);
        strip[pix2] = (255, 255, 255);
        strip[pix3] = (255, 255, 255);
        strip.show()
        pix = pix2
        #time.sleep(0.02)
        #microcontroller.delay_us(154)
        
        complex_samples = sampling(samples, n_samples)
        magnitudes      = do_fft(complex_samples)

while True:
    #animations.animate()
    complex_samples = sampling(samples, n_samples)
    magnitudes      = do_fft(complex_samples)
    hori_loop(samples, n_samples, complex_samples, magnitudes)