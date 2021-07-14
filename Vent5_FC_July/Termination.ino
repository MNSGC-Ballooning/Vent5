// *****************************Main Termination Function + Helper Functions************************************

// ************************************************MAIN FUNCTION**************************************

// Function to release the vent from the balloon; we currently use two 10-Ohm Resistors and L9110H H-drivers to cut the string and release the vent from the balloon neck
// Overall strategy: burn each resistor individually and not at the same time; burn resistor-1 for 7 seconds, then burn resistor-2 for 7 seconds, then R1 for 30 seconds, then R2 for 30 seconds, then stop.
void terminate()
{
  
  currTimeS = millis()/1000; // Resetting currTimeS just in case it's off by a certain amount

    
  if(!terminationBegun){
    terminationStartTimeS = currTimeS;
    terminationBegun = true;
  }

  if(terminationStartTimeS + 20 < currTimeS) //20 second countdown until termination begins- this gives us some time to appropriately flash LEDs and make us aware of termination
  {
    if(!Res1Burned)
    {
      if(!Res1on)
      {
        burnTime1StartS = currTimeS;
      }
      burnResistor();
      if(currTimeS-burnTime1StartS>7.9)
      {
        stopBurn();
        Res1Burned = true;
      }
    }
    else if(!Res1Burned2)
    {
      if(!Res1on)
      {
        burnTime1StartS = currTimeS;
      }
      burnResistor();
      if(currTimeS-burnTime1StartS>35)
      {
        stopBurn();
        Res1Burned2 = true;
      }
    }
    //Re2Burned and Res2Burned2 checks have been taken out to allow for ease of burning (This is an EMERGENCY cut- we aren't making it too difficult. Though they may be added back in)
    else if(((currTimeS - burnTime1StartS) > 240) && (pressureAltFeet > 2000) && (currentState != DESCENT)) //After 4min and above 2000ft, if the balloon has not reached descent speeds, the main line will be cut
    {
      if(!Res2on)
      {
        burnTime2StartS = currTimeS;
      }
      burnResistor2();
      if(currTimeS-burnTime2StartS>7.9)
      {
        stopBurn2();
        Res2Burned = true;
      }
    }
    else if(((currTimeS - burnTime1StartS) > 240) && (pressureAltFeet > 2000) && (currentState != DESCENT))
    {
      if(!Res2on) 
      {
        burnTime2StartS = currTimeS;
      }
      burnResistor2();
      if(currTimeS-burnTime2StartS>35)
      {
        stopBurn2();
        Res2Burned2 = true;
      }
    }
    if(Res2Burned && Res2Burned2 && (currentState == DESCENT || currentState == SLOW_DESCENT)){
      openVent(); //Only open the vent if the balloon has not reached ascent or slow ascent after the second resistor cut. We now intend for the balloon to burst to get us down.
      ventReason = "Permanent vent opening";
    }
  }
}


// *******************************HELPER FUNCTIONS**********************************

//Setup Resistor Cutter 1 and L9110H H-driver
void burnSetup()
{
  // Set the burn pins to output so they actually work
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  // Make sure that they're set to OFF
  digitalWrite(4, LOW); 
  digitalWrite(5, LOW); 
  //End Resistor Cutter Setup
}

// Function for initiating the burn of the resistor
void burnResistor()
{
  //digitalWrite(2, HIGH); //DON'T BURN BOTH HIGH AT ONCE! ONLY ONE OF THE TWO HIGH FOR BURNING!
  digitalWrite(4, HIGH); 
  Serial.println("BURNING RESISTOR NUMBER ONE!!!");
  cutterState = "Burning Res1";
  Res1on = true;
}

// function for manually stopping the burn of the resistor cutter
void stopBurn()
{
  digitalWrite(4, LOW); 
  digitalWrite(5, LOW); 
  cutterState = "Res1 Stopped Burning";
  Res1on = false;
}

//Setup Resistor Cutter 2 and L9110H H-driver
void burn2Setup()
{
  // Set the burn pins to output so they actually work
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  // Make sure that they're set to OFF
  digitalWrite(6, LOW); 
  digitalWrite(7, LOW); 
  //End Resistor Cutter Setup
}

// Function for initiating the burn of the resistor
void burnResistor2()
{
  Res2on = true;
  //digitalWrite(2, HIGH); //DON'T BURN BOTH HIGH AT ONCE! ONLY ONE OF THE TWO HIGH FOR BURNING!
  digitalWrite(6, HIGH); 
  Serial.println("RESISTOR 2 BURNING!!!!!");
  cutterState = "Burning Res2";
  Res2on = true;
}

// function for manually stopping the burn of the resistor cutter
void stopBurn2()
{
  digitalWrite(6, LOW); 
  digitalWrite(7, LOW); 
  cutterState = "Res2 Stopped Burning";
  Res2on = false;
}
