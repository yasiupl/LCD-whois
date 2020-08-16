#include <Wire.h>
#include <LiquidCrystal.h>
#include <WiFi.h>

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

const char *host = "whois.at.hs3.pl";
String url = "/api/now/";
char temp[1024];

struct frame
{
  int headcount = 0;
  int unknown = 0;
  String users;
};

struct frame whois_frame;

uint8_t cursor = 0;

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_5V_ENA, OUTPUT);

  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_5V_ENA, HIGH);

  lcd.backlight();
  lcd.begin(16, 2); // initialize the lcd

  lcd.home(); // go home
  lcd.print("Connecting");
  lcd.setCursor(0, 1); // go to the next line
  while (WiFi.status() != WL_CONNECTED)
  {
    if (cursor > 16)
    {
      cursor = 0;
      lcd.setCursor(cursor, 1);
      lcd.print("                ");
      lcd.setCursor(cursor, 1);
    };
    delay(500);
    lcd.print("*");
    ++cursor;
  }
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(1000);
}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");

  String line;
  Serial.print("Connecting: ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort))
  {
    Serial.println("connection failed");
    return;
  }

  Serial.print("GET ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (cursor > 16)
    {
      cursor = 0;
      lcd.setCursor(cursor, 1);
      Serial.print("                ");
      lcd.setCursor(cursor, 1);
    };
    delay(500);
    lcd.print("*");
    ++cursor;
    if (millis() - timeout > 5000)
    {
      Serial.println("Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available())
  {
    line = client.readStringUntil('\r');
    lcd.setCursor(0, 1);
    Serial.println(line);
  }
  line.toCharArray(temp, line.length() + 1);
  sscanf(temp, "{\"headcount\":%d,\"unknown_devices\":%d,", whois_frame.headcount, whois_frame.unknown);


  lcd.setCursor(0, 0);
  lcd.print("People: ");
  lcd.print(whois_frame.headcount);

  digitalWrite(PIN_LED, HIGH);
  delay(1 * 1000);
  digitalWrite(PIN_LED, LOW);
  delay(1 * 1000);
}