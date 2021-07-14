// ************************************************ Function For Permanent Vent Opening + Both Pre-Venting Functions *************************************************
// ****************************************** Also Includes Functions to Open, Close Vent, and Monitor the Servo's Position ******************************************

// ***********************************************************One-Time Permanent Opening******************************************************
// ***********************************************************One-Time Permanent Opening******************************************************

// Function for main Vent opening (permanent opening)
void oneTimeOpen(unsigned long currTimeS, float lon) // Aim for slow descent
{
  if(!AlreadyReachedPermanentOpening) //Only goes through this statement once. Starts the timer for balloon descent / termination. Else statement not required since the vent won't be closing
  {
    openVent();
    AlreadyReachedPermanentOpening = true;
    ventReason = "Permanent vent opening";
    timeAtPermanentOpening = currTimeS;
  }
  if(currentState == DESCENT){
    closeVent();
    ventReason = "Closed since Descent has been reached after final opening";
  }
  //Ascent / Slow ascent case - If the balloon vent has been open for 10 minutes and is in ascent / slow ascent rather than slow descent, terminate the balloon.
  if(timeAtPermanentOpening + 600 < currTimeS && (currentState == SLOW_ASCENT || currentState == ASCENT)){
    terminate();
    cutterState = "Released because balloon failed to descend after final opening (Ascent case)";
  }
  //Float case - If the balloon vent has been open for 10 minutes and is in float, try venting another 10 minutes. If that fails, cut
  if(timeAtPermanentOpening + 1200 < currTimeS && currentState == FLOAT){
    terminate();
    cutterState = "Released because balloon failed to descend after final opening (Float case)";
  }
  //Descent / Slow descent case - If the balloon is in either of these 2 states, carry on for 30 minutes, then cut
  if(timeAtPermanentOpening + 1800 < currTimeS && (currentState == SLOW_DESCENT || currentState == DESCENT)){
    terminate();
    cutterState = "Released because balloon was in descent for 30 minutes after the final vent opening";
  }
}

// *********************************************************** Pre-Venting Functions ***********************************************************
// *********************************************************** Pre-Venting Functions ***********************************************************

// Function to (hopefully) increase our balloon's altitude ceiling, so that we don't prematurely burst! Commented pieces of logic aren't entirely "fleshed-out"
void PreVenting1(unsigned long currTimeS, double avg_ascent_rate) //Two pieces of logic are included here; one is commented out. The first is a timed vent opening, the 2nd is based on ascent rate.
{
  if(!AlreadyStartedPreVenting1)
  {
    AlreadyStartedPreVenting1 = true;
    Serial.println("Starting first pre-vent");
    preVentingTimeS1 = currTimeS;
    openVent();
    ventReason = "First pre-vent";
    timeAtPreVenting1S = currTimeS;
  }
  if(!AlreadyFinishedPreVenting1 && (preVentingTimeS1 + 60 < currTimeS))
  {
    AlreadyFinishedPreVenting1 = true;
    closeVent();
    ventReason = "Closed after first pre-vent";
    Serial.println("Completing first pre-vent");
  }
  
//  else if((avg_ascent_rate < 3.802457252010) && !AlreadyFinishedPreVenting1)
//  {
//    closeVent();
//    AlreadyFinishedPreVenting1 = true;
//    Serial.println("Completing first pre-vent");
//  }

}

// Second Function to (hopefully) increase our balloon's altitude ceiling, so that we don't prematurely burst!
void PreVenting2(unsigned long currTimeS, double avg_ascent_rate) //Two pieces of logic are included here; one is commented out. The first is a timed vent opening, the 2nd is based on ascent rate.
{
  if(!AlreadyStartedPreventing2)
  {
    AlreadyStartedPreventing2 = true;
    Serial.println("Starting second pre-vent");
    preVentingTimeS2 = currTimeS;
    openVent();
    ventReason = "Second pre-vent";
//    if(preVentingTimeS2 + 60 < currTimes){
//      closeVent();
//    }
    timeAtPreventing2S = currTimeS;
  }
  else if((currentState == SLOW_ASCENT || altFeet > 100000) && !AlreadyFinishedPreventing2) //Shut after we have slowed down to slow descent or have gone past 100k feet
  {
    closeVent();
    ventReason = "Closed after second pre-vent";
    AlreadyFinishedPreventing2 = true;
    Serial.println("Completing second pre-vent");
  }
}

// **************************************************************************HELPER FUNCTIONS FOR SERVO CONTROL AND POSITION MONITORING***********************************************************
// **************************************************************************HELPER FUNCTIONS FOR SERVO CONTROL AND POSITION MONITORING***********************************************************

// Function to open the vent (simply command the servo to rotate to the proper position)
void openVent() {
  ventServo.write(20);
  //releaseServo.write(0);
  flapperState = "Open";
  if(!jiggleTimerStarted) // Timer used to pace the servo before the if-statement. This way we can read the actual servo position
  {
    jiggleTimerStarted = true;
    jiggleTimer = currTimeS;
  }
  if(jiggleTimer + 5 < currTimeS && jiggleTimerStarted)
  {
    if(servoPos() < -30 || servoPos() > 70) //The servo is out of position- this attempts to "jiggle" the servo back into position. Delays are used since this takes priority
    { 
        ventServo.write(10);
        delay(500);
        ventServo.write(30);
        delay(500);
        ventServo.write(10);
        delay(500);
        ventServo.write(30);
        delay(500);
        ventServo.write(20);
      }
    ventServo.write(20);
    jiggleTimerStarted = false;
  }
}

// Function to close the vent (simply command the servo to rotate to the proper position)
void closeVent() {
  ventServo.write(135);
  //releaseServo.write(0);
  flapperState = "Closed";
  if(!jiggleTimerStarted){
    jiggleTimerStarted = true;
    jiggleTimer = currTimeS;
  }
  if(jiggleTimer + 5 < currTimeS && jiggleTimerStarted)
  {
    if((servoPos() < 85 || servoPos() > 185) && jiggleTimer + 5 < currTimeS) //The servo is out of position- this attempts to "jiggle" the servo back into position. Delays are used since this takes priority
    { 
        ventServo.write(125);
        delay(500);
        ventServo.write(145);
        delay(500);
        ventServo.write(125);
        delay(500);
        ventServo.write(145);
        delay(500);
        ventServo.write(135);
      }
    ventServo.write(135);
    jiggleTimerStarted = false;
  }
}

// Function to obtain the current position of the servo via its analog feedback pin (might need some work)
int servoPos() // obtain the position of the servo through the analog pin
{
  int val = analogRead(FEEDBACK_PIN);            // Feedback is pin A2 on the current Teensy 3.5 / PCB setup - reads the value of the potentiometer (value between 0 and 1023)
  val = map(val, 0, 16383, 0, 179);     // scale it to use it with the servo (value between 0 and 180)
  return val;
}
