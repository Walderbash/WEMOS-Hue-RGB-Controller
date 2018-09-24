
#include <Arduino.h>
#include <RGBConverter.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "config.h"

int fddf;

int totalChannels = 3;

int addressA = D1; //8;
int addressB = D2; //9;
int addressC = D3; //10;
int switchPin = D6;

int A = 0;      //Address pin A
int B = 0;      //Address pin B
int C = 0;      //Address pin C

byte rgbArray[3];
long prevColor;
int prevOnOff;

String apiKey = HUEAPIKEY; 
String hubIP = HUEHUBIP;
String lightId  = HUELIGHTID;


void setup() {


  Serial.begin(9600 );
  // Prepare address pins for output
  pinMode(addressA, OUTPUT);
  pinMode(addressB, OUTPUT);
  pinMode(addressC, OUTPUT);
  // Prepare read pin 
  pinMode(A0, INPUT);
  pinMode(switchPin,INPUT_PULLUP);

  WiFi.begin(SSID, PASSWORD);   //WiFi connection
   while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
    
    delay(500);
    Serial.println("Waiting for connection");
  }

}

void setLight(long hueArray[3],bool turnOff= 0) {

  if(WiFi.status()== WL_CONNECTED) {   //Check WiFi connection status
    
    HTTPClient http;    //Declare object of class HTTPClient

    
    http.begin("http://"+hubIP+"/api/"+apiKey+"/lights/"+lightId+"/state");      //Specify request destination
    http.addHeader("Content-Type", "application/json");  //Specify content-type header

    char body[100];
    if (turnOff)
    {
      String offbody = "{\"on\":false}"; 
      offbody.toCharArray(body,100);
    }
    else
    {
      sprintf(body,"{\"on\": true, \"hue\": %d, \"bri\": %d, \"sat\": %d}",hueArray[0],hueArray[2],hueArray[1]);
    }

    Serial.println(body);
    int httpCode = http.sendRequest("PUT", body);
    //int httpCode = http.PUT(body);   //Send the request
    String payload = http.getString();                  //Get the response payload
    
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    
    http.end();  //Close connection
  
  } else {
    Serial.println("Error in WiFi connection");   
  }
}

void loop() {
  //Select each pin and read value
  for(int i=0; i<totalChannels; i++){
    A = bitRead(i,0); //Take first bit from binary value of i channel.
    B = bitRead(i,1); //Take second bit from binary value of i channel.
    C = bitRead(i,2); //Take third bit from value of i channel.

    //Write address to mux
    digitalWrite(addressA, A);
    digitalWrite(addressB, B);
    digitalWrite(addressC, C);

    //Read and print value
    /*
    Serial.print("Channel ");
    Serial.print(i);
    Serial.print(" value: ");
    Serial.println(analogRead(A0));
    */
    int val = analogRead(A0);
    rgbArray[i] = map(val, 0, 1024, 0, 255);

  }

  //convert our (backwards) RGB value to HSV
  double hsvArray[3];
  RGBConverter converter;
  converter.rgbToHsv(rgbArray[2],rgbArray[1],rgbArray[0],hsvArray);

  //convert double 0-1 HSV to hue values
  long longArray[3];
  longArray[0] = hsvArray[0] * 10000L;
  longArray[1] = hsvArray[1] * 10000L;
  longArray[2] = hsvArray[2] * 10000L;

  long hueArray[3];
//map-old doesnt work higher then 10000 
  hueArray[0] =map(longArray[0],0L,10000L,0L,65535L);
  hueArray[1] =map(longArray[1],0L,10000L,0L,254L);
  hueArray[2] =map(longArray[2],0L,10000L,0L,254L);

  char buffer[50];
  sprintf(buffer, "RGB %d %d %d ",rgbArray[2],rgbArray[1],rgbArray[0]);
  Serial.println(buffer);
/*
  sprintf(buffer, "HSV %f %f %f ",hsvArray[0],hsvArray[1],hsvArray[2]);
  Serial.println(buffer);

  sprintf(buffer, "long %d %d %d ",longArray[0],longArray[1],longArray[2]);
  Serial.println(buffer);

  char buffer2[50];
  sprintf(buffer2, "Hue HSB %d %d %d !",hueArray[0],hueArray[1],hueArray[2]);
  Serial.println(buffer2);
*/  

  int onOff = digitalRead(switchPin);
  //Serial.print("Switch: ");
  //Serial.println(onOff);

  long nowColor = rgbArray[0]+rgbArray[1]+rgbArray[2];

  if (nowColor < prevColor-10 || nowColor > prevColor+10 || onOff != prevOnOff)
  {
    prevColor = nowColor;
    prevOnOff = onOff;
    setLight(hueArray,onOff);
  }
  delay(200);
}

 