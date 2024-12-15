#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* ssid = "Redmi 8";
const char* password = "oyeyo123";

// MQTT broker information
const char* mqtt_server = "broker.mqtt.cool";
const int mqtt_port = 1883;
const char* mqtt_topic = "soil/moisture/hapid";

// Define the analog pin where the sensor is connected
const int soilMoisturePin = A0; // Analog pin on ESP8266
const int relayPin = D1;        // Digital pin for relay control

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);  // Initialize Serial Monitor
  
  // Setup relay pin mode
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);  // Ensure relay is off initially
  
  // Setup WiFi connection
  setup_wifi();
  
  // Setup MQTT client
  client.setServer(mqtt_server, mqtt_port);
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("Connected to WiFi.");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Connected to MQTT broker.");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Read the soil moisture sensor value
  int soilMoistureValue = analogRead(soilMoisturePin);
  
  // Convert sensor value to a percentage (100% = dry, 0% = wet)
  int moisturePercent = map(soilMoistureValue, 0, 1023, 100, 0);

  // Control relay directly based on moisture value
  if (moisturePercent > 20) {
    digitalWrite(relayPin, LOW);   // Turn relay on if moisture is above 20%
  } else {
    digitalWrite(relayPin, HIGH);  // Turn relay off if moisture is 20% or below
  }

  // Publish the moisture data to the MQTT topic for monitoring
  char moistureStr[8];
  snprintf(moistureStr, sizeof(moistureStr), "%d", moisturePercent);
  client.publish(mqtt_topic, moistureStr);

  // Print the value to Serial Monitor
  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  // Print relay status
  Serial.print("Relay is ");
  Serial.println(moisturePercent > 20 ? "ON" : "OFF");

  // Wait before next reading
  delay(2000);
}