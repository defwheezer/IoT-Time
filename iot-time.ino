/**********************************************************************
 
 Interfacing ESP8266 NodeMCU with ILI9341 TFT display (240x320 pixel).
 https://simple-circuit.com/

  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651
 
  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
 **********************************************************************/
 
#include <Wire.h>                  // installed by default
#include <YoutubeApi.h>            // version: 1.1.0 - https://github.com/witnessmenow/arduino-youtube-api
#include <ArduinoJson.h>           // version: 5.2.0 - https://github.com/bblanchon/ArduinoJson
#include <LedControl.h>            // version: 1.0.6 - https://github.com/wayoda/LedControl
                                   // LedControl is an Arduino library for MAX7219 and MAX7221 Led display drivers.

#include <ESP8266WiFi.h>           // version: 2.3.0
#include <WiFiClientSecure.h>
#include <NTPClient.h>             // to get time and date
#include <WiFiUdp.h>

// Define NTP Client to get time
const long utcOffsetInSeconds = -25200;  //-8 hr offset from gmt, For UTC -8.00 : -8 * 60 * 60 : -28800
                                         //-7 hr offset from gmt for dlst, For UTC -7.00 : -7 * 60 * 60 : -25200
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String formattedDate;
String dayStamp;
String timeStamp;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// for dsplay
#include <Adafruit_GFX.h>    // Core graphics library
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include <Adafruit_GFX.h>       // include Adafruit graphics library
#include <Adafruit_ILI9341.h>   // include Adafruit ILI9341 TFT library
#include <SPI.h> // needed?
 
#define TFT_CS    D2     // TFT CS  pin is connected to NodeMCU pin D2
#define TFT_RST   D3     // TFT RST pin is connected to NodeMCU pin D3
#define TFT_DC    D4     // TFT DC  pin is connected to NodeMCU pin D4
// initialize ILI9341 TFT library with hardware SPI module
// SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
// MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

#define LED_pin    D8     // Blue Alert LED on top of arcade
#define SWITCH_Pin  D1   // "reset" switch on top of arcade (resets alert)

//------- Replace the following! ------
//-------------------------------------
char ssid[] = "yourssid"; // your network SSID (name)
char password[] = "yourpassphrase";   // your network password

// Google API key
// create yours: https://support.google.com/cloud/answer/6158862?hl=en
#define API_KEY "yourkey"

// Youtube channel ID
// find yours: https://support.google.com/youtube/answer/3250431?hl=en
#define CHANNEL_ID "yourchannelID"
//-------------------------------------
//-------------------------------------

WiFiClientSecure client;

YoutubeApi api(API_KEY, client);

unsigned long api_mtbs = 60000; //mean time between api requests (60 seconds)
unsigned long api_lasttime;     //last time api request has been done

int subscriberCount_last=0; // to check for change in count
int viewCount_last=0; // to check for change in count
int commentCount_last=0; // to check for change in count
int videoCount_last=0; // to check for change in count

int subscriberCount_current=0; //count
int viewCount_current=0; //count
int commentCount_current=0; //count
int videoCount_current=0; //count

int dispVal = 0;

float p = 3.1415926;

void setup() {

  pinMode(LED_pin, OUTPUT);
  pinMode(SWITCH_Pin, INPUT);
  digitalWrite(LED_pin, LOW);
  
  tft.begin();
  tft.setRotation(3);
  
  Serial.begin(115200);
  Serial.println("ILI9341 Test!"); 
  
  client.setInsecure();  //added to make youttubeapi work again
  // Set WiFi to station mode and disconnect from an AP if it was previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(0,10);
  tft.print("ESP8266: ");
  tft.setTextSize(2);
  tft.print(" NodeMCU");
  tft.setTextSize(3);
  tft.println(" V2");
  delay(1000);
  tft.setTextSize(3);
  tft.println("ILI9341 TFT");
  delay(1000);
  tft.println("240x320 pixel");
  delay(1000);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,10);
  tft.println("Connecting Wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    tft.print(".");
    delay(500);
  }
  tft.println("");
  Serial.println("");
  Serial.println("WiFi connected!");
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,10);
  tft.println("WiFi connected!");
  delay(1000);

  timeClient.begin();  // for interent time and date

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,10);
  tft.setTextSize(2);
  tft.println("NTPClient date/time");
  delay(1000);
  tft.setTextSize(3);
    
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,0);
  
  //get time and date
  timeClient.update();
  formattedDate = timeClient.getFormattedDate();
  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  tft.print("Date: ");
  tft.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  tft.print("Time: ");
  tft.println(timeStamp);  
  delay(5000);
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,10);
  tft.println("Test LED 3x");
  delay(1000);
  int i=0;
  for(i=0; i<3; i++){
    digitalWrite(LED_pin, HIGH); //test LED
    tft.println("Blink!");
    delay(500);
    digitalWrite(LED_pin, LOW);
    delay(500);
  }

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,10);
  tft.println("Call YouTube API");
  delay(1000);
  
 if(api.getChannelStatistics(CHANNEL_ID)) {
    Serial.println("---------Stats---------");
    Serial.print("Subscriber Count: ");
    Serial.println(api.channelStats.subscriberCount);
    Serial.print("View Count: ");
    Serial.println(api.channelStats.viewCount);
    Serial.print("Comment Count: ");
    Serial.println(api.channelStats.commentCount);
    Serial.print("Video Count: ");
    Serial.println(api.channelStats.videoCount);
    Serial.print("hiddenSubscriberCount: ");
    Serial.println(api.channelStats.hiddenSubscriberCount);
    Serial.println("------------------------");
    //initialize count
    subscriberCount_last=api.channelStats.subscriberCount;
    viewCount_last=api.channelStats.viewCount;
    commentCount_last=api.channelStats.commentCount;
    videoCount_last=api.channelStats.videoCount;
    subscriberCount_current=subscriberCount_last; //count
    viewCount_current=viewCount_last; //count
    commentCount_current=commentCount_last; //count
    videoCount_current=videoCount_last; //count
    //print to tft screen
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0,10);
    tft.setTextSize(3);
    tft.println("Youtube stats");
    delay(500);
    tft.setTextSize(2);
    tft.println("CHANNEL_ID:");
    tft.println("UCfa6UutS3fmq8Csju8YQUsw");
    tft.println("");
    delay(500);
    tft.println("Views");
    delay(500);
    tft.println(api.channelStats.viewCount);
    delay(500);
    tft.println("Subscribers:");
    delay(500);
    tft.println(api.channelStats.subscriberCount);
    delay(500);
    tft.println("Comments:");
    delay(500);
    tft.println(api.channelStats.commentCount);
    delay(2000);
  }
  else {
    Serial.print("No getChannelStatistics(");
    Serial.print(CHANNEL_ID);
    Serial.println(")");
    Serial.println(api.getChannelStatistics(CHANNEL_ID));
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(5, 220);
    tft.setTextColor(ILI9341_CYAN);
    tft.setTextSize(2);
    tft.println("YouTube update failed");
  }

  // final set-up screen
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,0);
  Serial.println(draw_background());
  Serial.println(draw_title());
  delay(2000);
  int k=0;
  for(k=0; k<500; k++) {
    drawFilledCircles(20, 0xA000); //dark red
    drawFilledCircles(20,ILI9341_GREEN);
  }
  tft.fillScreen(ILI9341_BLACK);
  Serial.println(draw_text());
  Serial.println(draw_background());
  Serial.println(draw_text());
  Serial.println(draw_data());
}
 
unsigned draw_background() {
// int w, i, i2,
//     cx = tft.width()  / 2 - 1,
//     cy = tft.height() / 2 - 1;
//     cx = cx; //less than full width of screen
//     cy = cy; //less than full height of screen
//  w = min(tft.width(), tft.height());
//  
//  for(i=0; i<w; i+=6) {
//    i2 = i / 2;
//    tft.drawRoundRect(cx-i2, cy-i2, i, i, i/8, 0x01E0); //dark green
//  }
}

unsigned draw_TimeDate() {
  timeClient.update();
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-4);
  tft.setTextColor(0xBDF7); //light grey
  tft.setTextSize(2); 
  tft.setCursor(4,222); //height = 240 - font 16 - border 2= 222
  tft.print(dayStamp);
  tft.setCursor(256,222);
  tft.println(timeStamp);
}

unsigned long drawBorder() {
  int x1, x2 = tft.width();
  int y1, y2 = tft.height();
  //Serial.println("Drawing border");
  tft.drawLine(x1, y1, x2, y1, ILI9341_PURPLE);
  tft.drawLine(x2, y1, x2, y2, ILI9341_PURPLE);
  tft.drawLine(x2, y2, x1, y2, ILI9341_PURPLE);
  tft.drawLine(x1, y2, x1, y1, ILI9341_PURPLE);
  x1 = 0+1;
  x2 = tft.width()-1;
  y1 = 0+1;
  y2 = tft.height()-1;
  tft.drawLine(x1, y1, x2, y1, ILI9341_WHITE);
  tft.drawLine(x2, y1, x2, y2, ILI9341_WHITE);
  tft.drawLine(x2, y2, x1, y2, ILI9341_WHITE);
  tft.drawLine(x1, y2, x1, y1, ILI9341_WHITE);
}

unsigned draw_title() {
  //Serial.println("Drawing title");
  tft.setTextSize(4);
  tft.setTextColor(ILI9341_WHITE);    
  tft.setCursor(66, 62);
  tft.println("IoT-Time");
  tft.setTextColor(ILI9341_GREEN);    
  tft.setCursor(62, 58);
  tft.println("IoT-Time");
  tft.setTextColor(ILI9341_YELLOW);    
  tft.setCursor(64, 60);
  tft.println("IoT-Time");
  
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(114, 132);
  tft.println("2020");
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(110, 128);
  tft.println("2020");
  tft.setTextColor(ILI9341_YELLOW);   
  tft.setCursor(112, 130);
  tft.println("2020");
  
  delay(2000);
  tft.fillScreen(ILI9341_RED);
  tft.setTextSize(4);
  tft.setTextColor(ILI9341_BLACK); 
  tft.setCursor(126, 52);
  tft.println("The");
  tft.setCursor(42, 102);
  tft.println("'RonaVirus");
  tft.setCursor(78, 152);
  tft.println("Edition");
  tft.setTextColor(ILI9341_PURPLE);   
  tft.setCursor(122, 48);
  tft.println("The");
  tft.setCursor(40, 98);
  tft.println("'RonaVirus");
  tft.setCursor(74, 148);
  tft.println("Edition");
  tft.setTextColor(ILI9341_YELLOW);   
  tft.setCursor(124, 50);
  tft.println("The");
  tft.setCursor(40, 100);
  tft.println("'RonaVirus");
  tft.setCursor(76, 150);
  tft.println("Edition");
}

unsigned draw_text() {
  //Serial.println("Drawing text");
  tft.setTextSize(4);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(100, 21);
  tft.println("Views");
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(100, 20);
  tft.println("Views");
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(62, 100);
  tft.println("Subscribers");
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(61, 99);
  tft.setTextSize(3);
  tft.println("Subscribers");
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(88, 164);
  tft.println("Comments");
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(88, 163);
  tft.println("Comments");
}

unsigned draw_data() {
  //Serial.println("Drawing data");
  tft.setTextSize(4);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(88,58);
  tft.println(viewCount_current);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(90,60);            
  tft.println(viewCount_current);
  tft.setTextColor(ILI9341_GREEN);
  //orange
  //  tft.setTextColor(0xFC00); 
  tft.setCursor(89,59);            
  tft.println(viewCount_current);  
    
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(124, 128);
  tft.println(subscriberCount_current);
  tft.setTextColor(ILI9341_WHITE);    
  tft.setCursor(126, 130);
  tft.println(subscriberCount_current);
  tft.setTextColor(ILI9341_GREEN);
  //orange
  //  tft.setTextColor(0xFC00); 
  tft.setCursor(125, 129);
  tft.println(subscriberCount_current);
    
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(151, 194);
  tft.println(commentCount_current);
  tft.setTextColor(ILI9341_WHITE);    
  tft.setCursor(154, 196);
  tft.println(commentCount_current);
  tft.setTextColor(ILI9341_GREEN);
  //orange
  //  tft.setTextColor(0xFC00);    
  tft.setCursor(152, 195);
  tft.println(commentCount_current);
}

unsigned draw_channeltext() {
  //Serial.println("Drawing channel");
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.setCursor(18,5);
  tft.print("Defwheezer ");
  tft.setTextSize(2);
  tft.setCursor(209,10);
  tft.print("YouTube");  
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(20,7);
  tft.print("Defwheezer ");
  tft.setTextSize(2);
  tft.setCursor(211,12);
  tft.print("YouTube");
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(3);
  tft.setCursor(19,6);
  tft.print("Defwheezer ");
  tft.setTextSize(2);
  tft.setCursor(210,11);
  tft.print("YouTube");  
}

unsigned long drawFilledCircles(uint8_t radius, uint16_t color) {
  //Serial.println("Drawing drawFilledCircles");
  unsigned long start;
  //red1 = 0xFD35
  //red2 = 0xBA0C
  int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;
  int rand_w = random(0,w);
  int rand_h = random(0,h);
  int rand_radius = random(0,radius);
  x=rand_w; //middle
  y=rand_h; //center
  radius = rand_radius;
  tft.fillCircle(x-1, y-1, radius, ILI9341_GREEN); //drop shadow
  tft.fillCircle(x, y, radius, color);
}

void draw_Inoculation() {
  Serial.println("begin draw_Inoculation fxn");
  int x, y, w = tft.width(), h = tft.height(), r;
  x=w/2; //middle
  y=h/2; //center
  tft.setTextSize(3);
  for(r=1; r<=(w/1.5);r=r+10){
     tft.fillCircle(x, y, r, ILI9341_GREEN);
     tft.setTextColor(ILI9341_BLACK);
     tft.setCursor(61,108);
     tft.println("INOCULATING");
     delay(2);
     tft.fillCircle(x, y, r-2, ILI9341_BLACK);
     tft.setTextColor(ILI9341_GREEN);
     tft.setCursor(61,108);
     tft.println("INOCULATING");
     delay(2);
  }
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(61,108);
  tft.println("INOCULATED!");
  delay(500);
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(61,108);
  tft.println("INOCULATED!");
  delay(100);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(61,108);
  tft.println("INOCULATED!");
  delay(200);
}

void checkSwitch(){
  //switch change sensed
  bool state = digitalRead(D1);
  Serial.print("Switch: ");
  Serial.println(state);
  if(state) {
    digitalWrite(LED_pin, LOW); //reset LED to off
    viewCount_last=viewCount_current;
    Serial.println("viewCount_current -> viewCount_last");
  }
}

void loop() {
  checkSwitch();
  drawBorder();
  draw_text();
  draw_data();
  draw_TimeDate();
  //draw_channeltext();
  int index, spots = random(1,13);
  for(index=0; index<=spots;index++){
    drawFilledCircles(8, 0xA000); //dark red
  }
  drawBorder();
  draw_background();
  draw_text();
  draw_data();
  //draw_channeltext();
  draw_TimeDate();
  delay(random(10,500));
  if (millis() > api_lasttime + api_mtbs)  {
    if(api.getChannelStatistics(CHANNEL_ID)) {
      // update count
      subscriberCount_current=api.channelStats.subscriberCount;
      viewCount_current=api.channelStats.viewCount;
      commentCount_current=api.channelStats.commentCount;
      videoCount_current=api.channelStats.videoCount;
      
      Serial.println("---------Stats---------");
      Serial.print("Current Subscriber Count: ");
      Serial.print(subscriberCount_current);
      Serial.print(", Previous Subscriber Count: ");
      Serial.println(subscriberCount_last);
      Serial.print("Current View Count: ");
      Serial.print(viewCount_current);
      Serial.print(", Previous View Count: ");
      Serial.println(viewCount_last);
      Serial.print("Current Comment Count: ");
      Serial.print(commentCount_current);
      Serial.print(", Previous Comment Count: ");
      Serial.println(commentCount_last);
      Serial.print("Current Video Count: ");
      Serial.print(videoCount_current);
      Serial.print(", Previous Video Count: ");
      Serial.println(videoCount_last);
      Serial.println("------------------------");

      if(viewCount_current >viewCount_last) {
        Serial.println("View Count Increased - Light the LEDs!!!");
        digitalWrite(LED_pin, HIGH);
      }
      draw_Inoculation();
      tft.fillScreen(ILI9341_BLACK);
      //draw_channeltext();
      draw_text();
      draw_background();
      draw_text();
      draw_data();
      draw_TimeDate();
    }
    else {
      Serial.print("No getChannelStatistics(");
      Serial.print(CHANNEL_ID);
      Serial.println(")");
      Serial.println(api.getChannelStatistics(CHANNEL_ID));
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(5, 220);
      tft.setTextColor(ILI9341_CYAN);
      tft.setTextSize(2);
      tft.println("YouTube update failed");
    }
    api_lasttime = millis();
  }
}
