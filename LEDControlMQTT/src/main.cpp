
#include <WiFi.h>
#include <Arduino.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Preferences.h>

WiFiClient espClient;
PubSubClient client(espClient);
Preferences preferences;

//MQTT VARIABLES
const char* mqtt_server = "192.168.68.113";
long lastMsg = 0;
char msg[50];
int value = 0;
//LED VARIABLES
int ledState1 = 0;
int ledState2 = 0;
int ledState3 = 0;

int potPin = 34;

int feedbackVal = 0;
int target_val = 0; 

int ledPins[3] = {15,2,4};
int max_fb_val = 0;
int min_adc_val = 0;

//WIFI VARIABLES
const char* ssid = "ZipherWifi";
const char* password = "Sh3r1d4nZ1ph3r";



void setup_wifi();
void callback(char*, byte* , unsigned int); 
void reconnect(void*);
void lightControlTask( void *pvParameters );
void controlTask( void *pvParameters );

// void lightControlTask();
void controlTask();

void saveState(int,int);
void updateLED(int,int);
void publishTopics();
void subscribeToTopics();
void calibrate_feedback();

void loadState();

void calibrate_feedback(){
  int tempVal = 0;
  int max_val = 0;
  int feedbackVal1 = 0;
  analogWrite(ledPins[2], 0);
  int min_adc = analogRead(potPin);
  for(int i = 0;i<255; i++){
      analogWrite(ledPins[2], i);
      int tempVal = analogRead(potPin);
      Serial.print(i);
      Serial.print(" , ");
      Serial.print(tempVal);
      Serial.print(" , ");
      feedbackVal1 = map(tempVal, 0 , 4096, 0 ,255);
      Serial.println(feedbackVal1);
      max_val = i;
      if (tempVal >= 4095){
        break;
      }
      delay(100);
    }
  Serial.print("Minimum ADC Value: ");
  Serial.println(min_adc);
  Serial.print("Maximum Feedback Value: ");
  Serial.println(max_val);

  preferences.begin("last_state2", false);
  Serial.println("Saving Minimum ADC Value");
  preferences.putInt("min_adc_val", min_adc);
  Serial.println("Saving Maximum Feedback Value");
  preferences.putInt("max_fb_val", max_val);
  preferences.end();
}
//LED Functions

void updateLED(int ledNum, int state){
 
    digitalWrite(ledPins[ledNum], state);
    saveState(ledNum, state);
    
  
  
}
//MQTT Functions
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Changes the output state according to the message
  if (String(topic) == "esp32/led1") {
    Serial.print("Changing output to ");
    if(messageTemp == "1"){
      Serial.println("ON");
      // updateLED(0, HIGH);
      ledState1 = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("OFF");
      // updateLED(0, LOW);
      ledState1 = 0;
    }
  }
  else if (String(topic) == "esp32/led2") {
    Serial.print("Changing output to ");
    if(messageTemp == "1"){
      Serial.println("ON");
      // updateLED(1, HIGH);
      ledState2 = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("OFF");
      // updateLED(1, LOW);
      ledState2 = 0;
    }
  }
  else if (String(topic) == "esp32/analog") {
    Serial.print("Set Analog Value to ");
    target_val = messageTemp.toInt();
    Serial.println(target_val);
    preferences.begin("last_state2", false);
    Serial.print("Saving FeedbackVal: ");
    Serial.println(target_val);
    preferences.putInt("feedbackVal", target_val);
    preferences.end();
  }
}
void subscribeToTopics(){
  Serial.println("Subscribe Topics");
  client.subscribe("esp32/led1");
  client.subscribe("esp32/led2");
  client.subscribe("esp32/analog");
}
void publishTopics(){
  char ledState1_str[8];
  char ledState2_str[8];
  char feedbackVal_str[8];
  // dtostrf(ledState1, 1, 2, ledState1_str);
  // dtostrf(ledState1, 1, 2, ledState2_str);
  // itoa( int value, char *str, int base)
  Serial.println(feedbackVal);

  itoa(ledState1, ledState1_str,10);
  itoa(ledState2, ledState2_str,10);
  itoa(feedbackVal, feedbackVal_str,10);
  Serial.println("Publish Topics");
  
  client.publish("esp32/led1", ledState1_str);
  client.publish("esp32/led2", ledState2_str);
  client.publish("esp32/analog", feedbackVal_str);
}
void reconnectMQTT(){
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      subscribeToTopics();
      publishTopics();
      

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Wifi Functions
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // reconnectMQTT();

}


void reconnect(){
  // Loop until we're reconnected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  reconnectMQTT();
  
  delay(1000);
}


void saveState(int num, int state){
  // Serial.println("Saving to Preferences");
  if (num == 0){

    preferences.begin("last_state2", false);
    if(preferences.getInt("ledState1", 0) != state){
      Serial.println("Saving ledState1");
    preferences.putInt("ledState1", state);
    }
    preferences.end();
    
  }
  else if (num == 1){

    preferences.begin("last_state2", false);
    if(preferences.getInt("ledState2", 0) != state){
      Serial.println("Saving ledState2");
    preferences.putInt("ledState2", state);
    }
    preferences.end();
  }
}

void loadState(){
  preferences.begin("last_state2", true);
  ledState1 = preferences.getInt("ledState1", 0);
  ledState2 = preferences.getInt("ledState2", 0);
  feedbackVal = preferences.getInt("feedbackVal", 0);
  max_fb_val = preferences.getInt("max_fb_val", 0);
  min_adc_val = preferences.getInt("min_adc_val", 0);

  Serial.print("LEDState1: ");
  Serial.println(ledState1);
  Serial.print("LEDState2: ");
  Serial.println(ledState2);
  Serial.print("LEDState3: ");
  Serial.println(feedbackVal);
  

  Serial.print("Minimum ADC Value: ");
  Serial.println(min_adc_val);
  Serial.print("Maximum Feedback Value: ");
  Serial.println(max_fb_val);
  preferences.end();
  
}
void setup() {
  Serial.begin(115200);
  // calibrate_feedback();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  for (int i = 0; i < 3; i++){
    pinMode(ledPins[i],OUTPUT);
  }
  loadState();
  
   xTaskCreatePinnedToCore (
    lightControlTask,     // Function to implement the task
    "lightControlTask",   // Name of the task
    2048,      // Stack size in bytes
    NULL,      // Task input parameter
    1,         // Priority of the task
    NULL,      // Task handle.
    1         // Core where the task should run
  );

  xTaskCreatePinnedToCore (
    controlTask,     // Function to implement the task
    "controlTask",   // Name of the task
    2048,      // Stack size in bytes
    NULL,      // Task input parameter
    1,         // Priority of the task
    NULL,      // Task handle.
    1         // Core where the task should run
  );
  

}




void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
}


void controlTask(void *pVParameters){
  (void) pVParameters;

    Serial.println("Start LightControl Task");
    for(;;){
  int tolerance = 5;
  
  // Serial.print(feedbackVal);
  // Serial.print(" - ");
  // Serial.println(target_val);
  // // for(int i =0; i<max_fb_val; i++){
  //   Serial.print(feedbackVal);
  //   Serial.print(" - ");
  //   Serial.print(target_val);
  //   Serial.print(" : ");
  //   analogWrite(ledPins[2], i);
  //   int tempVal = analogRead(potPin);
  //   feedbackVal = map(tempVal, min_adc_val , 4096, 0 ,max_fb_val);
  //   Serial.print(tempVal);
  //   Serial.print(" , ");
  //   Serial.println(feedbackVal);
    // delay(100);
    int tempVal = analogRead(potPin);
    feedbackVal = map(tempVal, 0 , 4096, 0 ,255);
    // Serial.print(target_val);
    // Serial.print(" , ");
    // Serial.print(tempVal);
    // Serial.print(" , ");
    // Serial.println(feedbackVal);
    if(abs(feedbackVal - target_val) > tolerance){
      analogWrite(ledPins[2], target_val);
      int tempVal = analogRead(potPin);
      feedbackVal = map(tempVal, 0 , 4096, 0 ,255);
      // Serial.print(target_val);
      // Serial.print(" , ");
      // Serial.print(tempVal);
      // Serial.print(" , ");
      // Serial.println(feedbackVal);
      // preferences.begin("last_state2", false);
      // Serial.print("Saving FeedbackVal: ");
      // Serial.println(feedbackVal);
      // preferences.putInt("feedbackVal", feedbackVal);
      // preferences.end();
    }
    

  // }
    }
}

void lightControlTask(void *pVParameters){
  (void) pVParameters;
  // void lightControlTask(){
    Serial.println("Start LightControl Task");
    for (;;) // A Task shall never return or exit.
  {
  updateLED(0, ledState1);
  updateLED(1, ledState2);
  // updateLED(2, feedbackVal);
vTaskDelay(10); 
  }
  // delay(500);
  // updateLED(0, ledState1);
}


