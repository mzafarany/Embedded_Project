#include "mbed.h"

// --- Serial Configuration ---
BufferedSerial pc(USBTX, USBRX, 9600); 

// --- Hardware Configuration ---
InterruptIn echo(D2);          
DigitalOut  trigger(D3);       

PwmOut      left_motors_PWM(D5);   
DigitalOut  left_IN1(D7);          
DigitalOut  left_IN2(D4);          

PwmOut      right_motors_PWM(D6);  
DigitalOut  right_IN3(D11);         
DigitalOut  right_IN4(D8);        

// --- Global Variables ---
Timer pulseTimer;
volatile float distance_cm = 0;

// Lower these constants to slow the robot down
const float SPEED_FAR  = 0.35f;  // Was 0.7
const float SPEED_NEAR = 0.20f;  // Was 0.4
const float DIST_TARGET = 25.0f; 
const float DIST_SAFE   = 12.0f; 

void onEchoRise() {
    pulseTimer.reset();
    pulseTimer.start();
}

void onEchoFall() {
    pulseTimer.stop();
    distance_cm = (pulseTimer.read_us() * 0.0343f) / 2.0f;
}

void driveRobot(float speed) {
    left_IN1 = 1; left_IN2 = 0;
    right_IN3 = 1; right_IN4 = 0;
    left_motors_PWM = speed;
    right_motors_PWM = speed;
}

void stopRobot() {
    left_motors_PWM = 0;
    right_motors_PWM = 0;
}

int main() {
    echo.rise(&onEchoRise);
    echo.fall(&onEchoFall);
    
    left_motors_PWM.period(0.001f); 
    right_motors_PWM.period(0.001f);

    while(1) {
        // Clear trigger and pulse
        trigger = 0;
        wait_us(2);
        trigger = 1;
        wait_us(10);
        trigger = 0;

        // Printing as an integer to avoid the ".2f" formatting error
        int displayDist = (int)distance_cm;
        printf("Distance: %d cm\r\n", displayDist);

        if (distance_cm < DIST_SAFE || distance_cm > 400.0f) { 
            // Stop if too close OR if sensor reads out of range (400cm+)
            stopRobot();
        } 
        else if (distance_cm > 50.0f) {
            driveRobot(SPEED_FAR); 
        }
        else if (distance_cm > DIST_TARGET) {
            driveRobot(SPEED_NEAR); 
        }
        else {
            stopRobot();      
        }

        ThisThread::sleep_for(100ms); 
    }
}