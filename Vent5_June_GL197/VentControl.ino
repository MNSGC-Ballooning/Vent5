// ************************************************ Function For Permanent Vent Opening + Both Pre-Venting Functions *************************************************
// ****************************************** Also Includes Functions to Open, Close Vent, and Monitor the Servo's Position ******************************************



void VentToFloat(unsigned long currTimeS, double avg_ascent_rate) {

  if (!AlreadyStartedVentToFloat) {
    AlreadyStartedVentToFloat = true;
    Serial.println("|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
    Serial.println("Starting The Vent to float");
    Serial.println("Current Altitude: " + String(altFeet));
    Serial.println("Current Time(currTimeS): " + String(currTimeS));
    Serial.println("Time since 5kft: " + String(timeSince5kFeetS));
    Serial.println("Time before flight: " + String(timeBeforeFlight));
    Serial.println("|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
    ventToFloatTime = currTimeS;
    openVent();
    ventReason = "Venting to float";
  }
  if (!AlreadyFinishedVentToFloat) {
    if (currentState == FLOAT || avg_ascent_rate <= 0.25) {
      AlreadyFinishedVentToFloat = true;
      finishVentToFloat = currTimeS;
      closeVent();
      if (avg_ascent_rate <= 0.25) {
        ventReason = "Finished venting avg ascent rate <= 0.25";
      }
      else {
        ventReason = "Finished venting because we are in float state";
      }
      Serial.println("|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
      Serial.println("Finished The Vent to float");
      Serial.println("Length of Venting: " + String(finishVentToFloat - ventToFloatTime));
      Serial.println("Current Avg Ascent Rate: " + String(avg_ascent_rate));
      Serial.println("|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
    }
  }
}//end of vent to float statement





// ***********************************************************Big Vent******************************************************
// ***********************************************************Big Vent******************************************************

// Function for main Vent opening (Big vent opening to reach target ascent rate
void bigVent(unsigned long currTimeS, double avg_ascent_rate) {
  //no more venting function checks if venting is allowed. venting not allowed if average_ascent_rate is below 1
  isVentingAllowed();

  //if not already been inside this function then calculated estimate times required to vent and save them, then open the vent
  if (AlreadyStartedBigVent == false) {
    AlreadyStartedBigVent = true;
    Serial.println("{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{");
    Serial.println("Starting BigVent");
    Serial.println("Current Altitude: " + String(altFeet));
    Serial.println("Current Time(currTimeS): " + String(currTimeS));
    Serial.println("Time since 5kft: " + String(timeSince5kFeetS));
    Serial.println("Time before flight: " + String(timeBeforeFlight));
    bigVentTimeS = currTimeS; //records the time that the big vent begun
    beforeBigVentRate = avg_ascent_rate;
    Serial.println("Current Avg Ascent Rate: " + String(beforeBigVentRate));
    Serial.println("{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{");
    openVent();
    ventReason = "Open for Big Vent at " + String(altFeet) + "ft";
  }

  if (AlreadyFinishedBigVent == false) {
    if ((beforeBigVentRate - avg_ascent_rate) >= 1 || avg_ascent_rate <= targetAscentRate) {
      AlreadyFinishedBigVent = true; //stop venting now
      finishBigVentTimeS = currTimeS;
      closeVent();
      if (avg_ascent_rate >= targetAscentRate) {
        ventReason = "Close for big vent b/c avg ascent rate dropped by 1";
      }
      else {
        ventReason = "Close for big vent b/c target ascent rate reached";
      }
      Serial.println("{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{");
      Serial.println("Finished Big Vent");
      Serial.println("Length of Venting: " + String(finishBigVentTimeS - bigVentTimeS));
      Serial.println("Before Avg Ascent Rate: " + String(beforeBigVentRate));
      Serial.println("Current Avg Ascent Rate: " + String(avg_ascent_rate));
      Serial.println("{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{");
    }
  }

}//end of bigVent statement


/* Here is the call for an estimated time that is required for the big vent when running
   //at the beginning of this prevent find the estimated time needed to vent using the following function
  //need to change the postVentAveAscentRate variable to one of the aveAscentRateAt#s
  secondsToVent(beforeRate, avgAscentRateAt120PV1, average_ascent_rate, targetAscentRateAfterBigVent, preventLengthS1);
*/

// *********************************************************** Pre-Venting Functions ***********************************************************
// *********************************************************** Pre-Venting Functions ***********************************************************

// Function to (hopefully) increase our balloon's altitude ceiling, so that we don't prematurely burst! Commented pieces of logic aren't entirely "fleshed-out"
void PreVenting1(unsigned long currTimeS, double avg_ascent_rate) //Two pieces of logic are included here; one is commented out. The first is a timed vent opening, the 2nd is based on ascent rate.
{
  isVentingAllowed(); //checks if ascent rate is below 2.5
  if (!AlreadyStartedPreVenting1) //if havent already started preventing
  {
    AlreadyStartedPreVenting1 = true;
    Serial.println("Starting first pre-vent");
    Serial.println("Current Altitude: " + String(altFeet));
    Serial.println("Current Time(currTimeS): " + String(currTimeS));
    Serial.println("Time since 5kft: " + String(timeSince5kFeetS));
    Serial.println("Time before flight: " + String(timeBeforeFlight));
    preVentingTimeS1 = currTimeS;
    beforeRatePV1 = avg_ascent_rate; //saving before venting average ascent rate
    openVent();
    ventReason = "First pre-vent";
    timeAtPreVenting1S = currTimeS;
  }
  if (!AlreadyFinishedPreVenting1 && (preVentingTimeS1  + preventLengthS1 < currTimeS)) //condition happens if its been enough time that preventing should end, change preventLengthS1
  {
    AlreadyFinishedPreVenting1 = true;
    closeVent();
    finishventTimeS1 = currTimeS;
    ventReason = "Closed after first pre-vent";
    Serial.println("Completing first pre-vent");
    Serial.println("Length of actual Venting: " + String(finishventTimeS1 - preVentingTimeS1));
    Serial.println("Length told to prevent: " + String(preventLengthS1));
  }
  //function that if already finished venting, records the average ascent rate at 30s intervals for calculations in prevent 2
  avgAscentRateAfterVenting(AlreadyFinishedPreVenting1, finishventTimeS1, currTimeS, avgAscentRateAt30PV1, avgAscentRateAt60PV1, avgAscentRateAt90PV1, avgAscentRateAt120PV1);


} //end of prevent 1 function

// Second Function to (hopefully) increase our balloon's altitude ceiling, so that we don't prematurely burst!
void PreVenting2(unsigned long currTimeS, double avg_ascent_rate) //Two pieces of logic are included here; one is commented out. The first is a timed vent opening, the 2nd is based on ascent rate.
{
  isVentingAllowed(); //check is ascent rate avg is less than 2.5
  if (!AlreadyStartedPreventing2) //if not already started second prevent
  {
    AlreadyStartedPreventing2 = true;
    Serial.println("Starting second pre-vent");
    Serial.println("Current Altitude: " + String(altFeet));
    Serial.println("Current Time(currTimeS): " + String(currTimeS));
    Serial.println("Time since 5kft: " + String(timeSince5kFeetS));
    Serial.println("Time before flight: " + String(timeBeforeFlight));
    preVentingTimeS2 = currTimeS;
    beforeRatePV2 = avg_ascent_rate;
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

  //function that if already finished venting, records the average ascent rate at 30s intervals from PV2 for calculations in big vent time to open
  avgAscentRateAfterVenting(AlreadyFinishedPreventing2, finishventTimeS2, currTimeS, avgAscentRateAt30PV2, avgAscentRateAt60PV2, avgAscentRateAt90PV2, avgAscentRateAt120PV2);

}

//************************************************************HELPER FUNCTIONS THAT NO NEW PREVENTING MATH*******************************
void avgAscentRateAfterVenting(bool ventingIsFinished, int timeEndedVenting, int currTimeS , float &avgAt30, float &avgAt60, float &avgAt90, float &avgAt120) {
  //making sure the venting if not currently happening
  if (ventingIsFinished == true) {

    switch (currTimeS - timeEndedVenting) { //assuming that we will get good timestamps for 30,60,90,120 seconds
      case (30):
        avgAt30 = avg_ascent_rate; //average ascent rate was calculated from last 10 acsnet rates (20s)
        Serial.println("average rate after venting at 30s");
        break;

      case (60):
        avgAt60 = avg_ascent_rate; //average ascent rate was calculated from last 10 acsnet rates (20s)
        Serial.println("average rate after venting at 60s");
        break;

      case (90):
        avgAt90 = avg_ascent_rate; //average ascent rate was calculated from last 10 acsnet rates (20s)
        Serial.println("average rate after venting at 90s");
        break;

      case (120):
        avgAt120 = avg_ascent_rate; //average ascent rate was calculated from last 10 acsnet rates (20s)
        Serial.println("average rate after venting at 120s: " + String(avgAt120));
        break;
    }//end of switch

  }//end of venting is finished if statement
}//end of avgascentrate function


///<<<<<<<<<<<<<<<<<<<------------------- Funciton that finds the calculated seconds needed to vent to get the desired ascent rate

//this function shoudl be ran before prevent 2 and big vent functions uses the amout the previous vent reduced the ascent rate and the current ascent rate to get a estimate for the time required to
//vent the next time
int secondsToVent(float beforeVentAvgAscentRate, float postVentAvgAscentRate, float currentAvgAscentRate, float targetAscentRate, int lengthOfLastVent) {
  //finding the reduction of ascentRate per seconds of venting

  float ascentRateReductionPerSecond = (postVentAvgAscentRate - beforeVentAvgAscentRate) / float(lengthOfLastVent);
  Serial.println("ascent rate reduction per second :" + String(ascentRateReductionPerSecond));

  //finding seconds to vent in the next vent to reach required. this is staying an int and not rounding to air on the side of venting less than needed rather than more

  int lengthOfNextVentS = (targetAscentRate - currentAvgAscentRate) / (ascentRateReductionPerSecond);
  Serial.println("lengthOfNextVentS :" + String(lengthOfNextVentS));
  return lengthOfNextVentS;
}

//isVentingAllowed() function. checks if the averave ascent rate is below 1 if true sets all venting variables to true
void isVentingAllowed() {
  if (avg_ascent_rate <= 1) {
    Serial.println("No more Venting Allowed because ascent Rate is to low ( <=1 )");
    ventReason = "No Venting, aveAR <= 1";

    AlreadyStartedPreVenting1 = true;
    AlreadyFinishedPreVenting1 = true;

    AlreadyStartedPreventing2 = true;
    AlreadyFinishedPreventing2 = true;

    AlreadyStartedBigVent = true;
    AlreadyFinishedBigVent = true;

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
  if (servoSetupFinished == false) {
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
