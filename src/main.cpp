#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

WiFiClient client;

const int httpPort = 80;
const char *ssid = "hs3:lan";
const char *password = "przyjmujemy_datki_x86";

#define PIN_EN 5
#define PIN_RS 17
#define PIN_4 18
#define PIN_5 19
#define PIN_6 22
#define PIN_7 21

#define PIN_LED 15
#define PIN_5V_ENA 13

#define CONTRAST_PIN 9
#define BACKLIGHT_PIN 7
#define CONTRAST 110

LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_4, PIN_5, PIN_6, PIN_7, 23, POSITIVE);

unsigned long lastRequest = 0;
StaticJsonDocument<512> json;
StaticJsonDocument<512> getWhois();
String requestWhois();
void connect();

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_5V_ENA, OUTPUT);

  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_5V_ENA, HIGH);

  lcd.backlight();
  lcd.begin(16, 2); // initialize the lcd

  lcd.home(); // go home

  connect();
}

void loop()
{
  if(WiFi.status() != WL_CONNECTED) connect();

  if (millis() - lastRequest >= 60 * 1000)
  {
    json = getWhois();
    lastRequest = millis();
  }

  static int counter = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("People: ");
  lcd.print(json["headcount"].as<long>());
  if (json["users"].size() > 0)
  {
    int index = (counter++) % (json["users"].size());
    lcd.setCursor(0, 1);
    lcd.print(json["users"][index].as<String>());
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.print("Pool's Closed");
  }
  delay(1000);
}

StaticJsonDocument<512> getWhois()
{
  StaticJsonDocument<512> json;
  DeserializationError error = deserializeJson(json, requestWhois());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Parsing");
  if (error != DeserializationError::Ok)
  {
    lcd.setCursor(0, 1);
    lcd.print(error.c_str());
    delay(1000);
  }
  return json;
}

String requestWhois()
{
  String payload = "";
  const char *root_ca =
      "-----BEGIN CERTIFICATE-----\n"
      "MIICiTCCAg+gAwIBAgIQH0evqmIAcFBUTAGem2OZKjAKBggqhkjOPQQDAzCBhTEL\n"
      "MAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n"
      "BxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNVBAMT\n"
      "IkNPTU9ETyBFQ0MgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDgwMzA2MDAw\n"
      "MDAwWhcNMzgwMTE4MjM1OTU5WjCBhTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdy\n"
      "ZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09N\n"
      "T0RPIENBIExpbWl0ZWQxKzApBgNVBAMTIkNPTU9ETyBFQ0MgQ2VydGlmaWNhdGlv\n"
      "biBBdXRob3JpdHkwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAQDR3svdcmCFYX7deSR\n"
      "FtSrYpn1PlILBs5BAH+X4QokPB0BBO490o0JlwzgdeT6+3eKKvUDYEs2ixYjFq0J\n"
      "cfRK9ChQtP6IHG4/bC8vCVlbpVsLM5niwz2J+Wos77LTBumjQjBAMB0GA1UdDgQW\n"
      "BBR1cacZSBm8nZ3qQUfflMRId5nTeTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/\n"
      "BAUwAwEB/zAKBggqhkjOPQQDAwNoADBlAjEA7wNbeqy3eApyt4jf/7VGFAkK+qDm\n"
      "fQjGGoe9GKhzvSbKYAydzpmfz1wPMOG+FDHqAjAU9JM8SaczepBGR7NjfRObTrdv\n"
      "GDeAU/7dIOA1mjbRxwG55tzd8/8dLDoWV9mSOdY=\n"
      "-----END CERTIFICATE-----\n";

  HTTPClient http;
  http.begin("https://whois.at.hs3.pl/api/now", root_ca); //Specify the URL and certificate
  int httpCode = http.GET();                              //Make the request

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Requesting");
  if (httpCode > 0)
  { //Check for the returning code

    payload = http.getString();
    http.end(); //Free the resources
    lcd.setCursor(0, 1);
    lcd.print(httpCode + " " + payload);
  }
  else
  {
    Serial.println(F("Error on HTTP request"));
    lcd.print(httpCode + " error");
    delay(1000);
    return "";
  }
  http.end(); //Free the resources
  return payload;
}

void connect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for WiFi");
  lcd.setCursor(0, 1); // go to the next line
  
  uint8_t cursor = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (cursor > 16)
    {
      cursor = 0;
      lcd.setCursor(cursor, 1);
      lcd.print("                ");
      lcd.setCursor(cursor, 1);
    };
    delay(100);
    lcd.print("*");
    ++cursor;
  }

  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(1000);

  json = getWhois();
}