#define HOMELOAD 1
#define REALSERVERNTP 0
#include "Password.h"
#include "LangRU.h"

/*--------------------------------------------------------------------*/
/*----- WiFi and Blynk define ----------------------------------------*/
/*--------------------------------------------------------------------*/
//#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*----- Ntp Time Prepare define --------------------------------------*/
/*--------------------------------------------------------------------*/
#include <TimeLib.h>
#include <WiFiUdp.h>
/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*----- OTA HTTP Update Server ---------------------------------------*/
/*--------------------------------------------------------------------*/
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
bool RESTART_SYSTEM = false;
/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*----- Weather head include -----------------------------------------*/
/*--------------------------------------------------------------------*/
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*----- Display drive include ----------------------------------------*/
/*--------------------------------------------------------------------*/
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include "Font_Clock.h"
#include "Fonts.h"

/*------END-----------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------------------------*/
/*-----------------------==================================----------------------------------------------------------*/
/*-----------------------= CONSTANT AND VARIABLE FOR VOID =----------------------------------------------------------*/
/*-----------------------==================================----------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------ WiFi and Blynk const ----------------------------------------*/
/*--------------------------------------------------------------------*/
const char *ssid = SEC_SSID; //Имя точки доступа
const char *pass = SEC_PASS; //Пароль к точке доступа
#if HOMELOAD
const char *blynk_ad = SEC_BLYNK_ADDRESS_HOME;
const char *auth = SEC_BLYNK_AUTH_HOME;
#else
const char *blynk_ad = SEC_BLYNK_ADDRESS_WORK;
const char *auth = SEC_BLYNK_AUTH_WORK;
#endif
WidgetTerminal terminal(V120);
BlynkTimer InfoTimer;
BlynkTimer UpdateTimer;

WiFiClient client;

/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------ Ntp Time Prepare const --------------------------------------*/
/*--------------------------------------------------------------------*/

#if REALSERVERNTP
IPAddress timeServer(81, 200, 153, 182);
//IPAddress timeServer(85,  21,  78,  8  );
#else
IPAddress timeServer(192, 168, 7, 1);
#endif
WiFiUDP Udp;
const int timeZone = 5; // Ufa
const String &Clock_Delime = ":";
unsigned int localPort = 8888;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*----- Const OTA HTTP Update Server ---------------------------------*/
/*--------------------------------------------------------------------*/
const char *update_path = "/firmware";
const char *update_username = "admin";
const char *update_password = "admin";
//String content;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*----- Const and variable weather -----------------------------------*/
/*--------------------------------------------------------------------*/
const char *weatherHost = "api.openweathermap.org";
const char *weatherHostz = "api.openweathermap.org";

String weatherKey = SEC_KEY_WEATHER;
String weatherLang = "&lang=ru";
String weatherMetric = "&units=metric";
String cityID = "479561"; //Ufa

bool weather_OK;
int weather_ID;
int weather_Temp;
int weather_TempMin;
int weather_TempMax;
int weather_Humi;
int weather_Pres;
int weather_Cloud;
float weather_Visib;
int weather_WindDeg;
int weather_WindSpeed;
String weather_DescripRu;
String weather_DescripEn;
String weather_Sity;
String weather_Country;
String weather_Text;
String weather_Sunrise;
String weather_Sunset;
String weather_UpdateWeather;
unsigned long weather_DateTime_U;

String weatherF_Date;
int weatherF_TemMorn;
int weatherF_Humi;
int weatherF_Press;
int weatherF_Speed;
String weatherF_Descrip;
String weatherF_PressDesc;

/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*----- Const and variable dysplay -----------------------------------*/
/*--------------------------------------------------------------------*/
#define SCROLL_SPEED 30
#define SPLAT_PAUSE_TIME 50
#define TEXT_PAUSE_TIME 3000
#define ZONE_CLOCK_UP 1
#define ZONE_CLOCK_LO 0
#define ZONE_TEMPERA 3
#define ZONE_DATES 2
#define ZONE_WEATHER 4
#define CS_PIN 12
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, 24);

char szTime[9]; // mm:ss\0
char szData[7]; // 00/00\0
char szTemp[6]; // -99c\0
//char *szInfo;      // -99c\0
//char szText[100];   // -99c\0
char buf[256];
//char szText2[] = {};   // -99c\0
uint8_t degM[] = {3, 8, 8, 8};
uint8_t degP[] = {3, 8, 28, 8};
uint8_t degD[] = {3, 62, 28, 8};
uint8_t degC[] = {7, 0, 3, 3, 56, 68, 68, 68};
uint8_t degR1[] = {8, 0, 124, 20, 8, 0, 4, 124, 4};
uint8_t degR2[] = {7, 64, 0, 56, 68, 68, 0, 64};
uint8_t degW1[] = {7, 0, 124, 8, 16, 8, 124, 0};
uint8_t degW2[] = {7, 96, 16, 12, 0, 124, 68, 68};
//uint8_t degH[] = { 39, 3, 62, 66, 126, 3, 0, 4, 42, 62, 2, 0, 62, 42, 20, 0, 624, 32, 62, 0, 28, 42, 26, 062, 8, 62, 0, 31, 4, 8, 620, 28, 42, 26, 0, 18, 0};
/*------END-----------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------------------------*/
/*-----------------------==================================----------------------------------------------------------*/
/*-----------------------= START PROCEDURE VOID FUNCTION  =----------------------------------------------------------*/
/*-----------------------==================================----------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------WiFi Start Connect--------------------------------------------*/
/*--------------------------------------------------------------------*/
void WiFi_StartConnect()
{
  WiFi.mode(WIFI_AP_STA);
  //Blynk.begin(auth, ssid, pass, blynk_ad, 8080);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
}
/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------------- NTP---------------------------------------------------*/
/*--------------------------------------------------------------------*/
void sendNTPpacket(IPAddress &address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0)
    ;
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 5500)
  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  return 0;
}

String FormatNow()
{
  String curtim = "";
  curtim = String(year()) + F(".") + ((month() < 10) ? "0" : "") + String(month()) + F(".") + ((day() < 10) ? "0" : "") + String(day()) + F(" ") + ((hour() < 10) ? "0" : "") + String(hour()) + ":" + ((minute() < 10) ? "0" : "") + String(minute()) + Clock_Delime + ((second() < 10) ? "0" : "") + String(second());
  return curtim;
}

void NtpTimePrepare()
{
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
}

String FormatNowLog()
{
  return ("[" + FormatNow() + "]");
}
/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------------- OTA---------------------------------------------------*/
/*--------------------------------------------------------------------*/
static void handleRoot(void)
{
  String content;
  content = F("<!DOCTYPE HTML>\n<html>HTTP Update Server ready");
  content += F("<p>");
  content += F("<a href='/firmware'>Go page update</a>");
  content += F("</html>");
  httpServer.send(200, F("text/html"), content);
}

void OTA_update_prepare()
{
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.on("/", []() {
    if (!httpServer.authenticate(update_username, update_password))
      return httpServer.requestAuthentication();
    handleRoot();
  });
  httpServer.begin();
  httpServer.on("/", handleRoot);
  httpServer.begin();
}
/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------------- Info text Setup process ------------------------------*/
/*--------------------------------------------------------------------*/
void Info_To_Terminal()
{
  terminal.println();
  terminal.println(F("-----------------------------------"));
  terminal.println(F("WiFi.mode(WIFI_STA)"));
  terminal.print(F("Router SSID: "));
  terminal.println(ssid);
  terminal.print(F("Router password: "));
  terminal.println(pass);
  terminal.println(F("WiFi connected."));
  terminal.print(F("Local IP address: "));
  terminal.println(WiFi.localIP().toString());
  terminal.print(F("BLYNK Server: "));
  terminal.println(blynk_ad);
  terminal.print(F("BLYNK Auth: "));
  terminal.println(auth);
  terminal.println(F("HTTP Update Server: ready!"));
  terminal.print(F("Path: "));
  terminal.println(update_path);
  terminal.print(F("User name: "));
  terminal.println(update_username);
  terminal.print(F("Update password: "));
  terminal.println(update_password);
  terminal.print(F("Reset Reason: "));
  terminal.println(ESP.getResetReason());
  terminal.print(F("Starting from: "));
  terminal.println(FormatNow());
  terminal.print(F("Free memori first start: "));
  terminal.println(String(ESP.getFreeHeap()));
  terminal.println(F("-----------------------------------"));
  terminal.println();
  terminal.flush();
  Blynk.virtualWrite(V11, FormatNow());

  Serial.println();
  Serial.println(F("-----------------------------------"));
  Serial.println(F("WiFi.mode(WIFI_STA)"));
  Serial.print(F("Router SSID: "));
  Serial.println(ssid);
  Serial.print(F("Router password: "));
  Serial.println(pass);
  Serial.println(F("WiFi connected."));
  Serial.print(F("Local IP address: "));
  Serial.println(WiFi.localIP().toString());
  Serial.print(F("BLYNK Server: "));
  Serial.println(blynk_ad);
  Serial.print(F("BLYNK Auth: "));
  Serial.println(auth);
  Serial.println(F("HTTP Update Server: ready!"));
  Serial.print(F("Path: "));
  Serial.println(update_path);
  Serial.print(F("User name: "));
  Serial.println(update_username);
  Serial.print(F("Update password: "));
  Serial.println(update_password);
  Serial.print(F("Reset Reason: "));
  Serial.println(ESP.getResetReason());
  Serial.print(F("Starting from: "));
  Serial.println(FormatNow());
  Serial.print(F("Free memori first start: "));
  Serial.println(String(ESP.getFreeHeap()));
  Serial.println(F("-----------------------------------"));
  Serial.println();
}

void LogToTerminal(String text)
{
  terminal.print(FormatNowLog());
  terminal.print(" ");
  terminal.println(text);
  terminal.flush();

  Serial.print(FormatNowLog());
  Serial.print(" ");
  Serial.println(text);
}

void LogToTerminaK(String text, String value)
{
  terminal.print(FormatNowLog());
  terminal.print(" ");
  terminal.print(text);
  terminal.println(value);
  terminal.flush();

  Serial.print(FormatNowLog());
  Serial.print(" ");
  Serial.print(text);
  Serial.println(value);
}

void myInfoDateTime()
{
  if (timeStatus() != timeNotSet)
  {
    Blynk.virtualWrite(V10, FormatNow());
  }
  else
  {
    Blynk.virtualWrite(V10, "No set");
  }
}
/*------END-----------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------------------------*/
/*-----------------------==============================--------------------------------------------------------------*/
/*-----------------------= START GET WEATHER FUNCTION =--------------------------------------------------------------*/
/*-----------------------==============================--------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------------- Get Weather Data -------------------------------------*/
/*--------------------------------------------------------------------*/
String Unix_To_TimeHM(unsigned long unix_time)
{
  tmElements_t tmq;
  breakTime((unix_time + timeZone * SECS_PER_HOUR), tmq);
  String ForTime;
  ForTime = ((tmq.Hour < 10) ? "0" : "") + String(tmq.Hour) + Clock_Delime + ((tmq.Minute < 10) ? "0" : "") + String(tmq.Minute);
  return ForTime;
}

String Unix_To_TimeDM(unsigned long unix_time)
{
  tmElements_t tmq;
  breakTime((unix_time + timeZone * SECS_PER_HOUR), tmq);
  String ForTime;
  ForTime = ((tmq.Day < 10) ? "0" : "") + String(tmq.Day) + "/" + ((tmq.Month < 10) ? "0" : "") + String(tmq.Month);
  return ForTime;
}

String Unix_To_TimeDMMM(unsigned long unix_time)
{
  tmElements_t tmq;
  breakTime((unix_time + timeZone * SECS_PER_HOUR), tmq);
  String ForTime;
  String mntrus;

  if (tmq.Month == 1)
    mntrus = L_mon_January;
  if (tmq.Month == 2)
    mntrus = L_mon_February;
  if (tmq.Month == 3)
    mntrus = L_mon_March;
  if (tmq.Month == 4)
    mntrus = L_mon_April;
  if (tmq.Month == 5)
    mntrus = L_mon_May;
  if (tmq.Month == 6)
    mntrus = L_mon_June;
  if (tmq.Month == 7)
    mntrus = L_mon_July;
  if (tmq.Month == 8)
    mntrus = L_mon_August;
  if (tmq.Month == 9)
    mntrus = L_mon_September;
  if (tmq.Month == 10)
    mntrus = L_mon_October;
  if (tmq.Month == 11)
    mntrus = L_mon_November;
  if (tmq.Month == 12)
    mntrus = L_mon_December;

  ForTime = String(tmq.Day) + " " + mntrus;
  return ForTime;
}
/*
   http://api.openweathermap.org/data/2.5/forecast/daily?id=479561&units=metric&appid=02a442964e22b48fe1b1c4b2d1da9454&lang=ru&cnt=2
*/

void Info_To_Weather()
{
  terminal.println();
  terminal.println(F("-----------------------------------"));
  terminal.println(F("WiFi.mode(WIFI_STA)"));
  terminal.print(F("Router SSID: "));
  terminal.println(ssid);
  terminal.print(F("Router password: "));
  terminal.println(pass);
  terminal.println(F("WiFi connected."));
  terminal.print(F("Local IP address: "));
  terminal.println(WiFi.localIP().toString());
  terminal.print(F("BLYNK Server: "));
  terminal.println(blynk_ad);
  terminal.print(F("BLYNK Auth: "));
  terminal.println(auth);
  terminal.println(F("HTTP Update Server: ready!"));
  terminal.print(F("Path: "));
  terminal.println(update_path);
  terminal.print(F("User name: "));
  terminal.println(update_username);
  terminal.print(F("Update password: "));
  terminal.println(update_password);
  terminal.print(F("Reset Reason: "));
  terminal.println(ESP.getResetReason());
  terminal.print(F("Starting from: "));
  terminal.println(FormatNow());
  terminal.print(F("Free memori first start: "));
  terminal.println(String(ESP.getFreeHeap()));
  terminal.println(F("-----------------------------------"));
  terminal.println();
  terminal.flush();
  Blynk.virtualWrite(V11, FormatNow());
}

void getWeatherData()
{
  return;
  HTTPClient http;
  String payload = "";
  //Serial.println(">>>>>>>>>>>>>>>>>>>>> getWeatherData() <<<<<<<<<<<<<<<<<<<<<<<<");
  //http.begin(F("https://api.openweathermap.org/data/2.5/weather?&units=metric&appid=02a442964e22b48fe1b1c4b2d1da9454&id=479561&lang=ru"), "6C:9D:1E:27:F1:13:7B:C7:B6:15:90:13:F2:D0:29:97:A4:5B:3F:7E");
  //http.begin(F("http://api.openweathermap.org/data/2.5/forecast/daily?id=479561&units=metric&appid=02a442964e22b48fe1b1c4b2d1da9454&lang=ru&cnt=2"));
  //int httpCode = http.GET();
  //Serial.println(">>http.errorToString(httpCode) = " + http.errorToString(httpCode));
  //http.end();
  //Serial.println(">>>88888888888888888 httpCode weather ");
  http.begin(F("https://api.openweathermap.org/data/2.5/weather?&units=metric&appid=02a442964e22b48fe1b1c4b2d1da9454&id=479561&lang=ru"), "6C:9D:1E:27:F1:13:7B:C7:B6:15:90:13:F2:D0:29:97:A4:5B:3F:7E");
  
  int httpCode = http.GET();
  //Serial.println(">>http.errorToString(httpCode) = " + http.errorToString(httpCode));

  weather_OK = false;
  //Serial.println(">>>>>>>>>>>>>>>>>>>>> httpCode weather = " + httpCode);

  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      payload = http.getString();
      DynamicJsonBuffer jsonBuf;
      JsonObject &root = jsonBuf.parseObject(payload);
      if (!root.success())
      {
        Serial.println("!root.success() = FALSE !!!!");
        return;
      }
      weather_OK = true;
      Serial.println("weather_OK weather = " + weather_OK);

      weather_DescripEn = root["weather"][0]["main"].as<String>();
      weather_ID = root["weather"][0]["id"];
      weather_DescripRu = root["weather"][0]["description"].as<String>();
      weather_DescripRu.toLowerCase();
      Serial.println("root['weather'][0]['main'] = " + weather_DescripEn);

      weather_Sity = root["name"].as<String>();
      weather_Country = root["sys"]["country"].as<String>();

      weather_Temp = root["main"]["temp"];
      weather_Humi = root["main"]["humidity"];
      weather_Pres = root["main"]["pressure"];
      weather_Pres = weather_Pres * 0.75006375541921;

      weather_Visib = root["visibility"];
      weather_Visib = weather_Visib / 1000;

      weather_TempMin = root["main"]["temp_min"];
      weather_TempMax = root["main"]["temp_max"];

      weather_WindSpeed = root["wind"]["speed"];
      weather_Cloud = root["clouds"]["all"];

      unsigned long temp_U_T = root["sys"]["sunrise"];
      weather_Sunrise = Unix_To_TimeHM(temp_U_T);

      temp_U_T = root["sys"]["sunset"];
      weather_Sunset = Unix_To_TimeHM(temp_U_T);

      temp_U_T = root["dt"];
      weather_UpdateWeather = Unix_To_TimeHM(temp_U_T);

      if (weather_DescripRu == "")
      {
        weather_Text = "Error connect";
      }
      else
      {
        weather_Text = weather_DescripRu;
        if (weather_WindSpeed > 8)
          weather_Text += ", ветер " + String(weather_WindSpeed, 0) + " м/с";
      }
    }
  }

  //http.begin(F("https://api.openweathermap.org/data/2.5/forecast/daily?id=479561&units=metric&appid=02a442964e22b48fe1b1c4b2d1da9454&lang=ru&cnt=2"), "6C:9D:1E:27:F1:13:7B:C7:B6:15:90:13:F2:D0:29:97:A4:5B:3F:7E");
  http.begin(F("http://api.openweathermap.org/data/2.5/forecast/daily?id=479561&units=metric&appid=02a442964e22b48fe1b1c4b2d1da9454&lang=ru&cnt=2"));
  httpCode = http.GET();

  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      payload = http.getString();
      DynamicJsonBuffer jsonBuf;
      JsonObject &root = jsonBuf.parseObject(payload);
      if (!root.success())
      {
        Serial.println("!root.success() = FALSE !!!!");
        return;
      }
      weather_OK = true;
      Serial.println("weather_OK daily = " + weather_OK);
      weatherF_Date = Unix_To_TimeDMMM(root["list"][1]["dt"]);
      weatherF_TemMorn = root["list"][1]["temp"]["morn"];
      weatherF_Humi = root["list"][1]["humidity"];
      weatherF_Press = root["list"][1]["pressure"];
      weatherF_Press = weatherF_Press * 0.75006375541921;
      if (weather_Pres > weatherF_Press)
        weatherF_PressDesc = "падает";
      if (weather_Pres < weatherF_Press)
        weatherF_PressDesc = "растет";
      if (weather_Pres == weatherF_Press)
        weatherF_PressDesc = "стабильно";
      weatherF_Speed = root["list"][1]["speed"];
      weatherF_Descrip = root["list"][1]["weather"][0]["description"].as<String>();
      Serial.println("root[weatherF_Descrip] = " + weatherF_Descrip);
    }
  }
  //LogToTerminaK(F("Free mem http.end Weather = "), String(ESP.getFreeHeap()));
  http.end();
}
/*------END-----------------------------------------------------------*/

// =======================================================================
// Берем погоду с сайта openweathermap.org
// =======================================================================




String lang = "ru";

String weatherMain = "";
String weatherDescription = "";
String weatherLocation = "";
String country;
int humidity;
int pressure;
float pressureFIX;
float temp;
String tempz;
float lon;
float lat;

int clouds;
float windSpeed;
int windDeg;

String date;
String date1;
String currencyRates;
String weatherString;
String weatherString1;
String weatherStringz;
String weatherStringz1;
String weatherStringz2;


// =======================================================================
void tvoday(String line)
{
  String s;
  int strt = line.indexOf('}');
  for (int i = 1; i <= 4; i++)
  {
    strt = line.indexOf('}', strt + 1);
  }
  s = line.substring(2 + strt, line.length());
  tempz = s;
}

void getWeatherDataNN()
{
  Serial.print("connecting to ");
  Serial.println(weatherHost);
  Serial.println(String("GET /data/2.5/weather?id=") + cityID + "&units=metric&appid=" + weatherKey + "&lang=" + lang + "\r\n" +
                 "Host: " + weatherHost + "\r\nUser-Agent: ArduinoWiFi/1.1\r\n" +
                 "Connection: close\r\n\r\n");

  if (client.connect(weatherHost, 80))
  {
    client.println(String("GET /data/2.5/weather?id=") + cityID + "&units=metric&appid=" + weatherKey + "&lang=" + lang + "\r\n" +
                   "Host: " + weatherHost + "\r\nUser-Agent: ArduinoWiFi/1.1\r\n" +
                   "Connection: close\r\n\r\n");
  }
  else
  {
    Serial.println("connection failed");
    return;
  }
  String line;
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 10)
  {
    delay(500);
    Serial.println("w.");
    repeatCounter++;
  }
  while (client.connected() && client.available())
  {
    char c = client.read();
    if (c == '[' || c == ']')
      c = ' ';
    line += c;
  }

  client.stop();
  Serial.println(line + "\n");
  DynamicJsonBuffer jsonBuf;
  JsonObject &root = jsonBuf.parseObject(line);
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return;
  }
  weatherMain = root["weather"]["main"].as<String>();
  weatherDescription = root["weather"]["description"].as<String>();
  weatherDescription.toLowerCase();
  weatherLocation = root["name"].as<String>();
  country = root["sys"]["country"].as<String>();
  temp = root["main"]["temp"];
  humidity = root["main"]["humidity"];
  pressure = root["main"]["pressure"];
  pressureFIX = (pressure / 1.3332239) - 24;
  windSpeed = root["wind"]["speed"];
  windDeg = root["wind"]["deg"];
  clouds = root["clouds"]["all"];
  String deg = String(char('~' + 25));

  if (weatherDescription == "shower sleet")
    weatherDescription = L_weatherDescription_shower_sleet;
  if (weatherDescription == "light shower snow")
    weatherDescription = L_weatherDescription_light_shower_snow;

  weatherString = L_outdoor + " " + String(temp, 0) + "\xB0" + "C ";
  weatherString += weatherDescription;
  weatherString += " " + L_Humidity + " " + String(humidity) + "% ";
  weatherString += L_Atmospheric + " " + String(pressureFIX, 0) + " " + L_Atmospheric_mm + " ";
  weatherString += L_Cloudiness + " " + String(clouds) + "% ";

  String windDegString;

  if (windDeg >= 345 || windDeg <= 22)
    windDegString = L_Wind_Northern;
  if (windDeg >= 23 && windDeg <= 68)
    windDegString = L_Wind_Northeastern;
  if (windDeg >= 69 && windDeg <= 114)
    windDegString = L_Wind_East;
  if (windDeg >= 115 && windDeg <= 160)
    windDegString = L_Wind_Southeastern;
  if (windDeg >= 161 && windDeg <= 206)
    windDegString = L_Wind_Southern;
  if (windDeg >= 207 && windDeg <= 252)
    windDegString = L_Wind_Southwestern;
  if (windDeg >= 253 && windDeg <= 298)
    windDegString = L_Wind_West;
  if (windDeg >= 299 && windDeg <= 344)
    windDegString = L_Wind_Northwestern;

  weatherString += L_Wind + " " + windDegString + " " + String(windSpeed, 1) + " " + L_Windspeed;

  Serial.println("POGODA: " + String(temp, 0) + "\n");
}


// =======================================================================
// Берем ПРОГНОЗ!!! погоды с сайта openweathermap.org
// =======================================================================


void getWeatherDataNNz()
{
  Serial.print("connecting to ");
  Serial.println(weatherHostz);
  Serial.println(String("GET /data/2.5/forecast/daily?id=") + cityID + "&units=metric&appid=" + weatherKey + "&lang=" + lang + "&cnt=2" + "\r\n" +
                 "Host: " + weatherHostz + "\r\nUser-Agent: ArduinoWiFi/1.1\r\n" +
                 "Connection: close\r\n\r\n");

  if (client.connect(weatherHostz, 80))
  {
    client.println(String("GET /data/2.5/forecast/daily?id=") + cityID + "&units=metric&appid=" + weatherKey + "&lang=" + lang + "&cnt=2" + "\r\n" +
                   "Host: " + weatherHostz + "\r\nUser-Agent: ArduinoWiFi/1.1\r\n" +
                   "Connection: close\r\n\r\n");
  }
  else
  {
    Serial.println("connection failed");
    return;
  }
  String line;
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 10)
  {
    delay(500);
    Serial.println("w.");
    repeatCounter++;
  }
  while (client.connected() && client.available())
  {
    char c = client.read();
    if (c == '[' || c == ']')
      c = ' ';
    line += c;
  }
  Serial.println("line: " + line + "\n");
  tvoday(line);
  Serial.println(tempz + "\n");

  client.stop();

  DynamicJsonBuffer jsonBuf;
  JsonObject &root = jsonBuf.parseObject(tempz);
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return;
  }
  lon = root["coord"]["lon"];
  lat = root["coord"]["lat"];

  float wSpeed = root["speed"];
  int wDeg = root["deg"];
  float tempMin = root["temp"]["min"];
  float tempMax = root["temp"]["max"];
  weatherDescription = root["weather"]["description"].as<String>();

  weatherStringz = "Завтра температура " + String(tempMin, 0) + "_" + String(tempMax, 0) + "\xB0" + "C  " + weatherDescription;
  Serial.println("!!!!!PROGNOZ: " + weatherStringz + " Wind: " + wSpeed + " WindDeg: " + (wDeg) + "\n");

  String windDegString;

  if (wDeg >= 345 || wDeg <= 22)
    windDegString = L_Wind_Northern;
  if (wDeg >= 23 && wDeg <= 68)
    windDegString = L_Wind_Northeastern;
  if (wDeg >= 69 && wDeg <= 114)
    windDegString = L_Wind_East;
  if (wDeg >= 115 && wDeg <= 160)
    windDegString = L_Wind_Southeastern;
  if (wDeg >= 161 && wDeg <= 206)
    windDegString = L_Wind_Southern;
  if (wDeg >= 207 && wDeg <= 252)
    windDegString = L_Wind_Southwestern;
  if (wDeg >= 253 && wDeg <= 298)
    windDegString = L_Wind_West;
  if (wDeg >= 299 && wDeg <= 344)
    windDegString = L_Wind_Northwestern;

  weatherStringz1 = L_Wind + " " + windDegString + " " + String(wSpeed, 1) + " " + L_Windspeed;
}


// =======================================================================

/*-------------------------------------------------------------------------------------------------------------------*/
/*-----------------------==================--------------------------------------------------------------------------*/
/*-----------------------= BLYNK FUNCTION =--------------------------------------------------------------------------*/
/*-----------------------==================--------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------------- Manual Hot Restart from Blynk ------------------------*/
/*--------------------------------------------------------------------*/

void restartMCU()
{
  LogToTerminal(F("Manual RESTART !!!"));
  //yield();
  //delay(0);
  //ESP.restart();
  yield();
  ESP.reset();
  yield();
}

/*
BLYNK_WRITE(V117)
{
  int pinValue = param.asInt();
  if (pinValue == 1)
  {
    RESTART_SYSTEM = true;
  }
}

BLYNK_WRITE(V118)
{
  int pinValue = param.asInt();
  if (pinValue == 1)
  {
    LogToTerminaK(F("Free memory = "), String(ESP.getFreeHeap()));
  }
}

BLYNK_WRITE(V110)
{
  int pinValue = param.asInt();
  if (pinValue == 1)
  {
    LogToTerminaK(F("Free mem start get Weather = "), String(ESP.getFreeHeap()));
    //getWeatherData();

    LogToTerminaK(F("Погода на сегодня = "), weather_UpdateWeather);
    LogToTerminaK(F(" >Температура = "), String(weather_Temp));
    LogToTerminaK(F(" >Влажность = "), String(weather_Humi));
    LogToTerminaK(F(" >Давление = "), String(weather_Pres));
    LogToTerminaK(F(" >Ветер = "), String(weather_WindSpeed));
    LogToTerminaK(F(" >Погода = "), weather_DescripRu);
    LogToTerminaK(F(" >Восход = "), (weather_Sunrise));
    LogToTerminaK(F(" >Заход = "), (weather_Sunset));

    LogToTerminaK(F("Прогноз на завтра = "), weatherF_Date);
    LogToTerminaK(F(" >Температура утром = "), String(weatherF_TemMorn));
    LogToTerminaK(F(" >Влажность = "), String(weatherF_Humi));
    LogToTerminaK(F(" >Давление = "), String(weatherF_Press));
    LogToTerminaK(F(" >Давление "), weatherF_PressDesc);
    LogToTerminaK(F(" >Ветер = "), String(weatherF_Speed));
    LogToTerminaK(F(" >Погода = "), weatherF_Descrip);
    LogToTerminaK(F("weather_OK = "), ((weather_OK = true) ? "True" : "False"));
    LogToTerminaK(F("Free mem end Weather = "), String(ESP.getFreeHeap()));
  }
}
*/

/*------END-----------------------------------------------------------*/

void TerminalLogWeather()
{
  LogToTerminaK(F("Free mem start get Weather = "), String(ESP.getFreeHeap()));

  LogToTerminaK(F("Погода на сегодня = "), weather_UpdateWeather);
  LogToTerminaK(F(" >Температура = "), String(weather_Temp));
  LogToTerminaK(F(" >Влажность = "), String(weather_Humi));
  LogToTerminaK(F(" >Давление = "), String(weather_Pres));
  LogToTerminaK(F(" >Ветер = "), String(weather_WindSpeed));
  LogToTerminaK(F(" >Погода = "), weather_DescripRu);
  LogToTerminaK(F(" >Восход = "), (weather_Sunrise));
  LogToTerminaK(F(" >Заход = "), (weather_Sunset));

  LogToTerminaK(F("Прогноз на завтра = "), weatherF_Date);
  LogToTerminaK(F(" >Температура утром = "), String(weatherF_TemMorn));
  LogToTerminaK(F(" >Влажность = "), String(weatherF_Humi));
  LogToTerminaK(F(" >Давление = "), String(weatherF_Press));
  LogToTerminaK(F(" >Давление "), weatherF_PressDesc);
  LogToTerminaK(F(" >Ветер = "), String(weatherF_Speed));
  LogToTerminaK(F(" >Погода = "), weatherF_Descrip);
  LogToTerminaK(F("weather_OK = "), ((weather_OK = true) ? "True" : "False"));
  LogToTerminaK(F("Free mem end Weather = "), String(ESP.getFreeHeap()));
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*-----------------------=================---------------------------------------------------------------------------*/
/*-----------------------= START DYSPLAY =---------------------------------------------------------------------------*/
/*-----------------------=================---------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------------- Dysplay process --------------------------------------*/
/*--------------------------------------------------------------------*/
void getTime(char *psz, bool f = true)
{
  String ForTime;
  ForTime = ((hour() < 10) ? "0" : "") + String(hour()) + (f ? ":" : " ") + ((minute() < 10) ? "0" : "") + String(minute());
  ForTime.toCharArray(psz, ForTime.length() + 1);
}

void getData(char *psz)
{
  String ForTime;
  ForTime = "" + String(((day() < 10) ? "0" : "")) + String(day()) + "." + ((month() < 10) ? "0" : "") + String(month());
  ForTime.toCharArray(psz, ForTime.length() + 1);
}

void getTemp(char *psz)
{
  String ForTemp;
  ForTemp = String(((weather_Temp < 0) ? "$" : "")) + String(((weather_Temp > 0) ? "^" : "")) + String(abs(weather_Temp)) + "&";
  ForTemp.toCharArray(psz, ForTemp.length() + 1);
}

void StartDisplay()
{
  P.begin(5);
  P.setInvert(false);

  P.addChar('$', degM);
  P.addChar('^', degP);
  P.addChar('~', degD);
  P.addChar('&', degC);
  P.addChar('[', degR1);
  P.addChar('<', degR2);
  P.addChar('}', degW1);
  P.addChar(']', degW2);
  // P.addChar('{', degH);

  P.setCharSpacing(1);

  String ForTime;
  ForTime = "00:00";
  ForTime.toCharArray(szTime, ForTime.length() + 1);
  ForTime = "~01.01";
  ForTime.toCharArray(szData, ForTime.length() + 1);
  ForTime = "$10&";
  ForTime.toCharArray(szTemp, ForTime.length() + 1);

  P.setZone(ZONE_CLOCK_UP, 19, 23);
  P.setFont(ZONE_CLOCK_UP, ClockBigFontUpper);
  P.setZone(ZONE_TEMPERA, 16, 18);
  P.setZone(ZONE_CLOCK_LO, 11, 15);
  P.setFont(ZONE_CLOCK_LO, ClockBigFontLower);
  P.setZone(ZONE_WEATHER, 0, 7);
  P.setFont(ZONE_WEATHER, fontUA); //Font_RusText
  P.setZone(ZONE_DATES, 8, 10);
  P.setFont(ZONE_DATES, SmallFont);

  P.displayZoneText(ZONE_CLOCK_LO, szTime, PA_CENTER, SCROLL_SPEED, 0, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(ZONE_CLOCK_UP, szTime, PA_CENTER, SCROLL_SPEED, 0, PA_PRINT, PA_NO_EFFECT);

  P.displayZoneText(ZONE_TEMPERA, szTemp, PA_CENTER, SCROLL_SPEED, 0, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(ZONE_DATES, szData, PA_CENTER, SCROLL_SPEED, 2000, PA_PRINT, PA_NO_EFFECT);

  char *str = (char *)"zone_weather";
  P.displayZoneText(ZONE_WEATHER, str, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_DISSOLVE);

  P.setIntensity(0);
  P.displayAnimate();
}

void myUpdateTimer()
{
  NtpTimePrepare();
  getWeatherData();
  LogToTerminal(F("Update: Date, Timer and Weather"));
}

/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------------- Яркость дисплея --------------------------------------*/
/*--------------------------------------------------------------------*/
void Brightnes()
{
  // Яркость дисплея
  if (hour() >= 8 && hour() < 21)
    P.setIntensity(0);
  else
    P.setIntensity(0);
}
/*------END-----------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------------------------*/
/*-----------------------===============-----------------------------------------------------------------------------*/
/*-----------------------= START SETUP =-----------------------------------------------------------------------------*/
/*-----------------------===============-----------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------------- StartSetup -------------------------------------------*/
/*--------------------------------------------------------------------*/
void StartSetup()
{
  Serial.begin(115200);
  //ESP.wdtEnable(WDTO_8S);
  WiFi_StartConnect();
  NtpTimePrepare();
  OTA_update_prepare();
  Info_To_Terminal();
  InfoTimer.setInterval(1000L, myInfoDateTime);
  StartDisplay();
  UpdateTimer.setInterval(60000L * 10, myUpdateTimer);
  delay(0);
  delay(1);
  yield();
  myUpdateTimer();
  yield();
  Brightnes();
}
/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*------------- System Loop ------------------------------------------*/
/*--------------------------------------------------------------------*/
void SystemLoop()
{
  //Blynk.run();
  httpServer.handleClient();
  InfoTimer.run();
  UpdateTimer.run();
  if (RESTART_SYSTEM)
    restartMCU();
}

/*------END-----------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*----------------- utf8rus ------------------------------------------*/
/*--------------------------------------------------------------------*/
String utf8rus(String source)
{
  int i, k;
  String target;
  unsigned char n;
  char m[2] = {'0', '\0'};

  k = source.length();
  i = 0;

  while (i < k)
  {
    n = source[i];
    i++;

    if (n >= 0xC0)
    {
      switch (n)
      {
      case 0xD0:
      {
        n = source[i];
        i++;
        if (n == 0x81)
        {
          n = 0xA8;
          break;
        }
        if (n >= 0x90 && n <= 0xBF)
          n = n + 0x30;
        break;
      }
      case 0xD1:
      {
        n = source[i];
        i++;
        if (n == 0x91)
        {
          n = 0xB8;
          break;
        }
        if (n >= 0x80 && n <= 0x8F)
          n = n + 0x70;
        break;
      }
      }
    }
    m[0] = n;
    target = target + String(m);
  }
  return target;
}
/*------END-----------------------------------------------------------*/

//***********************************************************************************************************
