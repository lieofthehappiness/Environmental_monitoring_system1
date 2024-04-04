//納入需要的函式庫
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <WiFiEsp.h>
#include <DHT.h>

//設定DHT22的腳位
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//設定GP2Y1014AU的相關數據
int measurePin = 0; 
int ledPower = 4;
int samplingTime = 200;
int deltaTime = 40;
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

//設定Wi-Fi資料
const char* ssid = "WIFINAME";
const char* password = "YOURPASSWORD";
const char* mqtt_server = "120.107.168.127";

/*
由於Esp-01的通訊方式使用的Rx, Tx(原為0, 1腳位)會影響Arduino UNO的Serial print功能
所以我們使用此函示庫功能另外設定兩個數位腳位當作Esp-01的通訊腳位
 */
SoftwareSerial ESP8266(6, 7);

int WiFi_Status = WL_IDLE_STATUS;

//設定計時器
long now = millis();
long lastMeasure = 0;

WiFiEspClient espClient;
PubSubClient client(espClient);

//設定傳輸字串的相關資訊
long lastMsg = 0;
char msg[50];
int value = 0;

//程式的開頭並呼叫相關函示啟動模組
void setup() {   
  Serial.begin(115200);
  setup_wifi();//連線WiFi
  client.setServer(mqtt_server, 1883);//設定MQTT伺服器位置
  dht.begin();//啟動DHT22
  pinMode(ledPower,OUTPUT);//設定GP2Y1014AU0F的led燈腳位
}

void setup_wifi() {

  pinMode(2, OUTPUT);
  ESP8266.begin(9600);
  
  WiFi.init(&ESP8266);

  Serial.print("正在設定WiFi!\r\n");
  do {
    Serial.println("WiFi 連線中 ...");
    WiFi_Status = WiFi.begin(ssid, password);
    delay(500);
  } while (WiFi_Status != WL_CONNECTED);

  Serial.println("ＷiFi 已連線!");
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.println("WiFi 設定完成");
}


void reconnect() {  //連線MQTT
  while (!client.connected()) {
    Serial.print("嘗試連線MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("已連線");
    } else {
      Serial.print("連線失敗, rc=");
      Serial.print(client.state());
      Serial.println("5秒後重試");
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) { //若以斷線，執行函式S並重新連線
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) { 
    lastMsg = now;
    float t = dht.readTemperature();//讀取溫度資料
    snprintf (msg, 75, String(t).c_str());//組成字元陣列
    Serial.print("上傳溫度資料: ");
    Serial.println(msg);
    client.publish("Temperature", msg);//Publish溫度資料
    float h = dht.readHumidity();//讀取濕度資料
    snprintf (msg, 75, String(h).c_str());//組成字元陣列
    Serial.print("上傳濕度資料: ");
    Serial.println(msg);
    client.publish("Humidity", msg);//Publish濕度資料
    
    voMeasured = 0;
    calcVoltage = 0;
    dustDensity = 0;
    digitalWrite(ledPower,LOW); 
    delayMicroseconds(samplingTime);
    voMeasured = analogRead(measurePin);
    digitalWrite(ledPower,HIGH); 
    delayMicroseconds(sleepTime);
    //感測空氣品質資料
    
    calcVoltage = (voMeasured * 5.0) / 1024.0;
    dustDensity = 0.168 * calcVoltage - 0.088;
    dustDensity = dustDensity*1000 ;
    //透過官方公布的datasheet運算轉換公式並將A0的類比訊號轉換成AQI
    
    snprintf (msg, 75, String(dustDensity).c_str());//組成字元陣列
    Serial.println(voMeasured);
    Serial.print("上傳空氣品質資料 : ");
    Serial.println(msg);
    client.publish("AQI", msg);//上傳AQI資料
    delay (1000);
  }
}