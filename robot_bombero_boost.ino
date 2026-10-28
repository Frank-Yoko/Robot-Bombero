#include <Servo.h>

//Servo
Servo servi;
int serviM = 80;
int serviL = 30;
int serviR = 130;


//L298n
int ENA = 5;
int IN1 = 23;
int IN2 = 22;

int ENB = 6;
int IN3 = 25;
int IN4 = 24;

const int SPEED = 60;

//Hc SR04
const int TRIG = 26;
const int ECHO = 27;


bool OBSTACULO_R;
bool OBSTACULO_L;
const int DIST_OBSTACULO = 20;

//Modulo flama
int FLAME_LEFT = A0;
int FLAME_RIGHT = A1;
int FLAME_MIDDLE = A2;
int LED = 30;
const int FIRE = 100;
const int MUESTRAS = 5;        

//Tiempos
const int TIEMPO_RETROCESO = 400;
const int TIEMPO_GIRO = 500;
const int TIEMPO_SERVO = 500;

//Agua
#define mosfet 3
const int POTENCIA_AGUA = 160;

void setup() {
  Serial.begin(9600);

  pinMode (ENA, OUTPUT);
  pinMode (IN1, OUTPUT);
  pinMode (IN2, OUTPUT);

  pinMode (ENB, OUTPUT);
  pinMode (IN3, OUTPUT);
  pinMode (IN4, OUTPUT);

  
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  servi.attach(7);

  pinMode (FLAME_LEFT, INPUT);
  pinMode (FLAME_RIGHT, INPUT);
  pinMode (FLAME_MIDDLE, INPUT);
  pinMode (LED, OUTPUT);

  pinMode(mosfet, OUTPUT);
}

//Motores
void avanzar(){
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, SPEED);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, SPEED);
}

void detener(){
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

int leerFlama(int pin){
  analogRead(pin);
  long suma = 0;
  for (int i = 0; i < MUESTRAS; i++){
    suma += analogRead(pin);
  }
  return suma / MUESTRAS;
}


int detectarFlama(){
  int l = leerFlama(FLAME_LEFT);
  int m = leerFlama(FLAME_MIDDLE);
  int r = leerFlama(FLAME_RIGHT);

  int minimo = min(l, min(m, r));
  if (minimo >= FIRE) return 0;

  if (minimo == m) return 2;
  if (minimo == l) return 1;
  return 3;
}

bool esperar(unsigned long ms){
  unsigned long inicio = millis();
  while (millis() - inicio < ms){
    if (detectarFlama() != 0) return true;
  }
  return false;
}



void retroceder(){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, SPEED);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, SPEED);

  esperar(TIEMPO_RETROCESO);     

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 0);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, SPEED);
}

void girarIzquierda(){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 0);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, SPEED);
}

void girarDerecha(){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, SPEED);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
}


float calcularDistancia(){
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duracion = pulseIn(ECHO, HIGH, 30000UL);
  if (duracion == 0) return 400; 
  return duracion * 0.034 / 2;
}

void movimiento(){
  //Ultrasonic
  float DISTANCIA = calcularDistancia();
  bool OBSTACULO = DISTANCIA < 20;

  //STRAIGHT
  servi.write(serviM);

  //Logic
   if (!OBSTACULO) {
    avanzar();
    return;
  }
  else{
    detener();

    servi.write(serviL);
    if (esperar(TIEMPO_SERVO)) { detener(); return; }

    float DISTANCIA_R = calcularDistancia();
    OBSTACULO_R = DISTANCIA_R < 30;

    if (!OBSTACULO_R ){
      girarDerecha();

      if (esperar(TIEMPO_GIRO)) { detener(); return; }

      avanzar();

    }
    else{
      servi.write(serviR);
      if (esperar(TIEMPO_SERVO)) { detener(); return; }

      float DISTANCIA_L = calcularDistancia();
      OBSTACULO_L = DISTANCIA_L < 30;
      if (!OBSTACULO_L){
        girarIzquierda();

        if (esperar(TIEMPO_GIRO)) { detener(); return; }

        avanzar();
      }
      else{
        retroceder();

        if (esperar(TIEMPO_GIRO)) { detener(); return; }

      }
    }
  }
  servi.write(serviM);
  if (esperar(TIEMPO_SERVO)) { detener(); return; }
}


void loop() {
  int direccion = detectarFlama();

  /*
  Serial.print("Direccion flama: ");
  Serial.println(direccion);
  Serial.print("Izq: ");
  Serial.print(leerFlama(FLAME_LEFT));
  Serial.print("  Med: ");
  Serial.print(leerFlama(FLAME_MIDDLE));
  Serial.print("  Der: ");
  Serial.println(leerFlama(FLAME_RIGHT));
  delay(700);
  */

  if (direccion != 0){
    detener();                          
    digitalWrite(LED, HIGH);

    if (direccion == 1)      servi.write(serviR); 
    else if (direccion == 2) servi.write(serviM);
    else                     servi.write(serviL);
    delay(TIEMPO_SERVO);                

    analogWrite(mosfet, POTENCIA_AGUA);
    delay(1000);
    analogWrite(mosfet, 0);
    
  }
  else{
    movimiento();
    digitalWrite(LED, LOW);
    analogWrite(mosfet, 0);
    servi.write(serviM);
  }

}
