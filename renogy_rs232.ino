/*

Reads the data from the Renology charge controller. Tested with Wanderer 30A (CTRL-WND30-LI)

See my Github repo for notes on building the cable:
https://github.com/wrybread/ESP32ArduinoRenogy

Notes: 
- I don't think can power the ESP32 from the Renogy's USB port.. Maybe it's so low power that it shuts off?


To do:
- find out how much of a load the load port can handle... 
- test with an Arduino


*/

// https://github.com/syvic/ModbusMaster
#include <ModbusMaster.h>
ModbusMaster node;




/*
A note about which pins to use: 
- I was originally using pins 17 and 18 (aka RX2 and TX2 on some ESP32 devboards) for RX and TX, 
which worked on an ESP32 Wroom but not an ESP32 Rover. So I switched to pins 13 and 14, which works on both.
I haven't tested on an Arduino board though.
*/
#define RXD2 13
#define TXD2 14

/*
Number of registers to check. I think all Renogy controllers have 35
data registers (not all of which are used) and 17 info registers.
*/
const uint32_t num_data_registers = 35;
const uint32_t num_info_registers = 17;


// if you don't have a charge controller to test with, can set this to true to get non 0 readings
bool simulator_mode = false;








// A struct to hold the controller data
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
  float max_discharging_amps_today;   // amps
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
  float battery_temperatureF;       // fahrenheit
  float controller_temperatureF;    // fahrenheit
  float battery_charging_watts;      // watts. necessary? Does it ever differ from solar_panel_watts?
  long last_update_time;             // millis() of last update time
};
Controller_data renology_data;


// A struct to hold the controller info params
struct Controller_info {
  
  uint8_t voltage_rating;            // volts
  uint8_t amp_rating;                // amps
  uint8_t discharge_amp_rating;      // amps
  uint8_t type;
  uint8_t controller_name;
  char software_version[40];
  char hardware_version[40];
  char serial_number[40];
  uint8_t modbus_address;  

  float wattage_rating;
  long last_update_time;           // millis() of last update time
};
Controller_info renology_info;






void setup()
{
  Serial.begin(115200);
  Serial.println("Started!");

  // create a second serial interface for modbus
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); 

  // my Renogy Wanderer has an (slave) address of 255! Not in docs??? 
  // Do all Renogy charge controllers use this address?
  int modbus_address = 255; 
  node.begin(modbus_address, Serial2); 
}


void loop()
{
  static uint32_t i;
  i++;
  
  // set word 0 of TX buffer to least-significant word of counter (bits 15..0)
  node.setTransmitBuffer(0, lowWord(i));  
  // set word 1 of TX buffer to most-significant word of counter (bits 31..16)
  node.setTransmitBuffer(1, highWord(i));

  renogy_read_data_registers();
  renogy_read_info_registers();

  Serial.println("Battery voltage: " + String(renology_data.battery_voltage));
  Serial.println("Battery charge level: " + String(renology_data.battery_soc) + "%");
  Serial.println("Panel wattage: " + String(renology_data.solar_panel_watts));
  Serial.println("controller_temperatureF=" + String(renology_data.controller_temperatureF)); 
  Serial.println("battery_temperatureF=" + String(renology_data.battery_temperatureF));
  Serial.println("---");


  // turn the load on for 10 seconds
  //renogy_control_load(1)
  //delay(10000);
  //renogy_control_load(0)
  
    
  delay(1000); 

}



void renogy_read_data_registers() 
{
  uint8_t j, result;
  uint16_t data_registers[num_data_registers];
  char buffer1[40], buffer2[40];
  uint8_t raw_data;

  // prints data about each read to the console
  bool print_data=0; 
  
  result = node.readHoldingRegisters(0x100, num_data_registers);
  if (result == node.ku8MBSuccess)
  {
    if (print_data) Serial.println("Successfully read the data registers!");
    for (j = 0; j < num_data_registers; j++)
    {
      data_registers[j] = node.getResponseBuffer(j);
      if (print_data) Serial.println(data_registers[j]);
    }

    renology_data.battery_soc = data_registers[0]; 
    renology_data.battery_voltage = data_registers[1] * .1; // will it crash if data_registers[1] doesn't exist?
    renology_data.battery_charging_amps = data_registers[2] * .1;

    renology_data.battery_charging_watts = renology_data.battery_voltage * renology_data.battery_charging_amps;
    
    //0x103 returns two bytes, one for battery and one for controller temp in c
    uint16_t raw_data = data_registers[3]; // eg 5913
    renology_data.controller_temperature = raw_data/256;
    renology_data.battery_temperature = raw_data%256; 
    // for convenience, fahrenheit versions of the temperatures
    renology_data.controller_temperatureF = (renology_data.controller_temperature * 1.8)+32;
    renology_data.battery_temperatureF = (renology_data.battery_temperature * 1.8)+32;

    renology_data.load_voltage = data_registers[4] * .1;
    renology_data.load_amps = data_registers[5] * .01;
    renology_data.load_watts = data_registers[6];
    renology_data.solar_panel_voltage = data_registers[7] * .1;
    renology_data.solar_panel_amps = data_registers[8] * .01;
    renology_data.solar_panel_watts = data_registers[9];
     //Register 0x10A - Turn on load, write register, unsupported in wanderer - 10
    renology_data.min_battery_voltage_today = data_registers[11] * .1;
    renology_data.max_battery_voltage_today = data_registers[12] * .1; 
    renology_data.max_charging_amps_today = data_registers[13] * .01;
    renology_data.max_discharging_amps_today = data_registers[14] * .1;
    renology_data.max_charge_watts_today = data_registers[15];
    renology_data.max_discharge_watts_today = data_registers[16];
    renology_data.charge_amphours_today = data_registers[17];
    renology_data.discharge_amphours_today = data_registers[18];
    renology_data.charge_watthours_today = data_registers[19];
    renology_data.discharge_watthours_today = data_registers[20];
    renology_data.controller_uptime_days = data_registers[21];
    renology_data.total_battery_overcharges = data_registers[22];
    renology_data.total_battery_fullcharges = data_registers[23];
    renology_data.last_update_time = millis();

    // Add these registers:
    //Registers 0x118 to 0x119- Total Charging Amp-Hours - 24/25    
    //Registers 0x11A to 0x11B- Total Discharging Amp-Hours - 26/27    
    //Registers 0x11C to 0x11D- Total Cumulative power generation (kWH) - 28/29    
    //Registers 0x11E to 0x11F- Total Cumulative power consumption (kWH) - 30/31    
    //Register 0x120 - Load Status, Load Brightness, Charging State - 32    
    //Registers 0x121 to 0x122 - Controller fault codes - 33/34

    if (print_data) Serial.println("---");
  }
  else 
  {
    if (result == 0xE2) 
    {
    Serial.println("Timed out reading the data registers!");
    }
    else 
    {
      Serial.print("Failed to read the data registers... ");
      Serial.println(result, HEX); // E2 is timeout
    }
    // Reset some values if we don't get a reading
    renology_data.battery_voltage = 0; 
    renology_data.battery_charging_amps = 0;
    renology_data.battery_soc = 0;
    renology_data.battery_charging_amps = 0;
    renology_data.controller_temperature = 0;
    renology_data.battery_temperature = 0;    
    renology_data.solar_panel_amps = 0;
    renology_data.solar_panel_watts = 0;
    renology_data.battery_charging_watts = 0;
    if (simulator_mode) {
      renology_data.battery_voltage = 13.99;    
      renology_data.battery_soc = 55; 
    }
  }


}


void renogy_read_info_registers() 
{
  uint8_t j, result;
  uint16_t info_registers[num_info_registers];
  char buffer1[40], buffer2[40];
  uint8_t raw_data;

  // prints data about the read to the console
  bool print_data=0;
  
  result = node.readHoldingRegisters(0x00A, num_info_registers);
  if (result == node.ku8MBSuccess)
  {
    if (print_data) Serial.println("Successfully read the info registers!");
    for (j = 0; j < num_info_registers; j++)
    {
      info_registers[j] = node.getResponseBuffer(j);
      if (print_data) Serial.println(info_registers[j]);
    }

    // read and process each value
    //Register 0x0A - Controller voltage and Current Rating - 0
    // Not sure if this is correct. I get the correct amp rating for my Wanderer 30 (30 amps), but I get a voltage rating of 0 (should be 12v)
    raw_data = info_registers[0]; 
    renology_info.voltage_rating = raw_data/256; 
    renology_info.amp_rating = raw_data%256;
    renology_info.wattage_rating = renology_info.voltage_rating * renology_info.amp_rating;
    //Serial.println("raw ratings = " + String(raw_data));
    //Serial.println("Voltage rating: " + String(renology_info.voltage_rating));
    //Serial.println("amp rating: " + String(renology_info.amp_rating));


    //Register 0x0B - Controller discharge current and type - 1
    raw_data = info_registers[1]; 
    renology_info.discharge_amp_rating = raw_data/256; // not sure if this should be /256 or /100
    renology_info.type = raw_data%256; // not sure if this should be /256 or /100

    //Registers 0x0C to 0x13 - Product Model String - 2-9
    // Here's how the nodeJS project handled this:
    /*
    let modelString = '';
    for (let i = 0; i <= 7; i++) {  
        rawData[i+2].toString(16).match(/.{1,2}/g).forEach( x => {
            modelString += String.fromCharCode(parseInt(x, 16));
        });
    }
    this.controllerModel = modelString.replace(' ','');
    */

    //Registers 0x014 to 0x015 - Software Version - 10-11
    itoa(info_registers[10],buffer1,10); 
    itoa(info_registers[11],buffer2,10);
    strcat(buffer1, buffer2); // should put a divider between the two strings?
    strcpy(renology_info.software_version, buffer1); 
    //Serial.println("Software version: " + String(renology_info.software_version));

    //Registers 0x016 to 0x017 - Hardware Version - 12-13
    itoa(info_registers[12],buffer1,10); 
    itoa(info_registers[13],buffer2,10);
    strcat(buffer1, buffer2); // should put a divider between the two strings?
    strcpy(renology_info.hardware_version, buffer1);
    //Serial.println("Hardware version: " + String(renology_info.hardware_version));

    //Registers 0x018 to 0x019 - Product Serial Number - 14-15
    // I don't think this is correct... Doesn't match serial number printed on my controller
    itoa(info_registers[14],buffer1,10); 
    itoa(info_registers[15],buffer2,10);
    strcat(buffer1, buffer2); // should put a divider between the two strings?
    strcpy(renology_info.serial_number, buffer1);
    //Serial.println("Serial number: " + String(renology_info.serial_number)); // (I don't think this is correct)

    renology_info.modbus_address = info_registers[16];
    renology_info.last_update_time = millis();
  
    if (print_data) Serial.println("---");
  }
  else
  {
    if (result == 0xE2) 
    {
      Serial.println("Timed out reading the info registers!");
    }
    else 
    {
      Serial.print("Failed to read the info registers... ");
      Serial.println(result, HEX); // E2 is timeout
    }
    // anything else to do if we fail to read the info reisters?
  }
}


// control the load pins on Renogy charge controllers that have them
void renogy_control_load(bool state) {
  if (state==1) node.writeSingleRegister(0x010A, 1);  // turn on load
  else node.writeSingleRegister(0x010A, 0);  // turn off load
}
