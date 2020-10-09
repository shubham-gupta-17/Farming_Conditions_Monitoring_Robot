//Libraries
#include <DHT.h>
#include <SoftwareSerial.h>
#define DEBUG true
SoftwareSerial esp8266(2, 3);

//Constants
#define DHTPIN A0         // humidity and temperature.
#define DHTTYPE DHT22     // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

//Variables
int chk;
const int Moisture_pin = A1; /* Soil moisture sensor O/P pin */

void setup()
{
  Serial.begin(9600); ///////For Serial monitor
  dht.begin();

  esp8266.begin(115200); ///////ESP Baud rate
  pinMode(11, OUTPUT);   /////used if connecting a LED to pin 11
  digitalWrite(11, LOW);

  sendData("AT+RST\r\n", 2000, DEBUG);            // reset module
  sendData("AT+CWMODE=2\r\n", 1000, DEBUG);       // configure as access point
  sendData("AT+CIFSR\r\n", 1000, DEBUG);          // get ip address
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);       // configure for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG); // turn on server on port 80
}

float sensetemp() //function to sense temperature.
{
  float temp = dht.readTemperature();

  // Serial.print(" %, Temp: ");
  // Serial.print(temp);
  // Serial.println(" Celsius");

  return (temp);
}

float sensehumidity() //function to sense humidity.
{
  float hump = dht.readHumidity();

  // Serial.print("Humidity: ");
  // Serial.print(hum);

  return (hump);
}

float senseMoisture() //function to sense Moisture.
{
  float moisture_percentage;
  int sensor_analog;
  sensor_analog = analogRead(Moisture_pin);
  moisture_percentage = (100 - ((sensor_analog / 1023.00) * 100));

  // Serial.print("Moisture Percentage = ");
  // Serial.print(moisture_percentage);

  return (moisture_percentage);
}

int senseWaterLevel() //function to sense water level.
{
  int waterLevel = analogRead(A2); // Incoming analog signal read and appointed sensor

  // Serial.print(", Water level: ");
  // Serial.println(waterLevel);   //Wrote serial port

  return (waterLevel);
}

int connectionId;
void loop()
{
  if (esp8266.available())
  {
    /////////////////////Recieving from web browser to toggle led
    if (esp8266.find("+IPD,"))
    {
      delay(300);
      connectionId = esp8266.read() - 48;
      if (esp8266.find("pin="))
      {
        Serial.println("recieving data from web browser");
        int pinNumber = (esp8266.read() - 48) * 10;
        pinNumber += (esp8266.read() - 48);
        digitalWrite(pinNumber, !digitalRead(pinNumber));
      }

      /////////////////////Sending data to browser
      else
      {
        String webpage = "<h1>Hello World</h1>";
        espsend(webpage);
      }

      if (sensetemp() != 0)
      {
        String add1 = "<h4>Temperature= </h4>";
        String two = String(sensetemp(), 3);
        add1 += two;
        add1 += " &#x2103"; //////////Hex code for degree celcius
        espsend(add1);
      }

      if (sensehumidity() != 0)
      {
        String add1 = "<h4>Humidity= </h4>";
        String two = String(sensehumidity(), 3);
        add1 += two + " gram per ";
        add1 += "&#x33A5"; //////////Hex code for degree celcius
        espsend(add1);
      }

      if (senseMoisture() != 0)
      {
        String add1 = "<h4>Moisture= </h4>";
        String two = String(senseMoisture(), 3);
        add1 += two;
        add1 += "%"; //////////Hex code for degree celcius
        espsend(add1);
      }

      else
      {
        String c = "sensor is not conneted";
        espsend(c);
      }

      String closeCommand = "AT+CIPCLOSE="; ////////////////close the socket connection////esp command
      closeCommand += connectionId;         // append connection id
      closeCommand += "\r\n";
      sendData(closeCommand, 3000, DEBUG);
    }
  }
}

//////////////////////////////sends data from ESP to webpage///////////////////////////

void espsend(String d)
{
  String cipSend = " AT+CIPSEND=";
  cipSend += connectionId;
  cipSend += ",";
  cipSend += d.length();
  cipSend += "\r\n";
  sendData(cipSend, 1000, DEBUG);
  sendData(d, 1000, DEBUG);
}

//////////////gets the data from esp and displays in serial monitor///////////////////////
String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  esp8266.print(command);
  long int time = millis();
  while ((time + timeout) > millis())
  {
    while (esp8266.available())
    {
      char c = esp8266.read(); // read the next character.
      response += c;
    }
  }

  if (debug)
  {
    Serial.print(response); //displays the esp response messages in arduino Serial monitor
  }
  return response;
}
