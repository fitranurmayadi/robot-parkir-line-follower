#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define A     digitalRead(7) //pin button A
#define B     digitalRead(8) //pin button B

#define inki     digitalRead(2)
#define inka     digitalRead(3)

#define BUZZ    13 //buzzer

#define Lpwm  5 //pin pwm motor
#define Rpwm  10 //pin pwm motor
#define Ldir  6 //pin arah/direction
#define Rdir  9 //pin arah/direction

char data[33];

char slot[3];

bool menu0;
bool menu_utama;
bool menu_set_pid, menu_set_kec, menu_set_adc, menu_set_eeprom, menu_set_sensor;
bool menu_set_pid_yes, menu_set_kec_yes, menu_set_adc_yes, menu_set_eeprom_yes, menu_set_sensor_yes;
bool set_kp, set_ki, set_kd, selesai_set_pid;
bool set_a0, set_a1, set_a2, set_a3, set_a4, set_a5;

int P, I, D, kpF, kiF, kdF, posisi, error, last_error, mvF, Min, m1, m2, gas, hasil;

int Z = 0;
unsigned int dd0, dd1; //untuk arah motor
unsigned int dat, dut, sayap, sensor, x, yoo, yon;

unsigned int counter = 0;
int ski = 0;
int ska = 0;
int t;
int T = 0;

float tF, pid;
unsigned int a[6];
unsigned int b[6];
unsigned int c[6];
unsigned int s[10];



unsigned char d[6] = {
  EEPROM.read(0),
  EEPROM.read(1),
  EEPROM.read(2),
  EEPROM.read(3),
  EEPROM.read(4),
  EEPROM.read(5)
};


unsigned char gki = EEPROM.read(6); //generator pwm
unsigned char gka = EEPROM.read(7); //

unsigned char kp = EEPROM.read(8); //konstanta P
unsigned char ki = EEPROM.read(9); //konstanta I
unsigned char kd = EEPROM.read(10); //konstanta D
unsigned char ts = EEPROM.read(11); //konstanta waktu sampling analog
unsigned char mv = EEPROM.read(12); //konstanta kecepatan maksimal


void menu() {
  menu0 = 1;
  while (menu0) {
    //menu utama
    menu_utama = 1;
    while (menu_utama) {
      delay(100); lcd.clear();
      lcd.setCursor(0, 0); lcd.print(">>>>>>>><<<<<<<<");
      lcd.setCursor(0, 1); lcd.print("SETUP  ||  START");
      if (A == 0) {
        menu_set_pid = 1 ;
        menu_utama = 0;
      }
      if (B == 0) {
        menu_utama = 0;
        menu0 = 0;
      }
    }

    //MENU
    //ke menu seting pid?
    while (menu_set_pid) {
      delay(100); lcd.clear();
      lcd.setCursor(0, 0); lcd.print("    SET PID     ");
      lcd.setCursor(0, 1); lcd.print("OK    ||    NEXT");
      if (A == 0) {
        menu_set_pid_yes = 1;
        set_kp = 1;
        menu_set_pid = 0;
      }
      if (B == 0) {
        menu_set_kec = 1;
        menu_set_pid = 0;
      }
    }
    //ke menu seting kecepatan?
    while (menu_set_kec) {
      delay(100); lcd.clear();
      lcd.setCursor(0, 0); lcd.print(" SET KECEPATAN  ");
      lcd.setCursor(0, 1); lcd.print("OK    ||    NEXT");
      if (A == 0) {
        menu_set_kec_yes = 1;
        menu_set_kec = 0;
      }
      if (B == 0) {
        menu_set_adc = 1;
        menu_set_kec = 0;
      }
    }
    //ke menu seting adc?
    while (menu_set_adc) {
      delay(100); lcd.clear();
      lcd.setCursor(0, 0); lcd.print("    SET ADC     ");
      lcd.setCursor(0, 1); lcd.print("OK    ||    NEXT");
      if (A == 0) {
        menu_set_adc_yes = 1;
        menu_set_adc = 0;
      }
      if (B == 0) {
        menu_set_eeprom = 1;
        menu_set_adc = 0;
      }
    }
    //ke menu seting eeprom ?
    while (menu_set_eeprom) {
      delay(100); lcd.clear();
      lcd.setCursor(0, 0); lcd.print("  RESET EEPROM  ");
      lcd.setCursor(0, 1); lcd.print("OK    ||    NEXT");
      if (A == 0) {
        menu_set_eeprom_yes = 1;
        menu_set_eeprom = 0;
      }
      if (B == 0) {
        menu_set_sensor = 1;
        menu_set_eeprom = 0;
      }
    }
    //ke menu cek sensor?
    while (menu_set_sensor) {
      delay(100); lcd.clear();
      lcd.setCursor(0, 0); lcd.print("   CEK SENSOR   ");
      lcd.setCursor(0, 1); lcd.print("OK    ||    BACK");
      if (A == 0) {
        menu_set_sensor_yes = 1;
        menu_set_sensor = 0;
      }
      if (B == 0) {
        menu_utama = 1;
        menu_set_sensor = 0;
      }
    }

    //***********************************************************************************
    // SUB MENU 1
    while (menu_set_pid_yes) {
      while (set_kp) {
        delay(100); lcd.clear();

        lcd.setCursor(0, 0); sprintf(data, "set kp : %d", kp); lcd.print(data);
        lcd.setCursor(0, 1); lcd.print("OK    ||  TAMBAH");
        if (A == 0) {
          set_ki = 1;
          set_kp = 0;
        }
        if (B == 0) {
          if (kp < 25) {
            kp += 1;
          }
          else {
            kp = 0;
          }
        }
      }

      while (set_ki) {
        delay(100); lcd.clear();
        lcd.setCursor(0, 0); sprintf(data, "set ki : %d", ki); lcd.print(data);
        lcd.setCursor(0, 1); lcd.print("OK    ||  TAMBAH");
        if (A == 0) {
          set_kd = 1;
          set_ki = 0;
        }
        if (B == 0) {
          if (ki < 25) {
            ki += 1;
          }
          else {
            ki = 0;
          }
        }
      }

      while (set_kd) {
        delay(100); lcd.clear();
        lcd.setCursor(0, 0); sprintf(data, "set kd : %d", kd); lcd.print(data);
        lcd.setCursor(0, 1); lcd.print("OK    ||  TAMBAH");
        if (A == 0) {
          selesai_set_pid = 1;
          set_kd = 0;
        }
        if (B == 0) {
          if (kd < 25) {
            kd += 1;
          }
          else {
            kd = 0;
          }
        }
      }

      while (selesai_set_pid) {
        delay(100); lcd.clear();
        lcd.setCursor(0, 0); lcd.print("SELESAI      ???");
        lcd.setCursor(0, 1); lcd.print("YA    ||  ULANGI");
        if (A == 0) {

          EEPROM.update(8, kp);
          delay(21);
          EEPROM.update(9, ki);
          delay(21);
          EEPROM.update(10, kd);
          delay(21);

          menu_set_pid = 1;
          menu_set_pid_yes = 0;
          selesai_set_pid = 0;
        }
        if (B == 0) {
          set_ki = 1;
          selesai_set_pid = 0;
        }
      }
    }

    //********************************************************************

    //SUB MENU 2
    while (menu_set_kec_yes) {
      delay(100); lcd.clear();
      lcd.setCursor(0, 0); sprintf(data, "set kec : %d", mv); lcd.print(data);
      lcd.setCursor(0, 1); lcd.print("OK    ||    TAMBAH");
      if (A == 0) {
        EEPROM.update(12, mv);
        delay(21);
        menu_set_kec = 1;
        menu_set_kec_yes = 0;
      }
      if (B == 0) {
        if (kd < 255) {
          mv += 10;
        }
        else {
          mv = 0;
        }
      }
    }
    //********************************************************************
    //SUB MENU 3
    while (menu_set_adc_yes) {

      while (set_a0) {
        delay(100); lcd.clear();
        lcd.setCursor(0, 0); lcd.print("SET SENSOR B0   ");
        lcd.setCursor(0, 1); lcd.print("OK    ||     SET");
        if (A == 0) {
          set_a1 = 1;
          set_a0 = 0;
        }
        if (B == 0) {
          d[0] += 10;
        }
      }

      while (set_a1) {
        delay(100); lcd.clear();
        lcd.setCursor(0, 0); lcd.print("SET SENSOR B1   ");
        lcd.setCursor(0, 1); lcd.print("OK    ||     SET");
        if (A == 0) {
          set_a2 = 1;
          set_a1 = 0;
        }
        if (B == 0) {
          d[1] += 10;
        }
      }

      while (set_a2) {
        delay(100); lcd.clear();
        lcd.setCursor(0, 0); lcd.print("SET SENSOR B2   ");
        lcd.setCursor(0, 1); lcd.print("OK    ||     SET");
        if (A == 0) {
          set_a3 = 1;
          set_a2 = 0;
        }
        if (B == 0) {
          d[2] += 10;
        }
      }

      while (set_a3) {
        delay(100); lcd.clear();
        lcd.setCursor(0, 0); lcd.print("SET SENSOR B3   ");
        lcd.setCursor(0, 1); lcd.print("OK    ||     SET");
        if (A == 0) {
          set_a4 = 1;
          set_a3 = 0;
        }
        if (B == 0) {
          d[3] += 10;
        }
      }

      while (set_a4) {
        delay(100); lcd.clear();
        lcd.setCursor(0, 0); lcd.print("SET SENSOR B4   ");
        lcd.setCursor(0, 1); lcd.print("OK    ||     SET");
        if (A == 0) {
          set_a5 = 1;
          set_a4 = 0;
        }
        if (B == 0) {
          d[4] += 10;
        }
      }

      while (set_a5) {
        delay(100); lcd.clear();
        lcd.setCursor(0, 0); lcd.print("SET SENSOR B5   ");
        lcd.setCursor(0, 1); lcd.print("SELESAI ||   SET");
        if (A == 0) {
          menu_set_adc = 1;
          set_a1 = 0;
        }
        if (B == 0) {
          d[5] += 10;
        }
      }
    }
    //**********************************************************
    //SUB MENU 4
    while (menu_set_eeprom_yes) {
      delay(100); lcd.clear();
      lcd.setCursor(0, 0); lcd.print("RESET         ??");
      lcd.setCursor(0, 1); lcd.print("SELESAI || RESET");
      if (A == 0) {
        menu_set_eeprom = 1;
        menu_set_eeprom_yes = 0;
      }
      if (B == 0) {


      }
    }
    //***********************************************************
    //SUB MENU 5
    while (menu_set_sensor_yes) {
      delay(100); lcd.clear();
      scan();

      lcd.setCursor(0, 1); lcd.print("OK    ||    BACK");
      if (A == 0) {
        menu_set_sensor = 1;
        menu_set_sensor_yes = 0;
      }
      if (B == 0) {
        menu_utama = 1;
        menu_set_sensor_yes = 0;
      }
    }
  }
}



void scan() {

  a[0] = analogRead(A0);
  a[1] = analogRead(A1);
  a[2] = analogRead(A2);
  a[3] = analogRead(A3);
  a[4] = analogRead(A6);
  a[5] = analogRead(A7);

  c[0] = 55;
  c[1] = 55;
  c[2] = 55;
  c[3] = 55;
  c[4] = 55;
  c[5] = 55;
  
  lcd.clear();
  lcd.setCursor(4, 0);
  if (inki == HIGH) { s[9] = 0; lcd.print("0"); } else { s[9] = 1; lcd.print("1"); }
  if (a[5] > c[5])  { s[6] = 1; lcd.print("1"); } else if (a[5] < c[5]) { s[6] = 0; lcd.print("0"); }
  if (a[4] > c[4])  { s[5] = 1; lcd.print("1"); } else if (a[4] < c[4]) { s[5] = 0; lcd.print("0"); }
  if (a[3] > c[3])  { s[4] = 1; lcd.print("1"); } else if (a[3] < c[3]) { s[4] = 0; lcd.print("0"); }
  if (a[2] > c[2])  { s[3] = 1; lcd.print("1"); } else if (a[2] < c[2]) { s[3] = 0; lcd.print("0"); }
  if (a[1] > c[1])  { s[2] = 1; lcd.print("1"); } else if (a[1] < c[1]) { s[2] = 0; lcd.print("0"); }
  if (a[0] > c[0])  { s[1] = 1; lcd.print("1"); } else if (a[0] < c[0]) { s[1] = 0; lcd.print("0"); }
  if (inka == HIGH) { s[8] = 0; lcd.print("0"); } else { s[8] = 1; lcd.print("1"); }

  s[0] = s[7] = 0;
  dat = (s[7] * 128) + (s[6] * 64) + (s[5] * 32) + (s[4] * 16) + (s[3] * 8) + (s[2] * 4) + (s[1] * 2)+ (s[0] * 1);
  sensor = dat;
  dut = (s[9] * 2) + (s[8] * 1);
  sayap = dut;  

  yoo = dat;
  yon = dat;
  yon &= 0b01000010;
  yoo &= 0b01011010;

  if(yoo==0b00011010){T=1;}
  if(yoo==0b01011010){T=1;}
  if(yoo==0b01011000){T=1;}
}


void PID() {
  digitalWrite(13, 0);
  if (sensor == 0b00011000) { posisi = 0;  Z = 1; }
  else if (sensor == 0b00001000) {    posisi = -1; }
  else if (sensor == 0b00001100) {    posisi = -2; }
  else if (sensor == 0b00000100) {    posisi = -3; }
  else if (sensor == 0b00000110) {    posisi = -4; }
  else if (sensor == 0b00000010) {    posisi = -5; }
  else if (sensor == 0b00010000) {    posisi = 1;  }
  else if (sensor == 0b00110000) {    posisi = 2;  }
  else if (sensor == 0b00100000) {    posisi = 3;  }
  else if (sensor == 0b01100000) {    posisi = 4;  }
  else if (sensor == 0b01000000) {    posisi = 5;  }


    if (sensor == 0b00000000 && sayap == 0b10) { posisi = 12;    }
    else if (sensor == 0b00000000 && sayap == 0b01) { posisi = -12;   }
    else if (sensor == 0b00000000 && sayap == 0b00) {
      if (posisi > 0 && posisi < 12) { posisi = 12; }
      else if (posisi < 0 && posisi > -12) { posisi = -12; }
      if (posisi > 12) { posisi = 14; }
      else if (posisi < -12) { posisi = -14;}
    }

  error = (kpF * posisi);
  P = error;
  I = kiF * (error + last_error);
  D = kdF * (error - last_error);
  last_error = error;
  pid = (float)P + (float)I + (float)D;
  hasil = (int)pid;

  //*****motorkiri
  m1 = mvF - hasil;
  if (m1 > mvF) { m1 = mvF;}
  if (m1 < Min) { m1 = Min;}
  if (m1 > 0) { dd0 = 0;}
  if (m1 < 0) { dd0 = 1; gas = 0 - m1; m1 = gas; }
  if (error == 0) { m1 = mvF; }
  //*****motorkanan
  m2 = mvF + hasil;
  if (m2 > mvF) { m2 = mvF; }
  if (m2 < Min) { m2 = Min;}
  if (m2 > 0) { dd1 = 0;  }
  if (m2 < 0) { dd1 = 1; gas = 0 - m2; m2 = gas; }
  if (error == 0) {  m2 = mvF; }


  lcd.setCursor(0, 1); sprintf(data, "%d %d %d %d  %d", dd0, dd1, m1, m2, counter); lcd.print(data);
  motor(dd0, dd1, m1, m2);
}

void load() {
  delay(50);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("MENGUNDUH.......");
  kpF = kp;
  lcd.setCursor(11, 0); lcd.print(" 0%"); delay(12);
  kiF = ki;
  lcd.setCursor(11, 0); lcd.print(" 9%"); delay(12);
  kdF = (float)kd;
  lcd.setCursor(11, 0); lcd.print(" 17%"); delay(12);
  tF = (float)ts * (float)4;
  lcd.setCursor(11, 0); lcd.print(" 21%"); delay(12);
  mvF = mv;
  lcd.setCursor(11, 0); lcd.print(" 32%"); delay(12);
  Min = 0 - mv;
  lcd.setCursor(11, 0); lcd.print(" 45%"); delay(12);
  lcd.setCursor(11, 0); lcd.print(" 54%"); delay(12);
  lcd.setCursor(11, 0); lcd.print(" 64%"); delay(12);
  lcd.setCursor(11, 0); lcd.print(" 72%"); delay(12);
  lcd.setCursor(11, 0); lcd.print(" 83%"); delay(12);
  lcd.setCursor(11, 0); lcd.print(" 91%"); delay(12);
  lcd.setCursor(11, 0); lcd.print(" 98%"); delay(12);
  lcd.setCursor(11, 0); lcd.print("100%"); delay(12);
  lcd.clear();
}
void interupsi() {
  detachInterrupt(digitalPinToInterrupt(2));
  detachInterrupt(digitalPinToInterrupt(3));
  if (T == 1) {
    counter++;
  }
  T = 0;
  attachInterrupt(digitalPinToInterrupt(2), interupsi, RISING);
  attachInterrupt(digitalPinToInterrupt(3), interupsi, RISING);
}

void motor(unsigned char d0, unsigned char d1, unsigned char L, unsigned char R) {
  digitalWrite(Ldir, d0);
  digitalWrite(Rdir, d1);
  analogWrite(Lpwm, L);
  analogWrite(Rpwm, R);
}

void buzzing(int g ,int delaya, int delayb){
  for(int q= 0; q <=g; q++){
     digitalWrite(13, 1);
     delay(delaya);
     digitalWrite(13, 0);
     delay(delayb);
     }
}

void goesToGate(){
  attachInterrupt(digitalPinToInterrupt(2), interupsi, RISING);
  attachInterrupt(digitalPinToInterrupt(3), interupsi, RISING);
  counter = 0;
  Z = 0;
  bool menuIni = true;
  while(menuIni){  
    scan();  
    if(counter==1){
      counter++;
      motor(0,0,0,0);
      menuIni = false;
      for(int q = 0; q<=3; q++){
        digitalWrite(13,1);
        delay(100);
        digitalWrite(13,0);
        delay(500);
      } 
    }
    PID();
  }
  waitForSlot();
}
void waitForSlot(){
  bool menuIni = true;
  motor(0,0,0,0);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menunggu Slot");
  while(menuIni){
    if(Serial.available() > 0){
      int x = 0;
      while(Serial.available() > 0){
          char input = Serial.read();
          slot[x] = input;
          x++;
          delay(100);
      }
      Serial.flush();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("SLOT >> ");
      lcd.print(slot[0]);
      lcd.print(slot[1]);
      delay(3000);
      menuIni = false;
    }
  } 
  goesToSlot();
}

void goesToSlot(){
  attachInterrupt(digitalPinToInterrupt(2), interupsi, RISING);
  attachInterrupt(digitalPinToInterrupt(3), interupsi, RISING);
  counter = 0;
  Z = 0;
  bool parkStop = false;
  bool menuIni = true;
  String isiSlot = String(slot[1]);
  int slotIs = isiSlot.toInt();
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" >>>>GOOOO");
  delay(2000);
  
  while(menuIni){
    scan();    
    if(counter == slotIs){
      detachInterrupt(digitalPinToInterrupt(2));
      detachInterrupt(digitalPinToInterrupt(3));
      if(slot[0] == 'A'){
        while(yon != 0b00000010){
          scan();
          lcd.setCursor(0,0); sprintf(data,"%d",counter); lcd.print(data);
          digitalWrite(13, HIGH);
          motor(0,1,128,128);  
        }
      }
      else if(slot[0] == 'B'){
        while(yon != 0b01000000){
          scan();
          lcd.setCursor(0,0); sprintf(data,"%d",counter); lcd.print(data);
          digitalWrite(13, HIGH);
          motor(1,0,128,128);  
      }
    }
    digitalWrite(13, LOW);
    counter++;
    parkStop = true;
    }
    if(sensor == 0b01111110){        
      if(parkStop == true){
        menuIni = false;
        motor(0,0,0,0);
        buzzing(4,100,100);
      }
    }
    PID();
  }
  waitForCall();
}


void waitForCall(){
  bool menuIni = true;
  char dataOut[3];
  motor(0,0,0,0);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Menunggu Keluar");
  while(menuIni){
    if(Serial.available() > 0){
      int x = 0;
      while(Serial.available() > 0){
          char input = Serial.read();
          dataOut[x] = input;
          x++;
          delay(100);
      }
      Serial.flush();
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("OUT >> ");
      lcd.print(dataOut[0]);
      lcd.print(dataOut[1]);
      delay(1000);
    }
    
    if(dataOut[0] == 'G' && dataOut[1] == 'B'){
      menuIni = false;
    }
  } 
  uturn();
}

void uturn(){
  int menuIni = true;
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" >>>>GOOOO");
  delay(1000);
  while(menuIni){
    scan();
    if(sensor == 0b00011000 || sensor == 0b00010000|| sensor == 0b00001000){
      menuIni = false;
    }
    motor(1,1, 130, 130);
  }
  if(slot[0] == 'A'){
    while(yon != 0b01000000){
      scan();
      lcd.setCursor(0,0); sprintf(data,"%d",counter); lcd.print(data);
      digitalWrite(13, HIGH);
      motor(1,0,128,128);  
      }
   }
   else if(slot[0] == 'B'){
      while(yon != 0b00000010){
        scan();
        lcd.setCursor(0,0); sprintf(data,"%d",counter); lcd.print(data);
        digitalWrite(13, HIGH);
        motor(0,1,128,128);  
      }
   }
  
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" >>>>GOOOO");
  motor(0,0,0,0);
  delay(100);
  digitalWrite(13, 0);
  goesToQuit();
}

void goesToQuit(){
  attachInterrupt(digitalPinToInterrupt(2), interupsi, RISING);
  attachInterrupt(digitalPinToInterrupt(3), interupsi, RISING);
  counter = 0;
  Z = 0;
  
  bool menuIni = true;
  String isiSlot = String(slot[1]);
  int slotIs = isiSlot.toInt();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("GO HOME");
  motor(0,0,0,0);
  delay(100);
  while(menuIni){
    scan();    
    if(counter == 1){
      detachInterrupt(digitalPinToInterrupt(2));
      detachInterrupt(digitalPinToInterrupt(3));
      if(slot[0] == 'A'){
        while(yon != 0b00000010){
          scan();
          lcd.setCursor(0,0); sprintf(data,"%d",counter); lcd.print(data);
          digitalWrite(13, HIGH);
          motor(0,1,128,128);  
        }
      }
      else if(slot[0] == 'B'){
        while(yon != 0b01000000){
          scan();
          lcd.setCursor(0,0); sprintf(data,"%d",counter); lcd.print(data);
          digitalWrite(13, HIGH);
          motor(1,0,128,128);  
        }
      }
      digitalWrite(13, LOW);
      counter++;
      attachInterrupt(digitalPinToInterrupt(2), interupsi, RISING);
      attachInterrupt(digitalPinToInterrupt(3), interupsi, RISING);
      }
      if(counter == (6-slotIs)){
        motor(0,0,0,0);
        menuIni = false;
        buzzing(3,500,100);
      }
    PID();
      
  }
  motor(0,0,0,0);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SELESAI");
  while(1){
    //selesai
  }
}
void setup()
{
  lcd.begin();
  lcd.backlight();
  Serial.begin(115200);

  pinMode(7, INPUT_PULLUP); // button A
  pinMode(8, INPUT_PULLUP); // button B

  pinMode(BUZZ, OUTPUT);

  pinMode(Lpwm, OUTPUT);
  pinMode(Ldir, OUTPUT);
  pinMode(Rpwm, OUTPUT);
  pinMode(Rdir, OUTPUT);
  menu0 = 1;
  menu();
  load();
  goesToGate();
   
}
void loop() {
}
