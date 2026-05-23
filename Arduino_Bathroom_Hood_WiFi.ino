//  WIFI РЕЛЕ
//  Управление по темепратуре и влажности
//  Контроллер ESP-01
//  Датчик температуры и влажности DHT11 или DHT22

//Задачи
//2. Добавить кнопку включения и выключения устройства
//4. Добавить возможность включать и выключать реле вручную
//5. Добавить время для ручного режима.
//6. Добавить реверс для реле
//7. Добавить проверку пороговых значений.

#include <GyverPortal.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <LittleFS.h>

//ОТЛАДКА
#define DEBUG_ENABLE //Расскоментируй для включения отладки в порт

#ifdef DEBUG_ENABLE
#define DEBUG(x) Serial.print(x)
#define DEBUGLN(x) Serial.println(x)
#else
#define DEBUG(x)
#endif


#define INIT_ADDR 500  // номер резервной ячейки для инициализации и первой проверки памяти
#define INIT_KEY 51     // При любых изменениях в структурированных данных измените ключ  или стирайте память при прошивке.

#define SENSOR_PIN 2 //Пин датчика температуры и влажности (GPIO2)
#define SENSOR_TYPE DHT11 //Тип датчика DHT11 или DHT22
#define RELAY_PIN 0 //Пин реле (GPIO0) - меньше щелчков при старте

DHT dht(SENSOR_PIN, SENSOR_TYPE);
GyverPortal ui(&LittleFS);

/**************************СТРУКТУРА НАСТРОЕК В ПАМЯТИ EEPROM*************************************/
struct SettingsData {
  bool powerOnOff = false; // Ячейка 0 - Включение выключение устройства
  char ssid[20] = "SSID"; //SSID
  char pass[20] = "PASSWORD";  //Пароль
  bool useLocalAddress = false;
  char localAddress[20] = "hood";
  bool mqttEnable = false;  //Использовать MQTT или нет
  char mqttserver[40] = "test.mqtt.ru"; // имя сервера MQTT
  char mqttport[10] = "8080"; //Порт MQTT
  char mqttlogin[20] = "LOGIN"; //Логин MQTT
  char mqttpassword[20] = "PASSWORD"; //Пароль MQTT
  bool operatingMode = false; //Режим измерения
  int tempOn = 0;  // Температура включения
  int tempOff = 0; //Температура выключения
  int tempHyst = 1;  //Гистерезис температуры
  int humiOn = 0;  //Влажность для включения
  int humiOff = 0; //Влажность для отключения
  int humiHyst = 1;  //Гистерезис влажности
  bool invertOut = false;
};

SettingsData data;

/**************************ПЕРЕМЕННЫЕ*************************************/
uint32_t time_connect = 0; // временная переменная для расчета времени до запуска точки доступа
uint32_t time_manual_mode = 0; //Переменная времени для ручного режима запуска
bool enableAP = false; // проверка запущена ли точка доступа
bool statusFan = false;  //Состояние вентилятора вкл или выкл
bool mqttEnable = false; //Используем MQTT или нет
bool manualMode = false;  //Ручной режим или нет.
bool operatingMode = false; //Режим работы измерения
bool powerOnOff = false; //Включение/выключение устройства
bool useLocalAddress = false;
bool invertOut = false;
int currentTemperature = 0; //Текущая температура
int maxTemperature = 0; //Максимальная температура
int minTemperature = 0; //Минимальная температура
int tempHysteresis = 0; //Гистерезис температуры
int currentHumidity = 0;  //Текущая влажность
int maxHumidity = 0;  //Максимальная влажность
int minHumidity = 0;  //Минимальная влажность
int humiHysteresis = 0; //Гистерезис влажности

void setFanOutput(bool on) {
  statusFan = on;
  digitalWrite(RELAY_PIN, (invertOut ? !on : on) ? HIGH : LOW);
}


void setup() {
  delay(2000);
  #ifdef DEBUG_ENABLE
    Serial.begin(115200);
  #endif
  
  DEBUGLN("Start System");

  // Инициализация датчика температуры и влажности
  dht.begin();

  // инициализация памяти
  EEPROM.begin(512);
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) { // Проверяем на первый запуск и отсутстувие по адресу INIT_ADDR ключа INIT_KEY
    EEPROM.write(INIT_ADDR, INIT_KEY);    // Записываем ключ INIT_KEY по адресу INIT_ADDR
    EEPROM.put(0, data);  //Записываем данные по умолчанию в память
    if (EEPROM.commit()) { //Записываем в память
      DEBUGLN("EEPROM successfully committed");
    } else {
      DEBUGLN("ERROR! EEPROM commit failed");
    }
    DEBUGLN("Set Default Data In Memory");
  }
  delay(50);
  EEPROM.get(0, data); // Читаем данные из памяти
  delay(100);
  powerOnOff = data.powerOnOff; 
  useLocalAddress = data.useLocalAddress; 
  mqttEnable = data.mqttEnable;
  operatingMode = data.operatingMode;
  invertOut = data.invertOut;

  // Устанавливаем безопасный уровень ДО перевода пина в OUTPUT,
  // чтобы уменьшить щелчок реле при старте.
  digitalWrite(RELAY_PIN, invertOut ? HIGH : LOW);
  pinMode(RELAY_PIN, OUTPUT);
  
  // пытаемся подключиться
  DEBUG("Connect to: ");
  DEBUGLN(data.ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(data.ssid, data.pass);
  time_connect = millis();  //записываем текущее время
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG(".");
    
    if(millis() - time_connect >= 10000 && !enableAP){ // Если время вышло и точка доступа не запущена, то запускаем точку доступа с формой ввода SSID и пароля 
      enableAP = true;
      startAPAndFormForConnectToWIFI();
    }
  }
  DEBUGLN();
  DEBUG("Connected! Local IP: ");

  DEBUGLN(WiFi.localIP());

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  if (!LittleFS.begin()) {
    DEBUGLN("FS Error");
  }

  //Если подключились к точке доступа, то запускаем портал.
  ui.attachBuild(build);
  ui.attach(action);
  if(useLocalAddress){
    ui.start(data.localAddress);
    DEBUGLN();
    DEBUG("Portal Start: ");
    DEBUG(data.localAddress);
    DEBUGLN();    
  }else{
    ui.start();
    DEBUGLN();
    DEBUG("Portal Start");
    DEBUGLN(); 
  } 
  ui.enableOTA();   // OTA обновление прошивки без пароля
  //ui.enableOTA("admin", "pass");  // с паролем
  ui.downloadAuto(true);
}
/*******************Запускаем портал с формой для подключения к WiFi************************************/
void startAPAndFormForConnectToWIFI() {
  DEBUGLN("Start AP and Portal");

  // запускаем точку доступа
  WiFi.mode(WIFI_AP);
  WiFi.softAP("WiFi Hood");

  // запускаем портал с формой ввода
  ui.attachBuild(build);
  ui.attach(action);
  if(useLocalAddress){
    ui.start(data.localAddress);
    DEBUGLN();
    DEBUG("Portal Start: ");
    DEBUG(data.localAddress);
    DEBUGLN();    
  }else{
    ui.start();
    DEBUGLN();
    DEBUG("Portal Start");
    DEBUGLN(); 
  } 

  // работа портала
  while (ui.tick());
}

void loop() {
  ui.tick();

  static uint32_t readTime;

  if(millis() - readTime >= 5000){
    readTime = millis();    
    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();

    if (!isnan(newTemperature)) {
      currentTemperature = (int)newTemperature;
    } else {
      DEBUGLN("DHT read error: temperature");
    }

    if (!isnan(newHumidity)) {
      currentHumidity = (int)newHumidity;
    } else {
      DEBUGLN("DHT read error: humidity");
    }
    DEBUGLN();
    DEBUG("Temperature: ");
    DEBUG(currentTemperature);
    DEBUG(" Humidity: ");
    DEBUG(currentHumidity);
    DEBUG(" Reley Status: ");
    DEBUG(statusFan);    
    DEBUGLN();
  } //if(millis() - readTime >= 10000)
  //Режим работы
  if (manualMode){  
    if(statusFan){
      setFanOutput(true);
    }else{
      setFanOutput(false);
    } //if(statusFan)
  }else{
    if(operatingMode) {
      if(data.tempOn <= data.tempOff){
        if(currentTemperature <= data.tempOn - data.tempHyst){
          setFanOutput(true);
        }else if(currentTemperature >= data.tempOff + data.tempHyst){
          setFanOutput(false);          
        }
      }else{
        if(currentTemperature >= data.tempOn + data.tempHyst){
          setFanOutput(true);
        }else if (currentTemperature <= data.tempOff - data.tempHyst){
          setFanOutput(false); 
        }
      } 
    }else{
        if(data.humiOn > data.humiOff){
            if(currentHumidity >= data.humiOn + data.humiHyst){
              setFanOutput(true);
            }      
            if(currentHumidity <= data.humiOff - data.humiHyst) {
              setFanOutput(false);
            }
        }else{
            if(currentHumidity <= data.humiOn - data.humiHyst || currentHumidity >= data.humiOff + data.humiHyst){
              setFanOutput(true);
            }else{ 
              setFanOutput(false);
            }          
        }
    }
  } //if (manualMode)
} //void loop
