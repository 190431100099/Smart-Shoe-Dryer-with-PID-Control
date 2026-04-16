#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <PID_v1.h>
#include <Wire.h>
#include <RTClib.h>
#include <SD.h>
#include <SPI.h>
#include <dimmable_light.h>
#include <PZEM004Tv30.h>

// Konfigurasi hardware 
#define DIMMER_PIN 9
#define RELAY_PIN 10       // Pin untuk relay ultraviolet
#define DHTPIN 7           // Pin sensor DHT22
#define DHTTYPE DHT22      // Tipe sensor DHT
#define chipSelect 53       // Pin chip select untuk SD Card

PZEM004Tv30 pzem(&Serial1); // Gunakan Serial1 pada Arduino Mega untuk komunikasi dengan PZEM

// Konfigurasi LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Konfigurasi RTC dan SD
RTC_DS3231 rtc;
File dataFile;

// PID
double Kp = 3.9;
double Ki = 1.95;
double Kd = 1,95;
double Setpoint, Input, Output;
float power;
float voltage;
float current;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// Sensor dan kontrol
DHT dht(DHTPIN, DHTTYPE);

// Button
const int buttonKasualPin = 3;
const int buttonKulitPin = 4;
const int buttonOlahragaPin = 5;
const int buttonStartStopPin = 6;

// pin dimmer
const int syncPin = 2;
const int thyristorPin = 9;

// konfigurasi dimmer
DimmableLight light(thyristorPin);

// Status program
int selectedShoe = 0;    // 0: None, 1: Kasual, 2: Kulit, 3: Olahraga
bool isRunning = false;
bool isRelayActive = false;  // Status relay ultraviolet
unsigned long startTime;
unsigned long duration = 0;
unsigned long countdownTime = 0;
unsigned long relayStartTime = 0;  // Waktu mulai relay ultraviolet
unsigned long lastLogTime = 0;     // Waktu log terakhir untuk SD Card

void setup() {
  Serial.begin(9600);

  // Inisialisasi sensor, SD Card, RTC, dan LCD
  dht.begin();
  lcd.begin(20, 4);
  lcd.backlight();

  if (!rtc.begin()) {
    Serial.println("RTC tidak ditemukan");
    lcd.print("RTC Error");
  }
  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card gagal");
    lcd.print("SD Error");
    while(1);
  }

  if (!SD.exists("DataBoin.txt")){
    dataFile = SD.open("DataBoin.txt",FILE_WRITE);
    if(dataFile){
      dataFile.println("Tanggal,Waktu,Sepatu,Status,Suhu,PWM");
    }
  }
  
  // Setup tombol dan relay
  pinMode(buttonKasualPin, INPUT_PULLUP);
  pinMode(buttonKulitPin, INPUT_PULLUP);
  pinMode(buttonOlahragaPin, INPUT_PULLUP);
  pinMode(buttonStartStopPin, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Pastikan relay mati saat awal

  // Pengaturan PID
  Setpoint = 50.0;  // Setpoint suhu
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 255);

  //pengaturan dimmer
  DimmableLight::setSyncPin(syncPin);
  DimmableLight::begin();

  // Tampilkan pesan awal di LCD
  lcd.setCursor(0,0);
  lcd.print("PENGERING SEPATU");
  lcd.setCursor(7,1);
  lcd.print("SKRIPSI");
  lcd.setCursor(5,2);
  lcd.print("BOIN PURBA");
  lcd.setCursor(5,3);
  lcd.print("200431100049");
  delay(3000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.println("Pilih Jenis Sepatu");
  lcd.setCursor(0,1);
  lcd.println("1.sepatu kasual");
  lcd.setCursor(0,2);
  lcd.println("2.sepatu kulit");
  lcd.setCursor(0,3);
  lcd.println("3.sepatu olahraga");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Pemilihan jenis sepatu
  if (digitalRead(buttonKasualPin) == LOW) {
    selectedShoe = 1;
    duration = 90 * 60; // 50 menit
    updateLCD("Kasual dipilih");
  }
  if (digitalRead(buttonKulitPin) == LOW) {
    selectedShoe = 2;
    duration = 45 * 60; // 45 menit
    updateLCD("Kulit dipilih");
  }
  if (digitalRead(buttonOlahragaPin) == LOW) {
    selectedShoe = 3;
    duration = 2 * 60; // 40 menit
    updateLCD("Olahraga dipilih");
  }

  // Start/Stop program
  static int lastButtonState = HIGH;
  int buttonState = digitalRead(buttonStartStopPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    isRunning = !isRunning;
    if (isRunning) {
      startTime = millis();
      countdownTime = duration;
      updateLCD("Program dimulai");
      logData("Program dimulai");
    } else {
      updateLCD("Program dihentikan");
      logData("Program dihentikan");
    }
  }
  lastButtonState = buttonState;

  // Jalankan countdown jika program aktif
  if (isRunning) {
    unsigned long elapsedTime = (millis() - startTime) / 1000;
    if (elapsedTime < duration) {
      countdownTime = duration - elapsedTime;
      displayCountdown(countdownTime);
      runPID();
      logDataPerSecond();  // Simpan data setiap detik
    } else {
      isRunning = false;
      logData("Waktu selesai");
      digitalWrite(RELAY_PIN, HIGH);
      isRelayActive = true;
      relayStartTime = millis();
      lcd.setCursor(0,0);
      lcd.print("UV aktif 10 menit");
    }
  }

  // Kontrol relay ultraviolet
  if (isRelayActive) {
    unsigned long relayElapsedTime = (millis() - relayStartTime) / 1000;
    if (relayElapsedTime < 600) { // 10 menit = 600 detik
      displayRelayCountdown(600 - relayElapsedTime);
    } else {
        lcd.clear();
        digitalWrite(RELAY_PIN, LOW);
        lcd.setCursor(0,0);
        lcd.print("UV selesai");
    }
  }
}

// Fungsi PID untuk mengontrol dimmer
void runPID() {
  Input = dht.readTemperature();
  if (isnan(Input)) {
    Serial.println("Gagal membaca DHT");
    return;
  }
  myPID.Compute();
  light.setBrightness(Output);
}

// Tampilkan countdown pada LCD

void displayCountdown(unsigned long timeLeft) {
  power = pzem.power();
  voltage = pzem.voltage();
  current = pzem.current();
  int minutes = timeLeft / 60;
  int seconds = timeLeft % 60;
  lcd.setCursor(0, 1);
  lcd.print("Waktu: ");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
  lcd.setCursor(0, 2);
  lcd.print("Suhu: ");
  lcd.print(Input);
  lcd.setCursor(0, 3);
  lcd.print("PWM: ");
  lcd.print(Output);
  lcd.setCursor(13, 2);
  lcd.print("WT:");
  lcd.print(power);
  lcd.setCursor(13, 3);
  lcd.print("V:");
  lcd.print(voltage);
}

// Aktifkan relay ultraviolet

// Nonaktifkan relay ultraviolet
void deactivateRelay() {
  lcd.clear();
  digitalWrite(RELAY_PIN, LOW);
  isRelayActive = false;
  lcd.setCursor(0,0);
  lcd.print("UV selesai");
}

// Tampilkan countdown relay ultraviolet pada LCD
void displayRelayCountdown(unsigned long timeLeft){
  int minutes = timeLeft / 60;
  int seconds = timeLeft % 60;
  lcd.setCursor(0, 1);
  lcd.print("UV aktif: ");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
}

// Fungsi untuk update pesan pada LCD
void updateLCD(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

// Simpan log data ke SD Card
void logData(String status) {
  dataFile = SD.open("DataBoin.txt", FILE_WRITE);
  if (dataFile) {
    DateTime now = rtc.now();
    dataFile.print(now.year(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.day(), DEC);
    dataFile.print(",");
    dataFile.print(now.hour(), DEC);
    dataFile.print(':');
    dataFile.print(now.minute(), DEC);
    dataFile.print(':');
    dataFile.print(now.second(), DEC);
    dataFile.print(",");
    switch (selectedShoe) {
      case 1: dataFile.print("Kasual"); break;
      case 2: dataFile.print("Kulit"); break;
      case 3: dataFile.print("Olahraga"); break;
      default: dataFile.print("Tidak Dipilih"); break;
    }
    dataFile.print(",");
    dataFile.print(status);
    dataFile.print(",");
    dataFile.print(Input);
    dataFile.print(",");
    dataFile.println(Output);
    dataFile.close();
  } else {
    Serial.println("Gagal membuka file SD");
  }
}

// Fungsi untuk menyimpan log data setiap detik selama program berjalan
void logDataPerSecond() {
  if (millis() - lastLogTime >= 1000) {  // Setiap 1 detik
    dataFile = SD.open("DataBoin.txt", FILE_WRITE);
    if (dataFile) {
      DateTime now = rtc.now();
      dataFile.print(now.year(), DEC);
      dataFile.print('/');
      dataFile.print(now.month(), DEC);
      dataFile.print('/');
      dataFile.print(now.day(), DEC);
      dataFile.print(",");
      dataFile.print(now.hour(), DEC);
      dataFile.print(':');
      dataFile.print(now.minute(), DEC);
      dataFile.print(':');
      dataFile.print(now.second(), DEC);
      dataFile.print(",");
      switch (selectedShoe) {
        case 1: dataFile.print("Kasual"); break;
        case 2: dataFile.print("Kulit"); break;
        case 3: dataFile.print("Olahraga"); break;
        default: dataFile.print("Tidak Dipilih"); break;
      }
      dataFile.print(",");
      dataFile.print(Input);
      dataFile.print(",");
      dataFile.println(Output);
      dataFile.close();
    } else {
      Serial.println("Gagal membuka file SD");
    }
    lastLogTime = millis();  // Update waktu log terakhir
  }
}
