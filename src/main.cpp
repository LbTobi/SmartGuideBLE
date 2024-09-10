#include <Arduino.h>
#include "epd_driver.h"
#include "utilities.h"
#include "BLEScan.h"
#include "BLEDevice.h"
#include "BLEAdvertisedDevice.h"
#include "BLEUtils.h"

// Display
#define BLACK 0
#define WHITE 255
#define SEGMENT_WIDTH 20
#define HORIZONTAL_SEGMENT_LENGTH 80
#define VERTICAL_SEGMENT_LENGTH 100
#define NUMBER_WIDTH 140
#define NUMBER_HEIGHT 260
#define CLOCK_TOP_OFFSET 140
#define CLOCK_LEFT_OFFSET 180

// BLE
#define ADVERTISED_SERVICE_UUID "0000fe0d-0000-1000-8000-00805f9b34fb"
#define SERVICE_UUID "A0F0FF00-5047-4D53-8208-4F72616C2D42"
#define BRUSHING_TIME_CHARACTERISTIC_UUID "A0F0FF08-5047-4D53-8208-4F72616C2D42"

void drawSegment(int x, int y);
void drawNumber(int x, int y, int number);
void drawColon(int x, int y);
void displayTime(int hours, int minutes);
void updateSevenSegmentClock();
void resetClock();

void connectToServer();

uint8_t *framebuffer = NULL;
int seconds = 0;
int minutes = 0;
// BLE
bool dontScan = false;
const int scanTime = 5; // in seconds
BLEScan *pBLEScan;
BLEClient *pClient = BLEDevice::createClient();
BLEAdvertisedDevice *pAdvertisedDevice;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    if (advertisedDevice.haveServiceUUID())
    {
      BLEUUID serviceUUID = advertisedDevice.getServiceUUID();
      if (serviceUUID.equals(BLEUUID(ADVERTISED_SERVICE_UUID)))
      {
        pAdvertisedDevice = new BLEAdvertisedDevice(advertisedDevice);
        Serial.println("Found our device! Connecting now.");
        dontScan = true;
        BLEDevice::getScan()->stop();
      }
    }
  }
};

void setup()
{
  esp_log_level_set("*", ESP_LOG_VERBOSE); // Enable verbose logging
  Serial.begin(115200);
  delay(1000);
  framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  if (!framebuffer)
  {
    Serial.println("alloc memory failed !!!");
    while (1)
      ;
  }
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  epd_init();
  epd_poweron();
  epd_clear();

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); // create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value

  Serial.println("Hello World!");
  delay(100);
}

void loop()
{
  Serial.println("looping");
  if (!dontScan)
  {
    pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
    Serial.println("Devices found: ");
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

    Serial.println("Scan done!");
    if (dontScan)
    {
      connectToServer();
    }

    delay(1000);
  }
  delay(1000);

  // read batt_pin
  uint16_t batt = analogReadMilliVolts(BATT_PIN);
  Serial.print("Battery level: ");
  Serial.println(batt);
}

// put function definitions here:
void drawSegment(int x, int y, bool horizontal, bool on)
{
  if (on)
  {
    if (horizontal)
    {
      epd_fill_rect(x, y, HORIZONTAL_SEGMENT_LENGTH, SEGMENT_WIDTH, 0, framebuffer);
    }
    else
    {
      epd_fill_rect(x, y, SEGMENT_WIDTH, VERTICAL_SEGMENT_LENGTH, 0, framebuffer);
    }
  }
  else
  {
    if (horizontal)
    {
      epd_draw_rect(x, y, HORIZONTAL_SEGMENT_LENGTH, SEGMENT_WIDTH, 255, framebuffer);
    }
    else
    {
      epd_draw_rect(x, y, SEGMENT_WIDTH, VERTICAL_SEGMENT_LENGTH, 255, framebuffer);
    }
  }
}

void drawNumber(int x, int y, int number)
{
  bool segments[10][7] = {
      {1, 1, 1, 1, 1, 1, 0}, // 0
      {0, 1, 1, 0, 0, 0, 0}, // 1
      {1, 1, 0, 1, 1, 0, 1}, // 2
      {1, 1, 1, 1, 0, 0, 1}, // 3
      {0, 1, 1, 0, 0, 1, 1}, // 4
      {1, 0, 1, 1, 0, 1, 1}, // 5
      {1, 0, 1, 1, 1, 1, 1}, // 6
      {1, 1, 1, 0, 0, 0, 0}, // 7
      {1, 1, 1, 1, 1, 1, 1}, // 8
      {1, 1, 1, 1, 0, 1, 1}  // 9
  };

  // draw each segment

  // A segment (top)
  drawSegment(x + SEGMENT_WIDTH, y, true, segments[number][0]);
  // B segment (top right)
  drawSegment(x + SEGMENT_WIDTH + HORIZONTAL_SEGMENT_LENGTH, y + SEGMENT_WIDTH, false, segments[number][1]);
  // C segment (bottom right)
  drawSegment(x + SEGMENT_WIDTH + HORIZONTAL_SEGMENT_LENGTH, y + 2 * SEGMENT_WIDTH + VERTICAL_SEGMENT_LENGTH, false, segments[number][2]);
  // D segment (bottom)
  drawSegment(x + SEGMENT_WIDTH, y + 2 * SEGMENT_WIDTH + 2 * VERTICAL_SEGMENT_LENGTH, true, segments[number][3]);
  // E segment (bottom left)
  drawSegment(x, y + 2 * SEGMENT_WIDTH + VERTICAL_SEGMENT_LENGTH, false, segments[number][4]);
  // F segment (top left)
  drawSegment(x, y + SEGMENT_WIDTH, false, segments[number][5]);
  // G segment (middle)
  drawSegment(x + SEGMENT_WIDTH, y + SEGMENT_WIDTH + VERTICAL_SEGMENT_LENGTH, true, segments[number][6]);
}

void drawColon(int x, int y)
{
  epd_fill_rect(x, y + VERTICAL_SEGMENT_LENGTH, SEGMENT_WIDTH, SEGMENT_WIDTH, BLACK, framebuffer);
  epd_fill_rect(x, y + VERTICAL_SEGMENT_LENGTH + 2 * SEGMENT_WIDTH, SEGMENT_WIDTH, SEGMENT_WIDTH, BLACK, framebuffer);
}

void displayTime(int hours, int minutes)
{
  // draw the hours
  drawNumber(CLOCK_LEFT_OFFSET, CLOCK_TOP_OFFSET, hours / 10);
  drawNumber(CLOCK_LEFT_OFFSET + NUMBER_WIDTH, CLOCK_TOP_OFFSET, hours % 10);

  // draw the colon
  drawColon(CLOCK_LEFT_OFFSET + 2 * NUMBER_WIDTH, CLOCK_TOP_OFFSET);

  // draw the minutes
  drawNumber(CLOCK_LEFT_OFFSET + 2 * NUMBER_WIDTH + 2 * SEGMENT_WIDTH, CLOCK_TOP_OFFSET, minutes / 10);
  drawNumber(CLOCK_LEFT_OFFSET + 3 * NUMBER_WIDTH + 2 * SEGMENT_WIDTH, CLOCK_TOP_OFFSET, minutes % 10);

  // update the display
  epd_draw_grayscale_image(epd_full_screen(), framebuffer);
}

void updateSevenSegmentClock()
{
  epd_clear();
  if (minutes > 0 || seconds > 0)
  {
    displayTime(minutes, seconds);
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2); // clear the framebuffer
  }
}

void resetClock()
{
  seconds = 0;
  minutes = 0;
  updateSevenSegmentClock();
}

void connectToServer()
{
  bool con = pClient->connect(pAdvertisedDevice);
  if (!con)
  {
    Serial.println("Failed to connect.");
    dontScan = false;
    resetClock();
    return;
  }
  Serial.println("We are now connected to the BLE Server.");

  BLERemoteService *pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(SERVICE_UUID);
    pClient->disconnect();
    dontScan = false;
    resetClock();
    return;
  }
  BLERemoteCharacteristic *pRemoteCharacteristic = pRemoteService->getCharacteristic(BRUSHING_TIME_CHARACTERISTIC_UUID);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(BRUSHING_TIME_CHARACTERISTIC_UUID);
    pClient->disconnect();
    dontScan = false;
    resetClock();
    return;
  }
  if (pRemoteCharacteristic->canRead())
  {
    std::string value = pRemoteCharacteristic->readValue();
    // while the value is not empty keep reading and displaying
    while (value.length() > 0)
    {
      // the characteristic value is stored as raw bytes data: 2 bytes [min, sec] so we need to convert it to int
      minutes = (int)value[0];
      seconds = (int)value[1];
      Serial.print("The characteristic value was: ");
      Serial.print(minutes);
      Serial.print(":");
      Serial.println(seconds);
      updateSevenSegmentClock();
      delay(500);
      value = pRemoteCharacteristic->readValue();
    }
    pClient->disconnect();
    dontScan = false;
    resetClock();
  }
  else
  {
    Serial.println("The characteristic can't be read.");
    pClient->disconnect();
    dontScan = false;
    resetClock();
  }
}