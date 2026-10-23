# CG1111A The A-maze-ing Race 2026 – mBot

Arduino code for our mBot (mCore, ATmega328) for the CG1111A final project.

## Code structure

The sketch is split into tabs. Arduino joins all `.ino` files in the folder when compiling, so all `#define`s and global variables are kept in the main file.

| File | Contents |
|---|---|
| `mbot_maze/mbot_maze.ino` | Settings (`#define`s), global variables, `setup()`, `loop()` |
| `mbot_maze/a_movement.ino` | Motor functions: forward, turns, double turns, nudges |
| `mbot_maze/b_sensors.ino` | Decoder LEDs / IR emitter, ultrasonic sensor, IR sensor |
| `mbot_maze/c_testing.ino` | Start button, Checkpoint 1 test sequence, sensor printing |

`MODE` in `mbot_maze.ino` chooses what the mBot does:

| MODE | Behaviour |
|---|---|
| 0 | Robot stays still and prints ultrasonic / IR / line sensor readings to Serial Monitor (9600 baud) every 200 ms |
| 1 | Checkpoint 1: press the button once, the robot does every movement in turn |
| 2 | Checkpoint 2: press the button, the robot drives straight and keeps away from walls, stops at the black strip |

Library: **MakeBlock Drive Updated 3.30.0** (3.31.0 does not compile for Arduino Uno).

## Wiring

| Port / pin | Connected to |
|---|---|
| M1 / M2 | Left motor / right motor |
| Port 1 (D12) | Ultrasonic sensor, left side |
| Port 2 | Line sensor, bottom |
| Port 3 (A2, A3) | 2-to-4 decoder inputs A, B |
| Port 4 (A0) | IR detector, right side |
| Port 4 (A1) | LDR (colour sensor) |
| A7 | Onboard push button |

---

## Movement Algorithm Design

### 1. Motor setup

The left motor is on M1 and the right motor is on M2. The two motors are mounted facing opposite directions, so to drive the robot forward the left motor needs a **negative** speed and the right motor a **positive** speed:

```cpp
leftMotor.run(-LEFT_SPEED);
rightMotor.run(RIGHT_SPEED);
```

This also means that giving both motors the **same sign** makes the wheels spin in opposite directions, so the robot rotates on the spot (+ = left, − = right).

All movement settings are `#define` constants at the top of `mbot_maze.ino`, so we only change one place when tuning.

| Constant | Value | Purpose |
|---|---|---|
| `LEFT_SPEED`, `RIGHT_SPEED` | 200 | Wheel speeds when moving forward (out of 255) |
| `TURN_SPEED` | 200 | Wheel speed when turning on the spot |
| `NUDGE_SPEED` | 140 | Speed of the slower wheel when nudging |
| `MOVE_TIME` | 20 ms | Length of one forward step |
| `TURN_LEFT_TIME` | 410 ms | Time for a 90° left turn |
| `TURN_RIGHT_TIME` | 415 ms | Time for a 90° right turn |
| `TURN_180_TIME` | 780 ms | Time for a 180° U-turn |
| `ONE_GRID_TIME` | 760 ms | Time to drive forward one grid |

### 2. Moving straight

`moveForward()` runs both wheels forward for only **20 ms** and then returns. It does not stop the motors, so calling it again and again makes the robot move smoothly.

We use short steps instead of one long "drive forward" command because the robot has to check its sensors (black strip, distance to walls) all the time. In the main loop, each loop does one small step, so the sensors are read about every 20–30 ms and the robot can react quickly.

When we need to drive for a fixed time (e.g. one grid in a double turn), `moveStraight(time)` keeps calling `moveForward()` until the time is up, using `millis()`:

```
moveStraight(time):
    start = millis()
    while millis() - start < time:
        moveForward()          // 20 ms step
    stop motors
    wait 300 ms                // let the robot fully stop
```

The left and right speeds are separate constants (`LEFT_SPEED`, `RIGHT_SPEED`). If the robot drifts to one side because the two motors are not exactly the same, we can make one wheel slightly slower to correct it.

### 3. Turning (90° and 180°)

All turns are done **on the spot**: both wheels run at the same speed in opposite directions, then stop.

```
turnLeft():
    left motor  +TURN_SPEED
    right motor +TURN_SPEED      // same sign -> rotate left on the spot
    wait TURN_LEFT_TIME
    stop motors
    wait 100 ms
```

`turnRight()` is the same with both speeds negative, and `uTurn()` is a left rotation for `TURN_180_TIME`.

Why we turn on the spot instead of turning with one wheel slower:

- At every waypoint there is a wall directly in front of the robot, so there is no space to make a wide turn.
- After turning on the spot, the robot is still in the middle of the same grid, so it is lined up with the next corridor.
- The turning angle only depends on time, so it is easy to tune and repeat.

The mBot has no wheel encoder or gyroscope, so the angle is controlled by **time**. We tuned each time on the maze floor (see Section 6).

The left and right turns use **different times**. Even with the same speed and time, our robot turned a little less to the right than to the left, because the two motors and the weight on each wheel are not exactly the same.

The U-turn is done as **one continuous rotation** instead of two 90° turns. This avoids stopping and starting twice, which adds error each time. Its time is not exactly 2 × 90° time, because the robot speeds up at the start and slides a bit after stopping, and these only happen once in a single rotation. So it was tuned separately.

### 4. Double turns (successive turns)

A double left turn is: turn left → drive forward one grid → turn left. The double right turn is the same with right turns.

```
doubleLeftTurn():
    turnLeft()
    moveOneGrid()       // moveStraight(ONE_GRID_TIME)
    turnLeft()
```

At first, the second turn of the double right turn did not reach 90°, even though a single right turn was correct. The reason was that the second turn started only 100 ms after driving forward, so the robot was still moving forward a little, and this forward momentum reduced the rotation. We increased the wait after `moveStraight()` from 100 ms to **300 ms**, so every turn starts with the robot fully stopped, the same as a single turn.

### 5. Wall following: nudging (Checkpoint 2)

The ultrasonic sensor is on the left and the IR sensor is on the right. When the robot is not turning, the main loop does this every step:

```
loop:
    leftDist = readUltrasonic()      // cm, -1 if no wall
    irDiff   = readIR()              // bigger = closer to right wall

    if both line sensors see black:  // waypoint
        stop
    else if 0 < leftDist < TOO_NEAR_LEFT:
        nudgeRight()
    else if irDiff > TOO_NEAR_RIGHT:
        nudgeLeft()
    else:
        moveForward()
```

A **nudge** is one 20 ms forward step where one wheel is slower (`NUDGE_SPEED` = 140 instead of 200), so the robot curves slightly away from the wall while still moving forward:

| Function | Left wheel | Right wheel | Effect |
|---|---|---|---|
| `moveForward()` | 200 | 200 | Straight |
| `nudgeLeft()` | 140 | 200 | Curves slightly left |
| `nudgeRight()` | 200 | 140 | Curves slightly right |

Design choices:

- **Black strip is checked first**, so the robot always stops at a waypoint even if it is also close to a wall.
- **Missing walls**: when there is no left wall, the ultrasonic times out and `readUltrasonic()` returns −1. Any reading above `NO_WALL_DIST` (20 cm) is also treated as no wall, because that is the wall of the next grid, not ours. The `leftDist > 0` check makes sure the robot does not nudge when there is no wall, and simply goes straight.
- **Small corrections**: since each nudge is only 20 ms, the robot corrects a little at a time and goes back to straight as soon as it is far enough from the wall, instead of zig-zagging.

The thresholds were found using MODE 0 (see Sensor Design, Section 3): `TOO_NEAR_LEFT` = 9 cm and `TOO_NEAR_RIGHT` = 700.

### 6. Calibration of movement

Procedure:

1. Always tune with a **fully charged battery**, since lower battery voltage makes the motors slower and every turn smaller.
2. Tune in this order: straight line → 90° turns → U-turn → one grid. Driving straight has to be correct first, otherwise the double turn cannot be correct.
3. Change one constant at a time, upload, and run the MODE 1 sequence. Check the angle against a tape line on the floor.

Tuning record:

| Constant | Values tried (ms) | Final | Notes |
|---|---|---|---|
| `TURN_LEFT_TIME` | 330 → 370 → 390 → 400 → 410 | 410 | 330 was clearly under 90° |
| `TURN_RIGHT_TIME` | 330 → 360 → 390 → 400 → 410 → 415 | 415 | Right needs slightly longer than left |
| `TURN_180_TIME` | 640 → 760 → 800 → 780 | 780 | 760 under-rotated, 800 over-rotated |
| `ONE_GRID_TIME` | 700 → 760 | 760 | 700 was too short for one grid |
| Wait after `moveStraight()` | 100 → 300 | 300 | Fixed second turn of double right turn |

### 7. Start button

The robot waits for the onboard button (A7, reads below 100 when pressed) before it starts. In MODE 1 one press runs the whole movement sequence automatically.

At first the robot sometimes turned by itself after stopping. The reason was that when the motors stop, the voltage drops for a moment and the A7 reading dropped low enough to look like a button press. We fixed this by only accepting a press when the button reads "pressed" **10 times in a row, 5 ms apart** (50 ms in total). A short voltage dip cannot last that long, but a real press always does.

### 8. Difficulties and solutions (movement)

| Problem | Cause | Solution |
|---|---|---|
| Left and right turns gave different angles with the same time | The two motors are not identical | Separate `TURN_LEFT_TIME` and `TURN_RIGHT_TIME` |
| U-turn with 2 × 90° time over/under-rotated | Speeding up and sliding happen only once in one rotation | Tuned `TURN_180_TIME` separately |
| Second turn of double right turn was short | Robot was still moving forward when the turn started | Wait 300 ms after driving forward |
| Turns slowly became smaller during testing | Battery running low | Tune and run with a full battery |
| Robot turned by itself after stopping | Voltage dip read as a button press | Button must read "pressed" for 50 ms in a row |
| Code did not compile with the library from Canvas (3.31.0) | `MeMegaPi.cpp` uses pins A11–A13 that do not exist on Uno | Installed MakeBlock Drive Updated 3.30.0 |

---

## Sensor Design: Ultrasonic and IR (Checkpoint 2)

### 1. Ultrasonic sensor (left side)

The Makeblock ultrasonic sensor is on Port 1 and uses one signal pin (D12) for both trigger and echo.

```
readUltrasonic():
    send a 10 us HIGH pulse on D12
    duration = pulseIn(D12, HIGH, TIMEOUT)     // time for the echo to come back
    if duration == 0: return -1                // timeout, no wall
    dist = duration / 2 / 1000000 * 340 * 100  // cm, divide by 2 because sound goes there and back
    if dist > NO_WALL_DIST: return -1
    return dist
```

| Constant | Value | Reason |
|---|---|---|
| `TIMEOUT` | 2000 us | A wall 30 cm away gives an echo after about 1800 us, so 2000 us is enough. A short timeout keeps each loop fast when there is no wall |
| `NO_WALL_DIST` | 20 cm | When our left wall is missing, the sensor may see the wall of the next grid. Anything further than 20 cm is not our wall, so it is treated as "no wall" |
| `TOO_NEAR_LEFT` | 9 cm | Reading at which the robot nudges right (see Section 3) |

The sensor cannot measure below 3 cm, so it is mounted a few cm inward from the side of the robot. This way the sensor is still more than 3 cm from the wall even when the robot's side is very close to it.

### 2. IR proximity sensor (right side)

The IR emitter is switched by decoder output Y0 through the L293D motor driver (the decoder can only sink 8 mA, the emitter needs more). The detector output is read on A0.

**Removing ambient IR.** The detector voltage depends on both the IR reflected from the wall and the ambient IR in the room (lights, sunlight). Ambient IR changes across the maze, so one reading is not reliable. We read the detector twice, once with the emitter off and once with it on, and take the difference:

```
readIR():
    turn IR emitter OFF (select green LED on the decoder)
    wait IR_WAIT ms
    irOff = analogRead(A0)        // ambient IR only
    turn IR emitter ON
    wait IR_WAIT ms
    irOn = analogRead(A0)         // ambient IR + reflection from wall
    return irOff - irOn
```

In our detector circuit, more IR gives a lower voltage, so `irOff - irOn` gets **bigger as the wall gets closer**. Ambient IR changes both readings by about the same amount, so it cancels out in the difference.

| Constant | Value | Reason |
|---|---|---|
| `IR_WAIT` | 2 ms | Time for the detector voltage to settle after switching the emitter |
| `TOO_NEAR_RIGHT` | 700 | `irDiff` above this means too close to the right wall (see Section 3) |

### 3. Calibration

We used MODE 0, which prints the readings every 200 ms to the Serial Monitor, and placed a wall at measured distances from the side of the robot.

**IR sensor** (`IR_WAIT` = 2 ms unless stated):

| Distance from robot's right side | IR off | IR on | IR diff |
|---|---|---|---|
| No wall | ~635 | ~450 | ~183 |
| 7 cm | ~945 | ~670 | ~280 (IR_WAIT 500) |
| 4–5 cm | ~940 | ~350 | ~590 (IR_WAIT 500) |
| 3 cm | ~925 | ~250 | ~675 (IR_WAIT 500) |
| 2 cm | ~890 | ~50 | ~840 |

`TOO_NEAR_RIGHT` = 700 makes the robot steer away at about 3 cm, before it reaches 2 cm. It is far above the reading when the robot drives in the middle of the corridor (~280 at 7 cm), so the robot does not nudge for no reason.

When people walked past or blocked the room light, `IR off` and `IR on` both changed by up to ~100, but `IR diff` stayed almost the same. This shows the ambient IR compensation works.

**Ultrasonic sensor:** `TOO_NEAR_LEFT` = 9 cm was chosen from testing in the lab.

<!-- TODO: add the ultrasonic readings at each distance -->

### 4. Difficulties and solutions (sensors)

| Problem | What we saw | Cause | Solution |
|---|---|---|---|
| IR diff was tiny | `IR diff` stayed at ~7–14 even with a finger right in front of the sensor | The detector could not react within 2 ms, so the "off" and "on" readings were the same | Tested with `IR_WAIT` = 500 ms to check the circuit, then fixed the detector circuit (see next row). After that, 2 ms worked |
| IR detector saturated | `IR on` stayed at ~55 with or without a wall. `IR diff` only changed because the hand blocked room light (`IR off` went up), not because of reflection | Detector circuit too sensitive, so it was already "full" when the emitter was on | Changed R_receiver <!-- TODO: from ___ to ___ -->. `IR on` went up to ~450 with no wall and dropped to ~50 at 2 cm |
| Readings changed when the board was touched | Touching the IR circuit made the readings jump | Probably loose component legs on the breadboard | <!-- TODO: confirm what we fixed --> |
