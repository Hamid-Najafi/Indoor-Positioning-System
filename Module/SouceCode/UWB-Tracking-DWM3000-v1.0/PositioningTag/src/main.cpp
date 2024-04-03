// https://stackoverflow.com/questions/75540512/cannot-ping-esp32-mdns-webserver-by-hostname

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <TaskScheduler.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <DW1000.h>
#include <DW1000Device.h>
#include <DW1000Ranging.h>
#include "link.h"

#define SERIAL_DEBUG

// TFT-SX7789 Connection Pins
#define TFT_CS 27
#define TFT_RST 25
#define TFT_DC 26
#define TFT_MOSI 13 // Data out
#define TFT_SCLK 14 // Clock out
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
// Dsiplay Functions
void DisplayInit();
void DisplayWatermark(void);
void DisplayApp();
void DisplayUpdateUWB(struct MyLink *p);
void DisplayUpdateMPU(sensors_event_t *a, sensors_event_t *g);
void DisplayUpdateTemp(sensors_event_t *t);
// Dsiplay Parameters
float xa;
float ya;
float za;
float xg;
float yg;
float zg;
float lastt;
bool noAncher;
float range2072;
float dbm2072;
float range4C3D;
float dbm4C3D;

// UWB-DWM1000 Connection Pins
#define UWB_RST 17 // Reset pin
#define UWB_IRQ 32 // IRQ pin
#define UWB_SS 33  // SPI Select pin
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
// UWB-DWM1000 Functions
void UWBInit();
void UWBNewRange();
void UWBNewDevice(DW1000Device *device);
void UWBInactiveDevice(DW1000Device *device);
void UWBTaskCallback();
// UWB-DWM1000 Parameters
struct MyLink *data_uwb;
int index_num = 0;
long runtime = 0;
String data_uwb_json = "";
char TAG_Addr[24];
String TAG_Address_Str;

// WiFi Functions
void WiFiInit();
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
// WiFi Parameters
WiFiClient wifiClient;
const String hostname = "UWB-Tag";
const char *ssid = "C1Tech-IoT";
const char *password = "iotpass.24";
bool wifiConnection;

// MQTT Functions
void MQTTInit();
void MQTTTopicCallback(char *mqtt_topic_uwb_char, byte *payload, unsigned int length);
void MQTTPacketHandler(String &msg_json, char *Topic);
void MQTTTaskCallback();
// MQTT Parameters
PubSubClient MQTTClient(wifiClient);
const char *mqtt_broker = "mqtt.c1tech.group";
const char *mqtt_username = "user1";
const char *mqtt_password = "user1";
const int mqtt_port = 1883;
const char *mqtt_topic_imu_char = "imutag/";
char mqtt_topic_imu[32];
const char *mqtt_topic_uwb_char = "uwbtag/";
char mqtt_topic_uwb[32];

// MPU-6050 Functions
void MPUInit();
void MPUTaskCallback();
void MPUPacketHandler(sensors_event_t *a, sensors_event_t *g, sensors_event_t *t, String *s);

// MPU-6050 Parameters
Adafruit_MPU6050 mpu;
String mpu_json;
Adafruit_Sensor *mpu_temp;

// Tasks Schedule Functions
void TaskSchedulerInit();
// Tasks Schedule Parameters
Scheduler runner;
Task mpuTask(500, TASK_FOREVER, &MPUTaskCallback);
Task uwbTask(10000, TASK_FOREVER, &UWBTaskCallback);
Task mqttTask(120000, TASK_FOREVER, &MQTTTaskCallback);

// Temp
unsigned long startMillis;
unsigned long currentMillis;
unsigned long elapsedtime;

void setup()
{

  Serial.begin(115200);
  Serial.println("Initializing Device...");

  // UWB init
  UWBInit();

  // TFT ST7789 Init
  DisplayInit();
  DisplayWatermark();

  // WiFi init
  WiFiInit();

  // MQTT Connection Init
  MQTTInit();
  // MQTTTaskCallback();

  // MPU init
  MPUInit();

  // Task Scheduler Init
  TaskSchedulerInit();

  DisplayApp();
  startMillis = millis();

  Serial.println("Initializing End");
}

void loop()
{
  runner.execute();
  DW1000Ranging.loop();
}

// Task Scheduler Functions

void TaskSchedulerInit()
{

  Serial.println("Task Scheduler Init");

  runner.init();
  runner.addTask(mpuTask);
  runner.addTask(uwbTask);
  runner.addTask(mqttTask);
  mpuTask.enable();
  uwbTask.enable();
  mqttTask.enable();

  Serial.println("Task Scheduler Done");
}

// Display Functions

void DisplayInit()
{

  Serial.println("TFT ST7789 Init");
  tft.initSPI(1000000); // set 1 MHz
  tft.init(240, 280);
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);

  Serial.println("TFT ST7789 Done");
}

void DisplayWatermark(void)
{

  Serial.println("Watermark & Info Start");

  tft.setTextColor(ST77XX_BLUE);
  tft.setCursor(70, 10);
  tft.println("C1 Tech");
  tft.setCursor(70, 30);
  tft.println("UWB Tag");

  tft.setCursor(0, 50);
  tft.setTextColor(ST77XX_CYAN);
  tft.println("SSID:");
  tft.println(ssid);
  tft.println("Hostname:");
  tft.println(hostname);
  tft.println("MQTT:");
  tft.println(mqtt_broker);

  tft.println();
  tft.setTextColor(ST77XX_ORANGE);
  tft.println("Mode:");
  tft.println("Short Data");
  tft.println("Range - Accuracy");
  tft.println();

  tft.setTextColor(ST77XX_YELLOW);
  tft.println("Tag Address:");
  tft.setTextSize(1);
  tft.println();
  tft.println(TAG_Addr);
  tft.setTextSize(2);

  Serial.println("Watermark & Info Done");
}

void DisplayApp()
{
  tft.fillRect(0, 50, 250, 230, ST77XX_BLACK);
  tft.setTextColor(ST77XX_ORANGE);
  tft.setCursor(0, 90);
  tft.print("Distanse(m):");
  tft.setCursor(0, 130);
  tft.print("Signal(dBm):");

  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(0, 170);
  tft.print("Accel");
  tft.print("   ");
  tft.print("X:");
  tft.setCursor(0, 190);
  tft.print("Y:");
  tft.setCursor(95, 190);
  tft.print("Z:");

  tft.setCursor(0, 210);
  tft.print("Gyro");
  tft.print("    ");
  tft.print("X:");
  tft.setCursor(0, 230);
  tft.print("Y:");
  tft.setCursor(95, 230);
  tft.print("Z:");

  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(45, 250);
  tft.print("Temp: ");
  tft.setCursor(165, 250);
  tft.print(" C");
}

void DisplayUpdateUWB(struct MyLink *p)
{
  struct MyLink *temp = p;
  char c[30];
  if (temp->next == NULL)
  {
    tft.setTextColor(ST77XX_BLACK);
    tft.setCursor(0, 110);
    sprintf(c, "%.1f", range2072);
    tft.print(c);
    tft.setCursor(0, 150);
    sprintf(c, "%.2f", dbm2072);
    tft.println(c);
    tft.setCursor(120, 110);
    sprintf(c, "%.1f", range4C3D);
    tft.print(c);
    tft.setCursor(120, 150);
    sprintf(c, "%.2f", dbm4C3D);
    tft.println(c);

    tft.setTextColor(ST77XX_RED);
    tft.setCursor(0, 110);
    tft.println("No Anchor");
    noAncher = true;
    return;
  }

  while (temp->next != NULL)
  {
    tft.setTextColor(ST77XX_BLACK);
    temp = temp->next;
    char c[30];
    if (noAncher)
    {
      tft.setCursor(0, 110);
      tft.println("No Anchor");
      noAncher = false;
    }

    if (temp->anchor_addr == 0x2072)
    {
      tft.setCursor(0, 110);
      sprintf(c, "%.1f", range2072);
      tft.print(c);
      tft.setCursor(0, 150);
      sprintf(c, "%.2f", dbm2072);
      tft.println(c);

      range2072 = temp->range[0];
      dbm2072 = temp->dbm;

      tft.setCursor(0, 110);
      tft.setTextColor(ST77XX_ORANGE);
      sprintf(c, "%.1f", temp->range[0]);
      tft.print(c);
      tft.setCursor(0, 150);
      sprintf(c, "%.2f", temp->dbm);
      tft.println(c);
    }
    else if (temp->anchor_addr == 0x4C3D)
    {
      tft.setCursor(120, 110);
      sprintf(c, "%.1f", range4C3D);
      tft.print(c);
      tft.setCursor(120, 150);
      sprintf(c, "%.2f", dbm4C3D);
      tft.println(c);

      range4C3D = temp->range[0];
      dbm4C3D = temp->dbm;
      tft.setCursor(120, 110);
      tft.setTextColor(ST77XX_ORANGE);
      sprintf(c, "%.1f", temp->range[0]);
      tft.print(c);
      tft.setCursor(120, 150);
      sprintf(c, "%.2f", temp->dbm);
      tft.println(c);
    }
  }
  return;
}

void DisplayUpdateMPU(sensors_event_t *a, sensors_event_t *g)
{
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(120, 170);
  tft.print(xa);
  tft.setCursor(25, 190);
  tft.print(ya);
  tft.setCursor(120, 190);
  tft.print(za);
  tft.setCursor(120, 210);
  tft.print(xg);
  tft.setCursor(25, 230);
  tft.print(yg);
  tft.setCursor(120, 230);
  tft.println(zg);

  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(120, 170);
  tft.print(a->acceleration.x);
  tft.setCursor(25, 190);
  tft.print(a->acceleration.y);
  tft.setCursor(120, 190);
  tft.print(a->acceleration.z);
  tft.setCursor(120, 210);

  tft.print(g->gyro.x);
  tft.setCursor(25, 230);
  tft.print(g->gyro.y);
  tft.setCursor(120, 230);
  tft.println(g->gyro.z);

  xa = a->acceleration.x;
  ya = a->acceleration.y;
  za = a->acceleration.z;
  xg = g->gyro.x;
  yg = g->gyro.y;
  zg = g->gyro.z;
}

void DisplayUpdateTemp(sensors_event_t *t)
{
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(110, 250);
  tft.print(lastt);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(110, 250);
  tft.print(t->temperature);
  lastt = t->temperature;
}

// UWB Functions

void UWBInit()
{

  Serial.println("UWB Init");

  // Generate UWB EUI Addres
  TAG_Address_Str = WiFi.macAddress().substring(12, 17) + ":" + WiFi.macAddress();
  TAG_Address_Str.toCharArray(TAG_Addr, TAG_Address_Str.length() + 1);
  DW1000Ranging.initCommunication(UWB_RST, UWB_SS, UWB_IRQ); // Reset, CS, IRQ pin
  DW1000Ranging.attachNewRange(UWBNewRange);
  DW1000Ranging.attachNewDevice(UWBNewDevice);
  DW1000Ranging.attachInactiveDevice(UWBInactiveDevice);
  DW1000Ranging.startAsTag(TAG_Addr, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, true);
  data_uwb = init_link();

  Serial.println("UWB Init Done");
}

void UWBNewRange()
{
  char c[30];

  // Serial.print("from: ");
  // Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
  // Serial.print("\t Range: ");
  // Serial.print(DW1000Ranging.getDistantDevice()->getRange());
  // Serial.print(" m");
  // Serial.print("\t RX power: ");
  // Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
  // Serial.println(" dBm");

  fresh_link(data_uwb, DW1000Ranging.getDistantDevice()->getShortAddress(), DW1000Ranging.getDistantDevice()->getRange(), DW1000Ranging.getDistantDevice()->getRXPower());
}

void UWBNewDevice(DW1000Device *device)
{
  add_link(data_uwb, device->getShortAddress());

  Serial.print("ranging init; 1 device added ! -> ");
  Serial.print(" short:");
  Serial.println(device->getShortAddress(), HEX);
}

void UWBInactiveDevice(DW1000Device *device)
{

  Serial.print("delete inactive device: ");
  Serial.println(device->getShortAddress(), HEX);

  delete_link(data_uwb, device->getShortAddress());
}

void UWBTaskCallback()
{
  if (data_uwb->next != NULL)
  {
  DisplayUpdateUWB(data_uwb);
  make_link_json(data_uwb, &data_uwb_json);
  MQTTPacketHandler(data_uwb_json, mqtt_topic_imu);
  Serial.println(data_uwb_json);
  }
}

// WiFi Functions

void WiFiInit()
{

  Serial.println("WiFi Init");

  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str());
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  delay(5000);
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(500);
  //   Serial.print(".");
  // }

  // WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  // WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  Serial.println("WiFi Init Done");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{

  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());

  wifiConnection = true;
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  wifiConnection = false;

  Serial.println("Disconnected from WiFi access point");
  Serial.println("Trying to Reconnect");

  WiFi.begin(ssid, password);
}

// MQTT Functions

void MQTTInit()
{
  strcpy(mqtt_topic_imu, mqtt_topic_imu_char); /* copy name into the new var */
  strcat(mqtt_topic_imu, TAG_Addr);        /* add the extension */

  Serial.print("MQTT UWB Topic: ");
  Serial.println(mqtt_topic_imu);

  strcpy(mqtt_topic_imu, mqtt_topic_imu_char); /* copy name into the new var */
  strcat(mqtt_topic_imu, TAG_Addr);        /* add the extension */

  Serial.print("MQTT MPU Topic: ");
  Serial.println(mqtt_topic_imu);

  MQTTClient.setServer(mqtt_broker, mqtt_port);
  // MQTTClient.setCallback(MQTTTopicCallback);
}

void MQTTTaskCallback()
{
  if (WiFi.status() == WL_CONNECTED)
  {

    Serial.println("WiFi Connected");

    tft.setTextColor(ST77XX_BLACK);
    tft.setCursor(0, 50);
    tft.print("WiFi: Disconnected");

    tft.setTextColor(ST77XX_RED);
    tft.setCursor(0, 50);
    tft.print("WiFi: Connected");

    // tft.print("WiFi: ");
    // tft.println(WiFi.localIP());
    if (!MQTTClient.connected())
    {

      Serial.print("The client \"");
      Serial.print(TAG_Address_Str);
      Serial.println("\" connects to the public MQTT broker");

      if (MQTTClient.connect(TAG_Addr, mqtt_username, mqtt_password))
      {

        Serial.println("MQTT Broker Connected");

        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(0, 70);
        tft.println("MQTT: Disonnected");
        tft.setTextColor(ST77XX_RED);
        tft.setCursor(0, 70);
        tft.print("MQTT: Connected");
        // tft.print("MQTT: ");
        // tft.print(mqtt_broker);
      }
      else
      {
        tft.setTextColor(ST77XX_BLACK);
        tft.setCursor(0, 70);
        tft.println("MQTT: Connected");
        tft.setTextColor(ST77XX_RED);
        tft.setCursor(0, 70);
        tft.println("MQTT: Disonnected");

        Serial.println("Server Connection Error");
        Serial.print("MQTT failed with state ");
        Serial.println(MQTTClient.state());
      }
    }
  }
  else
  {

    Serial.println("WiFi Disconnected");

    tft.setTextColor(ST77XX_BLACK);
    tft.setCursor(0, 50);
    tft.print("WiFi: Connected");

    tft.setTextColor(ST77XX_RED);
    tft.setCursor(0, 50);
    tft.print("WiFi: Disconnected");
    WiFi.begin(ssid, password);
  }
}

void MQTTTopicCallback(char *mqtt_topic_uwb_char, byte *payload, unsigned int length)
{
  Serial.print("Message arrived in mqtt_topic_uwb_char: ");
  Serial.println(mqtt_topic_uwb_char);
  Serial.print("Message:");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

void MQTTPacketHandler(String &msg_json, char *Topic)
{
  char charArray[msg_json.length() + 1]; // +1 for the null terminator
  msg_json.toCharArray(charArray, msg_json.length() + 1);
  if (MQTTClient.publish(&*Topic, charArray))
  {
    Serial.println("MQTT Packet Sent");
    Serial.print("Topic: ");
    Serial.println(&*Topic);
  }
  else
  {
    Serial.println("MQTT Packet Not Sent");
  }
}

// MPU Functions
void MPUInit()
{

  Serial.println("MPU Init");

  mpu.begin();
  mpu_temp = mpu.getTemperatureSensor();
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);


  // setupt motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(2);
  mpu.setMotionDetectionDuration(200);
  mpu.setInterruptPinLatch(true); // Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);

  Serial.println("MPU Init Done");
}

void MPUPacketHandler(sensors_event_t *a, sensors_event_t *g,
                      sensors_event_t *t, String *s)
{
  *s = "{\"IMUEvent\":";
  char mpu_json[128];

  sprintf(mpu_json, "{\"Accel\": {\"X\":%.2f,\"Y\":%.2f,\"Z\":%.2f},\"Gyro\":{\"X\":%.2f,\"Y\": %.2f,\"Z\":%.2f},\"Temp\":%.2f}",
          a->acceleration.x, a->acceleration.y, a->acceleration.z, g->gyro.x, g->gyro.y, g->gyro.z, t->temperature);
  // With TimeStamp
  // sprintf(mpu_json, "{\"Accel\": {\"X\":%.2f,\"Y\":%.2f,\"Z\":%.2f},\"Gyro\":{\"X\":%.2f,\"Y\": %.2f,\"Z\":%.2f},\"Temp\":%.2f,\"TimeStamp\":%.lu}",
  //         a->acceleration.x, a->acceleration.y, a->acceleration.z, g->gyro.x, g->gyro.y, g->gyro.z, t->temperature, millis());
  *s += mpu_json;
  *s += "}";
}

void MPUTaskCallback()
{
  if (mpu.getMotionInterruptStatus())
  {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    MPUPacketHandler(&a, &g, &t, &mpu_json);

    Serial.println(mpu_json);

    MQTTPacketHandler(mpu_json, mqtt_topic_imu);
    DisplayUpdateMPU(&a, &g);
  }
  sensors_event_t temp;
  mpu_temp->getEvent(&temp);
  DisplayUpdateTemp(&temp);
}