# ESP32ArduinoRenogy

This lets you get data from Renogy charge controllers via their RS232 port using an ESP32 or Arduino. 

![Wanderer](https://sinkingsensation.com/stuff/renogy/wanderer.jpg)

The inspiration came when I needed to power a pump at a remote pond whenever there was sufficient power from the solar panel, and I was surprised to learn that the Renogy controllers don't have that ability natively. Their power pins seem to only want to control lights at night. Silly. This lets me easily overcome that narrow use case, and generally makes Renogy charge controllers super powerful. And without using something relatively heavy like a Raspberry Pi

So far it's only been tested on cheapie Wanderer 30A (CTRL-WND30-LI), and Wanderer 10A controllers, please post to the Issues section if you test on more. 

Here's a pic of my installation:

![installation](https://sinkingsensation.com/stuff/renogy/box.jpg)

## Making the Cable

Here's a wiring diagram:

![wiring diagram](https://sinkingsensation.com/stuff/renogy/wiring.png)

You'll need make a cable to connect the controller to your ESP32 or Arduino. Start with an RJ12 cable, which is an old phone cable with 6 pins. Make sure it doesn't only have 4 pins.

![rj12 cable jack](https://sinkingsensation.com/stuff/renogy/jack.jpg)

That shows the colored wires from my RJ12 cable. I don't know how standard the wire colors are, but as you can see in that pic the first three wires are red, black and white, those are all that's required.

You'll also need a TTL to RS232 level adjuster. I used (this one)[https://www.amazon.com/dp/B07BJJ3TZR]:

![level adjuster](https://sinkingsensation.com/stuff/renogy/converter.jpg)

If the listing goes away, it's the "NOYITO TTL to RS232 Module TTL RS232 Mutual Conversion Module", costs $7.

## Use

After connecting the cable and flashing your ESP you should be getting data from your charge controller in Serial Monitor:

![serial monitor](https://sinkingsensation.com/stuff/renogy/serial_monitor.jpg)

## Notes

- as of now this is untested on an Arduino, I've only used it with ESP32's. It should work fine with an Arduino, the only thing I'm not sure about is whether you can have two Serial interfaces with all Arduinos (and this script uses one Serial interface for communication with the Renogy controller and one to print status to the console). It should work fine though. If you test with an Arduino please post to this project's Issues section.

- I couldn't power the ESP32 directly from the charge controller, my theory is that it's so low power that the charge controller thinks nothing is connected and shuts off the port. Oh well, I used one of these 12v to 5v adaptors:

[pic]

https://www.amazon.com/gp/product/B08H89LTP5

- This can also turn on the load terminals, but I'm not sure what their load limit is. The pump I'm powering pulls 30 watts sometimes, which might be pushing it... So far so good though. I had originally planned to power the pump with a separate relay but the onboard load pins seem to do fine.

- for the record here's a screenshot from the Renogy Wanderer 10a manual showing the modes of the load pins:

![load modes](https://sinkingsensation.com/stuff/renogy/load_modes.jpg)

## See Also

There's a couple of other projects that get data from the Renogy charge controllers. Oddly I couldn't find anything that uses an ESP32 or Arduino though.

- here's a NodeJS project

- here's a pure javascript project

Much thanks to them! They made this much easier.

I also attached Renogy's manual, it's somewhat helpful if you squint at it long enough.

## Tested On

So far it's been tested and confirmed working well on these controllers:

- Wanderer 30A (CTRL-WND30-LI)
- Wanderer 10A
 
Please post to the Issues section if you test on more. 

## To Do

- I don't think it's correctly reading the serial number register, and possibly some of the other registers having to do with the model number. Please let me know if you figure out any fixes. (wrybread@gmail.com or post to Issues).







