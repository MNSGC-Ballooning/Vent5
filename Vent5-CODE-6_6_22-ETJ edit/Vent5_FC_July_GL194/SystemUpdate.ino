// ****************************************************************************** MAIN LOOP + STATE MACHINE **************************************************************

// System data collection and state machine
void systemUpdate()
{
  // Flash the LED appropriately
  led(); // flash the LED in the desired pattern

 // \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ Updating Variables 'n' such \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ 
  //Thermistor / heater / baro updates
  updateThermistor();
  setHeaterState();
  updateBaro();
  pressureToAltitude();
  // updateRadio();

  // BELOW is necessary if you want to be able to send serial commands (not critical for flight, but super handy for testing)**********
  if (Serial.available()) { // need to constantly check for serial bytes/commands (as opposed to only every 2 seconds) so that's why this isn't inside the "if(currTime-prevTime>interval)" loop.
    serialcommand = Serial.readStringUntil('\n');
    Serial.print("You typed: " );
    Serial.println(serialcommand);
  }
  // ABOVE is necessary if you want to be able to send serial commands (not critical for flight, but super handy for testing)**********

  // Below: Check for GPS data, update buffer if data available

  // New GPS Update
  gps.update(); // refresh the GPS so it gives you new data

  //delay(3); // For old GPS library only
  // ^^^^^^^^^^ABOVE: Check for GPS data, update buffer if data available^^^^^^^^^^^^^

  // obtain the current time (in milliseconds)
  unsigned long currTime = millis(); 

  //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ //Begining 2 second loop \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ 

  if (currTime - prevTime > interval) // Interval is currently 2000 ms (or 2 seconds) as defined - this is the main loop/cycle
  {
    
    // **************Get Sensor Data and record it**********************
    prevTime = millis(); //setting prevTime to the current time --> this is for all functions that need every 2 seconds. not only ascent rate
    currTimeS = currTime / 1000; // obtain the current time in seconds (convert from milliseconds to seconds)
    if(FlightHasBegun){ //getting flight timer to record how long the balloon is considered to be flying because of the pressure decrease
      flightTime = currTimeS - timeBeforeFlight;
    }

    gpsHR = gps.getHour() - 5; //obtain time for GPS timestamp in SD card if GPS lock is available. Need minus 5 because of a time zone issue
    gpsMIN = gps.getMinute();
    gpsSEC = gps.getSecond();

    // Data line to log to the CSV file on our SD card
    message = String(flightTime) + "," + String(currTime) + "," + String(gpsHR) + ":" + String(gpsMIN) + ":" + String(gpsSEC) + "," + String(heaterTempValue) + "," + String(msPressure) + "," + String(pressureAltFeet); // the beginning of the message to be sent to our SD card. the ","'s (commas) are inserted as placeholders here since those variables weren't tracked/logged for this flight (see header to see which variables)

    // *******If GPS Lock is obtained, log that data (if statement), otherwise we resort to using the pressure sensor, which can only reliably track altitude to 60k feet************
    latitude = gps.getLat(); // obtain the latitude from the GPS
    lon = gps.getLon(); // obtain the longitude from the GPS
    altFeet = gps.getAlt_feet(); // obtain the altitude (in feet) from the GPS
    numSats = gps.getSats(); // obtain the number of satellites from the gps

    Serial.println("Calculating gps_ascent_rate, currTime: " + String(currTime) + "\tprev_time2: " + String(prev_time2));
    gps_ascent_rate = 0.3048 * (((altFeet - prev_Control_Altitude) / (currTime - prev_time2)) * 1000); // calculate ascent rate and convert it to m/s
    press_ascent_rate = 0.3048 * (((pressureAltFeet - prev_Press_Altitude) / (currTime - prev_time2)) * 1000); // calculate ascent rate and convert it to m/s
    
    if (GPS_LOCK == true) //if GPS lock is true then use GPS ascent rate calcultaed above, if not use pressure ascent rate for average ascent rate.
    {
      if (abs(altFeet - prev_Control_Altitude) > 1000) //if there is more than 1000ft of differance in the altitude hits claim bad hit
      {
        Serial.println("Bad GPS hit!");
        altFeet = prev_Control_Altitude + ascent_rate / 0.3048 * (currTime - prev_time2) / 1000;
      } 

      ascent_rate = gps_ascent_rate;

    }
    else // This else statement is only initiated if the GPS doesn't have a lock. We now resort to pressure sensor backup
    {
      // obtain the altitude (in feet) from the pressure sensor - yeah not set to altFeet

      ascent_rate = press_ascent_rate;

      //since no GPS hit setting numSats equal to zero
      numSats = 0;

    }

    //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ Adding new ascent rate into list of past 10 and calculating new average ascent rate //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
    
    avg_ascent_rate = 0; // reset before/after every loop, then calculated in the following for loop (averaged over the last 10 hits--or 20 seconds worth of data--to reduce noise)
      
      for (int i = 0; i < 10; i++) // for loop to sum the ascent rates. 
      {
        if (i < 9)
        {
          last_ten_ascent_rates[i] = last_ten_ascent_rates[i + 1];
          if (last_ten_ascent_rates[i] != 0)
          {
            avg_ascent_rate += last_ten_ascent_rates[i];
            
          }
        }
        if (i == 9)
        {
          last_ten_ascent_rates[i] = ascent_rate;
          if (last_ten_ascent_rates[i] != 0)
          {
            avg_ascent_rate += last_ten_ascent_rates[i];
            
          }
        }
      }
      avg_ascent_rate = avg_ascent_rate / 10; // sumed up the last 10 ascent rates now divide by 10 to obtain the average ascent rate for this cycle

      // *****Below is printing to the serial monitor, convinent if you're testing with the serial cable*****
      Serial.println("Average Ascent Rate (below) is:"); // printing to the serial monitor, convinent if you're testing with the serial cable
      Serial.println(avg_ascent_rate); // printing to the serial monitor, convinent if you're testing with the serial cable
      Serial.println("Average Ascent Rate Printed Above."); // printing to the serial monitor, convinent if you're testing with the serial cable
      Serial.println("Last Ten Ascent Rates (below) are:"); // printing to the serial monitor, convinent if you're testing with the serial cable
      Serial.println(String(last_ten_ascent_rates[0]) + "," + String(last_ten_ascent_rates[1]) + "," + String(last_ten_ascent_rates[2]) + "," + String(last_ten_ascent_rates[3]) + "," + String(last_ten_ascent_rates[4]) + "," + String(last_ten_ascent_rates[5]) + "," + String(last_ten_ascent_rates[6]) + "," + String(last_ten_ascent_rates[7]) + "," + String(last_ten_ascent_rates[8]) + "," + String(last_ten_ascent_rates[9]) + ",");
      Serial.println("Last Ten Ascent Rates Printed Above."); // printing to the serial monitor, convinent if you're testing with the serial cable
      // *****Above is printing to the serial monitor, convinent if you're testing with the serial cable*****

    //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ Adding GPS data to the 'message' string to put onto the SD card //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ 
    prev_time2 = millis();
    prev_Press_Altitude = pressureAltFeet;
    prev_Control_Altitude = altFeet; //prev_Control_Altitude is set to the pressure altitude
    GPSdata = "," + String(latitude, 4) + "," + String(lon, 4) + ","  + String(altFeet) + "," + String(numSats) + "," + String(ascent_rate) + "," + String(avg_ascent_rate)+ "," + String(gps_ascent_rate) + "," + String(press_ascent_rate); 
    message += GPSdata; // add that GPS data string to the overall SD log message

    
    // **********************DECLARE WHETHER THE GPS HAS HAD A RECENT LOCK OR NOT (with "recent" bring defined as 5 minutes--300 seconds--here)*****************************************
    if ((millis() - prev_time2) / 1000 > 300) // if more than 300 seconds (5 minutes) have passed since fresh GPS hits, declare that the GPS has lost its lock, for backup logic purposes
    {
      GPS_LOCKED == false;

    }
    else
    {
      GPS_LOCKED == true;

    }

    if (gps.getAlt_feet() == 0) {
      GPS_LOCK = false;
      Serial.println("GPS LOCKED IS FALSE FALSE FALSE");
    }
    else {
      GPS_LOCK = true;
      Serial.println("GPS LOCKED IS TRUE TRUE TRUE");
    }



    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************
    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************
    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************
    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************
    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************
    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************

    // Start Flight Timer once 5000 feet threshold is surpassed (for the backup flight cutter)
    if (GPS_LOCK == true)
    {
      if (altFeet > 5000 && fivekFeetReached == false) // 5000 feet is the threshold to make sure that it doesn't prematurely vent or terminate when it's on the ground
      { // basically this if statement is only called the first moment after you cross 5000 feet for the first time and
        // it sets the flight status as having begun which starts the backup flight timer and allows you to start running through termination and vent logic.
        // This function sets the initial values of "ascent_rate," "avg_ascent_rate," and "last_ten_ascent_rates" to 5 m/s so that it avoids accidentally tripping the termination logic early
        fivekFeetReached = true;
        timeSince5kFeetS = currTimeS;
        ascent_rate = 5;
        last_ten_ascent_rates[1] = 5; last_ten_ascent_rates[2] = 5; last_ten_ascent_rates[3] = 5; last_ten_ascent_rates[4] = 5; last_ten_ascent_rates[5] = 5;
        last_ten_ascent_rates[6] = 5; last_ten_ascent_rates[7] = 5; last_ten_ascent_rates[8] = 5; last_ten_ascent_rates[9] = 5; last_ten_ascent_rates[0] = 5;
        avg_ascent_rate = 5;
      }
    }
    else if (GPS_LOCK == false)
    {
      if (pressureAltFeet > 5000) // 5000 feet is the threshold to make sure that it doesn't prematurely vent or terminate when it's on the ground
      { // basically this if statement is only called the first moment after you cross 5000 feet for the first time and
        // it sets the flight status as having begun which starts the backup flight timer and allows you to start running through termination and vent logic.
        // This function sets the initial values of "ascent_rate," "avg_ascent_rate," and "last_ten_ascent_rates" to 5 m/s so that it avoids accidentally tripping the termination logic early
        timeSince5kFeetS = currTimeS;
        ascent_rate = 5;
        last_ten_ascent_rates[1] = 5; last_ten_ascent_rates[2] = 5; last_ten_ascent_rates[3] = 5; last_ten_ascent_rates[4] = 5; last_ten_ascent_rates[5] = 5;
        last_ten_ascent_rates[6] = 5; last_ten_ascent_rates[7] = 5; last_ten_ascent_rates[8] = 5; last_ten_ascent_rates[9] = 5; last_ten_ascent_rates[0] = 5;
        avg_ascent_rate = 5;
      }
    }
    // to be added: to use the pressure drop when flight begins to estiblish an more accurate flight timer
    if (FlightHasBegun == false) {

      if (msPressure < prevPress) {
        pressCount++;
      }
      else {
        pressCount = 0;
      }
      prevPress = msPressure;
      if (pressCount >= 4) {
        FlightHasBegun = true;
        timeBeforeFlight = currTimeS;
      }
    }
    // Simply print to the serial monitor whether the flight has begun (0 or 1 boolean) and, if so, how much time since then
    Serial.println("FlightHasBegun and timeSince5kFeetS");
    Serial.println(FlightHasBegun);
    Serial.println(String(timeSince5kFeetS));

    //**************************************************************TERMINATION LOGIC BEGINS**************************************************************
    //**************************************************************TERMINATION LOGIC BEGINS**************************************************************
    //**************************************************************TERMINATION LOGIC BEGINS**************************************************************
    //**************************************************************TERMINATION LOGIC BEGINS**************************************************************
    //**************************************************************TERMINATION LOGIC BEGINS**************************************************************

    // *****Ultimate Emergency Timer Cut in case nothing burned or the vent didn't open for whatever reason before*******
    if (currTimeS > terminateTimeS) // Master Backup timer (currently cuts 14,400 seconds (4 hours) after turning on in case all else fails)
    {
      terminate();
      Serial.println("Released because of backup master timer");
      cutterState = "Released because of backup master timer";
    }

    // *********Backup Flight Cutter*************
    if (FlightHasBegun == true)
    {
      if (currTimeS - timeSince5kFeetS > 220 * 60) // Sort of redundant with the master timer, currently cuts 13,200 seconds (3 hours, 40 minutes) after the flight officially starts
      {
        terminate();
        Serial.println("Released because of backup flight timer");
        cutterState = "Released because of backup flight timer";
      }
      //     if(AlreadyReachedPermanentOpening==true && altFeet < 80000) // if you've opened at 100,000 feet and have now completed the full descent back down to 80,000 feet, terminate the flight
      //     {
      //      terminate();
      //      cutterState = "Popped because you went below 80000 feet upon descent, after opening"; //This is temporarily commented out since it now terminates at 100k altitude
      //     }
      //     if(avg_ascent_rate < -8.5) // **WARNING** April 11th, 2021 Flight Addition - Just for testing, as a backup for ensuring that we test termination; **WARNING** this is to ensure that you can release from the balloon neck and demonstrate termination, even if the balloon pops early and comes down fast. might or might not want for future flights? Ask Dr. Flaten. If you vent at high altitudes, it shouldn't be a problem, but if your venting descent rate is pretty fast, you might want to re-consider!
      //     {
      //      terminate();
      //      cutterState = "Released because balloon popped but we still want to test the neck-release mechanism";
      //     }
      if (currentState == OUT_OF_BOUNDS) //Check to terminate when the balloon has crossed the coordinate boundaries
      {
        terminate();
        cutterState = "Realeased because balloon crossed flight boundaries";
      }
      if ((preVentingTimeS1 + 10800 < currTimeS) && !AlreadyStartedPreVenting1) //Terminates if the flight has been in air for 3 hours and not hit the first pre-vent.
      {
        terminate();
        cutterState = "Released because it took over 3 hours to reach the first pre-vent from the flight start";
      }
      if ((preVentingTimeS1 + 3600 < currTimeS) && AlreadyFinishedPreVenting1 && !AlreadyStartedPreventing2) //Terminates when the balloon has hit pre-vent 1, but it has been an hour and not hit vent 2
      {
        terminate();
        cutterState = "Released because it took over an hour to reach the second pre-vent from the first pre-vent";
      }
      if ((preVentingTimeS2 + 3600 < currTimeS) && AlreadyFinishedPreventing2 && !AlreadyReachedPermanentOpening) //Terminates when the balloon has hit pre-vent 2, but it has been an hour and not reached the final opening
      {
        terminate();
        cutterState = "Released because it took over an hour to reach the final vent opening from the second pre-vent";
      }
      if ((currentState == DESCENT || currentState == SLOW_DESCENT) && !AlreadyReachedPermanentOpening) { //If the balloon is coming down and has not reached the final opening, wait 30 minutes, then cut.
        if (!descentTimerStarted) {
          descentTimerStarted = true;
          descentTimer = currTimeS;
        }
        if (descentTimer + 3600 < currTimeS) { //hour seems a bit long, float timer next to this for consistency? two false descent hour apart would terminate
          terminate();
          cutterState = "Released because balloon started coming down before final vent";
        }
      }
      if (altFeet < 4500 && altFeet > 3500 && (currentState == ASCENT || currentState == SLOW_ASCENT)) { //CHECK OVER THIS !!! If the balloon has dipped below 4500 feet after reaching 5000 feet to start the flight, we assume the balloon is in float along the initialization altitude
        terminate();
        cutterState = "Released because balloon dropped below 4500 feet after reaching 5000 feet";
      }
    }


    //**************************************************************TERMINATION LOGIC ENDS**************************************************************
    //**************************************************************TERMINATION LOGIC ENDS**************************************************************
    //**************************************************************TERMINATION LOGIC ENDS**************************************************************
    //**************************************************************TERMINATION LOGIC ENDS**************************************************************
    //**************************************************************TERMINATION LOGIC ENDS**************************************************************

    //**************************************************************VENTING LOGIC BEGINS**************************************************************
    //**************************************************************VENTING LOGIC BEGINS**************************************************************
    //**************************************************************VENTING LOGIC BEGINS**************************************************************
    //**************************************************************VENTING LOGIC BEGINS**************************************************************
    //**************************************************************VENTING LOGIC BEGINS**************************************************************

    // ****INSERT ALTITUE GPS-BASED AND TIMER-BASED VENTING DECISIONS HERE****

    //int est_altitude = lastAltFeet + avg_ascent_rate*(millis() - prev_time2)/1000; // (For the else statement, if the GPS hasn't had a good lock)

    // Regular State Machine
    if (GPS_LOCK == true) // if the GPS has had a hit in the last minute or so, assume that its working (this if statement is the primary (GPS-based) state machine logic
    {
      Serial.println("GPS Lock = " + String(GPS_LOCK));
      Serial.println("Going through regular state machine logic (GPS working)");
      Control();
      suggestedState = State();

      if (flapperState.equals("Open")) { //This checks to see if the vent needs to be "jiggled" to fix servo position in the case that it's unaligned
        openVent();
      } else {
        closeVent();
      }

      if (altFeet > preVentAlt1 || (currTimeS - timeBeforeFlight) > preVentTimeS1) // This is a temporary venting to help ensure that the balloon stack will reach the desired altitude later in the flight
      {
        PreVenting1(currTimeS, avg_ascent_rate); // 80,000 feet = 24.4 km = first pre-venting, we pre-vent here to make sure it doesn't pop prematurely (most balloons start popping above 100,000 feet or so, lighter ones sometimes higher)
      }
      if (altFeet > preVentAlt2 || (finishventTimeS1 + (15 * 60) < currTimeS)) // This is a second temporary venting to help ensure that the balloon stack will reach the desired altitude later in the flight
      {
        PreVenting2(currTimeS, avg_ascent_rate); // 90,000 feet = 27.4 km = second pre-venting; we have a second pre-venting instead of one large one since it would take much longer to acheive altitude if we slowed all the way down 10,000 feet (3 km) earlier
      }
      if (altFeet > terminateAlt || (finishventTimeS2 + (20 * 60) < currTimeS)) //Final vent, Nope , terminate.
      {
        terminate();
        //        cutterState = "Released because balloon crossed 100k feet";
        //        oneTimeOpen(currTimeS,lon); // this is the permanent opening at 30.5 km (100,000 feet) altitude. Feel free to set this higher for future flights since this might come up short
        //        Serial.println("One-time permanent opening");
      }
      if ((currentState == FLOAT || floatTimerStarted) && !AlreadyReachedPermanentOpening) {
        if (!floatTimerStarted) {
          floatTimerStarted = true;
          floatTimer = currTimeS;
          openVent();
          ventReason = "Open since balloon is in float before final vent opening"; //reached permanent opening?
        }
        if (floatTimer + 3600 < currTimeS) {
          terminate();
          cutterState = "Released because balloon started floating before final vent";
        }
      }
    }
    else // If GPS isn't working after one minute or so, resort to a  timer-based opening (This else statement is the backup logic, in the case that the GPS hasn't given you a hit for a few minutes)
    {
      Serial.println("Going through backup state machine logic");
      Control();
      suggestedState = State();

      //This logic is a TIMER-BASED backup for the GPS. At 60k feet the pressure sensor becomes unreliable
      if (FlightHasBegun == true)
      {
        System_Time_Min = (currTimeS - timeSince5kFeetS) / 60; // Time since the balloon crossed 5k feet // Note: Old Logic; Note: this logic relies on the GPS working around ~2500 feet in order to work properly, but not throughout the entire flight



        //        Serial.println("System_Time_Min is:");
        //        Serial.println(String(System_Time_Min));
        //        if(System_Time_Min > 110) // Vent will probably slow down, but it depends on
        //        {
        //          oneTimeOpen(currTimeS,lon); // Permanent vent opening
        //          Serial.println("100k feet timer-based termination");
        //      }

      }
      if ((currTimeS - timeBeforeFlight) > (80 * 60)) // This is a temporary venting to help ensure that the balloon stack will reach the desired altitude later in the flight
      {
        PreVenting1(currTimeS, avg_ascent_rate); // 80,000 feet = 24.4 km = first pre-venting, we pre-vent here to make sure it doesn't pop prematurely (most balloons start popping above 100,000 feet or so, lighter ones sometimes higher)
      }
      if ( (finishventTimeS1 + (15 * 60) < currTimeS)) // This is a second temporary venting to help ensure that the balloon stack will reach the desired altitude later in the flight
      {
        PreVenting2(currTimeS, avg_ascent_rate); // 90,000 feet = 27.4 km = second pre-venting; we have a second pre-venting instead of one large one since it would take much longer to acheive altitude if we slowed all the way down 10,000 feet (3 km) earlier
      }
      if ((finishventTimeS2 + (20 * 60) < currTimeS)) //Final vent, Nope , terminate.
      {
        terminate();
        //        cutterState = "Released because balloon crossed 100k feet";
        //        oneTimeOpen(currTimeS,lon); // this is the permanent opening at 30.5 km (100,000 feet) altitude. Feel free to set this higher for future flights since this might come up short
        //        Serial.println("One-time permanent opening");
      }
    }

    // *Note that all of the "Serial.println()"'s that are here are simply to tell someone what's going on if they are plugged into the Teensy and watching the serial monitor



    //**************************************************************VENTING LOGIC ENDS**************************************************************
    //**************************************************************VENTING LOGIC ENDS**************************************************************
    //**************************************************************VENTING LOGIC ENDS**************************************************************
    //**************************************************************VENTING LOGIC ENDS**************************************************************
    //**************************************************************VENTING LOGIC ENDS**************************************************************

    // BELOW IS FOR USE WITH THE SERIAL MONITOR ONLY**********************NOT FLIGHT-CRITICAL**********************
    if (serialcommand == "GOOF")
    {
      Serial.println("Testing vent 'jiggle'");
      ventServo.write(80);
      serialcommand = "";
    }
    if (serialcommand == "OPEN")
    {
      Serial.println("Testing");
      Serial.println("Vent open");
      ventReason = "Serial monitor test";
      openVent();
      serialcommand == "";
    }
    if (serialcommand == "CLOSE")
    {
      Serial.println("Vent closed");
      ventReason = "Closed";
      Serial.println("About to run closeVent() because of serial command");
      closeVent();
      serialcommand == "";
    }
    if (serialcommand == "BURN")
    {
      burnResistor();
      cutterState = "Burned";
      serialcommand == "";
    }
    if (serialcommand == "SBURN")
    {
      cutterState = "Stopped Burning";
      stopBurn();
      serialcommand == "";
    }
    if (serialcommand == "BURN2")
    {
      burnResistor2();
      cutterState = "Burned2";
      serialcommand == "";
    }
    if (serialcommand == "SBURN2")
    {
      cutterState = "Stopped Burning 2";
      stopBurn2();
      serialcommand == "";
    }
    if (serialcommand == "LEDON1")
    {
      digitalWrite(23, HIGH);
      serialcommand == "";
    }
    if (serialcommand == "LEDOFF1")
    {
      digitalWrite(23, LOW);
      serialcommand == "";
    }
    if (serialcommand == "LEDON2")
    {
      digitalWrite(22, HIGH);
      serialcommand == "";
    }
    if (serialcommand == "LEDOFF2")
    {
      digitalWrite(225, LOW);
      serialcommand == "";
    }
    if (serialcommand == "LEDON3")
    {
      digitalWrite(17, HIGH);
      serialcommand == "";
    }
    if (serialcommand == "LEDOFF3")
    {
      digitalWrite(17, LOW);
      serialcommand == "";
    }
    // ABOVE IS FOR USE WITH THE SERIAL MONITOR ONLY**********************NOT FLIGHT-CRITICAL**********************

    // Finally, add the last few variables to the message string, print out the data/message to the serial monitor, and log the message string to the SD Card
    message += "," + heaterStatus + "," + flapperState + "," + String(servoPos()) + "," + String(commandedServoPosition)+ "," + String(cutterState) + "," + stateSuggest + "," + currentState + "," + ventReason;
    //BELOW IS FOR TESTING
    Serial.println(header);
    //ABOVE IS FOR TESTING
    Serial.println(message);
    Serial.println("");
    // SD Card
    if (true) {
      logData(message);                                        //close file afterward to ensure data is saved properly
    }
  } //if statement that checks if it has been 2 seconds
}
