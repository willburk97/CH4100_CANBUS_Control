# CH4100_CANBUS_Control
Arduino Uno project to control (turn on) CH4100 Enpower charger for my Zero Motorcycle.  
Charger was OEM with 2.5kw ChargeTank.
Credit to https://github.com/Lennart-O/TCCharger-voltage-current-control for original code.  
I changed code to work with CH4100 charger and added code to blink led with current voltage.  
I had added voltage limiting code but it may not be necessary now that I figured out how to add 
the voltage limit to the CANBUS message, Thanks Arlin Sansome!

To make this work I used a Seeed CAN Shield V2.0.  On the (Zero specific I'm sure) signal cable into the charger:
Connect CANH to the black wire going to the charger and CANL to the yellow wire going to the charger. Ignore the red wire.
This will enable using the charger while feeding the C14 conector with 240v or 120v. If you want to use the J1772 connector,
you'll need to pop the charger itself off of the aluminum frame (four bolts) and reverse the input wires to both relays. These
two wires (on each relay) go to the J1772 and to the C14. Default is C14 and when the relay is energized (with the attached
controller that I'm not using) the input switches to the J1772.  Reversing the wires will enable the J1772 as default and effectively
make the C14 not usable (unless you can activate the relays.)  You can power the charger at home still (for testing, etc.)
by feeding power 'backwards' into the C13 connector. I haven't figured out the best way to power the Arduino yet, but presently
I terminated a C13 and C14 cable into a female NEMA 5-15 plug.  This is a bit sketchy but as long as you remember that there'll
be 240V in this plug when you're at a J1772, you're fine. FYI, most cell phone chargers can handle 120-240v, just read and make sure.
The C13 plugs into the onboard Zero charger port(C14), the C14 plugs into the CH4100 C13 plug and the 5-15 has a cell charger plugged into
it that feeds 5v to the arduino.  I'll try to attach a pic so you can see. Note, watch out for those pesky 5-15 receptacles that have
lights in them. The lights'll get hot when they get 240v. Get a receptacle with no lights. Also, get a C13/C14 cable that can handle
15 amps (14AWG)(Just remembered the onboard charger’s only going to pull about 6 Amps when connected to 240V...15 amp cable is probably
unnecessary.) FYI, you can get one with both connectors and just cut it in half.

The CH4100 seems good for about 2800W so with the onboard at 1300W I'm geting 4100W into the battery.
