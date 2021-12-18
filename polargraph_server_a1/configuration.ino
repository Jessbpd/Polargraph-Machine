/**
CONFIGURATION
Este es uno de los archivos principales del programa del servidor polargraph.
Configura los objetos del motor (AccelSteppers) y tiene valores predeterminados
valores para las combinaciones de motor, piñón y micropasos utilizadas
por polargrafos hasta ahora.
*/

//#ifdef ADAFRUIT_MOTORSHIELD_V1
const int stepType = INTERLEAVE;
AF_Stepper afMotorA(motorStepsPerRev, 1);
AF_Stepper afMotorB(motorStepsPerRev, 2);

void forwarda() { afMotorA.onestep(FORWARD, stepType); }
void backwarda() { afMotorA.onestep(BACKWARD, stepType); }
void forwardb() { afMotorB.onestep(FORWARD, stepType); }
void backwardb() { afMotorB.onestep(BACKWARD, stepType); }

AccelStepper motorA(forwarda, backwarda);
AccelStepper motorB(forwardb, backwardb);
//#endif

/*#ifdef ADAFRUIT_MOTORSHIELD_V2
const int stepType = MICROSTEP;

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *afMotorA = AFMS.getStepper(motorStepsPerRev, 1);
Adafruit_StepperMotor *afMotorB = AFMS.getStepper(motorStepsPerRev, 2);

void forwarda() { afMotorA->onestep(FORWARD, stepType); }
void backwarda() { afMotorA->onestep(BACKWARD, stepType); }
void forwardb() { afMotorB->onestep(FORWARD, stepType); }
void backwardb() { afMotorB->onestep(BACKWARD, stepType); }
AccelStepper motorA(forwarda, backwarda);
AccelStepper motorB(forwardb, backwardb);
#endif
*/
/*Configuración del motor si está utilizando controladores paso a paso en serie
(EasyDrivers, stepsticks, Pololu, etc.).

Si se está conectando usted mismo, introduzca aquí los números de los pines.

Tenga en cuenta que el servo de elevación del lápiz suele estar en el pin 9, así que evite
eso si puedes. Si no puede, entonces sabe cómo cambiarlo.
*/
/*#ifdef SERIAL_STEPPER_DRIVERS
#define MOTOR_A_ENABLE_PIN 8
#define MOTOR_A_STEP_PIN 2
#define MOTOR_A_DIR_PIN 5
  
#define MOTOR_B_ENABLE_PIN 4
#define MOTOR_B_STEP_PIN 3
#define MOTOR_B_DIR_PIN 6
AccelStepper motorA(1,MOTOR_A_STEP_PIN, MOTOR_A_DIR_PIN); 
AccelStepper motorB(1,MOTOR_B_STEP_PIN, MOTOR_B_DIR_PIN); 
#endif*/

// #ifdef UNL2003_DRIVER
// Contributed by @j0nson
// Inicializar el controlador paso a paso ULN2003
// El primer número es el tipo de motor paso a paso, 4 para un motor paso a paso normal de 4 hilos, 8 para un motor normal de 4 hilos a medio paso
// Direcciones de conexión
// MotorA
//ULN2003  Arduino  AcceStepper Init
//IN1      2        2
//IN2      3        4
//IN3      4        3
//IN4      5        5
// MotorB
//ULN2003  Arduino  AcceStepper Init
//IN1      6        6
//IN2      7        8
//IN3      8        7
//IN4      9        9

//for a 28YBJ-48 Stepper, change these parameters above
//Step angle (8-step) 5.625deg, 64 steps per rev
//Step angle (4-step) 11.25deg, 32 steps per rev
//gear reduction ratio 1/63.68395

// motorStepsPerRev = 32 * 63.68395 = 2038; //for 4 step sequence
// motorStepsPerRev = 64 * 63.68395 = 4076; //for 8 step sequence

// motorStepsPerRev = 4076;
// mmPerRev = 63;
// DEFAULT_STEPS_PER_REV = 4076;
// DEFAULT_MM_PER_REV = 63;

//AccelStepper motorA(8,6,8,7,9);
//AccelStepper motorB(8,2,4,3,5);
//#endif

void configuration_motorSetup()
{
#ifdef SERIAL_STEPPER_DRIVERS
  pinMode(MOTOR_A_ENABLE_PIN, OUTPUT);
  digitalWrite(MOTOR_A_ENABLE_PIN, HIGH);
  pinMode(MOTOR_B_ENABLE_PIN, OUTPUT);
  digitalWrite(MOTOR_B_ENABLE_PIN, HIGH);
  motorA.setEnablePin(MOTOR_A_ENABLE_PIN);
  motorA.setPinsInverted(false, false, true);
  motorB.setEnablePin(MOTOR_B_ENABLE_PIN);
  motorB.setPinsInverted(true, false, true); // éste gira en la dirección opuesta a A, por lo tanto invertida.
#endif
}

void configuration_setup()
{
  mmPerStep = mmPerRev / multiplier(motorStepsPerRev);
  stepsPerMM = multiplier(motorStepsPerRev) / mmPerRev;
  
#ifdef ADAFRUIT_MOTORSHIELD_V2
  AFMS.begin();  // crear con la frecuencia predeterminada 1.6KHz
#endif
  delay(500); 
}
