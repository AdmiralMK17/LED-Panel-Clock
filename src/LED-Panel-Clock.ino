#include "StartUp.h"

void setup()
{
  StartSetup();
}

/*
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
*/

void loop()
{
  SystemLoop();

  static uint32_t lastTime = 0;
  static bool flasher = false;
  static uint8_t cycle = 0;

  P.displayAnimate();

  if (P.getZoneStatus(ZONE_CLOCK_LO) && P.getZoneStatus(ZONE_CLOCK_UP) && P.getZoneStatus(ZONE_DATES) && P.getZoneStatus(ZONE_TEMPERA))
  {
    if (millis() - lastTime >= 500)
    {
      lastTime = millis();
      getTime(szTime, flasher);
      getData(szData);
      getTemp(szTemp);
      flasher = !flasher;

      P.displayReset(ZONE_CLOCK_UP);
      P.displayReset(ZONE_CLOCK_LO);
      P.displayReset(ZONE_DATES);
      P.displayReset(ZONE_TEMPERA);
    }
  }

  if (P.getZoneStatus(ZONE_WEATHER))
  {
    String ForInfo;

    switch (cycle)
    {
    case 0:
      ForInfo = weather_DescripRu;
      utf8rus(ForInfo).toCharArray(buf, 256);
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle++;
      /*------Log to Serial------*/
      TerminalLogWeather();
      getWeatherData();
      /*------Log to Serial------*/
      Brightnes();
      break;
    case 1:
      ForInfo = "Ветер:" + String(weather_WindSpeed) + "}]p";
      utf8rus(ForInfo).toCharArray(buf, 256);
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle++;
      break;
    case 2:
      ForInfo = "Давл.:" + String(weather_Pres) + "[<";
      utf8rus(ForInfo).toCharArray(buf, 256);
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle++;
      break;
    case 3:
      ForInfo = "Влажн:" + String(weather_Humi) + " %";
      utf8rus(ForInfo).toCharArray(buf, 256);
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle++;
      break;
    case 4:
      ForInfo = "Заход в: " + weather_Sunset;
      utf8rus(ForInfo).toCharArray(buf, 256);
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle++;
      break;
    case 5:
      ForInfo = "Прогноз на " + weatherF_Date;
      utf8rus(ForInfo).toCharArray(buf, 256);
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle++;
      break;
    case 6:
      ForInfo = "Темпер. утром: " + String(weatherF_TemMorn) + "&";
      utf8rus(ForInfo).toCharArray(buf, 256);
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle++;
      break;
    case 7:
      ForInfo = "Ожид: " + weatherF_Descrip;
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle++;
      break;
    case 8:
      ForInfo = "Давл.: " + String(weatherF_Press) + "[<";
      utf8rus(ForInfo).toCharArray(buf, 256);
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle++;
      break;
    case 9:
      ForInfo = "Давление " + weatherF_PressDesc;
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle++;
      break;
    case 10:
      ForInfo = "Ветер: " + String(weatherF_Speed) + "}]p";
      utf8rus(ForInfo).toCharArray(buf, 256);
      P.displayZoneText(ZONE_WEATHER, buf, PA_CENTER, SCROLL_SPEED, TEXT_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_DOWN);
      cycle = 0;
      break;
    }
  }
}