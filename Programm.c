#include <WiFi.h>
#include <FirebaseESP32.h>
#include <HTTPClient.h>
#include "DHT.h"

const char* ssid = "office";
const char* password = "12345678";

//Your Domain name with URL path or IP address with path
String serverName = "http://121.52.158.157:4000/battery/owais";

#define FIREBASE_HOST "smart-traps-58eac-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyCzIXGY_HjVuv24-gA1YZ8lWp6P1sVTWBI"



// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

#define relay 14
#define DHTPIN 13
DHT dht(DHTPIN, DHT22);

//Define FirebaseESP32 data object
FirebaseData fbdo;
//FirebaseJson json;
String clientID = "owais";
void printResult(FirebaseData &data);

float h;
float t;

void setup() {
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  dht.begin();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {

  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();
  if (isnan(h) || isnan(t)  ) {
    //if (isnan(value) || isnan(h) || isnan(t)  ){
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  String fireHumid = String(h) + String("%");                                         //convert integer humidity to string humidity
  Serial.print("Humidity: ");
  Serial.print(h);
  //delay(500);



  Serial.print("%  Temperature: ");  Serial.print(t);  Serial.println("°C ");
  String fireTemp = String(t) + String("°C");

  if (Firebase.pushInt(fbdo, "clientID+/Humidity", h))
  {
    //Success
    Serial.println("Set int data success");

  } else {
    //Failed?, get the error reason from fbdo

    Serial.print("Error in setInt, ");
    Serial.println(fbdo.errorReason());
  }

  // if(Firebase.setInt(fbdo, "/Temperature", t))
  if (Firebase.pushInt(fbdo, "clientID+/Temperature", t))
  {
    //Success
    Serial.println("Set int data success");

  } else {

    Serial.print("Error in setInt, ");
    Serial.println(fbdo.errorReason());
  }

  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String serverPath = serverName + "owais";

      // Your Domain name with URL path or IP address with path

      http.begin(serverPath.c_str());

      // Send HTTP GET request
      int httpResponseCode = http.GET();
      String valstring = http.getString();
      Serial.println(valstring);
      if (valstring  == "0") {
        digitalWrite(relay, HIGH);

      }
      if (valstring  == "1") {
        digitalWrite(relay, LOW);
        // Serial.println(payload);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
