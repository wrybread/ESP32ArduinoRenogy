# ESP32ArduinoRenogy

This lets you get data from Renogy charge controllers via their RS232 port using an ESP32 or Arduino. 

![Wanderer](https://sinkingsensation.com/stuff/renogy/wanderer.jpg)

The inspiration came when I needed to power a pump at a remote pond whenever there was sufficient power from a solar panel, and I was surprised to learn that the load switch on the Renogy controllers can't do that natively. The load switch is only designed to control lights at night. Silly. This lets me use the load switch to power other devices like a bilge pump during the day for one minute an hour when there's sufficient power. 

(See "Notes on the Load Switch" below, it's possible that my powering a pump directly from the load switch isn't a good idea).

Another nice thing about this project is that it doesn't require a relatively heavy computer like a Raspberry Pi. An ESP32 uses less than 1/10th the power of a Pi, even less if you use deep sleep, is instantly on with no slow bootup times, has no SD card to get corrupted, doesn't need to be shutdown properly, and you don't need to worry about keeping its OS up to date. Once you get it all working it just works, for years and years. There are times when a Pi is the better choice, but a remote system installed in a cooler next to a pond for a decade isn't one of them, ha.

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

## Notes on the Code

There's a struct that holds all the data from the charge controller. A look at the struct shows what data that is:

```
struct Controller_data {
  
  uint8_t battery_soc;               // percent
  float battery_voltage;             // volts
  float battery_charging_amps;       // amps
  uint8_t battery_temperature;       // celcius
  uint8_t controller_temperature;    // celcius
  float load_voltage;                // volts
  float load_amps;                   // amps
  uint8_t load_watts;                // watts
  float solar_panel_voltage;         // volts
  float solar_panel_amps;            // amps
  uint8_t solar_panel_watts;         // watts
  float min_battery_voltage_today;   // volts
  float max_battery_voltage_today;   // volts
  float max_charging_amps_today;     // amps
  float max_discharging_amps_today;  // amps
  uint8_t max_charge_watts_today;    // watts
  uint8_t max_discharge_watts_today; // watts
  uint8_t charge_amphours_today;     // amp hours
  uint8_t discharge_amphours_today;  // amp hours
  uint8_t charge_watthours_today;    // watt hours
  uint8_t discharge_watthours_today; // watt hours
  uint8_t controller_uptime_days;    // days
  uint8_t total_battery_overcharges; // count
  uint8_t total_battery_fullcharges; // count

  // convenience values
  float battery_temperatureF;        // fahrenheit
  float controller_temperatureF;     // fahrenheit
  float battery_charging_watts;      // watts. necessary? Does it ever differ from solar_panel_watts?
  long last_update_time;             // millis() of last update time
};
Controller_data renology_data;
```

For example it gets the battery_soc (battery state of charge), the battery_voltage, the battery_charging_amps, etc.

There's another struct that holds the info about the charge controller (it's voltage rating, amp rating, serial number, etc). I don't think all those are working yet. For example the serial number it reports is different than the one printed on my charge controller, which I think is just because of how my code processes the serial number data, but I don't need that functionality so I haven't fixed that yet. 

And note the commented out section in the code that turns the load on and off. For example this would turn the load on for 10 seconds:

```
renogy_control_load(1)
delay(10000);
renogy_control_load(0)
```

## Notes on the Load Switch

I think the load switch uses a MOSFET, which means anything with an inductive kickback (like a brush motor) might damage the switch or the unit itself. But I've been powering a bilge pump on it, which I think is an inductive load, for weeks without an issue. I'm willing to risk my cheapie PWM charge controller for that, but be careful what you power from the load switch directly.

Renogy is oddly uncommunicative about the limits of the load switch, and my theory is that that's because it's a bit complicated. My theory is that the capacity is shared by the battery input and switch output. So for example if the 10 amp charge controller is pulling 9 amps of power from the solar panels, that leaves only 1 amp for the load switch. That would explain both why they don't mention the actual load limit, and why they only let us control the load at night (when the load switch would have all the rated power). But still, it would be nice for them to mention that. And furthermore having options to control the load switch in the day would allow for someone to either have a larger controller than their panel so daytime power wouldn't be an issue, or to just use a relay connected to the load switch. Furthermore the manual says the controller will shut off the load switch and show an error code if the capacity reaches 105%, so maybe exceeding the load limit by a bit isn't catastrophic.

Anyway you can of course bypass all these issues by just controlling a relay directly from your ESP or Arduino.

## General Notes

- as of now this is untested on an Arduino, I've only used it with ESP32's (both a Rover and a Wroom). It should work fine with an Arduino, the only thing I'm not sure about is whether you can have two Serial interfaces with all Arduinos (and this script uses one Serial interface for communication with the Renogy controller and one to print status to the console). It should work fine though. If you test with an Arduino please post to this project's Issues section.

- I couldn't power the ESP32 directly from the charge controller, my theory is that it's so low power that the charge controller thinks nothing is connected and shuts off the port. Oh well, I used one of [these](https://www.amazon.com/gp/product/B08H89LTP5) 12v to 5v adaptors:

![12v to 5v](https://sinkingsensation.com/stuff/renogy/12v_to_5v.jpg)

- This can also turn on the load switch, but I'm not sure what their load limit is. The bilge pump I'm powering pulls about 30 watts / 2.5 amps and so far no problem. I think the load switch uses a MOSFET, which means it wants a resistive load, but I think my bilge pump uses a brush motor, which is inductive. I guess time will tell if the inductive kickback will damage the pins. Note that if someone wants to power an inductive load safely they could always use a relay, or just connect the relay directly to the EsP32/Arduino of course.

- for the record here's a screenshot from the Renogy Wanderer 10a manual showing the modes of the load pins:

![load modes](https://sinkingsensation.com/stuff/renogy/load_modes.jpg)

It's also interesting to note that according to the manual the charge controller will show a fault code if the draw of the load pins exceeds the capacity by 105%. But they don't tell us what that capacity is...

Such an odd limitation to design them only to handle lights. Oh well, nothing an ESP32 can't fix.

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

- would be nice to add Bluetooth support for monitoring from a phone, posting to MQTT brokers and pvwatts, etc.











