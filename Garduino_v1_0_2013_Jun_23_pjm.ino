/*
  Manoogian Manor Garduino Monitor
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5
	Photoresistor: A2
	DHT11: A3
	Moisture 1: A4
	Moisture 2: A5

 * TFT Display attached to digital pins 2, 5, 6, 7, 8
    VCC Arduino +5V
	GND Arduino Gnd
	Digital PINS:
		SCL: 2
		SDA: 5
		CS:  6
		DC:  7
		RES: 8
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>        // Required by Xively 
#include <Xively.h>            // IoT Support
//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library


// define pins
int lightSensor = 2;
int DHT11_PIN = 3;
int tempSensor = DHT11_PIN; //redundancy
int moistureSensor1 = 4;
int moistureSensor2 = 5;


//Regular LEDs
int dpLED1 = 9;
int dpLED2R = 3;
int dpLED2G = 5;
int dpLED2B = 6;

//TFT Display
// You can use any (4 or) 5 pins
 //#define sclk 2
 //#define mosi 5
 //#define cs   6
 //#define dc   7
 //#define rst  8  // you can also connect this to the Arduino reset

byte dht11_dat[5];

//Initialize the tft display
//Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi, sclk, rst);


// define global variables
int moisture_val1;
int moisture_val2;
int light_val;
int temperature_val;
int humidity_val;

// Define the strings for our datastream IDs
char moisturesensor1[] = "MoistureSensor1";
char moisturesensor2[] = "MoistureSensor2";
char lightsensor[] = "LightSensor";
char tempsensor[] = "TempSensor";
char humiditysensor[] = "HumiditySensor";

const int bufferSize = 384;
char bufferValue[bufferSize]; // enough space to store the string we're going to send
XivelyDatastream datastreams[] = {
  XivelyDatastream(moisturesensor1, strlen(moisturesensor1), DATASTREAM_FLOAT),
  XivelyDatastream(moisturesensor2, strlen(moisturesensor2), DATASTREAM_FLOAT),
  XivelyDatastream(lightsensor, strlen(lightsensor), DATASTREAM_FLOAT),
  XivelyDatastream(tempsensor, strlen(tempsensor), DATASTREAM_FLOAT),
  XivelyDatastream(humiditysensor, strlen(humiditysensor), DATASTREAM_FLOAT)
};

// Finally, wrap the datastreams into a feed
XivelyFeed feed(1479780420, datastreams, 5 /* number of datastreams */);

// Your Xively key to let you upload data
char xivelyKey[] = "uNVycARQ2qBbyRTHEYXpBhdbnHzpFGRKgt9KVGmpLeBj2yRv";

// String for reading from client
String request = String(100);
String parsedRequest = "";


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x00, 0xCC, 0x9A };
IPAddress ip(192,168,1,177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
//EthernetServer server(80);




void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  //server.begin();
 
  Serial.println("-----Init Begin -----");
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  

  // Pin modes for RGB LEDs
  //pinMode(dpLED1, OUTPUT);
  //pinMode(dpLED2R, OUTPUT);
  //pinMode(dpLED2G, OUTPUT);
  //pinMode(dpLED2B, OUTPUT);

  //tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

// get time to display "sensor up time"
   uint16_t time = millis();
//   tft.fillScreen(ST7735_BLACK);
   time = millis() - time;

   Serial.println(time, DEC);
   delay(500);

   //tftPrintTest(); // Show the logo
   delay(1000);

   Serial.println("-----init done-----");
}



void loop() {
  
 Serial.println("-----Start of Loop-----");

 // Read from the sensors
  readthesensors();
  delay(1000);

  //setColor(0,0,0);
  //delay(1000);

 //Write to TFT Display
//  writetotftdisplay();
  delay(2000);

// Now write to Xively 
  writetoxively();

  Serial.println("-----End of Loop-----");
}  ///// LOOP   /////



// Write our information to Xively
void writetoxively()
{
	// DO WRITE TO XIVELY
  Serial.println("----- Start of Xively -----");

  EthernetClient Xively_client;
  XivelyClient xxxivelyclient(Xively_client);


  datastreams[0].setFloat(moisture_val1);
  Serial.print("Xively Moisture 1 Sensor value: ");
  Serial.println(moisture_val1);

  datastreams[1].setFloat(moisture_val2);
  Serial.print("Xively Moisture 2 Sensor value: ");
  Serial.println(moisture_val2);

  datastreams[2].setFloat(light_val);
  Serial.print("Xively Light Sensor value: ");
  Serial.println(light_val);

  datastreams[3].setFloat(temperature_val);
  Serial.print("Xively Temp Sensor value: ");
  Serial.println(temperature_val);

  datastreams[4].setFloat(humidity_val);
  Serial.print("Xively Humidity Sensor value: ");
  Serial.println(humidity_val);


  Serial.println("Uploading it to Xively");
  
  int ret = xxxivelyclient.put(feed, xivelyKey);
  
  Serial.print("xivelyclient.put returned ");
  Serial.println(ret);
  
  Serial.println("Wait 2 more seconds");
  delay(2000); //Wait 2 seconds before we start flipping through again
  
  Xively_client.stop();
  Serial.println("----- End of Xively -----");

}


// Read the Sensor Data
void readthesensors()
{

moisture_val1 = 0;
moisture_val2 = 0;
light_val = 0;
temperature_val = 0;
humidity_val = 0;

  Serial.println("-----Begin reading sensors-----");
  // read the value of the moisture probe 1
  moisture_val1 = analogRead(moistureSensor1); 
  Serial.print("M1: ");
  Serial.println(moisture_val1);

  // check if moisture is low and set LED
  //if (moisture_val1 < 200) 
  //   {
  //    digitalWrite(dpLED1, HIGH);
  //}
  //else
  //{
	 // digitalWrite(dpLED1, LOW);
  //}
  

  delay(500);

  // read the value of the moisture probe 2
  moisture_val2 = analogRead(moistureSensor2);
  Serial.print("M2: ");
  Serial.println(moisture_val2);
  
  // check if moisture is low and set LED
  //if (moisture_val2 < 200) {
	 // digitalWrite(dpLED1, HIGH);
  //}
  //else {
	 // digitalWrite(dpLED1, LOW);
  //}


  delay(500);
 
  //read the value from the photosensor
  light_val = analogRead(lightSensor); 
  Serial.print("Light: ");
  Serial.println(light_val);
  
  delay(500);
 
  // Read temp and humidity and set global variables in that function.
  readtemphumidity();

  Serial.print("temperature: ");
  Serial.println(temperature_val);
  Serial.print("humidity: ");
  Serial.println(humidity_val);

  delay(5000);  // 2 second wait
  
  Serial.println("-----End reading Sensors-----");
}


// Get Humidity and Temp from DHT11
void readtemphumidity()
{
   DDRC |= _BV(DHT11_PIN); // Set DATA_PIN as output
   PORTC |= _BV(DHT11_PIN); // Pull DATA_PIN up

   byte dht11_check_sum;
   byte i;

   // Send data read signal
   PORTC &= ~_BV(DHT11_PIN); // Pull DATA_PIN down for 18ms
   delay(18);
   PORTC |= _BV(DHT11_PIN); // Pull DATA_PIN up for 18ms
   delayMicroseconds(40);

   DDRC &= ~_BV(DHT11_PIN); // Set DATA_PIN as input
   while (PINC & _BV(DHT11_PIN)); // Wait until falling edge
   delayMicroseconds(100);
   while (!PINC & _BV(DHT11_PIN)); // Wait until rising edge
   delayMicroseconds(80);

   // Recieve data
   for (i=0; i<5; i++) {
       dht11_dat[i] = read_dht11_dat();
   }

   DDRC |= _BV(DHT11_PIN); // Set DATA_PIN as output
   PORTC |= _BV(DHT11_PIN); // Pull DATA_PIN up

   // Verify checksum
   dht11_check_sum = dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3];
   if (dht11_dat[4] != dht11_check_sum) {
      Serial.println("DHT11 checksum error");
   return;
   }

   Serial.print("Relative humdity = ");
   Serial.print(dht11_dat[0], DEC);
   Serial.print(".");
   Serial.print(dht11_dat[1], DEC);
   Serial.print("%; ");
   Serial.print("Temperature = ");
   Serial.print(dht11_dat[2], DEC);
   Serial.print(".");
   Serial.print(dht11_dat[3], DEC);
   Serial.println("C");

   temperature_val = dht11_dat[2];
   humidity_val = dht11_dat[0];

   Serial.print("temp passed: "); 
   Serial.println(temperature_val);
   Serial.print("hum passed: "); 
   Serial.println(humidity_val);


delay(2000);

}


//DHT11 data read from pins
byte read_dht11_dat () 
{
   byte i = 0;
   byte result=0;
   for(i=0; i< 8; i++){
      while (!(PINC & _BV(DHT11_PIN))); // Wait until rising edge

      delayMicroseconds(30);

      if (PINC & _BV(DHT11_PIN)) 
         result |= 1 << (7-i);
         while ((PINC & _BV(DHT11_PIN))); // Wait until falling edge
   }
   return result;
}


//Set the color of the RGB LED
void setColor(int red, int green, int blue)
{
  analogWrite(dpLED2R, red);
  analogWrite(dpLED2G, green);
  analogWrite(dpLED2B, blue);  
}


//void writetotftdisplay()
//{
//   Serial.println("----- Start of TFT -----");
//   tft.fillScreen(ST7735_BLACK);
//   tft.setTextColor(ST7735_WHITE);
//   tft.setCursor(0,0);
//   tft.setTextSize(1);
//   tft.println("Sketch has been");
//   tft.println("running for: ");
//   tft.setCursor(50, 20);
//   tft.setTextSize(2);
//   tft.setTextColor(ST7735_BLUE);
//   tft.print(millis() / 1000);
//   tft.setTextSize(1);
//   tft.setCursor(40, 40);
//   tft.setTextColor(ST7735_WHITE);
//   tft.println("seconds");
//   
//   tft.setCursor(0, 60);
//   tft.drawLine(0, 50, tft.width()-1, 50, ST7735_WHITE); //draw line separator
//   
//   tft.setTextColor(ST7735_YELLOW);
//   tft.print("Temperature (C): ");
//   tft.setTextColor(ST7735_GREEN);
//   tft.println(temperature_val, 1);
//   tft.setTextColor(ST7735_WHITE);
//   tft.print("Humidity    (%): ");
//   tft.setTextColor(ST7735_RED);
//   tft.println(humidity_val, 1);
//
//   tft.setTextColor(ST7735_WHITE);
//   tft.print("Light intensity ");
////   tft.setCursor(60, 130);
//   tft.setTextColor(ST7735_YELLOW);
//   tft.println(light_val, 1);
//
//   tft.setTextColor(ST7735_YELLOW);
//   tft.print("Moisture (1): ");
//   tft.setTextColor(ST7735_GREEN);
//   tft.println(moisture_val1, 1);
//   
//   tft.setTextColor(ST7735_YELLOW);
//   tft.print("Moisture (2): ");
//   tft.setTextColor(ST7735_GREEN);
//   tft.println(moisture_val2, 1);
//
//   //tft.drawLine(0, 110, tft.width()-1, 110, ST7735_WHITE);
//   //tft.setCursor(0,115);
//   Serial.println("----- End of TFT -----");
//}



//Show the 'logo' on the tft display
//void tftPrintTest() {
//   tft.setTextWrap(false);
//   tft.fillScreen(ST7735_BLACK);
//   tft.setCursor(0, 60);
//   tft.setTextColor(ST7735_RED);
//   tft.setTextSize(2);
//   tft.println("Manoogian");
//   tft.setTextColor(ST7735_YELLOW);
//   tft.setTextSize(2);
//   tft.println("Garduino");
//   tft.setTextColor(ST7735_GREEN);
//   tft.setTextSize(2);
//   tft.println("Monitor");
//   tft.setTextColor(ST7735_BLUE);
//
//   delay(500);
//   }
//

