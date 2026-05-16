#include "mbed.h"

// -------------------- Serial Configuration --------------------
BufferedSerial pc(USBTX, USBRX, 9600);   // USB serial for debugging

// -------------------- Ultrasonic Sensor -----------------------
InterruptIn echo(D2);          // Echo pin (input, interrupt-driven)
DigitalOut  trigger(D3);       // Trigger pin (output)

// -------------------- Motor Driver Pins -----------------------
PwmOut      left_motors_PWM(D5);   
DigitalOut  left_IN1(D7);          
DigitalOut  left_IN2(D4);          

PwmOut      right_motors_PWM(D6);  
DigitalOut  right_IN3(D11);         
DigitalOut  right_IN4(D8);        

// -------------------- Global Variables ------------------------
Timer pulseTimer;              // Timer to measure echo pulse width
volatile float distance_cm = 0; // Latest measured distance

// -------------------- Constants -------------------------------
const float SPEED_FAR   = 0.35f;   // Speed when object is far
const float SPEED_NEAR  = 0.20f;   // Speed when object is near
const float DIST_TARGET = 25.0f;   // Desired following distance (cm)
const float DIST_SAFE   = 12.0f;   // Minimum safe distance (cm)

// -------------------- Interrupt Service Routines --------------
void onEchoRise() {
    pulseTimer.reset();
    pulseTimer.start();
}

void onEchoFall() {
    pulseTimer.stop();
    // Convert pulse width (µs) to distance (cm)
    distance_cm = (pulseTimer.read_us() * 0.0343f) / 2.0f;
}

// -------------------- Motor Control Functions -----------------
void driveRobot(float speed) {
    // Forward motion
    left_IN1 = 1; left_IN2 = 0;
    right_IN3 = 1; right_IN4 = 0;
    left_motors_PWM = speed;
    right_motors_PWM = speed;
}

void stopRobot() {
    // Stop motors
    left_motors_PWM = 0;
    right_motors_PWM = 0;
}

// -------------------- Main Program ----------------------------
int main() {
    // Attach interrupts to echo pin
    echo.rise(&onEchoRise);
    echo.fall(&onEchoFall);
    
    // Configure PWM frequency for motors
    left_motors_PWM.period(0.001f); 
    right_motors_PWM.period(0.001f);

    while (1) {
        // Trigger ultrasonic sensor
        trigger = 0;
        wait_us(2);
        trigger = 1;
        wait_us(10);
        trigger = 0;

        // Print distance (integer format for simplicity)
        int displayDist = (int)distance_cm;
        printf("Distance: %d cm\r\n", displayDist);

        // Decision logic based on distance
        if (distance_cm < DIST_SAFE || distance_cm > 400.0f) { 
            stopRobot();   // Stop if too close or out of range
        } 
        else if (distance_cm > 50.0f) {
            driveRobot(SPEED_FAR);   // Move faster if object is far
        }
        else if (distance_cm > DIST_TARGET) {
            driveRobot(SPEED_NEAR);  // Move slower if object is near
        }
        else {
            stopRobot();   // Stop if within target distance
        }

        // Small delay before next measurement
        ThisThread::sleep_for(100ms); 
    }
}
