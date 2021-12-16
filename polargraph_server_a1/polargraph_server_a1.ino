/**
Proyecto realizado por
 - Noguera, Ignacio
 - Pizarr0, Jessica
 - Velazquez, Hector
*/


// Controlador utilizado
#ifndef MICROCONTROLLER
#define MICROCONTROLLER MC_MEGA
#endif

// Librerías de Mega
#include <SPI.h>
#include <SD.h>
// Motor driver
#define ADAFRUIT_MOTORSHIELD_V1
#include <AFMotor.h>

#define SERIAL_STEPPER_DRIVERS 
//      ¿?¿?
//   iv. Using a signal amplifier like a UNL2003? 
//   --------------------------------------------
//   Don't forget to define your pins in 'configuration.ino'.
//   #define UNL2003_DRIVER


// Encienda algún código de depuración si quiere horror
//#define DEBUG
//#define DEBUG_COMMS
//#define DEBUG_PENLIFT
//#define DEBUG_PIXEL


// Desactive las funciones del programa si necesita 
// liberar espacio!
#define PIXEL_DRAWING
#define PENLIFT
#define VECTOR_LINES   

// ¿Innecesario?
#define MC_UNO 1
#define MC_MEGA 2

#include <AccelStepper.h>
#include <Servo.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

const String FIRMWARE_VERSION_NO = "0.0.1";

//  EEPROM addresses
const byte EEPROM_MACHINE_WIDTH = 0;
const byte EEPROM_MACHINE_HEIGHT = 2;
const byte EEPROM_MACHINE_MM_PER_REV = 14; // 4 bytes (float)
const byte EEPROM_MACHINE_STEPS_PER_REV = 18;
const byte EEPROM_MACHINE_STEP_MULTIPLIER = 20;

const byte EEPROM_MACHINE_MOTOR_SPEED = 22; // 4 bytes float
const byte EEPROM_MACHINE_MOTOR_ACCEL = 26; // 4 bytes float
const byte EEPROM_MACHINE_PEN_WIDTH = 30; // 4 bytes float

const byte EEPROM_MACHINE_HOME_A = 34; // 4 bytes
const byte EEPROM_MACHINE_HOME_B = 38; // 4 bytes

const byte EEPROM_PENLIFT_DOWN = 42; // 2 bytes
const byte EEPROM_PENLIFT_UP = 44; // 2 bytes

// Pen raising servo
Servo penHeight;
const int DEFAULT_DOWN_POSITION = 90;
const int DEFAULT_UP_POSITION = 180;
static int upPosition = DEFAULT_UP_POSITION; // defaults
static int downPosition = DEFAULT_DOWN_POSITION;
static int penLiftSpeed = 3; // ms between steps of moving motor
const byte PEN_HEIGHT_SERVO_PIN = A3; //Change the servo pin to A3, to pass through to the coolant enable pin.
boolean isPenUp = false;

// Machine specification defaults
const int DEFAULT_MACHINE_WIDTH = 650;
const int DEFAULT_MACHINE_HEIGHT = 650;
const int DEFAULT_MM_PER_REV = 95;
const int DEFAULT_STEPS_PER_REV = 400;
const int DEFAULT_STEP_MULTIPLIER = 1;

// Especificaciones de la maquina
static int motorStepsPerRev = DEFAULT_STEPS_PER_REV;
static float mmPerRev = DEFAULT_MM_PER_REV;
static byte stepMultiplier = DEFAULT_STEP_MULTIPLIER;
static int machineWidth = DEFAULT_MACHINE_WIDTH;
static int machineHeight = DEFAULT_MACHINE_HEIGHT;


static float currentMaxSpeed = 800.0;
static float currentAcceleration = 400.0;
static boolean usingAcceleration = true;

int startLengthMM = 800;

float mmPerStep = 0.0F;
float stepsPerMM = 0.0F;

long pageWidth = machineWidth * stepsPerMM;
long pageHeight = machineHeight * stepsPerMM;
long maxLength = 0;

//static char rowAxis = 'A';
const int INLENGTH = 50;
const char INTERMINATOR = 10;
const char SEMICOLON = ';';

float penWidth = 0.8F; // line width in mm

boolean reportingPosition = true;
boolean acceleration = true;

extern AccelStepper motorA;
extern AccelStepper motorB;

boolean currentlyRunning = true;

static char inCmd[10];
static char inParam1[14];
static char inParam2[14];
static char inParam3[14];
static char inParam4[14];

static byte inNoOfParams;

char lastCommand[INLENGTH+1];
boolean commandConfirmed = false;

int rebroadcastReadyInterval = 5000;
long lastOperationTime = 0L;
long motorIdleTimeBeforePowerDown = 600000L;
boolean automaticPowerDown = true;
boolean powerIsOn = false;

long lastInteractionTime = 0L;

#ifdef PIXEL_DRAWING
static boolean lastWaveWasTop = true;

//  Drawing direction
const static byte DIR_NE = 1;
const static byte DIR_SE = 2;
const static byte DIR_SW = 3;
const static byte DIR_NW = 4;

static int globalDrawDirection = DIR_NW;

const static byte DIR_MODE_AUTO = 1;
const static byte DIR_MODE_PRESET = 2;
static byte globalDrawDirectionMode = DIR_MODE_AUTO;
#endif

#if MICROCONTROLLER == MC_MEGA
  #define READY_STR "READY_100"
#else
  #define READY_STR "READY"
#endif

#define RESEND_STR "RESEND"
#define DRAWING_STR "DRAWING"
#define OUT_CMD_SYNC_STR "SYNC,"

char MSG_E_STR[] = "MSG,E,";
char MSG_I_STR[] = "MSG,I,";
char MSG_D_STR[] = "MSG,D,";

const static char COMMA[] = ",";
const static char CMD_END[] = ",END";
const static String CMD_CHANGELENGTH = "C01";
const static String CMD_CHANGEPENWIDTH = "C02";
#ifdef PIXEL_DRAWING
const static String CMD_DRAWPIXEL = "C05";
const static String CMD_DRAWSCRIBBLEPIXEL = "C06";
const static String CMD_CHANGEDRAWINGDIRECTION = "C08";
const static String CMD_TESTPENWIDTHSQUARE = "C11";
#endif
const static String CMD_SETPOSITION = "C09";
#ifdef PENLIFT
const static String CMD_PENDOWN = "C13";
const static String CMD_PENUP = "C14";
const static String CMD_SETPENLIFTRANGE = "C45";
#endif
#ifdef VECTOR_LINES
const static String CMD_CHANGELENGTHDIRECT = "C17";
#endif
const static String CMD_SETMACHINESIZE = "C24";
const static String CMD_GETMACHINEDETAILS = "C26";
const static String CMD_RESETEEPROM = "C27";
const static String CMD_SETMACHINEMMPERREV = "C29";
const static String CMD_SETMACHINESTEPSPERREV = "C30";
const static String CMD_SETMOTORSPEED = "C31";
const static String CMD_SETMOTORACCEL = "C32";
const static String CMD_SETMACHINESTEPMULTIPLIER = "C37";

void setup() 
{
  Serial.begin(57600);           // Configurar el Serial library en 57600 bps
  Serial.println("POLARGRAPH ON!");
  Serial.print("Hardware: ");
  Serial.println(MICROCONTROLLER);
  
  #if MICROCONTROLLER == MC_MEGA
  Serial.println("MC_MEGA");
  #elif MICROCONTROLLER == MC_UNO
  Serial.println("MC_UNO");
  #else
  Serial.println("No MC");
  #endif
  configuration_motorSetup();
  eeprom_loadMachineSpecFromEeprom();
  configuration_setup();

  motorA.setMaxSpeed(currentMaxSpeed);
  motorA.setAcceleration(currentAcceleration);  
  motorB.setMaxSpeed(currentMaxSpeed);
  motorB.setAcceleration(currentAcceleration);
  
  float startLength = ((float) startLengthMM / (float) mmPerRev) * (float) motorStepsPerRev;
  motorA.setCurrentPosition(startLength);
  motorB.setCurrentPosition(startLength);
  for (int i = 0; i<INLENGTH; i++) {
    lastCommand[i] = 0;
  }    
  comms_ready();

#ifdef PENLIFT
  penlift_penUp();
#endif
  delay(500);

}
void loop()
{
  if (comms_waitForNextCommand(lastCommand)) 
  {
#ifdef DEBUG_COMMS    
    Serial.print(F("Last comm: "));
    Serial.print(lastCommand);
    Serial.println(F("..."));
#endif
    comms_parseAndExecuteCommand(lastCommand);
  }
}

#if MICROCONTROLLER == MC_MEGA
const static String CMD_TESTPENWIDTHSCRIBBLE = "C12";
const static String CMD_DRAWSAWPIXEL = "C15,";
const static String CMD_DRAWCIRCLEPIXEL = "C16";
const static String CMD_SET_ROVE_AREA = "C21";
const static String CMD_DRAWDIRECTIONTEST = "C28";
const static String CMD_MODE_STORE_COMMANDS = "C33";
const static String CMD_MODE_EXEC_FROM_STORE = "C34";
const static String CMD_MODE_LIVE = "C35";
const static String CMD_RANDOM_DRAW = "C36";
const static String CMD_START_TEXT = "C38";
const static String CMD_DRAW_SPRITE = "C39";
const static String CMD_CHANGELENGTH_RELATIVE = "C40";
const static String CMD_SWIRLING = "C41";
const static String CMD_DRAW_RANDOM_SPRITE = "C42";
const static String CMD_DRAW_NORWEGIAN = "C43";
const static String CMD_DRAW_NORWEGIAN_OUTLINE = "C44";

//  Definiciones de pines EndStop 
const int ENDSTOP_X_MAX = 17;
const int ENDSTOP_X_MIN = 16;
const int ENDSTOP_Y_MAX = 15;
const int ENDSTOP_Y_MIN = 14;

long ENDSTOP_X_MIN_POSITION = 130;
long ENDSTOP_Y_MIN_POSITION = 130;

// Tamaño y ubicación del área de rove
long rove1x = 1000;
long rove1y = 1000;
long roveWidth = 5000;
long roveHeight = 8000;

boolean swirling = false;
String spritePrefix = "";
int textRowSize = 200;
int textCharSize = 180;

boolean useRoveArea = false;

int commandNo = 0;
int errorInjection = 0;

boolean storeCommands = false;
boolean drawFromStore = false;
String commandFilename = "";

// sd card stuff
const int chipSelect = 53;
boolean sdCardInit = false;

// configurar variables usando las funciones de 
// la biblioteca de la utilidad SD:
File root;
boolean cardPresent = false;
boolean cardInit = false;
boolean echoingStoredCommands = false;

// El archivo en sí
File pbmFile;

// Información que extraemos sobre el archivo de mapa de bits
long pbmWidth, pbmHeight;
float pbmScaling = 1.0;
int pbmDepth, pbmImageoffset;
long pbmFileLength = 0;
float pbmAspectRatio = 1.0;

volatile int speedChangeIncrement = 100;
volatile int accelChangeIncrement = 100;
volatile float penWidthIncrement = 0.05;
volatile int moveIncrement = 400;

boolean currentlyDrawingFromFile = false;
String currentlyDrawingFilename = "";

static float translateX = 0.0;
static float translateY = 0.0;
static float scaleX = 1.0;
static float scaleY = 1.0;
static int rotateTransform = 0;

#endif
