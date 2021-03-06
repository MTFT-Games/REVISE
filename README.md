# R.E.V.I.S.E.
Rfid kEy Vehicle Ignition StartEr

# Description
REVISE is an Arduino/pn532 project built as a replacement for the key switch in my car. 
Instead of using the original key, it allows me to use the RFID chip implanted in my hand to unlock and start my vehicle with the pn532 RFID reader mounted under the windshield. 
If desired, a normal RFID tag could be used in place of my implant.
![image](https://user-images.githubusercontent.com/70374194/115349587-08c6d180-a182-11eb-8197-09b3dc860d47.png)

# Assembly and Wiring
The system is pretty simple, contrary to what the mess of wires may look like. 
The first part is a cannibalized car port phone charger that I ripped the circuit board out of and soldered wires from the ground and 12v contacts to the 12v and ground of the car. A mini USB cable is then used from this to an Arduino nano (from bangood if you are cheap like me) for power. The Arduino is then wired to several relays that are then used to connect the car’s 12v supply to the various wires required for lock, unlock, ignition, and starting of the vehicle. The wires and configuration will likely differ depending on the vehicle if you try this yourself.

For my 2007 Jeep Grand Cherokee, it’s a weird situation that I don't think I understand fully, but I do know that sending 12v to an ignition wire, connecting the original key switch stuck in the on position, and temporarily sending 12v to a starter wire, will start and run the car without issue.

# Operation
After installation, the system should search for RFID tags every 5 seconds, and if one is found it will compare it against the id of the known keys (my implant or backup tag in this case but any rfid tag could be used). If the id matches, it will trigger the relay connected to the unlock button of the car for half a second and then wait up to the specified time (default 30 seconds) for a start button mounted in the car to short the D12 pin to ground. If the timer runs out it will relock the car, if the button is pressed, it will start the car and wait for another press to shut it off and relock after another delay. 
# Demo
A video bench demonstration can be found here: https://photos.app.goo.gl/Q5MuAbVF8RqJiTE8A as it is too big to upload here.
