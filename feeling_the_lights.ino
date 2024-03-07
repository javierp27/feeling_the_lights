#include <Adafruit_NeoPixel.h>
#include "arduinoFFT.h"

#define LED        2   // LED on the Arduino board, used for debugging if necessary
#define STRIP_PIN  2    // LED strip pin
#define N_LED      10   // Number of LEDs

// Variables to play with
const int    bass_th   = 500; // Threshold for the bass amplitude
const int    wait      = 20;  // Dely for the display of the leds, more wait ==> more slow the animation

// FFT
// samples should be power of 2
// Resolution of frequencies is:   res  = Fs/samples
// Number of frequencies bins are: n_f  = samples/2 
// Max frequency will be:          mx_f = Fs/2-resolution

// Asumming:
// Fs      = 6400 kHz
// samples = 128
// ==> res = ~50 Hz that means each component of the vector vReal will correspond to the intensity for the bin
const int    samples   = 128;
const double Fs        = 6400;
volatile double  vReal[samples];
double           vImag[samples];

// Variables for the ADC
const int        prescaler = 32; // Frequency divider for the ADC
volatile int     idx       = 0;  // index of the ADC reading
volatile boolean samp_done = 0;  // flag if the ADC finished the number of samples

uint32_t wheel_color; // Variabls with the 32bits value of the color wheel for the full strip

// Defining the objects
Adafruit_NeoPixel strip(N_LED, STRIP_PIN, NEO_GRB + NEO_KHZ800);
arduinoFFT FFT;

// Setup of the whole arduino, do not change!
void setup () {
  Serial.begin(115200);
  delay(200);

  pinMode(LED, OUTPUT);
  delay(200);

  // First clear all bits of the ADC Registers
  ADMUX  = 0x00;
  ADCSRA = 0x00;
  ADCSRB = 0x00;
  delay(200);

  // Configure the ADC
  // ADMUX[7:6] = 01   ==> AVCC with external capacitor at AREF pin
  // ADMUX[5]   = 0    ==> AVCC left adjust the result
  // ADMUX[4]          ==> Reserved
  // ADMUX[3:0] = 0000 ==> ADC0
  ADMUX  = 0b01000000;

  // ADCSRA[7]   = 1   ==> ADC Enable
  // ADCSRA[6]   = 0   ==> ADC NO Start Conversion
  // ADCSRA[5]   = 1   ==> ADC Auto Trigger Enable
  // ADCSRA[4]   = 0   ==> ADC Interrupt Flag - used by the ADC, DO NOT overwrite
  // ADCSRA[3]   = 0   ==> ADC Interrupt Disable
  // ADCSRA[2:0] = 111 ==> ADC Prescaler Select = 128
  switch (prescaler) {
    case 0:
      ADCSRA = 0b10100000;
      break;
    case 2:
      ADCSRA = 0b10100001;
      break;
    case 4:
      ADCSRA = 0b10100010;
      break;
    case 8:
      ADCSRA = 0b10100011;
      break;
    case 16:
      ADCSRA = 0b10100100;
      break;
    case 32:
      ADCSRA = 0b10100101;
      break;
    case 64:
      ADCSRA = 0b10100110;
      break;
    case 128:
      ADCSRA = 0b10100111;
      break;
  }

  // ADCSRB[7]   = 0   ==> Reserved
  // ADCSRB[6]   = 0   ==> Reserved
  // ADCSRB[5]   = 0   ==> Reserved
  // ADCSRB[4]   = 0   ==> Reserved
  // ADCSRB[3]   = 0   ==> Reserved
  // ADCSRB[2:0] = 000 ==> ADC Auto Trigger Source when ADCSRA[5] = 1 - Free Running mode
  ADCSRB = 0b00000000;
  delay(200);

  sei(); // Enable the Interrptions

  /* ADMUX – ADC Multiplexer Selection Register
  Bit	7	    6	    5   	4	   3	    2	    1	    0
  0x7C	REFS1	REFS0	ADLAR	–	   MUX3	MUX2	MUX1	MUX0

  REFS[1:0]: Reference Selection Bits
  00 - Use external VREF, Internal VREF turned off
  01 - AVCC with external capacitor at AREF pin
  10 - Reserved
  11 - Internal 1.1V Voltage Reference with external capacitor at AREF pin

  ADLAR: ADC Left Adjust Result    Bit	15	  14	  13	  12	  11	  10	  9	    8    7    6     5     4     3     2     1     0
  0 - left adjust the result           –	    –	    –	    – 	  –	    –	    ADC9	ADC8 ADC7	ADC6	ADC5  ADC4	ADC3	ADC2	ADC1	ADC0
  1 - right adjust the result          ADC9	ADC8	ADC7	ADC6	ADC5	ADC4	ADC3	ADC2 ADC1	ADC0	–	    –	    –	    –	    –	    –

  MUX[3:0]: Analog Channel Selection Bits
  0000 - ADC0
  0001 - ADC1
  0010 - ADC2
  0011 - ADC3
  0100 - ADC4
  0101 - ADC5
  0110 - ADC6
  0111 - ADC7
  1xxx - (reserved) 
  -------------------------------------------------------------------
    ADCSRA – ADC Control and Status Register A
  Bit	7	    6	    5   	4	    3	    2	    1	    0
  0x7A	ADEN	ADSC	ADATE	ADIF	ADIE	ADPS2	ADPS1	ADPS0

  ADEN  = 1 ==> ADC Enable
  ADSC  = 1 ==> ADC Start Conversion
  ADATE = 1 ==> ADC Auto Trigger Enable
  ADIF      ==> ADC Interrupt Flag
  ADIE  = 1 ==> ADC Interrupt Enable
  ADPS[2:0]: ADC Prescaler Select Bits
  000 - 2
  001 - 2
  010 - 4
  011 - 8
  100 - 16
  101 - 32
  110 - 64
  111 - 128 
  -------------------------------------------------------------------
    ADCSRB – ADC Control and Status Register B - If ADATE in ADCSRA = 1, the value of these bits selects which source will trigger an ADC conversion.
  Bit	7	    6	    5   	4	    3	    2	    1	    0
        –	    ACME	–   	–	    –	    ADTS2	ADTS1	ADTS0

  ADTS[2:0]: ADC Auto Trigger Source
  000 - Free Running mode
  001 - Analog Comparator
  010 - External Interrupt Request 0
  011 - Timer/Counter0 Compare Match A
  100 - Timer/Counter0 Overflow
  101 - Timer/Counter1 Compare Match B
  110 - Timer/Counter1 Overflow
  111 - Timer/Counter1 Capture Event
  */

  strip.begin();
  strip.setBrightness(20);  // Can be up to 255, but it will be too much and the power consumption will kill the battery
}

// Interruption of the ADC reading
ISR(ADC_vect) {
  vReal[idx] = ADC;
  vImag[idx] = 0;
  ++idx %= samples;
  if (idx == 0) {
    samp_done = 1;
    bitClear(ADCSRA, 3); // ADIE = 0 ==> ADC Interrupt Disable
    bitClear(ADCSRA, 6); // ADSC = 0 ==> ADC Don't Start Conversion
  }
}
// Funtion that will enable the ADC and do a continous reading. While being IDLE the wheel_color will be calculated
void sampling(int j) {
  samp_done = 0;
  bitSet(ADCSRA, 3); // ADIE = 1 ==> ADC Interrupt Enable
  bitSet(ADCSRA, 6); // ADSC = 1 ==> ADC Start Conversion
  while (!samp_done) {
    wheel_color = Wheel(j);
  }
}

// Processing the audio, that means doing the FFT with the ADC values read that were stored in vReal
void processAudio() {
  FFT.DCRemoval(vReal, samples);
  FFT = arduinoFFT(vReal, vImag, samples, Fs);
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();
}

// Will calculate the corresponding color for the full strip
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos%255;
  if (WheelPos < 85)
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  else 
    if (WheelPos < 170) {
      WheelPos -= 85;
      return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
      WheelPos -= 170;
      return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

// Will sample the ADC, the do the FFT and finally average the bin 3, 4, and 5 (corresponding to f = ~210 to ~351 Hz)
int avg_bass() {
  return((vReal[2] + vReal[3] + vReal[4]) / 3);
}

void loop () {
  javi_loop_1();
}

void javi_loop_1() {
  int volumen, direction;
  uint16_t i, j, vel, shift, counter;

  vel = 2;
  direction = 1;
  counter = 0;

  // This for loop could be better done... and I am looking at you Omar
   for(j = 0; j < 10000; j++) {
    strip.fill(wheel_color, 0, 0);  // Fill the full strip with the corresponding color of the wheel
    
    sampling(j+1);  // Sample the ADC
    processAudio(); // Calculate FFT
    // volumen = avg_bass();
    counter++;
    
    if (j%10 == 0) {
      Serial.print(String(vReal[0]) + " -- " + String(vReal[1]) + " -- " + String(vReal[2]) + " -- " + String(vReal[3]) + " -- " + String(vReal[4]) + " -- " + String(vReal[5]) + " -- " + String(vReal[6]) + " -- " + String(vReal[7]) + " -- ");
      Serial.println(String(vReal[8]) + " -- " + String(vReal[9]) + " -- " + String(vReal[10]) + " -- " + String(vReal[11]) + " -- " + String(vReal[12]) + " -- " + String(vReal[13]) + " -- " + String(vReal[14]) + " -- " + String(vReal[15]));
    } 
    // Finding the max value of the bass frequencies
    int max = vReal[1]; // Not counting component 0
    for (i = 2; i < 4; i++)
      if (vReal[i] > max)
        max = vReal[i];

    if (max>bass_th && counter>2) { // Check if there's a max, but also that the previos max happened "counter" steps ago
      direction = direction * -1;
      counter = 0;
    }

    // changing the direccion of the 3 LED if a beat was heard
    strip.setPixelColor((direction*j/vel)%10, strip.Color(255, 255, 255));
    strip.setPixelColor((direction*j/vel-1)%10, strip.Color(155, 155, 155));
    strip.setPixelColor((direction*j/vel+1)%10, strip.Color(155, 155, 155));
    strip.show();

    delay(wait); // this wait will control the speed of the refresh, more wait ==> slower
  }
}