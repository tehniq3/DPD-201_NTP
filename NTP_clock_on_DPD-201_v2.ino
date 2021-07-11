// NTP clock on serial LCD2004
// original sketch by Nicu FLORICA (niq_ro) from http://nicuflorica.blogspot.ro/
// ver.2, use info from // https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
// NTP info: https://randomnerdtutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/
// TimeZome: https://nicuflorica.blogspot.com/2021/02/ceas-gps-cu-reglaj-ora-locala-4.html


// original for Datecs DPD-201
// ver.1 - add DST (Daylight Saving Time) or Summer Time control using local webpage
// ver.1a - add EEPROM store for TimeZone and DST
// ver.1a1 - clear the sketch
// ver.2 - add WiFiManager by tzapu


#include <SoftwareSerial.h>

#define DATAOUT 14  // D5 on R1 Mini 
SoftwareSerial lcd(255, DATAOUT);  // This is required, to start an instance of an LCD (RX at pin D6)

#include <EEPROM.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

/*
const char *ssid     = "niq_ro";
const char *password = "needcoldbeers";
*/

float TIMEZONE = 2.;
int timezone0 = 16; 
float diferenta[38] = {-12., -11.,-10.,-9.5,-9.,-8.,-7.,-6.,-5.,-4.,-3.5,-3.,-2.,-1.,0,
                      1.,2.,3.,3.5,4.,4.5,5.,5.5,5.75,6.,6.5,7.,8.,8.75,9.,9.5,10.,10.5,
                      11.,12.,12.75,13.,14};   // added manualy by niq_ro
#define adresa  100  // adress for store the
byte zero = 0;  // variable for control the initial read/write the eeprom memory

char daysOfTheWeek[7][12] = {"Sunday    ", "Monday    ", "Tuesday   ", "Wednesday ", "Thursday  ", "Friday    ", "Saturday  "};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Web Host
String header; // Variable to store the HTTP request
unsigned long currentTime = millis(); // Current time
unsigned long previousTime = 0; // Previous time
const long timeoutTime = 2000; // Define timeout time in milliseconds (example: 2000ms = 2s)

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// DST State
String oravara = ""; //
byte DST = 0;
byte DST0 = 0;

int hh, mm, ss;
int yy, ll, dd, zz;

String text7 = " niq_ro's NTP clock  on DPD-201 display ";
byte numar = 0;

WiFiServer server(80); // Set web server port number to 80


void setup () {
  EEPROM.begin(512);  //Initialize EEPROM  - http://www.esp8266learning.com/read-and-write-to-the-eeprom-on-the-esp8266.php
  Serial.begin(9600);
  Serial.println("  ");
  Serial.println("  ");
  Serial.println("->");
  lcd.begin(9600);  // Start the LCD at 9600 baud

initializare();
delay(1000);
mop(); 

zero = EEPROM.read(adresa - 1); // variable for write initial values in EEPROM
if (zero != 16)
{
EEPROM.write(adresa - 1, 16);  // zero
EEPROM.write(adresa, timezone0); // time zone (0...24 -> -12...+12) // https://en.wikipedia.org/wiki/Coordinated_Universal_Time
EEPROM.write(adresa + 1, 0);  // 0 => 24hours format, 1 => 12hours format
EEPROM.commit();    //Store data to EEPROM
} 

// read EEPROM memory;
timezone0 = EEPROM.read(adresa);  // timezone +12
TIMEZONE = (float)diferenta[timezone0];  // convert in hours
DST = EEPROM.read(adresa+1);
DST0 = DST;

if (DST == 0) oravara = "off"; 
else oravara = "off"; 

Serial.print("TimeZone: ");
Serial.println(TIMEZONE);
Serial.print("DST: ");
Serial.println(DST);

    String text2 = "  Wi-Fi connect in       progress..     ";
    afisare(text2);
 /* 
  WiFi.mode(WIFI_STA);
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  delay(1000);
  // Print local IP address
  Serial.println("\nWiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
*/

 //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  String text8 = "Wi-Fi: AutoConnectAP";
  afisare(text8);

  String text9 = "IP: 192.168.4.1     ";
  afisare(text9);
  delay(1000);
  
  //reset settings - for testing
  //wifiManager.resetSettings();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(180);
  
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if(!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  } 

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

    // Print local IP address
  Serial.println("\nWiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

mop();
  String text3 = "Wi-Fi connected !   ";
   afisare(text3);

  String text5 = "IP: ";
  text5 = text5 + WiFi.localIP().toString();

  afisare0(text5);
  
  delay(5000);
  
  timeClient.begin();
  timeClient.setTimeOffset((TIMEZONE+DST)*3600);
  server.begin(); // Start web server!
  
delay (1500);
mop(); 
afisare(text7);
delay(5000);
mop();
  Serial.println("------------");
}  // end setup



void loop () {

timeClient.update();

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client is connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) { // If the current line is blank, you got two newline characters in a row. That's the end of the client HTTP request, so send a response:
            client.println("HTTP/1.1 200 OK"); // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            client.println("Content-type:text/html"); // and a content-type so the client knows what's coming, then a blank line:
            client.println("Connection: close");
            client.println();
            
            if (header.indexOf("GET /vara/on") >= 0) { // If the user clicked the alarm's on button
              Serial.println("Daylight saving time (DST) was activated !");
              oravara = "on";
              DST = 1;
              timeClient.setTimeOffset((TIMEZONE+1)*3600); // Offset time from the GMT standard
            } 
            else if (header.indexOf("GET /vara/off") >= 0) { // If the user clicked the alarm's off button
              Serial.println("Daylight saving time (DST) was deactivated !");
              oravara = "off";
              DST = 0;
              timeClient.setTimeOffset((TIMEZONE+0)*3600); // Offset time from the GMT standard
            }

            else if (header.indexOf("GET /time") >= 0) { // If the user submitted the time input form
              // Strip the data from the GET request
              int index = header.indexOf("GET /time");
              String timeData = header.substring(index + 15, index + 22);
            }

unsigned long epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  
  
    hh = timeClient.getHours();
    mm = timeClient.getMinutes();
    ss = timeClient.getSeconds();

//Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  dd = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(dd);

  ll = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(ll);

  String currentMonthName = months[ll-1];
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  yy = ptm->tm_year+1900-2000;
  Serial.print("Year: ");
  Serial.println(yy);

  zz = timeClient.getDay();

            
            // Display the HTML web page
            // Head
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<link rel=\"stylesheet\" href=\"//stackpath.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css\">"); // Bootstrap
            client.println("</head>");
            
            // Body
            client.println("<body>");
            client.println("<h1 class=\"text-center mt-3\"NTP / DSP Clock by niq_ro</h1>"); // Title

            // Current Time
            client.print("<h1 class=\"text-center\">"); 
            if (hh < 10) client.print ("0");
            client.print (hh);
            client.print (":");
            if (mm < 10) client.print ("0");
            client.print (mm);
            client.print ("   -   ");
            if (ll < 10) client.print ("0");
            client.print (ll);
            client.print ("/");
            if (dd < 10) client.print ("0");
            client.print (dd);
            client.print ("/");
            client.print ("20");
            client.print (yy);
            client.println("</h1>");       
            
            // Display current state, and ON/OFF buttons for DST mode 
            client.println("<h2 class=\"text-center\">Daylight Saving Time - " + oravara + "</h2>");
            if (oravara=="off") {
              client.println("<p class=\"text-center\"><a href=\"/vara/on\"><button class=\"btn btn-sm btn-danger\">ON</button></a></p>");
            }
            else {
              client.println("<p class=\"text-center\"><a href=\"/vara/off\"><button class=\"btn btn-success btn-sm\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            client.println(); // The HTTP response ends with another blank line
            break; // Break out of the while loop
            
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    
    header = ""; // Clear the header variable
    client.stop(); // Close the connection
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  
  
    hh = timeClient.getHours();
    mm = timeClient.getMinutes();
    ss = timeClient.getSeconds();

//Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  dd = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(dd);

  ll = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(ll);

  String currentMonthName = months[ll-1];
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  yy = ptm->tm_year+1900-2000;
  Serial.print("Year: ");
  Serial.println(yy);

  zz = timeClient.getDay();

if (DST0 != DST)
{
timeClient.setTimeOffset((TIMEZONE+DST)*3600);
EEPROM.write(adresa + 1, DST);  // 1 => summer format, 0 => winter format 
EEPROM.commit();    //Store data to EEPROM 
Serial.print("Write in EEPROM: DST = ");
Serial.println(DST);
/*
if(oravara == "on")
timeClient.setTimeOffset((TIMEZONE+1)*3600); // Offset time from the GMT standard
else
timeClient.setTimeOffset(TIMEZONE*3600); // Offset time from the GMT standard
*/
}
DST0 = DST;


String text = "";
text = text + daysOfTheWeek[zz];
text = text + dd/10 + dd%10 + "/" + ll/10 + ll%10 + "/" + "20" + yy;

String text3 = "";
text3 = text3 + "      ";
text3 = text3 + hh/10 + hh%10 + ":" + mm/10 + mm%10 + ":" + ss/10 + ss%10;
text3 = text3 + "      ";

if (mm%2 == 0)
{
  afisare(text);
  afisare(text3);
}
else
{
  afisare(text3);
  afisare(text);
}

delay(1000);
}  // end main loop


void mop()
{
  lcd.write(231); // CLR (clear the display)
  delay(100);
}  // end 'mop'subroutine     

void initializare()
{
  lcd.write(1);  // SPACE
  lcd.write(231); // CLR (clear the display)
  
  // initialize display
  lcd.write(201); // ESC
  lcd.write(127); // @ 
delay(1000);
}

void afisare(String texte)
{
   for (int i = 0; i < texte.length(); i++)
  {
    lcd.write(255-2*texte[i]);   // Push each char 1 by 1 on each loop pass
  }
}
void afisare0(String texte)
{
   for (int i = 0; i < 20; i++)
  {
    lcd.write(255-2*texte[i]);   // Push each char 1 by 1 on each loop pass
  }
}
