#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiMulti wifi_multi;

const char *ssid = "Dialog 4G 117";
const char *password = "qwertysca321";
const char *host = "http://192.168.8.1/goform/goform_get_cmd_process?isTest=false&cmd=system_status";

uint64_t session = 0;

uint64_t char2LL(const char *str)
{
   uint64_t result = 0;

   for (int i = 0; str[i] != '\0'; ++i)
      result = result * 10 + str[i] - '0';
   return result;
}

void setup()
{
   lcd.begin();
   lcd.backlight();
   lcd.setCursor(0, 0);

   lcd.print("Connecting");
   WiFi.mode(WIFI_STA);
   wifi_multi.addAP(ssid, password);
}

void loop()
{
   lcd.clear();

   if (wifi_multi.run() == WL_CONNECTED)
   {
      WiFiClient client;
      HTTPClient http;

      if (http.begin(client, host))
      {
         int httpCode = http.GET();

         if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
         {
            String payload = http.getString();

            int index = payload.indexOf("downlink_traffic") + 19;
            byte a;

            for (a = 0; a < 255; a++)
            {
               if (payload[index + a] == '"')
                  break;
            }

            uint64_t total = char2LL(payload.substring(index, index + a).c_str());

            index = payload.indexOf("uplink_traffic") + 17;

            for (a = 0; a < 255; a++)
            {
               if (payload[index + a] == '"')
                  break;
            }

            total += char2LL(payload.substring(index, index + a).c_str());
            if (session == 0)
               session = total;
            uint64_t tmp_total = total;

            lcd.print("Tot ");

            float data = total;

            if (data > 1024 * 1024 * 1024)
            {
               lcd.printf("%.2f GB", data / 1024 / 1024 / 1024);
            }
            else if (data > 1024 * 1024)
            {
               lcd.printf("%.2f MB", data / 1024 / 1024);
            }
            else if (data > 1024)
            {
               lcd.printf("%.2f KB", data / 1024);
            }
            else
            {
               lcd.printf("%.2f B", data);
            }

            lcd.setCursor(0, 1);

            lcd.print("Cur ");

            total = tmp_total - session;

            data = total;

            if (data > 1024 * 1024 * 1024)
            {
               lcd.printf("%.2f GB", data / 1024 / 1024 / 1024);
            }
            else if (data > 1024 * 1024)
            {
               lcd.printf("%.2f MB", data / 1024 / 1024);
            }
            else if (data > 1024)
            {
               lcd.printf("%.2f KB", data / 1024);
            }
            else
            {
               lcd.printf("%.2f B", data);
            }
         }

         http.end();
      }
   }
   else
   {
      lcd.print("Reconnecting..");
   }

   delay(2500);
}