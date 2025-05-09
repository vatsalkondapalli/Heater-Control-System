#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Arduino_FreeRTOS.h>

const float BETA = 3950;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo s1;

int led = 8;
int perfect_stable = 90;
float currentTemp = 0.0;
String tempMessage = "Initializing...";

void TaskReadTemperature(void *pvParameters);
void TaskUpdateLCD(void *pvParameters);

void setup() {
  Serial.begin(9600);

  pinMode(led, OUTPUT);
  lcd.begin(16, 2);
  s1.attach(9);


  xTaskCreate(TaskReadTemperature, "ReadTemp", 128, NULL, 2, NULL);
  xTaskCreate(TaskUpdateLCD, "UpdateLCD", 128, NULL, 1, NULL);
}

void loop() {
}

void TaskReadTemperature(void *pvParameters) {
  (void) pvParameters;
  int cs = 0;

  while (1) {
    int analogValue = analogRead(A0);
    currentTemp = 1 / (log(1 / (1023.0 / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15;

    Serial.print("Temperature: ");
    Serial.print(currentTemp);
    Serial.println(" °C");

    if (currentTemp >= -24 && currentTemp <= 0){
      tempMessage = "IDLE";
      digitalWrite(led, LOW);
      cs = 0;
    } 
    if (currentTemp > 0 && currentTemp <= 40){
      tempMessage = "Heating";
      digitalWrite(led, LOW);
      for(cs = 0;cs<=60;cs++);
    } 
    if (currentTemp > 40 && currentTemp <= 55){
      tempMessage = "Stabilizing";
      digitalWrite(led, LOW);
      for(cs = 60;cs<=89;cs++);
    } 
    if (currentTemp > 55 && currentTemp <= 65){
      tempMessage = "Target reached";
      digitalWrite(led, LOW);
      cs = perfect_stable;
    } 
    if (currentTemp > 65 && currentTemp <= 80){
      tempMessage = "OverHeating";
      digitalWrite(led, HIGH);
      cs = 0;
    } 

    s1.write(cs);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


void TaskUpdateLCD(void *pvParameters) {
  (void) pvParameters;
  while (1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(tempMessage);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
