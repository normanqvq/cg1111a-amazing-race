int detectColour()
{
    // Shine Red, read LDR after some delay
    shineRed();
    delay(LDR_WAIT);
    raw[0] = readLDR();

    // Shine Green, read LDR after some delay
    shineGreen();
    delay(LDR_WAIT);
    raw[1] = readLDR();

    // Shine Blue, read LDR after some delay
    shineBlue();
    delay(LDR_WAIT);
    raw[2] = readLDR();

    shineIR();   // IR back on for wall following

    // Run algorithm for colour decoding
    // step1. change raw readings to 0-255 using white and black
    //        e.g. black 300, white 800, raw 550 -> 127
    for (int i = 0; i < 3; i++) {
        rgb[i] = (raw[i] - black[i]) * 255.0 / (white[i] - black[i]);
    }
    // step2. find the closest colour
    return closestColour();
}

// average of a few LDR readings
int readLDR() {
    long total = 0;
    for (int i = 0; i < LDR_SAMPLES; i++) {
        total = total + analogRead(LDR_PIN);
        delay(5);
    }
    return total / LDR_SAMPLES;
}

// find which colour in colourValues is closest to rgb
// distance = (r1-r2)^2 + (g1-g2)^2 + (b1-b2)^2, smallest one wins
int closestColour() {
    int best = 0;
    long bestDist = 999999;
    for (int c = 0; c < 6; c++) {
        long dist = 0;
        for (int i = 0; i < 3; i++) {
            long d = rgb[i] - colourValues[c][i];
            dist = dist + d * d;
        }
        if (dist < bestDist) {
            bestDist = dist;
            best = c;
        }
    }
    return best;
}

void printColourName(int c) {
    if (c == RED) {
        Serial.println("RED");
    } else if (c == GREEN) {
        Serial.println("GREEN");
    } else if (c == ORANGE) {
        Serial.println("ORANGE");
    } else if (c == PINK) {
        Serial.println("PINK");
    } else if (c == LIGHT_BLUE) {
        Serial.println("LIGHT BLUE");
    } else {
        Serial.println("WHITE");
    }
}
