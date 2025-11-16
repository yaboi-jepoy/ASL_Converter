#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// AP credentials
const char *ap_ssid = "ESP32S3_ASL";
const char *ap_password = "HAHAHAHA";

// static IP configuration for AP
IPAddress local_IP(192, 168, 4, 1); // default
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// TODO make hand_detect.py connect to port 8080
// TCP server on port 8080
WiFiServer server(8080);
WiFiClient client;

// TODO add SSD1306 display
#define SDA_PIN 9
#define SCL_PIN 10
#define OLED_RESET -1 // No reset pin (or set to -1 if not connected)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C // Try 0x3D if 0x3C doesn't work

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void displayMessage(String letter)
{
  //! THIS IS FOR EARLY DEBUG ONLY
  // map ASL letters to phrases for cashier/public use
  Serial.print(">>> Display: ");

  if (letter == "A")
  {
    Serial.println("Hello / Hi");
  }
  else if (letter == "B")
  {
    Serial.println("Thank you");
  }
  else if (letter == "C")
  {
    Serial.println("Please");
  }
  else if (letter == "D")
  {
    Serial.println("Yes");
  }
  else if (letter == "L")
  {
    Serial.println("Love / Like");
  }
  else if (letter == "V")
  {
    Serial.println("Peace / Victory");
  }
  else if (letter == "3")
  {
    Serial.println("Three");
  }
  else if (letter == "5")
  {
    Serial.println("Five");
  }
  else
  {
    Serial.println("Unknown gesture");
  }
}

void convertToMessage(String letter)
{
  // Hello, welcome to Jonalyn's
  // What can I serve you today?
  // Yes

  // clear display on-call
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (letter == "A")
  {
    Serial.println("YES");
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(0, 24);
    display.println("    YES");
  }
  else if (letter == "B")
  {
    Serial.println("NO :(");
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(12, 24);
    display.println("NO, SORRY");
  }
  else if (letter == "C")
  {
    Serial.println("OKAY");
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(42, 24);
    display.println("OKAY");
  }
  else if (letter == "D")
  {
    Serial.println("hold on");
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(5, 14);
    display.println("ONE MOMENT");
    display.setCursor(30, 32);
    display.println("PLEASE");
  }
  else if (letter == "L")
  {
    Serial.println("Dine In?");
    display.setTextSize(2);
    display.setCursor(22, 10);
    display.println("DINE IN");
    display.setCursor(60, 28);
    display.setTextSize(1);
    display.println("OR");
    display.setTextSize(2);
    display.setCursor(30, 38);
    display.println("TO-GO?");
    display.display();
  }
  else if (letter == "V")
  {
    Serial.println("CASH?");
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(42, 10);
    display.println("CASH");
    display.setCursor(60, 28);
    display.setTextSize(1);
    display.println("OR");
    display.setTextSize(2);
    display.setCursor(38, 38);
    display.println("CARD?");
  }
  else if (letter == "3")
  {
    Serial.println("RECEIPT?");
    display.setTextSize(1);
    display.setFont(NULL);
    display.setCursor(32, 18);
    display.println("do you need");
    display.setCursor(18, 28);
    display.setTextSize(2);
    display.println("RECEIPT?");
  }
  else if (letter == "5")
  {
    Serial.println("THANK YOU");
    display.setTextSize(2);
    display.setCursor(7, 20);
    display.println("THANK YOU!");
    display.setCursor(14, 40);
    display.setTextSize(1);
    display.println("PLEASE COME AGAIN");
    display.drawRect(4, 17, 120, 20, 1);
  }
  else
  {
    Serial.println("Unknown gesture");
  }

  // draw text on the display
  display.display();
}

void startupMessage()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE); // CRITICAL: Set text color!
  display.setTextSize(1);
  display.setCursor(0, 12);
  display.println("  WELCOME TO");
  display.drawLine(12, 21, 117, 21, SSD1306_WHITE);
  display.setCursor(0, 24);
  display.setTextSize(2);
  display.println(" JONALYN'S");
  display.drawLine(12, 40, 117, 40, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 43);
  display.println("     READY TO SERVE!");
  display.display();
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // init SSSD1306
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

  // init AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ap_ssid, ap_password);

  // display AP information
  Serial.println("\n=== Access Point Started ===");
  Serial.print("SSID: ");
  Serial.print(ap_ssid);
  Serial.print(" | Password: ");
  Serial.println(ap_password);
  Serial.print(" | IP Address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("============================\n");

  // start TCP server
  server.begin();
  Serial.println("TCP Server started on port 8080");
  Serial.println("Waiting for Python feed to connect...");

  // display startup message when everything is okay
  startupMessage();
}

void loop()
{
  // Check for new client connections
  if (!client.connected())
  {
    client = server.available();
    if (client)
    {
      Serial.println("\n>>> Python client connected!");
    }
  }

  // Read data from Python client
  if (client && client.available())
  {
    String message = client.readStringUntil('\n');
    message.trim();

    if (message.length() > 0)
    {
      Serial.print("Received letter: ");
      Serial.println(message);

      // Update OLED display with message
      convertToMessage(message);

      // Send acknowledgment back to Python
      client.println("OK");
    }
  }

  delay(10);
}
