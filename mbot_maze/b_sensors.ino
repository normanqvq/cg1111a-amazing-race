void shineIR() {
    // Code for turning on the IR emitter only
    digitalWrite(DEC_A, LOW);
    digitalWrite(DEC_B, LOW);
}

void shineRed() {
    // Code for turning on the red LED only
    digitalWrite(DEC_A, HIGH);
    digitalWrite(DEC_B, HIGH);
}

void shineGreen() {
    // Code for turning on the green LED only
    digitalWrite(DEC_A, HIGH);
    digitalWrite(DEC_B, LOW);
}

void shineBlue() {
    // Code for turning on the blue LED only
    digitalWrite(DEC_A, LOW);
    digitalWrite(DEC_B, HIGH);
}

// distance to left wall in cm, -1 if no wall
float readUltrasonic() {
    pinMode(ULTRASONIC, OUTPUT);
    digitalWrite(ULTRASONIC, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC, LOW);
    pinMode(ULTRASONIC, INPUT);

    long duration = pulseIn(ULTRASONIC, HIGH, TIMEOUT);
    if (duration == 0) {
        return -1;
    }
    float dist = duration / 2.0 / 1000000 * SPEED_OF_SOUND * 100;
    if (dist > NO_WALL_DIST) {
        return -1;
    }
    return dist;
}

// IR off reading - IR on reading, bigger = closer
int readIR() {
    shineGreen();   // IR off
    delay(IR_WAIT);
    irOff = analogRead(IR_PIN);

    shineIR();
    delay(IR_WAIT);
    irOn = analogRead(IR_PIN);

    return irOff - irOn;
}
