#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#############################################
# Simulateur maquette DR400 par Flying Fox  #
# merci à https://forum.arduino.cc/u/kamill #
#############################################

class Flasher
{
  int ledPin;
    long OnTime;    
    long OffTime;  
    int ledState;
    unsigned long previousMs;
    unsigned long currentMs;
  
  public:
    Flasher(int pin, long on, long off)
    {
      ledPin = pin;
      OnTime = on;
      OffTime = off;
      ledState = HIGH;
      previousMs = 0;
    }
  
    void setup() {
      pinMode(ledPin, OUTPUT);
    }
  
    void loop()
    {
      currentMs = millis();
      if (currentMs - previousMs <= (OnTime + OffTime))
      {
        if (currentMs - previousMs >= OnTime)
          ledState = LOW; 
        else
          ledState = HIGH;  
        digitalWrite(ledPin, ledState);
      } else {
        previousMs = currentMs;
      }
    }
  
    void stop()
    {
      digitalWrite(ledPin, LOW);
    }
};

const int serial1 = 10;
const int serial2 = 11;

const int nav_v = 15;
const int nav_r = 12;
const int nav_b = 16;

const int indoor = 14;

const int phare = 9;
const int strobe = 8;
const int moteur = 7;

const int BUTTON_PHARE = 4;
const int BUTTON_NAV = 3;
const int BUTTON_STROBE = 5;
const int BUTTON_MOTEUR = 2;
const int BUTTON_MUSIC = 6;

const byte busyPin = 13;

int engine; // 0:init / 1:start / 2:stop

volatile byte state = LOW;


int bp1Mem = 0;         // variable pour l'état précédent du bouton poussoir
int ledOn = 0;  

int state_nav, state_phare, state_strobe, state_moteur = LOW;     
int read_nav, read_phare, read_strobe, read_moteur;   
int prev_nav, prev_phare, prev_strobe, prev_moteur = HIGH; 

SoftwareSerial mySoftwareSerial(serial1, serial2); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

void printDetail(uint8_t type, int value);

unsigned long time = 0;
unsigned long timer2 = 0;
unsigned long debounce = 200UL;
unsigned long currentMillis = 0;

Flasher fct_strobe(strobe,  100,  1200);
Flasher start_up(moteur,  30,  50);
Flasher start_up_fast(moteur,  10,  20);
Flasher start_up_ralenti(moteur,  30,  30);


Flasher start_up_0(moteur,  10,  60);
Flasher start_up_1(moteur,  20,  70);
Flasher start_up_5(moteur,  10,  80);

Flasher music(indoor,  200,  500);

void setup() {
  // sound
  Serial.begin(115200);
  pinMode(serial1, INPUT);
  pinMode(serial2, OUTPUT);
  mySoftwareSerial.begin(9600);
  Serial.println(F("Initialisation du module"));
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Prêt a démarrer !"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  else
  {
    Serial.println(F("DFPlayer Mini online."));
  }
  // ---
  myDFPlayer.volume(12);

  // lights
  pinMode(BUTTON_NAV, INPUT_PULLUP);
  pinMode(BUTTON_PHARE, INPUT_PULLUP);
  pinMode(BUTTON_STROBE, INPUT_PULLUP);
  pinMode(BUTTON_MOTEUR, INPUT_PULLUP);
  pinMode(BUTTON_MUSIC, INPUT_PULLUP);
  
  pinMode(indoor, OUTPUT);
  digitalWrite(indoor, LOW);
  
  pinMode(nav_r, OUTPUT);
  digitalWrite(nav_r, LOW);
  
  pinMode(nav_v, OUTPUT);
  digitalWrite(nav_v, LOW);
  
  pinMode(nav_b, OUTPUT);
  digitalWrite(nav_b, LOW);
  
  pinMode(phare, OUTPUT);
  digitalWrite(phare, LOW);
  
  pinMode(moteur, OUTPUT);
  digitalWrite(moteur, LOW);
  fct_strobe.setup();
  start_up.setup();
  start_up_fast.setup();
  start_up_ralenti.setup();
  start_up_0.setup();
  start_up_1.setup();
  start_up_5.setup();
  music.setup();

  int engine = 0;
}


void loop()
{
  
  currentMillis = millis();
    
  // on/off des nav
  read_nav = digitalRead(BUTTON_NAV);
  if (read_nav == HIGH && prev_nav == LOW && millis() - time > debounce)
  {
    (state_nav == HIGH)?state_nav = LOW:state_nav = HIGH;
    time = millis();
  }
  digitalWrite(nav_v, state_nav);
  digitalWrite(nav_r, state_nav);
  digitalWrite(nav_b, state_nav);
  prev_nav = read_nav;
  



  
  // on/off du phare
  read_phare = digitalRead(BUTTON_PHARE);
  if (read_phare == HIGH && prev_phare == LOW && millis() - time > debounce)
  {
    (state_phare == HIGH)?state_phare = LOW:state_phare = HIGH;
    time = millis();
  }
  digitalWrite(phare, state_phare);
  prev_phare = read_phare;


  
  
  // on/off du strobe
  read_strobe = digitalRead(BUTTON_STROBE);
  if (read_strobe == HIGH && prev_strobe == LOW && millis() - time > debounce)
  {
    (state_strobe == HIGH)?state_strobe = LOW:state_strobe = HIGH;
    time = millis();
  }
  (state_strobe == HIGH)?fct_strobe.loop():fct_strobe.stop();
  prev_strobe = read_strobe;



  // lancement de la music
  static bool lastButtonStateMusic = HIGH;
   bool buttonStateMusic = digitalRead(BUTTON_MUSIC);
   if (buttonStateMusic != lastButtonStateMusic){
      lastButtonStateMusic = buttonStateMusic;
      if (buttonStateMusic == LOW)
      {
        digitalWrite(moteur, LOW);
        engine = 0;
        byte fileToPlay = random(4, 8);
        myDFPlayer.play(fileToPlay);
        music.loop();
      }
   }



    
  static bool lastButtonState = HIGH;
   bool buttonState = digitalRead(BUTTON_MOTEUR);
   
    if (buttonState != lastButtonState) {
      lastButtonState = buttonState;
      if (buttonState == LOW) {
        if (engine != 1) {
          Serial.println("on demarre le moteur");
          myDFPlayer.play(3);
          timer2 = millis();
          engine = 1;
        } else {
          Serial.println("on arrete le moteur");
          myDFPlayer.play(2);
          timer2 = millis();
          engine = 2;
        }
      }
    }

  if(engine == 1){
    (digitalRead(busyPin) == LOW)?start_engine():digitalWrite(moteur, LOW); 
  }else if (engine == 2){
    (digitalRead(busyPin) == LOW)?stop_engine():digitalWrite(moteur, LOW);
  }
}


void start_engine(){
   music.stop();
   if (currentMillis - timer2 < 1523)
    digitalWrite(moteur, LOW);
   else if (currentMillis - timer2 < 2500)
      start_up_0.loop();
   else if (currentMillis - timer2 < 4100)
      start_up_1.loop();
   else if (currentMillis - timer2 < 4300)
      digitalWrite(moteur, LOW);
   else if (currentMillis - timer2 < 840050)
      start_up_ralenti.loop();
}


void stop_engine(){
   music.stop();
   if (currentMillis - timer2 < 2500)
      start_up_ralenti.loop();
   else if (currentMillis - timer2 < 2500)
      start_up_fast.loop();
   else if (currentMillis - timer2 < 4000)
      start_up_1.loop();
   else if (currentMillis - timer2 < 5309)
      start_up_0.loop();
   else if (currentMillis - timer2 < 5800)
      start_up_5.loop();
}
