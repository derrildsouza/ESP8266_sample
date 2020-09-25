#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define light D2
#define ON  0
#define OFF 1

byte flag_val, val;

const char* ssid = "Krypton";
const char* password = "itisnot12345";
unsigned char status_light = 0;
char status_time_buff[64];
unsigned long start_on_time = millis();
unsigned long start_off_time = millis();

unsigned long CurrentTime; // = millis();
unsigned long total_on_time_ms;
unsigned long total_off_time_ms;

unsigned int total_on_time_seconds = 0;
int minutes = 0, hours = 0, seconds = 0;

WiFiServer server(80);

void setup() {
  EEPROM.begin(512);  //Initialize EEPROM
  flag_val = EEPROM.read(5);
  
  Serial.begin(115200);
  pinMode(light, OUTPUT);
  Serial.println();
  Serial.println();
  digitalWrite(light, flag_val);
  //digitalWrite(light, OFF);
  Serial.println("light OFF");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // config static IP
  IPAddress ip(192, 168, 1, 66); // where xx is the desired IP Address
  IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your
  //network
  WiFi.config(ip, gateway, subnet);
 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  Serial.println("new client");
  while(!client.available())
  {
    delay(1);
  }
  String req = client.readStringUntil('\r');
  Serial.println();
  Serial.println(req);
  Serial.println();

  client.flush();
  if (req.indexOf("/lightoff") != -1) {
    if( status_light == ON) {
      start_off_time = millis();
      CurrentTime = millis();
      total_on_time_ms = CurrentTime - start_on_time;
      Serial.println(total_on_time_ms/1000);
      #if 0
      total_on_time_seconds = total_on_time_ms/1000;
      minutes = total_on_time_seconds/60;
      seconds = total_on_time_seconds%60;
      hours = minutes/60;
      minutes = minutes%60;
      #endif
      get_time(total_on_time_ms/1000, &hours, &minutes, &seconds);
      bzero(status_time_buff, sizeof(status_time_buff));
      snprintf(status_time_buff, sizeof(status_time_buff), "The light was ON for %d hours %d mins %d seconds", hours, minutes, seconds);
      Serial.print("status_time_buff:");
      Serial.println(status_time_buff);
    } else {
      CurrentTime = millis();
      total_off_time_ms = CurrentTime - start_off_time;
      Serial.println(total_off_time_ms/1000);
      get_time(total_off_time_ms/1000, &hours, &minutes, &seconds);
      bzero(status_time_buff, sizeof(status_time_buff));
      snprintf(status_time_buff, sizeof(status_time_buff), "The light is OFF since %d hours %d mins %d seconds", hours, minutes, seconds);
      //bzero(status_time_buff, sizeof(status_time_buff));
      //snprintf(status_time_buff, sizeof(status_time_buff), "The light is OFF since %ld seconds", total_off_time_ms/1000);      
    }

    status_light = OFF;   
    //digitalWrite(light, LOW);
    digitalWrite(light, OFF);
    Serial.println("light OFF");
    EEPROM.write(5, OFF);
    EEPROM.commit();    //Store data to EEPROM
    
    } else if(req.indexOf("/lighton") != -1) {
      if (status_light == OFF ) {
      start_on_time = millis();
      CurrentTime = millis();
      total_off_time_ms = CurrentTime - start_off_time;
      Serial.println(total_off_time_ms/1000);
      //total_on_time_seconds = total_on_time_ms/1000;
      get_time(total_off_time_ms/1000, &hours, &minutes, &seconds);
      bzero(status_time_buff, sizeof(status_time_buff));
      snprintf(status_time_buff, sizeof(status_time_buff), "The light was OFF for %d hours %d mins %d seconds", hours, minutes, seconds);
      
      Serial.print("status_time_buff:");
      Serial.println(status_time_buff);
    
    } else {
      CurrentTime = millis();
      total_on_time_ms = CurrentTime - start_on_time;
      Serial.println(total_on_time_ms/1000);
      bzero(status_time_buff, sizeof(status_time_buff));
      //snprintf(status_time_buff, sizeof(status_time_buff), "The light is ON since %ld seconds", total_on_time_ms/1000);
      get_time(total_on_time_ms/1000, &hours, &minutes, &seconds);
      snprintf(status_time_buff, sizeof(status_time_buff), "The light is ON since %d hours %d mins %d seconds", hours, minutes, seconds);
    }

    status_light = ON;
    //digitalWrite(light,HIGH);
    digitalWrite(light, ON);
    Serial.println("light ON");
    EEPROM.write(5, ON);
    EEPROM.commit();    //Store data to EEPROM
    
  } else {

  }

  String web = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  web += "<html>\r\n";
  web += "<body style='background-color:powderblue;'>\r\n";
  //web += "<h1>Status :</h1>\r\n";
  web += "<h1>Status : ";
  if( status_light == ON )
      web += " ON\r\n";
  else
      web += " OFF\r\n";
  web += "</h1>";
  web += "</p>\r\n";
  web += "</p>\r\n";

  web += "<a href=\"/lightoff\">\r\n";
  web += "<button style='FONT-SIZE: 50px; HEIGHT: 200px; WIDTH: 300px; 126px; Z-INDEX: 0; TOP: 200px; ";
  web += "border-radius: 12px; background-color: #00FA00; border: 2px solid #4CAF50; ";
  web += "box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2), 0 6px 80px 0 rgba(0,0,0,0.5);'>Turn OFF</button >\r\n";
  web += "</a>\r\n";
  
  web += "<a href=\"/lighton\">\r\n";
  web += "<button style='FONT-SIZE: 50px; HEIGHT: 200px; WIDTH: 300px; 126px; Z-INDEX: 0; TOP: 200px; ";
  web += "border-radius: 12px; background-color: #F00000; border: 2px solid #FF0000; ";
  web += "box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2), 0 6px 80px 0 rgba(0,0,0,0.5);'>Turn ON</button >\r\n";
  web += "</a>\r\n";

  web += "</p>\r\n";

  web += "<h1 style='color:#FF00FF;'>";
  web += status_time_buff; //This header is now yellow
  web += "</h1>"; 
  
  web += "</body>\r\n";
  web += "</html>\r\n";

  client.print(web);
}

int get_time(int in_time, int *hours, int *minutes, int *seconds ) {
  *minutes = in_time/60;
  *seconds = in_time%60;
  *hours = *minutes/60;
  *minutes = *minutes%60;
  return 0;
}
