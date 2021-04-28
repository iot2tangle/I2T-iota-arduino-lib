#include <Arduino.h>
#include "Wire.h"
#include <WiFi101.h>
#include <ArduinoHttpClient.h>

//Sensors
#include "BlueDot_BME280.h"
#include <MPU6050_light.h>
#include <BH1750_WE.h>

#define ENABLE_SOUND 5
#define VALUE_SOUND 4

BlueDot_BME280 bme;
MPU6050 mpu(Wire);
BH1750_WE myBH1750(0x23);


char json[1024];
char s_name[12][64], d[12][64];

bool en_bme, en_mpu, en_bh, en_sound;

char buffer[16];
char* s;

const char* int_str(int);
const char* float_str(float);

void print_init_msg()
{
	delay(3000);
	Serial.println("Hi");
	
	// Init Json static chars
	strcpy(s_name[0],"InternalTemperature"); 
    strcpy(s_name[1],"Temperature");
    strcpy(s_name[2],"Humidity");
    strcpy(s_name[3],"Pressure");
    strcpy(s_name[4],"SoundLevel");
    strcpy(s_name[5],"Light");
    strcpy(s_name[6],"X");
    strcpy(s_name[7],"Y");
    strcpy(s_name[8],"Z");
    strcpy(s_name[9],"X");
    strcpy(s_name[10],"Y");
    strcpy(s_name[11],"Z");
	
}

void init_WiFi(const char* ssid, const char* pass)
{
	int status = WL_IDLE_STATUS;
	
    // check for the presence of the shield:
  	if (WiFi.status() == WL_NO_SHIELD) {
    	Serial.println("WiFi shield not present");
    	// don't continue:
    	while (true);
  	}

	  // attempt to connect to WiFi network:
	while (status != WL_CONNECTED) {
		Serial.print("Attempting to connect to SSID: \"");
		Serial.print(ssid); Serial.println("\" ...");
		// Connect to WPA/WPA2 network. Change this line if using open or WEP network:
		status = WiFi.begin(ssid, pass);

		// wait 10 seconds for connection:
		delay(10000);
	}
	Serial.println("Connected to wifi");
	
	  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

}

void init_HTTP(const char* c, int d)
{
	Serial.println("Init HTTP");
}

void init_sensors(bool ft)
{
	if (ft){
		Wire.begin();
	}

	Serial.print("   Sensors Detection:  ||	BME280: ");
	if (ft){
		bme.parameter.communication = 0;                    //I2C communication for Sensor 2 (bme2)
		bme.parameter.I2CAddress = 0x76;                    //I2C Address for Sensor 2 (bme2)
		bme.parameter.sensorMode = 0b11;                    //Setup Sensor mode for Sensor 2 
		bme.parameter.IIRfilter = 0b100;                   //IIR Filter for Sensor 2
		bme.parameter.humidOversampling = 0b101;            //Humidity Oversampling for Sensor 2
		bme.parameter.tempOversampling = 0b101;              //Temperature Oversampling for Sensor 2
		bme.parameter.pressOversampling = 0b101;             //Pressure Oversampling for Sensor 2 
		bme.parameter.pressureSeaLevel = 1013.25;            //default value of 1013.25 hPa (Sensor 2)
		bme.parameter.tempOutsideCelsius = 15;               //default value of 15°C
		bme.parameter.tempOutsideFahrenheit = 59;            //default value of 59°F
	}
	if (bme.init() != 0x60) 
	{
		Serial.print("NOT Detected");
		en_bme = false;
	}
	else
	{
		Serial.print("OK");
		en_bme = true;
	}


	Serial.print("	||	MPU6050: ");
	if (mpu.begin() != 0)
	{
		Serial.print("NOT Detected");
		en_mpu = false;
	}
	else
	{
		Serial.print("OK");
		en_mpu = true;
	}
	
	Serial.print("	||	BH1750: ");
	myBH1750.init();
	if (myBH1750.getLux() > 4700.0)
	{
		Serial.print("NOT Detected");
		en_bh = false;
	}
	else
	{
		Serial.print("OK");
		en_bh = true;
	}

	Serial.print("	||	Acoustic: ");
	if (ft){
		pinMode(ENABLE_SOUND, INPUT);
		pinMode(VALUE_SOUND, INPUT);
	}
	if (digitalRead(ENABLE_SOUND))
	{
		Serial.print("NOT Detected");
		en_sound = false;
	}
	else
	{
		Serial.print("OK");
		en_sound = true;
	}
	
	Serial.print("	||\n");
}

void read_sensors()
{   
    strcpy(d[0] , float_str(45.5));
    
    if (en_bme)
    {
		strcpy(d[1] , float_str(54.0));
		strcpy(d[2] , float_str(23.0));
		strcpy(d[3] , float_str(234.0));
	}
	
	if (en_sound)
	{
		if (digitalRead(VALUE_SOUND))
    		strcpy(d[4] , "High");
    	else
    		strcpy(d[4] , "Low");
    }
    
    if (en_bh)
    	strcpy(d[5] , float_str(32.0));
    
    if (en_mpu)
    {
		strcpy(d[6] , float_str(0.0));
		strcpy(d[7] , float_str(2.4));
		strcpy(d[8] , float_str(823.0));
		strcpy(d[9] , float_str(4.5));
		strcpy(d[10] , float_str(6.23));
		strcpy(d[11] , float_str(69.3));
	}

}

char* generate_json()
{
    int i, aux;

    strcpy(json, "{\"iot2tangle\":[");
	
    aux = 0;
    strcat(json, "{\"sensor\":\"Internal\",\"data\":[");
    for (i=0;i<1;i++)
	{
			if (aux != i) strcat(json, ",");
			strcat(json, "{\"");
			strcat(json, s_name[i+0]);
			strcat(json, "\":\"");
			strcat(json, d[i+0]);
			strcat(json, "\"}");
    }
    strcat(json, "]}");

    if (en_bme)
    {
		aux = 0;
		strcat(json, ",{\"sensor\":\"Environmental\",\"data\":[");
		for (i=0;i<3;i++)
		{
			if (aux != i) strcat(json, ",");
			strcat(json, "{\"");
			strcat(json, s_name[i+1]);
			strcat(json, "\":\"");
			strcat(json, d[i+1]);
			strcat(json, "\"}");
		}
		strcat(json, "]}");
    }

    if (en_sound)
    {
		aux = 0;
		strcat(json, ",{\"sensor\":\"Acoustic\",\"data\":[");
		for (i=0;i<1;i++)
		{
			if (aux != i) strcat(json, ",");
			strcat(json, "{\"");
			strcat(json, s_name[i+4]);
			strcat(json, "\":\"");
			strcat(json, d[i+4]);
			strcat(json, "\"}");
		}
		strcat(json, "]}");
    }

    if (en_bh)
    {
		aux = 0;
		strcat(json, ",{\"sensor\":\"Light\",\"data\":[");
		for (i=0;i<1;i++)
		{
			if (aux != i) strcat(json, ",");
			strcat(json, "{\"");
			strcat(json, s_name[i+5]);
			strcat(json, "\":\"");
			strcat(json, d[i+5]);
			strcat(json, "\"}");
		}
		strcat(json, "]}");
    }

    if (en_mpu)
    {
		aux = 0;
		strcat(json, ",{\"sensor\":\"Acelerometer\",\"data\":[");
		for (i=0;i<3;i++)
		{
			if (aux != i) strcat(json, ",");
			strcat(json, "{\"");
			strcat(json, s_name[i+6]);
			strcat(json, "\":\"");
			strcat(json, d[i+6]);
			strcat(json, "\"}");
		}
		strcat(json, "]}");

		aux = 0;
		strcat(json, ",{\"sensor\":\"Gyroscope\",\"data\":[");
		for (i=0;i<3;i++)
		{
			if (aux != i) strcat(json, ",");
			strcat(json, "{\"");
			strcat(json, s_name[i+9]);
			strcat(json, "\":\"");
			strcat(json, d[i+9]);
			strcat(json, "\"}");
		}
		strcat(json, "]}");
    }

	strcat(json, "],\"device\": \"");
	strcat(json, "DEVICE_ID_1");
	strcat(json, "\",\"timestamp\": \"0\"}");	
	
	Serial.println(json);

	return json;
}

void send_HTTP(const char* jsondata)
{
	Serial.println("Send HTTP");
	
	char serverAddress[] = "192.168.1.115/sensor_data";  // server address
	int port = 8080;
	
	WiFiClient wifi;
	HttpClient client = HttpClient(wifi, serverAddress, port);


	  Serial.println("making POST request");
	  String contentType = "application/json";

	  client.post("/", contentType, jsondata);

	  // read the status code and body of the response
	  int statusCode = client.responseStatusCode();
	  String response = client.responseBody();

	  Serial.print("Status code: ");
	  Serial.println(statusCode);
	  Serial.print("Response: ");
	  Serial.println(response);

	  Serial.println("Wait five seconds");
	  delay(5000);

//  HTTPClient http;
//  String url="192.168.1.115:8080";


//  //String url="https://<IPaddress>/testurl";
//  //String jsondata=(<properly escaped json data here>);

//  http.begin(url); 
//  http.addHeader("Content-Type", "Content-Type: application/json"); 

//  int httpResponseCode = http.POST(jsondata); //Send the actual POST request

//  if(httpResponseCode>0){
//    String response = http.getString();  //Get the response to the request
//    Serial.println(httpResponseCode);   //Print return code
//    Serial.println(response);           //Print request answer
//  } else {
//    Serial.print("Error on sending POST: ");
//    Serial.println(httpResponseCode);

//    http.end();

// }
	
}

const char* int_str(int d)
{
    s = " ";
    sprintf(buffer, "%d", d);
    s=buffer;
    return s;
}

const char* float_str(float d)
{
    s = " ";
    sprintf(buffer, "%.02f", d);
    s=buffer;
    return s;
}
