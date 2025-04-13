#include <WiFiEsp.h>
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <DHT.h>
#define DEBUG
#define AP_SSID "embB"
#define AP_PASS "embB1234"
#define SERVER_NAME "10.10.141.71"
#define SERVER_PORT 5000
#define LOGID "LJH_ARD"
#define PASSWD "PASSWD"

#define CDS_PIN A0
#define BUTTON_PIN 2
#define LED_LAMP_PIN 12
#define DHTPIN 4
#define SERVO_PIN 5
#define WIFIRX 6
#define WIFITX 7
#define LED_BUILTIN_PIN 13
#define BUZZER_PIN 3

#define CMD_SIZE 50
#define ARR_CNT 5
#define DHTTYPE DHT11
bool timerIsrFlag = false;
boolean lastButton = LOW;    
boolean currentButton = LOW;    
boolean ledOn = false;      
boolean cdsFlag = false;

char sendId[10] = "LJH_ARD";
char sendBuf[CMD_SIZE];
char lcdLine1[17] = "Smart IoT By LJH";
char lcdLine2[17] = "WiFi Connecting!";

int cds = 0;
unsigned int secCount;
unsigned int myservoTime = 0;

char getSensorId[10];
int sensorTime;
float temp = 0.0;
float humi = 0.0;
bool updatTimeFlag = false;
typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
} DATETIME;
DATETIME dateTime = {0, 0, 0, 12, 0, 0};
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial wifiSerial(WIFIRX, WIFITX);
WiFiEspClient client;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo;

void setup() {
  lcd.init();
  lcd.backlight();
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);

  pinMode(CDS_PIN, INPUT);    
  pinMode(BUTTON_PIN, INPUT);    
  pinMode(LED_LAMP_PIN, OUTPUT);   
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
#ifdef DEBUG
  Serial.begin(115200);
#endif
  wifi_Setup();
  MsTimer2::set(1000, timerIsr);
  MsTimer2::start();
  myservo.attach(SERVO_PIN);
  myservo.write(0);
  myservoTime = secCount;
  dht.begin();
}

void loop() {
  if (client.available()) {
    socketEvent();
  }

  if (timerIsrFlag) 
  {
    timerIsrFlag = false;
    if (!(secCount % 5)) 
    {

      cds = map(analogRead(CDS_PIN), 0, 1023, 0, 100);
      if ((cds >= 50) && cdsFlag)
      {
        cdsFlag = false;
        sprintf(sendBuf, "[%s]CDS@%d\n", sendId, cds);
        client.write(sendBuf, strlen(sendBuf));
        client.flush();
      } else if ((cds < 50) && !cdsFlag)
      {
        cdsFlag = true;
        sprintf(sendBuf, "[%s]CDS@%d\n", sendId, cds);
        client.write(sendBuf, strlen(sendBuf));
        client.flush();
      }
      humi = dht.readHumidity();
      temp = dht.readTemperature();
#ifdef DEBUG
            Serial.print("Cds: ");
            Serial.print(cds);
            Serial.print(" Humidity: ");
            Serial.print(humi);
            Serial.print(" Temperature: ");
            Serial.println(temp);
     
#endif
      if (!client.connected()) {
        lcdDisplay(0, 1, "Server Down");
        server_Connect();
      }
    }
    if (myservo.attached() && myservoTime + 2 == secCount)
    {
      myservo.detach();
    }
    if (sensorTime != 0 && !(secCount % sensorTime ))
    {
      sprintf(sendBuf, "[%s]SENSOR@%d@%d@%d\n", getSensorId, cds, (int)temp, (int)humi);
      client.write(sendBuf, strlen(sendBuf));
      client.flush();
    }
    
    sprintf(lcdLine1, "%02d.%02d  %02d:%02d:%02d", dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec );
    lcdDisplay(0, 0, lcdLine1);
    if (updatTimeFlag)
    {
      client.print("[GETTIME]\n");
      updatTimeFlag = false;
    }
  }

  currentButton = debounce(lastButton);  
  if (lastButton == LOW && currentButton == HIGH)  
  {
    ledOn = !ledOn;      
    digitalWrite(LED_BUILTIN_PIN, ledOn);
    sprintf(sendBuf, "[HM_CON]GAS%s\n", ledOn ? "ON" : "OFF");
    client.write(sendBuf, strlen(sendBuf));
    client.flush();
  }
  lastButton = currentButton;     

}
void socketEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len;

  sendBuf[0] = '\0';
  len = client.readBytesUntil('\n', recvBuf, CMD_SIZE);
  client.flush();
#ifdef DEBUG
  Serial.print("recv : ");
  Serial.print(recvBuf);
#endif
  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }
  if ((strlen(pArray[1]) + strlen(pArray[2])) < 16)
  {
    sprintf(lcdLine2, "%s %s", pArray[1], pArray[2]);
    lcdDisplay(0, 1, lcdLine2);
  }
  if (!strncmp(pArray[1], " New", 4))
  {
#ifdef DEBUG
    Serial.write('\n');
#endif
    strcpy(lcdLine2, "Server Connected");
    lcdDisplay(0, 1, lcdLine2);
    updatTimeFlag = true;
    return ;
  }
  else if (!strncmp(pArray[1], " Alr", 4))
  {
#ifdef DEBUG
    Serial.write('\n');
#endif
    client.stop();
    server_Connect();
    return ;
  }
  else if (!strcmp(pArray[1], "LED")) {
    if (!strcmp(pArray[2], "ON")) {
      digitalWrite(LED_BUILTIN_PIN, HIGH);
    }
    else if (!strcmp(pArray[2], "OFF")) {
      digitalWrite(LED_BUILTIN_PIN, LOW);
    }
    sprintf(sendBuf, "[LJH_SQL]SET@%s@%d@%s\n", pArray[1],!strcmp(pArray[2],"ON")?1:0,pArray[0]);
    
    client.write(sendBuf, strlen(sendBuf));
    client.flush();
#ifdef DEBUG
  Serial.print(", send : ");
  Serial.print(sendBuf);
#endif
    return; 
  } else if (!strcmp(pArray[1], "LAMP")) {
    if (!strcmp(pArray[2], "ON")) {
      digitalWrite(LED_LAMP_PIN, HIGH);
    }
    else if (!strcmp(pArray[2], "OFF")) {
      digitalWrite(LED_LAMP_PIN, LOW);
    }
    sprintf(sendBuf, "[LJH_SQL]SET@%s@%d@%s\n", pArray[1],!strcmp(pArray[2],"ON")?1:0,pArray[0]);
  } else if (!strcmp(pArray[1], "GETSTATE")) {
    strcpy(sendId, pArray[0]);
    if (!strcmp(pArray[2], "LED")) {
      sprintf(sendBuf, "[%s]LED@%s\n", pArray[0], digitalRead(LED_BUILTIN_PIN) ? "ON" : "OFF");
    }
  }
  else if (!strcmp(pArray[1], "SERVO"))
  {
    myservo.attach(SERVO_PIN);
    myservoTime = secCount;
    if (!strcmp(pArray[2], "ON")){

    
      myservo.write(180);
      buzzOn();
    }
    else
      myservo.write(0);
    sprintf(sendBuf, "[%s]%s@%s\n", pArray[0], pArray[1], pArray[2]);

  }
  else if (!strncmp(pArray[1], "GETSENSOR", 9)) {
    if (pArray[2] != NULL) {
      sensorTime = atoi(pArray[2]);
      strcpy(getSensorId, pArray[0]);
      return;
    } else {
      sensorTime = 0;
      sprintf(sendBuf, "[%s]%s@%d@%d@%d\n", pArray[0], pArray[1], cds, (int)temp, (int)humi);
    }
  }
  else if (!strcmp(pArray[1], "BUZZ")) {
    if (!strcmp(pArray[2], "ON")) {
       buzzOn();
    }
  }
  else if(!strcmp(pArray[0],"GETTIME")) {
    dateTime.year = (pArray[1][0]-0x30) * 10 + pArray[1][1]-0x30 ;
    dateTime.month =  (pArray[1][3]-0x30) * 10 + pArray[1][4]-0x30 ;
    dateTime.day =  (pArray[1][6]-0x30) * 10 + pArray[1][7]-0x30 ;
    dateTime.hour = (pArray[1][9]-0x30) * 10 + pArray[1][10]-0x30 ;
    dateTime.min =  (pArray[1][12]-0x30) * 10 + pArray[1][13]-0x30 ;
    dateTime.sec =  (pArray[1][15]-0x30) * 10 + pArray[1][16]-0x30 ;
    return;
  } 
  else
    return;

  client.write(sendBuf, strlen(sendBuf));
  client.flush();

#ifdef DEBUG
  Serial.print(", send : ");
  Serial.print(sendBuf);
#endif
}
void timerIsr()
{
  timerIsrFlag = true;
  secCount++;
  clock_calc(&dateTime);
}
void clock_calc(DATETIME *dateTime)
{
  int ret = 0;
  dateTime->sec++;

  if(dateTime->sec >= 60) 
  { 
      dateTime->sec = 0;
      dateTime->min++; 
             
      if(dateTime->min >= 60)
      { 
          dateTime->min = 0;
          dateTime->hour++;
          if(dateTime->hour == 24) 
          {
            dateTime->hour = 0;
            updatTimeFlag = true;
          }
       }
    }
}
void buzzOn() {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);  
    digitalWrite(BUZZER_PIN, LOW);
}

void wifi_Setup() {
  wifiSerial.begin(38400);
  wifi_Init();
  server_Connect();
}

void wifi_Init()
{
  do {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_WIFI
      Serial.println("WiFi shield not present");
#endif
    }
    else
      break;
  } while (1);

#ifdef DEBUG_WIFI
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(AP_SSID);
#endif
  while (WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {
#ifdef DEBUG_WIFI
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);
#endif
  }
  sprintf(lcdLine1, "ID:%s", LOGID);
  lcdDisplay(0, 0, lcdLine1);
  sprintf(lcdLine2, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  lcdDisplay(0, 1, lcdLine2);

#ifdef DEBUG_WIFI
  Serial.println("You're connected to the network");
  printWifiStatus();
#endif
}
int server_Connect()
{
#ifdef DEBUG_WIFI
  Serial.println("Starting connection to server...");
#endif

  if (client.connect(SERVER_NAME, SERVER_PORT)) {
#ifdef DEBUG_WIFI
    Serial.println("Connect to server");
#endif
    client.print("["LOGID":"PASSWD"]");
  }
  else
  {
#ifdef DEBUG_WIFI
    Serial.println("server connection failure");
#endif
  }
}
void printWifiStatus()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
void lcdDisplay(int x, int y, char * str)
{
  int len = 16 - strlen(str);
  lcd.setCursor(x, y);
  lcd.print(str);
  for (int i = len; i > 0; i--)
    lcd.write(' ');
}
boolean debounce(boolean last)
{
  boolean current = digitalRead(BUTTON_PIN); 
  if (last != current)      
  {
    delay(5);         
    current = digitalRead(BUTTON_PIN);  
  }
  return current;       
}
