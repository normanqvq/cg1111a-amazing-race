#include <MeMCore.h>

// CG1111A A-maze-ing Race
// Port 1: ultrasonic (left), Port 2: line sensor
// Port 3: A2, A3 -> decoder, Port 4: A0 <- IR detector (right), A1 <- LDR
// Decoder: 00 IR, 01 green, 10 blue, 11 red

// 0 = only print sensor values (robot does not move)
// 1 = checkpoint 1 moves, 2 = checkpoint 2 wall following
// 3 = colour test (robot does not move, press button to read colour)
// 4 = full maze run (use this for the demo)
#define MODE 1

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

// colour sensor
#define LDR_PIN A1
#define LDR_WAIT 200          // ms to wait after switching LED, need to test
#define LDR_SAMPLES 10

// colour numbers
#define RED 0
#define GREEN 1
#define ORANGE 2
#define PINK 3
#define LIGHT_BLUE 4
#define WHITE 5

MeDCMotor leftMotor(M1);
MeDCMotor rightMotor(M2);
MeLineFollower lineFinder(PORT_2);
MeBuzzer buzzer;

float leftDist = 0;   // -1 if no wall
int irOff = 0;
int irOn = 0;
int irDiff = 0;
int started = 0;
int justTurned = 0;   // 1 = just turned at a black strip and may still be on it

// colour calibration, need to fill in after testing with MODE 3
// white and black are raw LDR readings for red, green, blue
int white[3] = {800, 800, 800};
int black[3] = {300, 300, 300};
// 0-255 values of each colour paper, same order as the colour numbers above
int colourValues[6][3] = {
    {230, 80, 90},    // red
    {90, 200, 120},   // green
    {250, 160, 90},   // orange
    {250, 180, 200},  // pink
    {120, 200, 240},  // light blue
    {255, 255, 255}   // white
};
int raw[3];   // last raw LDR readings (red, green, blue)
int rgb[3];   // last readings changed to 0-255


void setup()
{
    // Configure pinMode for A0, A1, A2, A3
    pinMode(DEC_A, OUTPUT);
    pinMode(DEC_B, OUTPUT);
    pinMode(IR_PIN, INPUT);
    pinMode(LDR_PIN, INPUT);
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
    } else if (MODE == 3) {
        colourTest();
    } else if (MODE == 4) {
        if (started == 0) {
            waitButton();
            started = 1;
        }

        // Read ultrasonic sensing distance
        leftDist = readUltrasonic();
        // Read IR sensing distance
        irDiff = readIR();

        int onBlack = 0;
        if (lineFinder.readSensors() == S1_IN_S2_IN) {
            onBlack = 1;
        }
        // after turning we may still be on the same black strip,
        // so ignore black until we have left it
        if (justTurned == 1 && onBlack == 0) {
            justTurned = 0;
        }

        // if within black line, stop motor, detect colour, and take corresponding action
        if (onBlack == 1 && justTurned == 0) {
            stopMotor();
            delay(200);   // let the robot stop before reading colour
            int colour = detectColour();
            if (colour == RED) {
                turnLeft();
            } else if (colour == GREEN) {
                turnRight();
            } else if (colour == ORANGE) {
                uTurn();
            } else if (colour == PINK) {
                doubleLeftTurn();
            } else if (colour == LIGHT_BLUE) {
                doubleRightTurn();
            } else {
                // white paper under the black strip = end of maze
                celebrate();
                while (1) {
                    // stay stopped
                }
            }
            justTurned = 1;
        } else if (leftDist > 0 && leftDist < TOO_NEAR_LEFT) {
            // too near to left wall, nudge right
            nudgeRight();
        } else if (irDiff > TOO_NEAR_RIGHT) {
            // too near to right wall, nudge left
            nudgeLeft();
        } else {
            moveForward();
        }
    }
}
