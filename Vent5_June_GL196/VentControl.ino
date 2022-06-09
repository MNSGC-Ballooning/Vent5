// ************************************************ Function For Permanent Vent Opening + Both Pre-Venting Functions *************************************************
// ****************************************** Also Includes Functions to Open, Close Vent, and Monitor the Servo's Position ******************************************

// ***********************************************************One-Time Permanent Opening******************************************************
// ***********************************************************One-Time Permanent Opening******************************************************

// Function for main Vent opening (permanent opening)

void oneTimeOpen(unsigned long currTimeS, float lon) // Aim for slow descent
{
  if (!AlreadyReachedPermanentOpening) //Only goes through this statement once. Starts the timer for balloon descent / termination. Else statement not required since the vent won't be closing
  {
    openVent();
    AlreadyReachedPermanentOpening = true;
    ventReason = "Permanent vent opening";
    timeAtPermanentOpening = currTimeS;
  }
  if (currentState == DESCENT) {
    Serial.println("///////////////////////////// VENT HAS TRIED TO CLOSE VENT/////////////////////////////");
    closeVent();
    ventReason = "Closed since Descent has been reached after final opening";
  }
  //Ascent / Slow ascent case - If the balloon vent has been open for 10 minutes and is in ascent / slow ascent rather than slow descent, terminate the balloon.
  if (timeAtPermanentOpening + 600 < currTimeS && (currentState == SLOW_ASCENT || currentState == ASCENT)) {
    terminate();
    cutterState = "Released because balloon failed to descend after final opening (Ascent case)";
  }
  //Float case - If the balloon vent has been open for 10 minutes and is in float, try venting another 10 minutes. If that fails, cut
  if (timeAtPermanentOpening + 1200 < currTimeS && currentState == FLOAT) {
    terminate();
    cutterState = "Released because balloon failed to descend after final opening (Float case)";
  }
  //Descent / Slow descent case - If the balloon is in either of these 2 states, carry on for 30 minutes, then cut
  if (timeAtPermanentOpening + 1800 < currTimeS && (currentState == SLOW_DESCENT || currentState == DESCENT)) {
    terminate();
    cutterState = "Released because balloon was in descent for 30 minutes after the final vent opening";
  }
}


// *********************************************************** Pre-Venting Functions ***********************************************************
// *********************************************************** Pre-Venting Functions ***********************************************************

// Function to (hopefully) increase our balloon's altitude ceiling, so that we don't prematurely burst! Commented pieces of logic aren't entirely "fleshed-out"
void PreVenting1(unsigned long currTimeS, double avg_ascent_rate) //Two pieces of logic are included here; one is commented out. The first is a timed vent opening, the 2nd is based on ascent rate.
{
  if (!AlreadyStartedPreVenting1)
  {
    AlreadyStartedPreVenting1 = true;
    Serial.println("Starting first pre-vent");
    Serial.println("Current Altitude: " + String(altFeet));
    Serial.println("Current Time(currTimeS): " + String(currTimeS));
    Serial.println("Time since 5kft: " + String(timeSince5kFeetS));
    Serial.println("Time before flight: " + String(timeBeforeFlight));
    preVentingTimeS1 = currTimeS;
    beforeRate = ascent_rate;
    openVent();
    ventReason = "First pre-vent";
    timeAtPreVenting1S = currTimeS;2
  }
  if (!AlreadyFinishedPreVenting1 && (preVentingTimeS1 + preventLengthS1 < currTimeS))
  {
    AlreadyFinishedPreVenting1 = true;
    closeVent();
    finishventTimeS1 = currTimeS;
    roundsOfVenting1++;
    ventReason = "Closed after first pre-vent";
    Serial.println("Completing first pre-vent");
    Serial.println("Rounds Of Venting: " + String(roundsOfVenting1));
    Serial.println("Length of Venting: " + String(finishventTimeS1 - preVentingTimeS1));
  }
  if (roundsOfVenting1 < 2 && AlreadyFinishedPreVenting1) {
    Serial.println("Calculating the ascent rate reduction... ");
    reachedDesiredRateReduction1 = ascentRateReached(beforeRate, ascent_rate, desieredRateReduction);

    if (!reachedDesiredRateReduction1) {
      AlreadyStartedPreVenting1 = false;
      AlreadyFinishedPreVenting1 = false;
      PreVenting1(currTimeS, avg_ascent_rate);
    }
    else {
      roundsOfVenting1 = 10;
    }
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
  if (moreVent == false && !AlreadyStartedPreventing2) {
    //this is the case that the first vent has slowed the vent down beyond 50% of initial ascent rate and we dont want to vent anymore
    preventLengthS2 = 0;
    Serial.println("More Venting: " + String(moreVent));
    ventReason = "Ascent rate too slow to vent";
  }
  if (reachedDesiredRateReduction1 && moreVent) {
    preventLengthS2 = 30;
  }
  else if (!reachedDesiredRateReduction1 && moreVent) {
    preventLengthS2 = 60;
  }

  if (!AlreadyStartedPreventing2)
  {
    AlreadyStartedPreventing2 = true;
    Serial.println("Starting second pre-vent");
    Serial.println("Current Altitude: " + String(altFeet));
    Serial.println("Current Time(currTimeS): " + String(currTimeS));
    Serial.println("Time since 5kft: " + String(timeSince5kFeetS));
    Serial.println("Time before flight: " + String(timeBeforeFlight));
    preVentingTimeS2 = currTimeS;
    beforeRate = ascent_rate;
    openVent();
    ventReason = "Second pre-vent";
    //    if(preVentingTimeS2 + 60 < currTimes){
    //      closeVent();
    //    }
    timeAtPreventing2S = currTimeS;
  }
  else if ((preVentingTimeS2 + preventLengthS2 < currTimeS) && !AlreadyFinishedPreventing2) //Shut after we have slowed down to slow descent or have gone past 100k feet
  {
    closeVent();
    ventReason = "Closed after second pre-vent";
    AlreadyFinishedPreventing2 = true;
    finishventTimeS2 = currTimeS;
    roundsOfVenting2++;

    Serial.println("Completing second pre-vent");
    Serial.println("Rounds Of Venting: " + String(roundsOfVenting2));
    Serial.println("Length of Venting: " + String(finishventTimeS1 - preVentingTimeS1));
  }
  if (roundsOfVenting2 < 2 && AlreadyFinishedPreventing2) {
    //bool that checks if the desired ascent rate for this vent has been reached. if not it will vall prevent 2 again
    Serial.println("Calculating the ascent rate reduction... ");
    reachedDesiredRateReduction2 = ascentRateReached(beforeRate, ascent_rate, desieredRateReduction);
    if (!reachedDesiredRateReduction2) {
      AlreadyStartedPreventing2 = false;
      Serial.println("I'm Alseasdfg");
      AlreadyFinishedPreventing2 = false;
      PreVenting2(currTimeS, avg_ascent_rate);
    }
    else {
      roundsOfVenting2 = 10;
    }
  }


}

// **************************************************************************HELPER FUNCTIONS FOR SERVO CONTROL AND POSITION MONITORING***********************************************************
// **************************************************************************HELPER FUNCTIONS FOR SERVO CONTROL AND POSITION MONITORING***********************************************************

// Function to open the vent (simply command the servo to rotate to the proper position)
void openVent() {
  commandedServoPosition = openServo;
  ventServo.write(openServo);
  
  //releaseServo.write(0);
  flapperState = "Open";

//  jiggle(servoMinPos); // servoMinPos is the minimum position in degrees that the servo can move to as found in the first loop of system update
  
  if (emulationCheck == true && ventSent == false) { //bool condition for communication to emulator ////NEW
    Serial5.print("<RATE" + String(avg_ascent_rate) + ">");
    delay(5);
    Serial5.print("<VENT1>");                 //communicating with the emulator that the vent has been opened
    ventSent = true;
  }
  Serial.println("commandedServoPosition: " + String(commandedServoPosition) + "    ServoPosition: " + String(servoPos()));
}

// Function to close the vent (simply command the servo to rotate to the proper position)
void closeVent() {
  commandedServoPosition = closeServo;
  ventServo.write(closeServo);
  
  //releaseServo.write(0);
  flapperState = "Closed";

  //if closeVent is called from system setup then make sure that the close vent degrees are the same
  if(servoSetupFinished == false){
    Serial.println("commandedServoPosition: " + String(commandedServoPosition) + "    ServoPosition: " + String(servoPos()));
  }

 // jiggle(servoMaxPos); //calling jiggle function, servoMaxPos is the maximum position in degrees that the servo can move to as found in the first loop of system update

  if (emulationCheck == true && ventSent == true) { //bool condition for communication to emulator   ////NEW
    Serial5.print("<VENT0>");                      //telling emulator that the vent has been closed
    ventSent = false;
  }
 
}

/////////////////////////////////////////////////////////////////////// THE JIGGLE FUNCTION dun dun dun /////////////////////////////////////////////////////
/* Function to jiggle the servo if the servo is not within a specified range (Currently +/- 40 degrees) ie it got stuck for some reason
Keep in mind: The range of +/- 40 is because there is not currently a fix for the servoPosition discrepency issue. possibly this is caused by calibration issue of output voltage when 
getting the servoPos values. has not been resolved.

*/

void jiggle(int desiredServoPosition) { 
  Serial.println("GOING INTO JIGGLE");
 
  //starting jiggle timer and setting jiggletimerStarted to true
  if (!jiggleTimerStarted) {
    jiggleTimerStarted = true;
    jiggleTimer = currTimeS;
  }

  if (jiggleTimer + 5 < currTimeS && jiggleTimerStarted) //checks to see if has been jiggling for 5 seconds, if not yet 5 then continue
  {
    Serial.println("Checking servoPos: " + String(servoPos()) + "     Checking desiredServoPosition: " + String(desiredServoPosition));

    if ((servoPos() < (desiredServoPosition - 20) || servoPos() > (desiredServoPosition + 20)) && jiggleTimer + 5 < currTimeS)
      //above if statement checks if the servo is more than 40 degrees away from commandedServoPosition (132) --> closedServo is a variabel that can differ for each mechanisim
    { //The servo is out of position- this attempts to "jiggle" the servo back into position. Delays are used since this takes priority
      Serial.println("############Jiggle delay jiggle delay ##################\nServo Pos: " + String(servoPos()));
      
      ventServo.write(desiredServoPosition - 10); //the "jiggling doesnt appear for closeVent because it does minus first
      delay(500);
      ventServo.write(desiredServoPosition + 10);
      delay(500);
      ventServo.write(desiredServoPosition - 10);
      delay(500);
      ventServo.write(desiredServoPosition + 10);
      delay(500);
      ventServo.write(desiredServoPosition);
    }
    ventServo.write(desiredServoPosition);
    jiggleTimerStarted = false;
  }
}
/////////////////////////////////////////////////////////////////////// END OF JIGGLE FUNCTION /////////////////////////////////////////////////////

//function to check if the desired ascent rate has been reached.
//inputs are ascent rate befeor venting (before rate) the current ascent rate, and desired ascent rate reduction in percentage
//output is a bool telling if the current ascent rate is <= the desired ascent rate
bool ascentRateReached(double beforeRate, double currentRate, double desiredRateReduction) { //NOTE: THE CURRENT RATE MIGHT WANT TO BE THE AVERAGE ASCENT RATE
  bool outputBool;
  //calculating percentage that prevent ascentrate has dropped since beginning of prevent 1 sequence
  double actualAscentRateReduction = 100  - ((currentRate / beforeRate) * 100);
  Serial.println("Ascent Rate Reduction: " + String(actualAscentRateReduction));

  //checking if current ascent rate has sufficiently been reduced
  if (actualAscentRateReduction >= desiredRateReduction) {
    outputBool = true;
    if (actualAscentRateReduction > 50) { //if the actual ascent rate reduction is more than half you dont need to vent any more
      moreVent = false; //this bool says whether the Vent should be allowed to do any more prevent or big venting
    }
  }
  else {
    outputBool = false;
  }

  return outputBool;
}







// Function to obtain the current position of the servo via its analog feedback pin (might need some work)
int servoPos() // obtain the position of the servo through the analog pin
{
  int val = analogRead(FEEDBACK_PIN);            // Feedback is pin A2 on the current Teensy 3.5 / PCB setup - reads the value of the potentiometer (value between 0 and 1023)
  val = map(val, 0, 16383, 0, 179);       // scale it to use it with the servo (value between 0 and 180)
  return val;
}
