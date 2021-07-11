// NTP clock on serial LCD2004
// original sketch by Nicu FLORICA (niq_ro) from http://nicuflorica.blogspot.ro/
// ver.2, use info from // https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
// NTP info: https://randomnerdtutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/

// original for Datecs DPD-201


#include <SoftwareSerial.h>

#define DATAOUT 14  // D5 on R1 Mini 
SoftwareSerial lcd(255, DATAOUT);  // This is required, to start an instance of an LCD (RX at pin D6)


#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid     = "bbk2";
const char *password = "internet2";

const long utcOffsetInSeconds = 3*3600;

char daysOfTheWeek[7][12] = {"Sunday    ", "Monday    ", "Tuesday   ", "Wednesday ", "Thursday  ", "Friday    ", "Saturday  "};


//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

int hh, mm, ss;
int yy, ll, dd, zz;

String text7 = " niq_ro's NTP clock  on DPD-201 display ";
byte numar = 0;


void setup () {
  Serial.begin(9600);
  lcd.begin(9600);  // Start the LCD at 9600 baud
  
  
  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
 //  lcd.setPosition(4, 19);
 //  lcd.print(".");
    delay(500);
 //   lcd.setPosition(4, 19);
 //  lcd.print(" ");
  }
//  Serial.println("WiFi connected");
//  lcd.setPosition(4, 0);
  Serial.println("WiFi connected      ");
  
  delay (1500);
//  lcd.clear();
//  ceas();
timeClient.begin();
//lcd.clear();

//delay(3000);
initializare();

delay(1000);
mop(); 
for (int i = 0; i < text7.length(); i++)
  {
    lcd.write(255-2*text7[i]);   // Push each char 1 by 1 on each loop pass
  }
delay(5000);
mop();
}  // end setup



void loop () {

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


String text = "";
text = text + daysOfTheWeek[zz];
text = text + dd/10 + dd%10 + "/" + ll/10 + ll%10 + "/" + "20" + yy;

String text3 = "";
text3 = text3 + "      ";
text3 = text3 + hh/10 + hh%10 + ":" + mm/10 + mm%10 + ":" + ss/10 + ss%10;
text3 = text3 + "      ";

if (mm%2 == 0)
{
 for (int i = 0; i < text.length(); i++)
  {
    lcd.write(255-2*text[i]);   // Push each char 1 by 1 on each loop pass
  }
   for (int i = 0; i < text3.length(); i++)
  {
    lcd.write(255-2*text3[i]);   // Push each char 1 by 1 on each loop pass
  }
}
else
{
   for (int i = 0; i < text3.length(); i++)
  {
    lcd.write(255-2*text3[i]);   // Push each char 1 by 1 on each loop pass
  }
   for (int i = 0; i < text.length(); i++)
  {
    lcd.write(255-2*text[i]);   // Push each char 1 by 1 on each loop pass
  }
}

delay(1000);
}  // end main loop


void mop()
{
  lcd.write(231); // CLR (clear the display)
  delay(10);
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
