#include <user_interface.h>

const int MIO_INDIRIZZO = 1;
const int MIA_SCALA = 1;
long int durata_impulso = 1000;  // durata impulso in uscita quando si riceve una chiamata (ms)
int BUS_RX = 12;                 // D6
int BUS_TX = 5;                  // D1
int PIN_CHIAMATA_RICEVUTA = 4;   // D2
int PIN_APRI_PORTONE_1 = 13;     // D7
int PIN_APRI_PORTONE_2 = 15;     // D8

const int FREQUENZA_TX = 25000;      // frequenza della portante di trasmissione

volatile unsigned long t_on = 0;
volatile unsigned long t_off = 0;
volatile bool impulso_salita = false;
volatile bool impulso_discesa = false;
volatile bool in_ricezione = false;
bool comando_apri_portone_1, comando_apri_portone_2 = false;
bool impulso_positivo_ok = false;
int indirizzo, comando, bits = 0;
char message[18];
unsigned long timer_chiamata, ultimo_impulso;

RF_PRE_INIT() {
  system_phy_set_powerup_option(2);  // shut down the RFCAL at boot
}

void setup() {
  wifi_set_opmode_current(NULL_MODE);  // set Wi-Fi working mode to unconfigured, don't save to flash
  wifi_fpm_set_sleep_type(MODEM_SLEEP_T);  // set the sleep type to modem sleep
  wifi_fpm_open();  // enable Forced Modem Sleep
  wifi_fpm_do_sleep(0xFFFFFFF);  // force the modem to enter sleep mode
  delay(10);  // Spegnimento modulo radio

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUS_RX, INPUT);
  pinMode(BUS_TX, OUTPUT);
  pinMode(PIN_CHIAMATA_RICEVUTA, OUTPUT);
  pinMode(PIN_APRI_PORTONE_1, INPUT);
  pinMode(PIN_APRI_PORTONE_2, INPUT);
  digitalWrite(BUS_TX, LOW);
  digitalWrite(PIN_CHIAMATA_RICEVUTA, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(BUS_RX), fronte_rilevato, CHANGE);
  analogWrite(BUS_TX, 0);
  analogWriteFreq(FREQUENZA_TX);
}

void loop() {
  if (in_ricezione) {
    analizza_impulso();
  }

  if (digitalRead(PIN_APRI_PORTONE_1) == HIGH) {
    if (comando_apri_portone_1 == false) {
      Serial.println("Invio comando apertura portone 1");
      comelit_encode(16, MIO_INDIRIZZO);
      comando_apri_portone_1 = true;
    }
  }
  else comando_apri_portone_1 = false;

  if (digitalRead(PIN_APRI_PORTONE_2) == HIGH) {
    if (comando_apri_portone_2 == false) {
      Serial.println("Invio comando apertura portone 2");
      comelit_encode(29, MIA_SCALA);
      comando_apri_portone_2 = true;
    }
  }
  else comando_apri_portone_2 = false;


  if (millis() - ultimo_impulso > 10) reset_ricezione();


  if ((digitalRead(PIN_CHIAMATA_RICEVUTA) == HIGH) && (millis() - timer_chiamata) > durata_impulso)
  {
    digitalWrite(PIN_CHIAMATA_RICEVUTA, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
  }

}

IRAM_ATTR void fronte_rilevato() {
  if (digitalRead(BUS_RX) == HIGH) {
    if (in_ricezione) {
      impulso_salita = true;    
    }
    t_on = micros();
    in_ricezione = true;
  }

  else {
    impulso_discesa = true;
    t_off = micros();
  }
}

void analizza_impulso() {
  if (impulso_discesa)
  {
    impulso_discesa = false;
    ultimo_impulso = millis();
    long int t1 = t_off - t_on;
    if (t1 > 3500 && t1 < 5000) impulso_positivo_ok = true;
    else impulso_positivo_ok = false;
  }

  if (impulso_salita)
  {
    impulso_salita = false;
    long int t2 = t_on - t_off;
    if (impulso_positivo_ok)
    {
      if (t2 < 2500 && t2 > 1000)
      {
        message[bits] = 0;
        bits += 1;
      }
      else if (t2 < 6200 && t2 > 3500)
      {
        message[bits] = 1;
        bits += 1;
      }
      else reset_ricezione();

      if (bits == 18) decodifica();
    }
  }
}


void decodifica() {
  if (checksum() != true) reset_ricezione();
  else
  {
    bintodec();

    Serial.print("RX <- comando: ");
    Serial.print(comando);
    Serial.print(" indirizzo: ");
    Serial.println(indirizzo);

    if (indirizzo == MIO_INDIRIZZO && comando == 50)
    {
      Serial.println("Chiamata ricevuta");
      chiamata_ricevuta();
    }
    reset_ricezione();
  }
}

bool checksum() {
  int sum = 0;
  int checksum = 0;
  for (int i = 0; i < 14; i++)
  {
    if (message[i] == 1)
    {
      sum++;
    }
  }
  checksum = (message[17] * 8) + (message[16] * 4) + (message[15] * 2) + message[14];
  return (checksum == sum);
}

void bintodec() {
  int msgAddr[6];
  int msgCode[8];
  for (int j = 0; j < 14; j++)
  {
    if (j < 6)
    {
      msgAddr[j] = message[j];
    }
    else
    {
      msgCode[j - 6] = message[j];
    }
  }
  comando = (msgAddr[5] * 32) + (msgAddr[4] * 16) + (msgAddr[3] * 8) + (msgAddr[2] * 4) + (msgAddr[1] * 2) + msgAddr[0];
  indirizzo = (msgCode[7] * 128) + (msgCode[6] * 64) + (msgCode[5] * 32) + (msgCode[4] * 16) + (msgCode[3] * 8) + (msgCode[2] * 4) + (msgCode[1] * 2) + msgCode[0];
}

void chiamata_ricevuta() {
  digitalWrite(PIN_CHIAMATA_RICEVUTA, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  timer_chiamata = millis();
}

void reset_ricezione() {
  in_ricezione = false;
  bits = 0;
}


void comelit_encode(int codice, int indirizzo) {
    detachInterrupt(digitalPinToInterrupt(BUS_RX));
    digitalWrite(LED_BUILTIN, LOW);

    int contauno = 0;
    genera_portante(3);
    genera_silenzio(16);

    for (int i=0; i<6; i++){
        genera_portante(3);
        if (bitRead(codice, i)) {
            genera_silenzio(6);
            contauno++;
        } else {
            genera_silenzio(3);
        }
    }

    for (int i=0; i<8; i++) {
        genera_portante(3);
        if (bitRead(indirizzo, i)) {
            genera_silenzio(6);
            contauno++;
        } else {
            genera_silenzio(3);
        }
    }

    for (int i=0; i<4; i++) {
        genera_portante(3);
        if (bitRead(contauno, i)) {
            genera_silenzio(6);
        } else {
            genera_silenzio(3);
        }
    }

    genera_portante(3);
    genera_silenzio(20);

    attachInterrupt(digitalPinToInterrupt(BUS_RX), fronte_rilevato, CHANGE);
    digitalWrite(LED_BUILTIN, HIGH);
}

void genera_portante(int durata) {
  unsigned long fine_modulazione = millis() + durata;
  analogWrite(BUS_TX, 127);
  delay(durata);
  analogWrite(BUS_TX, 0);
}

void genera_silenzio(int durata) {
  unsigned long fine_silenzio = millis() + durata;
  analogWrite(BUS_TX, 0);
  delay(durata);
}
