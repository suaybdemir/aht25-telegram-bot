#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>
#include <WiFi.h>             // ESP32 WiFi library
#include <WiFiClientSecure.h> // Secure client for Telegram HTTPS
#include <UniversalTelegramBot.h>

// --- Screen Settings ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Sensor Settings ---
Adafruit_AHTX0 aht;

#define BUTTON_PIN 14 

// --- WiFi & Telegram Settings ---
#define WIFI_SSID "" 
#define WIFI_PASSWORD ""

#define BOT_TOKEN "" // Telegram Bot Token
#define CHAT_ID ""

WiFiClientSecure clientSecure; //Https on!
UniversalTelegramBot bot(BOT_TOKEN, clientSecure);

// --- Time Settings (ms) ---
const unsigned long SENSOR_READ_INTERVAL = 1000; 
const unsigned long TELEGRAM_SEND_INTERVAL = 3600000;
const unsigned long TELEGRAM_CHECK_INTERVAL = 20000; 

int pageIndex = 0;            
unsigned long lastButtonPress = 0; // Son buton basılma zamanı (debounce için)
float tempHistory[60];          // Last 60 seconds data of temperature
float humHistory[60];           // ....... of Humidity
int dataIndex = 0;      

unsigned long lastSensorRead = 0;   
unsigned long lastTelegramSend = 0;  
unsigned long lastTelegramCheck = 0;  

void handleButton();
void readSensorAndUpdateHistory();
void sendTelegramMessage(String message);
void handleTelegramMessages(int numNewMessages);
void showTemperature(float temp);
void showHumidity(float hum);
void drawGraph(float *data, const char* label, float minVal, float maxVal);
void showName();
void connectWiFi();

void setup() {
  Serial.begin(115200);
  Serial.println("Sistem baslatiliyor...");

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!aht.begin()) {
    Serial.println("HATA: AHTx0 sensörü bulunamadı!");
    while (1) delay(10); 
  }
  Serial.println("AHTx0 sensörü bulundu.");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("HATA: SSD1306 OLED bulunamadı!");
    while (1) delay(10);
  }
  Serial.println("OLED ekran bulundu.");
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Baslatiliyor...");
  display.display();
  delay(1000);

  connectWiFi();

  clientSecure.setInsecure(); 

  Serial.println("Kurulum tamamlandi.");
  lastTelegramSend = millis(); 
}

void loop() {
  unsigned long currentMillis = millis(); 

  handleButton();

	//Periodical Sensor Reading!
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;
    readSensorAndUpdateHistory();
  }

  // Periodical Sends Telegram Update 
  if (WiFi.status() == WL_CONNECTED && (currentMillis - lastTelegramSend >= TELEGRAM_SEND_INTERVAL)) {
    lastTelegramSend = currentMillis;
    float latestTemp = tempHistory[(dataIndex - 1 + 60) % 60];
    float latestHum = humHistory[(dataIndex - 1 + 60) % 60];
    String message = "Periyodik Guncelleme:\n";
    message += "Sicaklik: " + String(latestTemp, 1) + " C\n";
    message += "Nem: " + String(latestHum, 1) + " %";
    sendTelegramMessage(message);
  }

  // Scree Update
  display.clearDisplay();
  float displayTemp = tempHistory[(dataIndex - 1 + 60) % 60]; // Gösterim için en son veri
  float displayHum = humHistory[(dataIndex - 1 + 60) % 60];
  switch (pageIndex) {
    case 0: showTemperature(displayTemp); break;
    case 1: showHumidity(displayHum); break;
    case 2: drawGraph(tempHistory, "Sicaklik", 0, 50); break; // Sıcaklık min/max aralığı
    case 3: drawGraph(humHistory, "Nem", 0, 100); break;     // Nem min/max aralığı
    case 4: showName(); break;
  }
  display.display(); // Updating Screen!

  // Periodically telegram messages are checking here!
   if (WiFi.status() == WL_CONNECTED && (currentMillis - lastTelegramCheck >= TELEGRAM_CHECK_INTERVAL)) {
       lastTelegramCheck = currentMillis;
       int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
       if (numNewMessages > 0) {
          Serial.println("Yeni Telegram mesaj(lar)ı alındı.");
          handleTelegramMessages(numNewMessages);
       }
   }

   // If there is not connection try to connect until been connected
   if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi bağlantısı koptu! Tekrar bağlanmaya çalışılıyor...");
      connectWiFi();
   }

  //dont put here delay()!!! Because of microcontroller speed you cannot use . If you does your display screen will no longer run after 2 or 3 pressing to button!!!
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return; 
  }

  Serial.print("WiFi'ye baglaniliyor ");
  Serial.print(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("WiFi Baglaniyor...");
  display.print(WIFI_SSID);
  display.display();

  int চেষ্টা = 0;
  while (WiFi.status() != WL_CONNECTED && চেষ্টা < 20) { // ~10 saniye dene
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
    চেষ্টা++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi baglantisi saglandi.");
    Serial.print("IP Adresi: ");
    Serial.println(WiFi.localIP());
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi Baglandi!");
    display.println(WiFi.localIP());
    display.display();
    delay(1500);
  } else {
    Serial.println("\nWiFi baglantisi kurulamadi!");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi Hatasi!");
    display.display();
    delay(1500);
  }
}

void handleButton() {
  static bool lastState = HIGH; 
  bool currentState = digitalRead(BUTTON_PIN);

  if (lastState == HIGH && currentState == LOW) {
    unsigned long now = millis();
    // Debounce Controlling
    if (now - lastButtonPress > 300) {
      pageIndex = (pageIndex + 1) % 5; // Pagination
      Serial.print("Butona basildi. Yeni sayfa: ");
      Serial.println(pageIndex);
      lastButtonPress = now; 
    }
  }
  lastState = currentState;
}

void readSensorAndUpdateHistory() {
  sensors_event_t humidity, temp;
  if (aht.getEvent(&humidity, &temp)) { 
    tempHistory[dataIndex] = temp.temperature;
    humHistory[dataIndex] = humidity.relative_humidity;
    dataIndex = (dataIndex + 1) % 60; 
  } else {
    Serial.println("HATA: Sensör verisi okunamadı!");
  }
}

void sendTelegramMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Telegram'a gonderiliyor: ");
    Serial.println(message);
    if (bot.sendMessage(CHAT_ID, message, "")) {
      Serial.println("Mesaj basariyla gonderildi.");
    } else {
      Serial.println("HATA: Telegram mesajı gonderilemedi!");
    }
  } else {
    Serial.println("HATA: WiFi baglantisi yok, mesaj gonderilemedi.");
  }
}

void handleTelegramMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.print("Gelen Mesaj ("); Serial.print(from_name); Serial.print(", "); Serial.print(chat_id); Serial.print("): "); Serial.println(text);

    // Sadece belirli bir komuta yanıt ver
    if (text.equalsIgnoreCase("hey bot") || text.equalsIgnoreCase("/status")) { 
      // En son kaydedilen verileri al
      float currentTemperature = tempHistory[(dataIndex - 1 + 60) % 60];
      float currentHumidity = humHistory[(dataIndex - 1 + 60) % 60];

      String replyMessage = "Merhaba " + from_name + "!\n";
      replyMessage += "Anlik Sicaklik: " + String(currentTemperature, 1) + " C\n";
      replyMessage += "Anlik Nem: " + String(currentHumidity, 1) + " %";

      if (!bot.sendMessage(chat_id, replyMessage, "")) {
         Serial.println("HATA: Telegram yaniti gonderilemedi!");
      } else {
         Serial.println("Telegram yaniti gonderildi.");
      }
    } else {
        String replyMessage = "Anlasilmadi. 'hey bot' veya '/status' komutunu deneyin.";
        bot.sendMessage(chat_id, replyMessage, "");
    }
  }
}

void showTemperature(float temp) {
  display.setTextSize(2); 
  display.setCursor(0, 10);
  display.print("Sicaklik:");
  display.setTextSize(3); 
  display.setCursor(0, 35);
  display.print(temp, 1);
  display.print((char)247); // For (° )
  display.print("C");
}

void showHumidity(float hum) {
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("Nem:");
  display.setTextSize(3); 
  display.setCursor(0, 35);
  display.print(hum, 1);
  display.print(" %");
}

void drawGraph(float *data, const char* label, float minVal, float maxVal) {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(label);
  display.print(" (Son 60sn)");

  int graphHeight = SCREEN_HEIGHT - 12;
  int graphYOffset = 12; 


  for (int i = 1; i < 60; i++) {

    int i1 = (dataIndex + i - 1 + 60) % 60;
    int i2 = (dataIndex + i + 60) % 60;  
    int x1 = map(i - 1, 0, 59, 0, SCREEN_WIDTH - 1);
    int x2 = map(i, 0, 59, 0, SCREEN_WIDTH - 1);

    int y1 = map(data[i1], minVal, maxVal, SCREEN_HEIGHT - 1, graphYOffset);
    int y2 = map(data[i2], minVal, maxVal, SCREEN_HEIGHT - 1, graphYOffset);

    y1 = constrain(y1, graphYOffset, SCREEN_HEIGHT - 1);
    y2 = constrain(y2, graphYOffset, SCREEN_HEIGHT - 1);

    // if (data[i1] > minVal - 1 && data[i2] > minVal - 1) { 
       display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    // }
  }
}

void showName() {
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print("Suayb");
  display.setCursor(10, 45);
  display.print("Demir");
}