#include <Arduino.h>
#include "epd_driver.h"
#include "utilities.h"
/*#include "BLEScan.h"
#include "BLEDevice.h"
#include "BLEAdvertisedDevice.h"
#include "BLEUtils.h" */

/* class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
  }
}; */

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

void drawSegment(int x, int y);
void drawNumber(int x, int y, int number);
void drawColon(int x, int y);
void displayTime(int hours, int minutes);
void updateSevenSegmentClock();

uint8_t *framebuffer = NULL;
int seconds = 0;
int minutes = 0;
// BLE
/* bool isConnected = false;
const int scanTime = 5; // in seconds
BLEScan *pBLEScan;
 */
void setup()
{
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

  /*   BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); // create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); // active scan uses more power, but get results faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99); // less or equal setInterval value
   */
  Serial.println("Hello World!");
  delay(100);
}

void loop()
{
  epd_clear(); // clear the display
  if (seconds > 59)
  {
    seconds = 0;
    minutes++;
  }
  if (minutes > 59)
  {
    minutes = 0;
  }
  Serial.print("Counter: ");
  Serial.println(seconds);
  // epd_draw_rect(0, 0, 200, 200, 0, framebuffer);
  displayTime(minutes, seconds);

  delay(1000);
  seconds++;
  minutes++;
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2); // clear the framebuffer

  /* if (!isConnected)
  {
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.println("Devices found: ");

    pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory, also lists them in the serial monitor

    delay(2000);

  } */
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
}