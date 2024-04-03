// This program calibrates an ESP32_UWB module intended for use as a fixed anchor point
// uses binary search to find anchor antenna delay to calibrate against a known distance
//
// modified version of Thomas Trojer's DW1000 library is required!

// Remote tag (at origin) must be set up with default antenna delay (library default = 16384)

// user input required, possibly unique to each tag:
// 1) accurately measured distance from anchor to tag
// 2) address of anchor
//
// output: antenna delay parameter for use in final anchor setup.
// S. James Remington 2/20/2022

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <DW1000.h>
#include <DW1000Device.h>
#include <DW1000Ranging.h>

// # TFT Connection Pins
#define TFT_CS 27
#define TFT_RST 25
#define TFT_DC 26
#define TFT_MOSI 13 // Data out
#define TFT_SCLK 14 // Clock out
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// UWB Connection Pins
#define UWB_RST 17 // Reset pin
#define UWB_IRQ 32 // IRQ pin
#define UWB_SS 33  // SPI Select pin

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

char Anchor_Addr[24];
String Anchor_Address_Str;

int this_anchor_target_distance = 2; // measured distance to anchor in m

uint16_t this_anchor_Adelay = 16600; // starting value
uint16_t Adelay_delta = 100;         // initial binary search step size

void newRange();
void newDevice(DW1000Device *device);
void inactiveDevice(DW1000Device *device);

void setup()
{
  Serial.begin(115200);

  // Init ST7789 280x240
  tft.init(240, 280);
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(2);

  tft.setCursor(80, 10);
  tft.println("C1 Tech");
  tft.setCursor(65, 30);
  tft.println("UWB Anchor");
  tft.setCursor(35, 50);
  tft.setTextColor(ST77XX_RED);
  tft.println("Calibraion Mode");
  tft.println();
  // init the configuration
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(UWB_RST, UWB_SS, UWB_IRQ); // Reset, CS, IRQ pin

  Serial.print("Initial Adelay: ");
  Serial.println(this_anchor_Adelay);
  Serial.println("Calibration Distance: ");
  Serial.println(this_anchor_target_distance);

  tft.setTextColor(ST77XX_YELLOW);

  // tft.println("Initial Adelay: ");
  // tft.println(this_anchor_Adelay);
  // tft.println();
  tft.setCursor(0, 90);
  tft.println("Calibr Distance:");
  tft.setCursor(0, 110);
  tft.print(this_anchor_target_distance);
  tft.println(" Meters");
  tft.setCursor(0, 130);
  tft.setTextColor(ST77XX_ORANGE);
  tft.print("Tag Addr: ");
  tft.setCursor(0, 170);
  tft.setTextColor(ST77XX_ORANGE);
  tft.print("Antena Delay: ");
  DW1000.setAntennaDelay(this_anchor_Adelay);

  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  // Enable the filter to smooth the distance
  // DW1000Ranging.useRangeFilter(true);

  // start the module as anchor, don't assign random short address
  DW1000Ranging.startAsAnchor(Anchor_Addr, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
}

void loop()
{
  DW1000Ranging.loop();
}
uint16_t lastadelay;
void newRange()
{
  static float last_delta = 0.0;
  Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
  tft.setCursor(0, 150);
  tft.setTextColor(ST77XX_ORANGE);
  tft.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);

  float dist = 0;
  for (int i = 0; i < 100; i++)
  {
    // get and average 100 measurements
    dist += DW1000Ranging.getDistantDevice()->getRange();
  }
  dist /= 100.0;
  Serial.print(",");
  Serial.print(dist);

  if (Adelay_delta < 3)
  {
    Serial.print(", Final Antena Delay ");
    Serial.println(this_anchor_Adelay);

    tft.setTextColor(ST77XX_RED);
    tft.setCursor(0, 210);
    tft.println("Final Delay: ");
    tft.setCursor(0, 230);
    tft.println(this_anchor_Adelay);
    //    Serial.print("Check: stored Adelay = ");
    //    Serial.println(DW1000.getAntennaDelay());
    while (1)
      ; // done calibrating
  }

  float this_delta = dist - this_anchor_target_distance; // error in measured distance

  if (this_delta * last_delta < 0.0)
    Adelay_delta = Adelay_delta / 2; // sign changed, reduce step size
  last_delta = this_delta;

  if (this_delta > 0.0)
    this_anchor_Adelay += Adelay_delta; // new trial Adelay
  else
    this_anchor_Adelay -= Adelay_delta;

  Serial.print(", Adelay: ");
  Serial.println(this_anchor_Adelay);

  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(0, 190);
  tft.println(lastadelay);
  tft.setTextColor(ST77XX_ORANGE);
  tft.setCursor(0, 190);
  tft.println(this_anchor_Adelay);
  lastadelay = this_anchor_Adelay;
  //  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); //Reset, CS, IRQ pin
  DW1000.setAntennaDelay(this_anchor_Adelay);
}

void newDevice(DW1000Device *device)
{
  Serial.print("Device added: ");
  Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device)
{
  Serial.print("delete inactive device: ");
  Serial.println(device->getShortAddress(), HEX);
}
