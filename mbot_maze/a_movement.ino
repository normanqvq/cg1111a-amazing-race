void celebrate() {
    // Code for playing celebratory tune
    buzzer.tone(392, 150);
    buzzer.tone(523, 150);
    buzzer.tone(659, 150);
    buzzer.tone(784, 300);
    buzzer.tone(659, 150);
    buzzer.tone(784, 600);
    buzzer.noTone();
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
    leftMotor.run(TURN_SPEED);
    rightMotor.run(TURN_SPEED);
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
