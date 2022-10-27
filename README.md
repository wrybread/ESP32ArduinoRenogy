# ESP32ArduinoRenogy

This lets you get data from Renogy charge controllers via their RS232 port using an ESP32 or Arduino. 

![Wanderer](https://sinkingsensation.com/stuff/renogy/wanderer.jpg)

The inspiration came when I needed to power a pump at a remote pond whenever there was sufficient power from the solar panel, and I was surprised to learn that the load pins on the Renogy controllers can't do that natively. The load pins are only designed to control lights at night. Silly. This lets me use the load pins to power other devices like a bilge pump during the day for one minute an hour when there is sufficient power. And without the need for a relatively heavy small computer like a Raspberry Pi. An ESP32 uses less than 1/10th the power of a Pi, even less if you use deep sleep, has no slow bootup times, there's no SD card to get corrupted, and you don't need to worry about keeping an OS up to date. Once you get it all working it just works, for years and years. 

So far I've only tested this with Renogy Wanderer 30A (CTRL-WND30-LI) and Wanderer 10A charge controllers, please post to the Issues section if you test on more and I'll add it to the list below. It *should* work with any Renogy charge controller that has an RS232 port, which I think is all of them since they want to sell you their bluetooth module that works with that port.

Here's a pic of my installation:

![installation](https://sinkingsensation.com/stuff/renogy/box.jpg)

## Making the Cable

Here's a wiring diagram:

![wiring diagram](https://sinkingsensation.com/stuff/renogy/wiring.png)

You'll need make a cable to connect the controller to your ESP32 or Arduino. Start with an RJ12 cable, which is an old phone cable with 6 wires. Make sure it doesn't have only 4 wires, which is an RJ11 cable.

Here's the RJ12 jack on my cable, note how there's 6 wires (white, black, red, green, yellow, blue):

![rj12 cable jack](https://sinkingsensation.com/stuff/renogy/jack.jpg)

I don't know how standard the wire colors are but those are the colors on mine. You need the first 3 wires from the left, which on mine are the red, black and white wire.

You'll also need a TTL to RS232 level adjuster. I used [this one](https://www.amazon.com/dp/B07BJJ3TZR) but there are lots of other options (google "max3232"):

![level adjuster](https://sinkingsensation.com/stuff/renogy/converter.jpg)

If the listing goes away, it's the "NOYITO TTL to RS232 Module TTL RS232 Mutual Conversion Module", costs $7.

## Use

After connecting the cable as in the wiring diagram above and flashing your ESP you should be seeing data from your charge controller in Serial Monitor:

![serial monitor](https://sinkingsensation.com/stuff/renogy/serial_monitor.jpg)

Feel free to post to Issues if not.

## Notes

- as of now this is untested on an Arduino, I've only used it with ESP32's (both a Rover and a Wroom). It should work fine with an Arduino, the only thing I'm not sure about is whether you can have two Serial interfaces with all Arduinos (and this script uses one Serial interface for communication with the Renogy controller and one to print status to the console). It should work fine though. If you test with an Arduino please post to this project's Issues section.

- I couldn't power the ESP32 directly from the charge controller, my theory is that it's so low power that the charge controller thinks nothing is connected and shuts off the port. Oh well, I used one of [these](https://www.amazon.com/gp/product/B08H89LTP5) 12v to 5v adaptors:

![12v to 5v](https://sinkingsensation.com/stuff/renogy/12v_to_5v.jpg)

- This can also turn on the load terminals, but I'm not sure what their load limit is. The bilge pump I'm powering pulls 30 watts sometimes, which might be pushing it... So far so good though. I had originally planned to power the pump with a separate relay but the onboard load pins seem to do fine.

- for the record here's a screenshot from the Renogy Wanderer 10a manual showing the modes of the load pins:

![load modes](https://sinkingsensation.com/stuff/renogy/load_modes.jpg)

Such an odd limitation to make them only apply to lights.

## See Also

There's a couple of other projects that get data from the Renogy charge controllers. Oddly I couldn't find anything that uses an ESP32 or Arduino though.

- [here's a NodeJS project](https://github.com/mickwheelz/NodeRenogy)

- [here's a pure javascript project](https://github.com/menloparkinnovation/renogy-rover)

Much thanks to them! They made this much easier.

I also attached Renogy's manual, it's somewhat helpful if you squint at it long enough.

## Tested On

So far it's been tested and confirmed working well on these controllers:

- Wanderer 30A (CTRL-WND30-LI)
- Wanderer 10A

It *should* work on any Renogy charge controller with an RS232 port. And most if not all of them have that port since Renogy wants to sell their [bluetooth module](https://www.renogy.com/bt-2-bluetooth-module/), which uses that port to get its data. This should also be pretty easy to adapt to other charge controllers that make their data available via modbus.
 
Please post to the Issues section if you test on other Renogy charge controllers, or email me (wrybread@gmail.com). 

## To Do

- I don't think it's correctly reading the serial number register, and possibly some of the other registers having to do with the model number. Please let me know if you figure out any fixes. (wrybread@gmail.com or post to Issues).

- would be nice to add Bluetooth support for monitoring from a phone

- 









