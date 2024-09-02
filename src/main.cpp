#include <Arduino.h>
#include "utilities.h"
#include "BLEScan.h"
#include "BLEDevice.h"
#include "BLEAdvertisedDevice.h"
#include "BLEUtils.h"
#include "epd_driver.h"

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
  }
};

#define SEGMENT_WIDTH 5
#define SEGMENT_HEIGHT 20

// put function declarations here:
void drawSegment(int x, int y);
void drawNumber(int x, int y, int number);
void drawColon(int x, int y);
void displayTime(int hours, int minutes);
void updateSevenSegmentClock();

bool isConnected = false;
const int scanTime = 5; // in seconds
BLEScan *pBLEScan;
int count = 0;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); // create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value

  Serial.println("Hello World!");

  updateSevenSegmentClock();

  epd_init();
  epd_poweron();
  epd_clear();
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!isConnected)
  {
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.println("Devices found: ");

    pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory, also lists them in the serial monitor

    delay(2000);

    count++;
  }
}

// put function definitions here:
void drawSegment(int x, int y, bool horizontal)
{
  if (horizontal)
  {
    epd_fill_rect(x, y, x + SEGMENT_WIDTH, y + SEGMENT_HEIGHT, BLACK);
  }
  else
  {
    epd_fill_rect(x, y, x + SEGMENT_HEIGHT, y + SEGMENT_WIDTH, BLACK);
  }
}
void updateSevenSegmentClock()
{
  epd_clear();
}