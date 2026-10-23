void waitButton() {
    // button reads less than 100 when pressed
    // only count it as a press if it stays pressed for 50 ms in a row,
    // otherwise a small voltage drop when the motors stop can look like a press
    int count = 0;
    while (count < 10) {
        if (analogRead(BUTTON) < 100) {
            count = count + 1;
        } else {
            count = 0;
        }
        delay(5);
    }
    // wait until the button is released
    while (analogRead(BUTTON) < 100) {
    }
    delay(300);   // time to take your hand away
}

// MODE 1
void movementTest() {
    waitButton();

    Serial.println("move straight");
    moveStraight(STRAIGHT_TIME);
    delay(PAUSE_TIME);

    Serial.println("turn left");
    turnLeft();
    delay(PAUSE_TIME);

    Serial.println("turn right");
    turnRight();
    delay(PAUSE_TIME);

    Serial.println("u-turn");
    uTurn();
    delay(PAUSE_TIME);

    Serial.println("double left turn");
    doubleLeftTurn();
    delay(PAUSE_TIME);

    Serial.println("double right turn");
    doubleRightTurn();

    Serial.println("done!");
    celebrate();
}

// MODE 0
void showSensors() {
    leftDist = readUltrasonic();
    irDiff = readIR();

    Serial.print("ultrasonic: ");
    Serial.print(leftDist);
    Serial.print(" cm   IR off: ");
    Serial.print(irOff);
    Serial.print("  IR on: ");
    Serial.print(irOn);
    Serial.print("  IR diff: ");
    Serial.print(irDiff);

    if (lineFinder.readSensors() == S1_IN_S2_IN) {
        Serial.println("   BLACK");
    } else {
        Serial.println("");
    }
}
