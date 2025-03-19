         ////////////////////////////////////////////////////  
        //   IoT Remote Bookcase Light and Message        //
       //                   Panel                        //
      //          -------------------------             //
     //              NodeMCU (ESP-12E)                 //           
    //               by Kutluhan Aktar                // 
   //                                                //
  ////////////////////////////////////////////////////

// By only subscribing to TheAmplituhedron, you can send data packets to NodeMCU without creating a web server, or any other micro-controller, from your Data Panel on your account page.
// TheAmplituhedron Data Panel is a web application(available system) for TheAmplituhedron subscribers only,which is designed for sending information to micro-controllers automatically.
// Follow the steps down below to create your data panel connection path on which you will be able to send data packets to NodeMCU.
// 1) Go to your Dashboard.
// 2) Click Data Panel under Available Systems.
// 3) Read the given instructions to better comprehend the application.
// 4) Just enter inputs to send information.
// As TheAmplituhedron API creates your connection path, you can get data packets from web by entering your WiFi settings and required information down below.
//
// As a reminder, my website has SSL protection so that you need to identify your NodeMCU connection by entering TheAmplituhedron FingerPrint or ThumbPrint.
// You can learn about it more from the link below.
// https://www.theamplituhedron.com/projects/IoT-Remote-Bookcase-Light-and-Message-Panel/
//
// Connections
// NodeMCU (ESP-12E) :           
//                                LCD Screen I2C
// D1  --------------------------- SCL
// D2  --------------------------- SDA
//                                Control Led
// D7  --------------------------- 
//                                2-Way Relay
// D4  --------------------------- IN_1


// Include required libraries to get data from your data panel connection page.
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

// include LiquidCrystal_I2C and Wire libraries to run I2C module.
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// initialize the library by associating I2C address for NodeMCU
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define your WiFi settings.
const char *ssid = "SSID";
const char *password = "password";
// Connect TheAmplituhedron.com with the current fingerprint.
const char *host = "www.theamplituhedron.com"; 
const char fingerprint[] PROGMEM = "46 3c 5c 2c 67 11 cd 88 b7 e9 76 74 41 34 48 bd bc a5 b9 cf";
const int httpsPort = 443;

// Create data holders to get data packets.
String connectionPath, URL, HEDRON, readString;
String Switch, Range, Message;

// Define 2-Way Relay Pin
#define relay D4

// Define led.
#define led D7

void setup() {
  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);

  // Wait until connected.
  Serial.begin(115200);
  // Initialize I2C lcd screen.
  Wire.begin(D2, D1);
  lcd.begin(16, 2);
 
  // It is just for assuring that if the connection is alive.
  WiFi.mode(WIFI_OFF);
  delay(1000);
  // This mode allows NodeMCU to connect any WiFi directly.
  WiFi.mode(WIFI_STA);        
  // Connect NodeMCU to your WiFi.
  WiFi.begin(ssid, password);
  
  Serial.print("\n\n");
  Serial.print("Try to connect to WiFi. Please wait! ");
  Serial.print("\n\n");
  // Halt the code until connected to WiFi.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("*");
  }
 
  // If connection is successful, write WiFi SSID to serial monitor along with assigned IPAddress.
  Serial.print("\n\n");
  Serial.print("-------------------------------------");
  Serial.print("\n\n");
  Serial.print("Connection is successful!");
  Serial.print("\n\n");
  Serial.print("Connected WiFi SSID : ");
  Serial.print(ssid);
  Serial.print("\n\n");
  Serial.println("Connected IPAddress : ");
  //Serial.println(WiFi.localIP());
  Serial.print("\n\n");

  // Give time to ESP8266 for rebooting properly.
  delay(3000);

  // Turn off 2-Way Relay.
  digitalWrite(relay, HIGH);

}

void loop() {
  
  // Define your data panel connection path.
  URL = "/dashboard/Data-Panel/";
  HEDRON = "your account hedron";
  connectionPath = URL + HEDRON + ".php"; 
  
  // Create a WiFi Client to get form information.
  WiFiClientSecure client;
  // Set the fingerprint to connect TheAmplituhedron API.
  client.setFingerprint(fingerprint);
  // If the host is not responding,return.
  if(!client.connect(host, httpsPort)){
    Serial.println("Connection Failed!");
    return;
  }
  
  // Send a GET request to the connection path ro receive variables.
  client.print(String("GET ") + connectionPath + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  // Detect whether client is responding properly or not.
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
       Serial.println(">>> Client Timeout !");
       client.stop();
       return;
    }
  }  
             
  // Save variables from the connecion path form inputs.
  while(client.available()){
      // By using the plus character, get whole response without dealing with the headers.
      readString = client.readStringUntil('+');
      // Split the response by a pre-defined delimiter in a simple way. '%'(percentage) is defined as the delimiter by TheAmplituhedron API for this project.
      int delimiter, delimiter_1, delimiter_2, delimiter_3;
      delimiter = readString.indexOf("%");
      delimiter_1 = readString.indexOf("%", delimiter + 1);
      delimiter_2 = readString.indexOf("%", delimiter_1 +1);
      delimiter_3 = readString.indexOf("%", delimiter_2 +1);
      // Define variables to be executed on the code later.
      Switch = readString.substring(delimiter + 1, delimiter_1);
      Range = readString.substring(delimiter_1 + 1, delimiter_2);
      Message = readString.substring(delimiter_2 + 1, delimiter_3);
  }

  // View the received form inputs on the serial monitor.
  Serial.println(Switch + '\n' + Range + '\n' + Message + "\n--------------------\n");
  // Depending on Switch, open the relay.
  if(Switch == "ON"){
    digitalWrite(relay, LOW);
    }else if(Switch == "OFF"){
      digitalWrite(relay,HIGH);
      }

  // Turn the control led on if the range is bigger or equal to 135.
  if(Range.toInt() >= 135 && Range != ""){
   digitalWrite(led, HIGH);
   }else{
     digitalWrite(led, LOW);
     }
  // Print the message from the webpage on the lcd display.
  if(Message != ""){
     // Turn on the blacklight and print a message.
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print(Message);
    // Show whole message string.
    //lcd.autoscroll();

    }

  // Wait for the next request.
  delay(1000);
}

