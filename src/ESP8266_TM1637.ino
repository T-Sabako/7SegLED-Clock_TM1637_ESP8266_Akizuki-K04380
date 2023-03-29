#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <TM1637.h>
#include <ButtonEvents.h>

//Wi-Fi
#define ssid "hogeid"
#define password "hogepw"
String newHostname = "Clock7segBig";

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

//NTP Client
#define JST     3600*9

//Button
#define ON 1
#define OFF 0

ButtonEvents change_button;

//Wate
#define WAT 500 //ウェイト時間(ms)

// Define Digital Pins
#define CLK 4
#define DIO 5
TM1637 tm1637(CLK, DIO);

int8_t TimeDisp[] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ClockPoint = 1;
const int mdled_pin = 12; //月日LED
const int button_pin    = 14; //表示切替ボタン
unsigned char DispMode = 0;
unsigned long wattime;

void initWiFi() {
  WiFi.hostname(newHostname.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi sucessfully.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi, trying to connect...");
  WiFi.disconnect();
  WiFi.hostname(newHostname.c_str());
  WiFi.begin(ssid, password);
}

void TimeUpdate() {
  digitalWrite(mdled_pin, LOW);
  time_t t;
  struct tm *tm;
  t = time(NULL);
  tm = localtime(&t);
  ClockPoint = (~ClockPoint) & 0x01;
  if (ClockPoint) {
    tm1637.point(POINT_ON);
  } else {
    tm1637.point(POINT_OFF);
  }
    TimeDisp[0] = ((tm->tm_hour) / 10);
    TimeDisp[1] = ((tm->tm_hour) % 10);
    TimeDisp[2] = ((tm->tm_min) / 10);
    TimeDisp[3] = ((tm->tm_min) % 10);
    TimeDisp[4] = 1;
}

void DateUpdate() {
  digitalWrite(mdled_pin, HIGH);
  time_t t;
  struct tm *tm;
  t = time(NULL);
  tm = localtime(&t);
  tm1637.point(POINT_OFF);
    TimeDisp[0] = ((tm->tm_mon+1) / 10);
    TimeDisp[1] = ((tm->tm_mon+1) % 10);
    TimeDisp[2] = ((tm->tm_mday) / 10);
    TimeDisp[3] = ((tm->tm_mday) % 10);
    TimeDisp[4] = 8;
}

void DMode() {
    DispMode = (~DispMode) & 0x01;
}

void setup() {
  Serial.begin(115200);

  //Register event handlers
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
   
  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
  Serial.printf("hostname: %s\n", WiFi.hostname().c_str());

  configTzTime("JST-9", "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");   // 2.7.0以降, esp32コンパチ
  tm1637.init();
  tm1637.set(BRIGHTEST);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  tm1637.clearDisplay();
  pinMode(mdled_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  change_button.attach(button_pin);
  change_button.doubleTapTime(200);
  change_button.holdTime(5000);
  wattime = millis();
}

void loop() {
  while((millis() - wattime) < WAT) {
    change_button.update();
    if(change_button.tapped() == true) {
      DMode();
    }
  }
  if(DispMode) {
    TimeUpdate();
  } else {
    DateUpdate();
  }
  tm1637.display(TimeDisp);
  wattime = millis();

  //time_t t;
  //struct tm *tm;
  //static const char *wd[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
  //t = time(NULL);
  //tm = localtime(&t);
  /*Serial.printf("ESP8266/Arduino ver%s :  %04d/%02d/%02d(%s) %02d:%02d:%02d\n",
        __STR(ARDUINO_ESP8266_GIT_DESC),
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        wd[tm->tm_wday],
        tm->tm_hour, tm->tm_min, tm->tm_sec);
        delay(1000); */
}
