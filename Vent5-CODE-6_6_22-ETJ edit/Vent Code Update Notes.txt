		Update List for Vent5 Code 
	

List of changes: 
(add new lines to top of list whenever changes are made
--------------------Format--------------------
Date of Update: 
Editor: 
Major changes to code: 
Emulated flight tested (y/n):
Notes/Observations:
Future suggestions or plans for changes: 
----------------------------------------------------
Date of Update: 6/7/22
Editor: Ethan Thompson-Jewell
Major changes to code: 
---> moved ascent rate logging to SD card (either pressure or GPS) outside of the GPSlock == true if else statement. Lines 149 System update is where new is
---> Added GPS time stamp [lines 48-50]
---> flight time to be logged to SD card. no using flightTime for any logic yet. im scared to mess something up. might do it for GL197 [line 53]
---> 10-cycle ave ascent rate now logged to SD every cycle [line 128 Sysyem update]
---> Confirmed suggest state and command state are logged
---> Servo commanded position is now logged to SD card every cycle and updated each time closeVent() and openVent() are called
---> Flight timer now starts after pressure has increased 4 times 
Emulated flight tested (y/n):
Notes/Observations: 
Future suggestions or plans for changes: 

## make timers in code are based off of flight timer not currTimeS
	prevent 1 & 2
	big vent timer
	terminate timer

##closeServo issue <<--------- wednesday with seyon
	*Delay because of jiggle function is back bc of this so i am setting clsoe servo to 132
## descent logic. make sure that vent knows when it in decent 
----------------------------------------------------
Date of Update: 6/6/22
Editor: Ethan Thompson-Jewell
Major changes to code: 
-------> changed suggest state to fix the problem that every 4 cycles the gps alt would not update and thus teh ascent rate would be set equal to zero
this change was in state_machine ln 35ish
-------> changed line 212 if statement to check if servoPos is +/- 40 from 132 degrees rather than 180 because that is how much the servo can close to not 180. this is something that 
may need to be changed with each avionics package. unless we impliment a function that puches servo to fully closed then sets a new closed servo position
-------> Changed around ave_ascent_rate calculation in system update. just moved a repeated if statement around in a loop. ~line 65-80
-------> Edited jiggle function and changes 'closeServo' variable so Vent doesnt try to jiggle flapper every cycle.
-------> If not said in last update of code, added a millis section to what is logged by SD card
Emulated flight tested (y/n): Yes
Notes/Observations: 
---> Code was able to prevent twice at 75K and 85K feet. 
---> During both prevents the ascent Rate was reduced to target amount (10% of pre-vent rate)
---> After prevent2 and termination were both started because of timers which were still quite close to desired altitude for both protocols
Future suggestions or plans for changes: 
--Should modify emulation to test vent's reaction to following scenarios:
*Ascent rate not reduced by desired amount during 1 round of venting (Ex prevent 1 doesnt reduce ascent rate by 10%)
*Different slowAscent/float scenarios (venting was too effective) 
*termination based on altitude rather than time since venting (time since prevent logic worked when tested)
