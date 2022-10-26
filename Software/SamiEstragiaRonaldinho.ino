//librerias
#include <Button.h>
#include <MotorSumo.h>
#include <Tatami.h>
#include <Sharp.h>
#include "BluetoothSerial.h"

//debug
#define DEBUG_SHARP 1
#define DEBUG_TATAMI 1
#define DEBUG_STATE 1
#define TICK_DEBUG 500
unsigned long currentTimeSharp = 0;
unsigned long currentTimeTatami = 0;
unsigned long currentTimeEstrategy = 0;

//configuramos el Serial Bluetooth
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;


//Variables y constantes para los sensores de tatami
#define PIN_SENSOR_TATAMI_IZQ 26
#define PIN_SENSOR_TATAMI_DER 27
int righTatamiRead;
int leftTatamiRead;
#define BORDE_TATAMI 300

//Variables y constantes para los sensores de distancia
#define PIN_SENSOR_DISTANCIA_DERECHO 32
#define PIN_SENSOR_DISTANCIA_IZQUIERDO 33
#define RIVAL 50
int distSharpRigh;
int distSharpLeft;

// Variables y constantes para los motores
#define PIN_MOTOR_MR1 22 //DIR
#define PIN_MOTOR_MR2PWM 21 //PWM
#define PIN_MOTOR_ML1 19 //DIR
#define PIN_MOTOR_ML2PWM 18 //PWM
#define SEARCH_SPEED 100
#define ATTACK_SPEED 250
#define ATTACK_SPEED_SNAKE 250
#define AVERAGE_SPEED 200
int righSpeed = 200;
int leftSpeed = 200;

//Pines para los botones
#define PIN_BUTTON_START 34
bool boton_start;
#define PIN_BUTTON_STRATEGY 35
bool boton_button2;

//<------------------------------------------------------------------------------------------------------------->//
//Instanciamos todos los objetos del robot
Motor *mDer = new Motor(PIN_MOTOR_MR1, PIN_MOTOR_MR2PWM, righSpeed);
Motor *mIzq = new Motor(PIN_MOTOR_ML1, PIN_MOTOR_ML2PWM, leftSpeed);

Tatami *rightTatami = new Tatami(PIN_SENSOR_TATAMI_DER);
Tatami *LeftTatami = new Tatami(PIN_SENSOR_TATAMI_IZQ);

Sharp *sharpRight = new Sharp(PIN_SENSOR_DISTANCIA_DERECHO);
Sharp *sharpLeft = new Sharp(PIN_SENSOR_DISTANCIA_IZQUIERDO);

Button *button2 = new  Button(PIN_BUTTON_STRATEGY);
Button *start = new  Button(PIN_BUTTON_START);
//<------------------------------------------------------------------------------------------------------------->//
//Funciones para indicar el lado del giro del motor y su velocidad
void forward()
{
  mDer->SetVelocidad(righSpeed);
  mIzq->SetVelocidad(leftSpeed);
  mDer->Forward();
  mIzq->Forward();
}

void backward()
{
  mDer->SetVelocidad(righSpeed);
  mIzq->SetVelocidad(leftSpeed);
  mDer->Backward();
  mIzq->Backward();
}

void left()
{
  mDer->SetVelocidad(righSpeed);
  mIzq->SetVelocidad(leftSpeed);
  mDer->Forward();
  mIzq->Backward();
}


void right()
{
  mDer->SetVelocidad(righSpeed);
  mIzq->SetVelocidad(leftSpeed);
  mDer->Backward();
  mIzq->Forward();
}

void stop()
{
  mDer->SetVelocidad(0);
  mIzq->SetVelocidad(0);
  mDer->Stop();
  mIzq->Stop();
}
//<------------------------------------------------------------------------------------------------------------->//
//Funcion para imprimir la distancia que leen los sharps en el puerto Bluetooth
void printSharp()
{
  if (millis() > currentTimeSharp + TICK_DEBUG)
  {
    currentTimeSharp = millis();
    SerialBT.print("Right dist: ");
    SerialBT.print(distSharpRigh);
    SerialBT.print("  //  ");
    SerialBT.print("Left dist: ");
    SerialBT.println(distSharpLeft);
  }
}
//Funcion para imprimir la lectura de los sensores de tatami en el puerto Bluetooth
void printTatami()
{
  if (millis() > currentTimeTatami + TICK_DEBUG)
  {
    currentTimeSharp = millis();
    SerialBT.print("Right tatami: ");
    SerialBT.print(righTatamiRead);
    SerialBT.print("  //  ");
    SerialBT.print("Left tatami: ");
    SerialBT.println(leftTatamiRead);
  }
}
//<------------------------------------------------------------------------------------------------------------->//
//Funcion para la lectura de los sensores
void sensorsReading()
  {
    distSharpRigh = sharpRight->SharpDist();
    distSharpLeft = sharpLeft->SharpDist();
    righTatamiRead = rightTatami->TatamiRead();
    leftTatamiRead = LeftTatami->TatamiRead();
  }

//<------------------------------------------------------------------------------------------------------------->//
enum ronaldinho
{
  STANDBY_RONALDINHO,
  GO_FORWARD_RONALDINHO,
  OLEE_RONALDINHO,
  STOP_RONALDINHO,
  SWITCH_STRATEGY_RONALDINHO
};
int ronaldinho = STANDBY_RONALDINHO;
//Maquina de estados para la estrategia de Ronaldinho (porque te comes un amague de esos que hacia en sus mejores epocas)
void Ronaldinho()
{
  switch (ronaldinho)
  {
    case STANDBY_RONALDINHO:
    {
      boton_start = start->GetIsPress();
        if (!boton_start)
        {
          delay(5000);
          ronaldinho = GO_FORWARD_RONALDINHO;
        } 
        else 
        {
          stop();
        }
        break;
    }

    case GO_FORWARD_RONALDINHO:
    {
      righSpeed = AVERAGE_SPEED;
      leftSpeed = AVERAGE_SPEED;
      forward();
      if(leftTatamiRead < 250 || righTatamiRead < 250) ronaldinho = OLEE_RONALDINHO;
      break;
    }

    case OLEE_RONALDINHO:
    {
      righSpeed = ATTACK_SPEED;
      leftSpeed = ATTACK_SPEED;
      backward();
      delay(1000);
      ronaldinho = STOP_RONALDINHO;
    }

    case STOP_RONALDINHO:
    {
      stop();
      delay(2000);
      ronaldinho = SWITCH_STRATEGY_RONALDINHO;
    }

    case SWITCH_STRATEGY_RONALDINHO:
    {
      stop();
    }
    
  }
}
//<------------------------------------------------------------------------------------------------------------->//
//Funcion para imprimir la estrategia y el caso en el puerto Bluetooth
void printStrategy() 
{
  if (millis() > currentTimeEstrategy + TICK_DEBUG)
  {
    currentTimeEstrategy = millis();
    String status = "";
    switch (ronaldinho)
    {
      case STANDBY_RONALDINHO:
      status = "STANDBY";
      break;
      case GO_FORWARD_RONALDINHO:
      status = "GO_FORWARD";
      break;
      case OLEE_RONALDINHO:
      status = "OLEE";
      break;
      case STOP_RONALDINHO:
      status = "STOP";
      break;
      case SWITCH_STRATEGY_RONALDINHO:
      status = "SWITCH_STRATEGY";
      break;
    }
    SerialBT.print("  //  Case: ");
    SerialBT.println(status);
  }
}
//<------------------------------------------------------------------------------------------------------------->//

void setup()
{
  SerialBT.begin("Sami");
  Serial.begin(9600);
}

void loop() 
{ 
  sensorsReading();
  Ronaldinho();
  if(DEBUG_SHARP) printSharp();
  if(DEBUG_TATAMI) printTatami();
  if(DEBUG_STATE) printStrategy();
}