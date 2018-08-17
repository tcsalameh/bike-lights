## Speed-sensitive bike lights

This is a fun project I did to create side-mounted lights for my bike that would react to different speeds.
The basic idea is a LED light strip that goes up the downtube, around the headtube, and back down again, secured with adhesive velcro strips, a hall sensor chip and magnet to detect the speed of the wheel, and a microcontroller + power supply inside a water bottle to coordinate the whole thing.

## Parts

- 1 Ardunio Uno
- 1 4AA battery holder
- 4 rechargable AA batteries. Note: The rechargable part is important! Alkaline batteries will have a higher voltage. 4AA rechargable is ~4.8V, close enough to 5V to power the whole system without issue. 4AA alkaline is ~6V, above the safe voltage for the Arduino and light strip..
- 1 toggle switch
- 1 hall sensor chip
- 1 spoke magnet
- 1 470-ohm resistor
- 1 large capacitor
- 1 Neopixel light strip, 60 pixels per strip
- 2 4-wire waterproof cables
- 1 mid-size water bottle storage container (or even just a water bottle). Main limiting factor here is whether the Arduino can fit through the mouth of the bottle.
- Adhesive velcro strips, and some velcro ties
- Some sticky pads for wall hangings (relevant for padding the sensor if you need to get it closer to the wheel)
- Lots of colored hook up wire, shrink wrap, etc.

## Wiring Diagram

![Wiring Diagram](wiring_diagram.png)
