#include <Adafruit_NeoPixel.h>
#include "QuantumLED.h"

#include "arduinoFFT.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"
#include "SD.h"

// #define LED_PIN 2  // LED strip pin
// #define N_LED 18  // Number of LEDs
#define MIC_PIN A0  // Microphone input pin

bool deb = 1;    // debug mode
volatile int counter = 1; // Interrupt counter

// Digital IO
#define SW         23   // Switch pin
#define USER_LED   25

// LED STRIP
#define STRIP_PIN  2    // LED strip pin
#define N_LED      18   // Number of LEDs

// TFT pin def SPI0 (default)
#define TFT_BLK  15   // Back_Light
#define TFT_DC   13   // Data_Command
#define TFT_RES  14   // Reset
#define TFT_CS   17   // Chip Select
//#define TFT_MISO 16   // Not used
//#define TFT_MOSI 19
//#define TFT_SCK  18
#define TFT_SPI_SPEED  100 * 1000000  // 130 MHz

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240
int blk_int = 128;   // tft backlight intensity

// SD def SPI1
#define SD_MISO 8   // AKA RX
#define SD_MOSI 11  // AKA TX
#define SD_CS   9
#define SD_SCK  10
#define SD_SPI_SPEED  23 * 1000000  // 23 MHz is the maximum

#define BUFFPIXEL 20
File f;

// ADC input 
const int MIC  = 26; // GPIO 26 (Analog ADC0)

Adafruit_NeoPixel strip(N_LED, STRIP_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RES);

void bmpDraw(char *filename, uint8_t x, uint16_t y);


// Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LED, LED_PIN, NEO_GRB + NEO_KHZ800); Javi version


const int tailIntensities3[] = {70, 20, 10};
const int tailIntensities4[] = {70, 20, 10, 5};
const int tailIntensities5[] = {70, 20, 10, 5, 5};
const int tailIntensities_eff2[] = {70, 70};

uint32_t ledColors[N_LED] = {0};
uint32_t headPositions[N_LED] = {0};

void setup() {

    SPI1.setRX(SD_MISO);
    SPI1.setTX(SD_MOSI);
    SPI1.setSCK(SD_SCK);
    bool sdInit = SD.begin(SD_CS, SD_SPI_SPEED, SPI1);

    pinMode(TFT_RES, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_BLK, OUTPUT);
    analogWriteFreq(5000);
    analogWriteRange(256);
    analogWriteResolution(8);
    analogWrite(TFT_BLK, blk_int);
    tft.begin(TFT_SPI_SPEED);
    //testFilledRoundRects();

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(STRIP_PIN, OUTPUT);
    pinMode(SW, INPUT);

    attachInterrupt(digitalPinToInterrupt(SW), handleInterrupt, FALLING);

    f = SD.open("image.bmp", FILE_READ);  // open SD card main root
    uint8_t nameSize = String(f.name()).length();  // get file name size
    String str1 = String(f.name()).substring( nameSize - 4 );  // save the last 4 characters (file extension)
    if (str1.equalsIgnoreCase(".bmp"))  // if the file has '.bmp' extension
      bmpDraw("image.bmp", 0, 0);       // draw it
    bmpDraw("javi.bmp", 0, 0);


    
    Serial.begin(115200);
    strip.begin();
    strip.setBrightness(100);
    strip.show();
    delay(20);
}



void handleInterrupt() {
  counter++;
  if (counter > 5) { // Reset counter if it exceeds number of effects
    counter = 1;
  }
}



void runEffect3(int volume_mapped) {
    // Define thresholds for each Quantum instance
    const int threshold_1 = 20;
    const int threshold_2 = 35;
    const int threshold_3 = 50;

    static Quantum<4, N_LED> Quantum_1(strip.Color(0, 0, 155), 3, tailIntensities4, 1, 1);
    static Quantum<4, N_LED> Quantum_2(strip.Color(0, 155, 0), 5, tailIntensities4, -1, 1);
    static Quantum<4, N_LED> Quantum_3(strip.Color(155, 0, 0), 4, tailIntensities4, 1, 2);
    static Quantum<4, N_LED> Quantum_4(strip.Color(100, 100, 100), 7, tailIntensities4, -1, 2);

    handleCollisions(Quantum_1, Quantum_2);
    handleCollisions(Quantum_2, Quantum_3);
    handleCollisions(Quantum_1, Quantum_3);
    handleCollisions(Quantum_1, Quantum_2);
    handleCollisions(Quantum_1, Quantum_4);
    handleCollisions(Quantum_2, Quantum_4);
    handleCollisions(Quantum_3, Quantum_4);

    Quantum_1.move();
    Quantum_2.move();
    Quantum_3.move();
    Quantum_4.move();

    uint32_t tempLedColors[N_LED] = {0};
    uint32_t tempHeadPositions[N_LED] = {0};

    // Update the intensity of each Quantum instance based on the volume_mapped
    int intensity_1 = 100;
    int intensity_2 = (volume_mapped > threshold_1) ? 100 : 0;
    int intensity_3 = (volume_mapped > threshold_2) ? 100 : 0;
    int intensity_4 = (volume_mapped > threshold_3) ? 100 : 0;

    Quantum_1.display(tempLedColors, tempHeadPositions, intensity_1);
    Quantum_2.display(tempLedColors, tempHeadPositions, intensity_2);
    Quantum_3.display(tempLedColors, tempHeadPositions, intensity_3);
    Quantum_4.display(tempLedColors, tempHeadPositions, intensity_4);

    for (int i = 0; i < N_LED; i++) {
        ledColors[i] = tempLedColors[i];
        headPositions[i] = tempHeadPositions[i];
        strip.setPixelColor(i, ledColors[i]);
    }
}


void runEffect4(int volume_mapped) {

    static Quantum<4, N_LED> Quantum_1(strip.Color(0, 0, 155), 3, tailIntensities4, 1, 1);
    static Quantum<4, N_LED> Quantum_2(strip.Color(0, 155, 0), 5, tailIntensities4, -1, 1);
    static Quantum<4, N_LED> Quantum_3(strip.Color(155, 0, 0), 4, tailIntensities4, 1, 2);
    static Quantum<4, N_LED> Quantum_4(strip.Color(100, 100, 100), 7, tailIntensities4, -1, 2);

    handleCollisions(Quantum_1, Quantum_2);
    handleCollisions(Quantum_2, Quantum_3);
    handleCollisions(Quantum_1, Quantum_3);
    handleCollisions(Quantum_1, Quantum_2);
    handleCollisions(Quantum_1, Quantum_4);
    handleCollisions(Quantum_2, Quantum_4);
    handleCollisions(Quantum_3, Quantum_4);

    Quantum_1.move();
    Quantum_2.move();
    Quantum_3.move();
    Quantum_4.move();

    uint32_t tempLedColors[N_LED] = {0};
    uint32_t tempHeadPositions[N_LED] = {0};

    Quantum_1.display(tempLedColors, tempHeadPositions, 100 -  2 * volume_mapped);
    Quantum_2.display(tempLedColors, tempHeadPositions, 100 - 2 * volume_mapped);
    Quantum_3.display(tempLedColors, tempHeadPositions, volume_mapped * 2 + 15);
    Quantum_4.display(tempLedColors, tempHeadPositions, volume_mapped * 2 + 15);

    for (int i = 0; i < N_LED; i++) {
        ledColors[i] = tempLedColors[i];
        headPositions[i] = tempHeadPositions[i];
        strip.setPixelColor(i, ledColors[i]);
    }
}


void runEffect5(int volume_mapped) {

    static Quantum<4, N_LED> Quantum_1(strip.Color(0, 0, 155), 3, tailIntensities4, 1, 1);
    static Quantum<4, N_LED> Quantum_2(strip.Color(155, 0, 0), 3, tailIntensities4, -1, 1);

    Quantum_1.move();
    Quantum_2.move();

    uint32_t tempLedColors[N_LED] = {0};
    uint32_t tempHeadPositions[N_LED] = {0};

    Quantum_1.changeVelocity(int(volume_mapped/2));
    Quantum_1.display(tempLedColors, tempHeadPositions, 100);

    Quantum_2.changeVelocity(int(volume_mapped/3));
    Quantum_2.display(tempLedColors, tempHeadPositions, 100);


    for (int i = 0; i < N_LED; i++) {
        ledColors[i] = tempLedColors[i];
        headPositions[i] = tempHeadPositions[i];
        strip.setPixelColor(i, ledColors[i]);
    }
}


void runEffect1(int volume_mapped) {
    // static Quantum<3, N_LED> Quantum_1(strip.Color(142, 69, 133), 2, tailIntensities3, 1, 1);
    // static Quantum<3, N_LED> Quantum_2(strip.Color(128, 0, 32), 4, tailIntensities3, 1, 2);
    // static Quantum<3, N_LED> Quantum_3(strip.Color(0, 0, 128), 6, tailIntensities3, 1, 3);

    static Quantum<3, N_LED> Quantum_1(strip.Color(255, 0, 0), 2, tailIntensities3, 1, 1);
    static Quantum<3, N_LED> Quantum_2(strip.Color(0, 255, 0), 4, tailIntensities3, 1, 2);
    static Quantum<3, N_LED> Quantum_3(strip.Color(0, 0, 255), 6, tailIntensities3, 1, 3);

    Quantum_1.move();
    Quantum_2.move();
    Quantum_3.move();

    uint32_t tempLedColors[N_LED] = {0};
    uint32_t tempHeadPositions[N_LED] = {0};

    int volume_factor = 6;
    Quantum_1.changeVelocity(2*int((volume_mapped)/volume_factor));
    Quantum_2.changeVelocity(3*int((volume_mapped)/volume_factor));
    Quantum_3.changeVelocity(4*int((volume_mapped)/volume_factor));

    if (true) {
      Quantum_2.changeSpin(1);
      Quantum_3.changeSpin(1);
    } else {
      Quantum_2.changeSpin(2);
      Quantum_3.changeSpin(3);
    }; // change spin is not working

    Quantum_1.display(tempLedColors, tempHeadPositions, 100);
    Quantum_2.display(tempLedColors, tempHeadPositions, 100);
    Quantum_3.display(tempLedColors, tempHeadPositions, 100);


    for (int i = 0; i < N_LED; i++) {
        ledColors[i] = tempLedColors[i];
        headPositions[i] = tempHeadPositions[i];
        strip.setPixelColor(i, ledColors[i]);
    }
}


void loop () {

  int volume = getVolume(MIC_PIN);
  int volume_mapped = map(volume, 0, 200, 0, 100);
  
  // Serial.print(volume_mapped);
  // Serial.print(" ");
  // Serial.print(-20);
  // Serial.print(" ");
  // Serial.println(120);

  counter = 5;

  switch (counter) {
    case 1:
      runEffect1(volume_mapped);
      break;
    case 2:
      runEffect2(volume_mapped);
      break;
    case 3:
      runEffect3(volume_mapped);
      break;
    case 4:
      runEffect4(volume_mapped);
      break;
    case 5:
      runEffect4(volume_mapped);
      break;
    default:
      strip.clear(); // Clear the strip if no valid effect is selected
      break;
  }

  strip.show();
  delay(1);

}

void bmpDraw(const char *filename, uint8_t x, uint16_t y) {

  File     bmpFile;
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0;

  if((x >= tft.width()) || (y >= tft.height())) return;

  // Open requested file on SD card
  if (!(bmpFile = SD.open(filename))) {
    if (deb) Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) {     // BMP signature
    read32(bmpFile);
    (void)read32(bmpFile);    // Read & ignore creator bytes
    read32(bmpFile);          // Start of image data
    read32(bmpFile);          // Read DIB header
    w = read32(bmpFile);
    h = read32(bmpFile);
    if(read16(bmpFile) == 1) {    // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true;              // Supported BMP format -- proceed!
        rowSize = (w * 3 + 3) & ~3;  // BMP rows are padded (if needed) to 4-byte boundary

        // If bmpHeight is negative, image is in top-down order.
        if(h < 0) {
          h    = -h;
          flip = false;
        }

        // Crop area to be loaded
        if((x+w-1) >= tft.width())  
          w = tft.width()  - x;
        if((y+h-1) >= tft.height()) 
          h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.startWrite();
        tft.setAddrWindow(x, y, w, h);

        for (row=0; row<h; row++) { // For each scanline...
          // Seek to start of scan line.  It might seem labor-intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (h - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) {
            tft.endWrite();
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            if (buffidx >= sizeof(sdbuffer)) {
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0;
              tft.startWrite();
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.color565(r,g,b));
          }
        }
        tft.endWrite();
      }
    }
  }

  bmpFile.close();
  if(!goodBmp) if (deb) Serial.println(F("BMP format not recognized."));
}

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

unsigned long testRoundRects() {
  unsigned long start;
  int           w, i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(GC9A01A_BLACK);
  w     = min(tft.width(), tft.height());
  start = micros();
  for(i=0; i<w; i+=6) {
    i2 = i / 2;
    tft.drawRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(i, 0, 0));
  }

  return micros() - start;
}

void testFilledRoundRects() {
  int           i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(GC9A01A_BLACK);
  for(i=min(tft.width(), tft.height()); i>20; i-=6) {
    i2 = i / 2;
    tft.fillRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(0, i, 0));
    yield();
  }
}


void runEffect2(int volume_mapped) { //Add a shift to start with a white and create colors afterwards
    static Quantum<2, N_LED> Quantum_1(strip.Color(0, 0, 255), 5, tailIntensities_eff2, 1, 1);
    static Quantum<2, N_LED> Quantum_2(strip.Color(255, 0, 0), 5, tailIntensities_eff2, 1, 2);
    static Quantum<2, N_LED> Quantum_3(strip.Color(0, 255, 0), 5, tailIntensities_eff2, 1, 3);

    Quantum_1.move();
    Quantum_2.move();
    Quantum_3.move();

    Quantum_1.setShift(max(0, int(volume_mapped/8)));
    Quantum_2.setShift(min(0, -int(volume_mapped/8)));

    uint32_t tempLedColors[N_LED] = {0};
    uint32_t tempHeadPositions[N_LED] = {0};

    //Quantum_1.addShift(2);

    Quantum_1.display(tempLedColors, tempHeadPositions, 100);
    Quantum_2.display(tempLedColors, tempHeadPositions, 100);
    Quantum_3.display(tempLedColors, tempHeadPositions, 100);


    for (int i = 0; i < N_LED; i++) {
        ledColors[i] = tempLedColors[i];
        headPositions[i] = tempHeadPositions[i];
        strip.setPixelColor(i, ledColors[i]);
    }
}
