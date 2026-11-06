void celebrate() {
    const int F5 = 698;
    const int G5 = 784;
    const int A5 = 880;
    const int D5 = 587;
    const int E5 = 659;
    const int D6 = 1175;

    // 0 means a rest.
    const int notes[] = {
        F5, F5, F5, 0, G5, F5,
        F5, F5, 0, A5, G5, F5,
        0, G5, G5, G5, D5, D5,
        E5, E5, E5, E5, F5, G5, A5,
        F5, G5, A5, D6
    };

    // Each tick is one third of a beat at 167 BPM.
    const byte ticks[] = {
        3, 3, 3, 3, 5, 7,
        3, 3, 2, 1, 2, 10,
        2, 3, 3, 1, 3, 3,
        2, 1, 2, 1, 3, 2, 7,
        2, 1, 2, 1
    };

    unsigned long start = millis();
    unsigned int totalTicks = 0;

    for (byte i = 0; i < sizeof(ticks) / sizeof(ticks[0]); i++) {
        totalTicks += ticks[i];

        unsigned long target = totalTicks * 60000UL / (167UL * 3);
        if (target > 10000UL) target = 10000UL;

        unsigned long elapsed = millis() - start;
        if (elapsed >= 10000UL) break;

        unsigned long duration = target > elapsed ? target - elapsed : 0;

        if (notes[i] != 0 && duration > 15) {
            // tone() already waits while playing.
            buzzer.tone(notes[i], duration - 15);
            buzzer.noTone();
        }

        while (millis() - start < target) {
            delay(1);
        }
    }

    buzzer.noTone();

    // Stay stopped after celebrating.
    while (true) {
        delay(100);
    }
}

void stopMotor() {
    // Code for stopping motor
    leftMotor.stop();
    rightMotor.stop();
}

void moveForward() {
    // Code for moving forward for some short interval
    // left motor is flipped so it needs a negative speed to go forward
    leftMotor.run(-LEFT_SPEED);
    rightMotor.run(RIGHT_SPEED);
    delay(MOVE_TIME);
}

void turnRight() {
    // Code for turning right 90 deg
    leftMotor.run(-TURN_SPEED);
    rightMotor.run(-TURN_SPEED);
    delay(TURN_RIGHT_TIME);
    stopMotor();
    delay(100);
}

void turnLeft() {
    // Code for turning left 90 deg
    leftMotor.run(TURN_SPEED);
    rightMotor.run(TURN_SPEED);
    delay(TURN_LEFT_TIME);
    stopMotor();
    delay(100);
}

void uTurn() {
    // Code for u-turn
    leftMotor.run(-TURN_SPEED);
    rightMotor.run(-TURN_SPEED);
    delay(TURN_180_TIME);
    stopMotor();
    delay(100);
}

void doubleLeftTurn() {
    // Code for double left turn
    turnLeft();
    moveOneGrid();
    turnLeft();
}

void doubleRightTurn() {
    // Code for double right turn
    turnRight();
    moveOneGrid();
    turnRight();
}

void nudgeLeft() {
    // Code for nudging slightly to the left for some short interval
    // left wheel slower than right wheel -> robot curves to the left
    leftMotor.run(-NUDGE_SPEED);
    rightMotor.run(RIGHT_SPEED);
    delay(MOVE_TIME);
}

void nudgeRight() {
    // Code for nudging slightly to the right for some short interval
    // right wheel slower than left wheel -> robot curves to the right
    leftMotor.run(-LEFT_SPEED);
    rightMotor.run(NUDGE_SPEED);
    delay(MOVE_TIME);
}


// keep moving forward for some time, then stop
void moveStraight(int time) {
    long start = millis();
    long now = millis();
    while (now - start < time) {
        moveForward();
        now = millis();
    }
    stopMotor();
    delay(300);   // wait for the robot to fully stop, or the next turn will be off
}

// used by double turns
void moveOneGrid() {
    moveStraight(ONE_GRID_TIME);
}
