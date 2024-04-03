
/*

For ESP32 UWB Pro with Display

*/

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <TaskScheduler.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <DW1000.h>
#include <DW1000Device.h>
#include <DW1000Ranging.h>
// #include "BluetoothSerial.h"

// # TFT Connection Pins
#define TFT_CS 11
#define TFT_RST 13
#define TFT_DC 12
#define TFT_MOSI 18 // Data out
#define TFT_SCLK 21 // Clock out
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// UWB Connection Pins
#define UWB_RST 17 // Reset pin
#define UWB_IRQ 32 // IRQ pin
#define UWB_SS 33  // SPI Select pin

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

// calibrated Antenna Delay setting for this anchor (2 Meter from Tag)
// default (16384)
// uint16_t Adelay = 16560; (A1)
// uint16_t Adelay = 16540; (A2)
uint16_t Adelay = 16560;

// MPU-6050
Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_temp;

char Anchor_Addr[24];
String Anchor_Address_Str;

void newRange();
void newBlink(DW1000Device *device);
void inactiveDevice(DW1000Device *device);
void logoshow(void);
void mpuInit();

// Callback methods prototypes
void mpuTaskCallback();

void taskSchedulerInit();

// Tasks
Task mpuTask(1000, TASK_FOREVER, &mpuTaskCallback);
Scheduler runner;

// BluetoothSerial SerialBT;

void setup()
{

  Serial.begin(115200);
  // SerialBT.begin("UWB-Anchor-" + WiFi.macAddress().substring(12, 14) + WiFi.macAddress().substring(15, 17)); 

  // MPU init
  mpuInit();

  // Task Scheduler Init
  taskSchedulerInit();

  // Generate UWB EUI Addres
  Anchor_Address_Str = WiFi.macAddress().substring(12, 17) + ":" + WiFi.macAddress();
  Anchor_Address_Str.toCharArray(Anchor_Addr, Anchor_Address_Str.length() + 1);

  // init the configuration
  tft.setSPISpeed(80000000);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  // DW1000Ranging.initCommunication(UWB_RST, UWB_SS, UWB_IRQ); // Reset, CS, IRQ pin

  // set antenna delay for anchors only. Tag is default (16384)
  // DW1000.setAntennaDelay(Adelay);

  // enable blinking
  //  DW1000.enableDebounceClock();
  //  DW1000.enableLedBlinking();
  //  DW1000.setGPIOMode(MSGP0, LED_MODE);

  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachBlinkDevice(newBlink);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);

  // DW1000Ranging.startAsAnchor(Anchor_Addr, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
  logoshow();
}

void loop()
{
  runner.execute();
  DW1000Ranging.loop();
}

void taskSchedulerInit()
{
  Serial.println("Task Scheduler Init");
  runner.init();
  runner.addTask(mpuTask);
  mpuTask.enable();
  Serial.println("Task Scheduler Done");
}

void newRange()
{
  Serial.print("from: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
  Serial.print("\t Range: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getRange());
  Serial.print(" m");
  Serial.print("\t RX power: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
  Serial.println(" dBm");
}

void newBlink(DW1000Device *device)
{
  Serial.print("blink; 1 device BLINK ! -> ");
  Serial.print(" short:");
  Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device)
{
  Serial.print("delete inactive device: ");
  Serial.println(device->getShortAddress(), HEX);
}

void logoshow(void)
{
  char msg[128];

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

  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(0, 50);
  tft.println("Device Address:");
  tft.setCursor(0, 70);
  tft.println(WiFi.macAddress().substring(12, 17));

  tft.setTextColor(ST77XX_ORANGE);
  tft.setCursor(0, 90);
  tft.println("Antenna Delay: ");
  tft.setCursor(0, 110);
  tft.println(Adelay);

  tft.setCursor(0, 130);
  tft.setTextColor(ST77XX_RED);
  tft.println("Mode:");
  tft.println("LongData");
  tft.println("Range");
  tft.println("LowPower");

  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(45, 250);
  tft.print("Temp: ");
  tft.setCursor(165, 250);
  tft.print(" C");
}

// MPU function
void mpuInit()
{
  Serial.println("MPU Init");

  // Try to initialize!
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
  }
  mpu_temp = mpu.getTemperatureSensor();
  Serial.println("MPU Init Done");
}
float t1;

void mpuTaskCallback()
{
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(110, 250);
  tft.print(t1);

  /* Get new sensor events with the readings */
  sensors_event_t temp;
  mpu_temp->getEvent(&temp);
  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(110, 250);
  tft.print(temp.temperature);
  t1 = temp.temperature;
}
