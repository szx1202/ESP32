/*
 * Utilizzabile solo via Blynk
 * in modalità automatica gestisce la pompa in base ai valori di Umidità del terreno. Disabilita la pompa quando l'umidità ragginge un livello predefinito
 * La selezione tra modalità automatica o manuale avviene via selettore.
 * Disponibile la modalita simulazione umidità tramite valore impostabile da seriale
 * AGGIORNA I VALORI DI LIVELLO ACQUA ED UMIDITà OGNI 2 SECONDI
 */


#include <WiFi.h>

// Blynk settings
#define BLYNK_TEMPLATE_ID "TMPL4ntYG9Ybb"
#define BLYNK_TEMPLATE_NAME "ESP32 A"
#define BLYNK_AUTH_TOKEN "NB7Wxvij1bbuTrrPjYfaFo8-yMkfq46U"

#include <BlynkSimpleEsp32.h>

const char* ssid = "LZ_24G";
const char* password = "*andromedA01.";

// Pin configurazione
const int pinPompa = 26;
const int pinWaterLev = 33;
// pinHumidity non viene più usato per lettura reale

// Stato e configurazione
bool pompaAttiva = false;
bool modalitaAutomatica = false;
unsigned long tempoAvvioPompa = 0;
const int durataPompa = 5000;
const int MinHumidity = 1000;

// Simulazione umidità
int humiditySimulata = 2000;  // Valore iniziale

//int humidityLev=2000; // valore iniziale con sensore

// Timer per Blynk
BlynkTimer timer;

void setup() {
  Serial.begin(115200);
  pinMode(pinPompa, OUTPUT);
  digitalWrite(pinPompa, LOW);
  
  setup_wifi();
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  
  modalitaAutomatica = false;       // Set manual mode at startup
  Blynk.virtualWrite(V0, 0);        // Ensure pump button is off
  Blynk.virtualWrite(V3, 0);        // Force mode selector to manual
  
  // Timer per invio dati a Blynk ogni 5s
  timer.setInterval(5000L, inviaDatiBlynk);
}

void loop() {

  
  // Lettura da Serial per simulare l'umidità
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    int valore = input.toInt();
    
    if (valore > 0 && valore <= 4095) {  // ESP32 ADC 12 bit
      humiditySimulata = valore;
      Serial.print("Valore simulato di umidità: ");
      Serial.println(humiditySimulata);
    }
  }

  Blynk.run();
  timer.run();

  if (modalitaAutomatica && (pompaAttiva && millis() - tempoAvvioPompa >= durataPompa)) {
    disattivaPompa();
  }

// ****************** attiva per simulazione seriale ******************
  // Se modalità automatica e umidità < soglia
  if (modalitaAutomatica && !pompaAttiva) {
    if (humiditySimulata < MinHumidity) {
      attivaPompa();
    }
  }
// *********************************************************************
    
//********************** da attivare con sensore ***********************
  // Se modalità automatica e umidità < soglia
  /*if (modalitaAutomatica && !pompaAttiva) {
    int HumidityLev = analogRead(pinHumidity);
    if (HumidityLev < MinHumidity) {
      attivaPompa();
    }
  }*/
//*********************************************************************

}

void attivaPompa() {
  digitalWrite(pinPompa, HIGH);
  pompaAttiva = true;
  tempoAvvioPompa = millis();
  Serial.println("Pompa attivata");
  Blynk.virtualWrite(V0, 1);  // Aggiorna lo stato del pulsante
}

void disattivaPompa() {
  digitalWrite(pinPompa, LOW);
  pompaAttiva = false;
  Serial.println("Pompa disattivata");
  Blynk.virtualWrite(V0, 0);  // Aggiorna lo stato del pulsante
}

// Invio valori a Blynk
void inviaDatiBlynk() {
  int WaterLev = analogRead(pinWaterLev);
  Blynk.virtualWrite(V1, WaterLev);

// ****************** attiva per simulazione seriale ******************  
  int HumidityLev = humiditySimulata;
  Blynk.virtualWrite(V2, HumidityLev);
// ****************** attiva con sensore ******************
/*  int HumidityLev = analogRead(pinHumidity);
  Blynk.virtualWrite(V2, HumidityLev);*/
}

// Pulsante ON/OFF pompa (solo in modalità manuale)
BLYNK_WRITE(V0) {
  if (!modalitaAutomatica) {
    int stato = param.asInt();
    if (stato == 1) {
      attivaPompa();
    } else {
      disattivaPompa();
    }
  }
}

// Menu selezione modalità (Manuale o Automatica)
BLYNK_WRITE(V3) {
  int selezione = param.asInt(); // 0 = Manuale, 1 = Automatica
  modalitaAutomatica = (selezione == 1);

  if (modalitaAutomatica) {
    Serial.println("Modalità: Automatica");
    disattivaPompa(); // assicura partenza da OFF
    Blynk.virtualWrite(V0, 0);  // reset pulsante
  } else {
    Serial.println("Modalità: Manuale");
  }
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
