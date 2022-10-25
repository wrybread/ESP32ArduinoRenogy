# ESP32ArduinoRenogy

This lets you get data from Renogy charge controllers via their RS232 port using an ESP32 or Arduino. 

The inspiration came when I needed to power a pump at a remote pond to keep a cow trough full once an hour if the solar panel was producing sufficient power. I was surprised that the Renogy controllers don't have that ability natively, they only let me power a load at night. This overcomes that overly narrow use case, and doesn't require anything relatively heavy like a Raspberry Pi.

Here's a pic of my installation:

[pic]

## Making the Cable

You'll need make a cable to connect it to your ESP32 or Arduino. Start with an RJ12 cable, which is an old phone cable with 6 pins. Make sure it doesn't only have 4 pins.

[pic of cable]

You'll also need a TTL to RS232 level adjuster. I used this one:

https://www.amazon.com/dp/B07BJJ3TZR

If the listing goes away, it's the "NOYITO TTL to RS232 Module TTL RS232 Mutual Conversion Module", costs $7.

Here's a wiring diagram:

[wiring diagram]

That shows the colored wires from my RJ12 cable. I don't know how standard the wire colors are on those, here's a pic of my jack:

[pic of jack]

Note that I only used the first three wires from the left: the white, black and red wires.


## Notes

- as of now this is untested on an Arduino, I've only used it with ESP32's. It should work fine with an Arduino, the only thing I'm not sure about is whether you can have two Serial interfaces with all Arduinos (and this script uses one Serial interface for communication with the Renogy controller and one to print status to the console). It should work fine though. If you test with an Arduino please post to this project's Issues section.

- I couldn't power the ESP32 directly from the charge controller, my theory is that it's so low power that the charge controller thinks nothing is connected and shuts off the port. Oh well, I used one of these 12v to 5v adaptors:

[pic]

https://www.amazon.com/gp/product/B08H89LTP5

- This can also turn on the load terminals, but I'm not sure what their load limit is. The pump I'm powering pulls 30 watts sometimes, which might be pushing it... So far so good though. I had originally planned to power the pump with a separate relay but the onboard load pins seem to do fine.

- for the record here's a screenshot from the Renogy Wanderer 10a manual showing the modes of the load pins:

[screenshot of load pins]





