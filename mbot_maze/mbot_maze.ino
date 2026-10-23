#include <MeMCore.h>

// CG1111A A-maze-ing Race
// Port 1: ultrasonic (left), Port 2: line sensor
// Port 3: A2, A3 -> decoder, Port 4: A0 <- IR detector (right), A1 <- LDR
// Decoder: 00 IR, 01 green, 10 blue, 11 red

// 0 = only print sensor values (robot does not move)
// 1 = checkpoint 1 moves, 2 = checkpoint 2 wall following
#define MODE 2

#define ULTRASONIC 12
#define DEC_A A2
#define DEC_B A3
#define IR_PIN A0
#define BUTTON A7

// speeds
#define LEFT_SPEED 200
#define RIGHT_SPEED 200
#define TURN_SPEED 200
#define NUDGE_SPEED 140

// timings in ms
#define MOVE_TIME 20
#define STRAIGHT_TIME 3000
#define TURN_LEFT_TIME 410
#define TURN_RIGHT_TIME 415
#define TURN_180_TIME 780
#define ONE_GRID_TIME 760
#define PAUSE_TIME 1000

// ultrasonic
#define TIMEOUT 2000
#define SPEED_OF_SOUND 340
#define TOO_NEAR_LEFT 9       // cm, tested in lab
#define NO_WALL_DIST 20       // cm

// IR
#define IR_WAIT 2
#define TOO_NEAR_RIGHT 700    // tested: no wall ~183, 4-5 cm ~590, 3 cm ~675, 2 cm ~840

MeDCMotor leftMotor(M1);
MeDCMotor rightMotor(M2);
MeLineFollower lineFinder(PORT_2);
MeBuzzer buzzer;

float leftDist = 0;   // -1 if no wall
int irOff = 0;
int irOn = 0;
int irDiff = 0;
int started = 0;


void setup()
{
    // Configure pinMode for A0, A1, A2, A3
    pinMode(DEC_A, OUTPUT);
    pinMode(DEC_B, OUTPUT);
    pinMode(IR_PIN, INPUT);
    shineIR();

    Serial.begin(9600);
}

void loop()
{
    if (MODE == 0) {
        showSensors();
        delay(200);
    // Used for checkpoint 1
    } else if (MODE == 1) {
        movementTest();
    } else if (MODE == 2) {
        if (started == 0) {
            waitButton();
            started = 1;
        }

        leftDist = readUltrasonic();
        irDiff = readIR();

        if (lineFinder.readSensors() == S1_IN_S2_IN) {
            stopMotor();
            waitButton();
            moveStraight(300);   // get off the black strip
        } else if (leftDist > 0 && leftDist < TOO_NEAR_LEFT) {
            nudgeRight();
        } else if (irDiff > TOO_NEAR_RIGHT) {
            nudgeLeft();
        } else {
            moveForward();
        }
    }
}
