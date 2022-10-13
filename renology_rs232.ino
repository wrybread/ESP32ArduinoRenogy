/*

Reads the data from the Renology charge controller. Tested with Wanderer 30A (CTRL-WND30-LI)

See my post here for wiring diagram and notes: https://forum.arduino.cc/t/trying-to-read-registers-from-a-solar-charge-controller-over-rs232-solved/1039864/11


To do:
- make a "turn on load" function with the other charge controller, which I think supports it
- other registers? Go through Renogy manual
- confirm temperature values once I have the battery temperature sensor
- verify that the info we're getting from the info registers is being processed correctly. The serial number for example doesn't match the serial number printed on the controller.
- confirm that the Renology 10amp works, and compare the values to the ones reported on the LCD screen

- can I use the load to power the bilge pump? Or is that just for lights?


*/

// https://github.com/syvic/ModbusMaster
#include <ModbusMaster.h>
ModbusMaster node;



// Pins used by ESP32 for the 2nd serial port. Not possible to have a second serial port on some Arduinos
#define RXD2 16 // aka "RX2" on some ESP32 devboards
#define TXD2 17 // aka "TX2" on some ESP32 devboards

// Number of registers to check. I think all Renogy controlls have 30
// data registers (not all of which are used) and 17 info registers.
const uint32_t num_data_registers = 30;
const uint32_t num_info_registers = 17;


struct Controller_data {
  
  uint8_t percent_battery_capacity;  // percent
  float battery_voltage;             // volts
  float battery_charge_current;      // amps
  uint8_t battery_temperature;       // celcius
  uint8_t controller_temperature;    // celcius
  float load_voltage;                // volts
  float load_current;                // amps
  uint8_t load_power;                // watts
  float solar_panel_voltage;         // volts
  float solar_panel_current;         // amps
  uint8_t solar_panel_power;         // watts
  float min_battery_voltage_today;   // volts
  float max_battery_voltage_today;   // volts
  float max_charge_current_today;    // amps
  float max_discharge_current_today; // amps
  uint8_t max_charge_power_today;    // watts
  uint8_t max_discharge_power_today; // watts
  uint8_t charge_amphours_today;     // amp hours
  uint8_t discharge_amphours_today;  // amp hours
  uint8_t charge_watthours_today;    // watt hours
  uint8_t discharge_watthours_today; // watt hours
  uint8_t controller_uptime;         // days
  uint8_t total_battery_overcharges; // count
  uint8_t total_battery_fullcharges; // count

  long last_update_millis;
};
Controller_data renology_data;

struct Controller_info {
  
  uint8_t voltage_rating;            // volts
  uint8_t current_rating;            // amps
  uint8_t discharge_current_rating;  // amps
  uint8_t type;
  uint8_t controller_name;
  char software_version[40];
  char hardware_version[40];
  char serial_number[40];
  uint8_t modbus_address;  
  long last_update_millis;

};
Controller_info renology_info;







void setup()
{
  Serial.begin(115200);
  Serial.println("Started!");

  // create a second serial interface for modbus
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); 

  int modbus_address = 255; // my Renogy Wanderer has an (slave) address of 255! Not in docs???
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

  read_renogy_data_registers();
  Serial.println("Battery voltage: " + String(renology_data.battery_voltage));
    
  read_renogy_info_registers();

  delay(5000);

}








void read_renogy_data_registers() {

  uint8_t j, result;
  uint16_t data_registers[num_data_registers];
  char buffer1[40], buffer2[40];
  uint8_t raw_data;
  
  //////////////////////////////
  // Read the 30 data registers
  //////////////////////////////
  result = node.readHoldingRegisters(0x100, num_data_registers);
  if (result == node.ku8MBSuccess)
  {
    Serial.println("Successfully read the data registers!");
    for (j = 0; j < num_data_registers; j++)
    {
      data_registers[j] = node.getResponseBuffer(j);
      Serial.println(data_registers[j]);
    }

    // process each register
    renology_data.percent_battery_capacity = data_registers[0]; 
    renology_data.battery_voltage = data_registers[1] * .1; // will it crash if data_registers[1] doesn't exist?
    renology_data.battery_charge_current = data_registers[2] * .1;
    
    //0x103 returns two bytes, one for battery and one for controller temp in c
    // I'm definitely not sure about these readings, they're erratic and super high
    uint16_t raw_data = data_registers[3]; // eg 5913
    renology_data.controller_temperature = raw_data/100;
    renology_data.battery_temperature = raw_data%100; 
    Serial.println("controller_temperature=" + String(renology_data.controller_temperature)); 
    Serial.println("battery_temperature=" + String(renology_data.battery_temperature)); 

    renology_data.load_voltage = data_registers[4] * .1;
    renology_data.load_current = data_registers[5] * .01;
    renology_data.load_power = data_registers[6];
    renology_data.solar_panel_voltage = data_registers[7] * .1;
    renology_data.solar_panel_current = data_registers[8] * .01;
    renology_data.solar_panel_power = data_registers[9];
     //Register 0x10A - Turn on load, write register, unsupported in wanderer - 10
    renology_data.min_battery_voltage_today = data_registers[11] * .1;
    renology_data.max_battery_voltage_today = data_registers[12] * .1; 
    renology_data.max_charge_current_today = data_registers[13] * .01;
    renology_data.max_discharge_current_today = data_registers[14] * .1;
    renology_data.max_charge_power_today = data_registers[15];
    renology_data.max_discharge_power_today = data_registers[16];
    renology_data.charge_amphours_today = data_registers[17];
    renology_data.discharge_amphours_today = data_registers[18];
    renology_data.charge_watthours_today = data_registers[19];
    renology_data.discharge_watthours_today = data_registers[20];
    renology_data.controller_uptime = data_registers[21];
    renology_data.total_battery_overcharges = data_registers[22];
    renology_data.total_battery_fullcharges = data_registers[23];
    renology_data.last_update_millis = millis();

    //Registers 0x118 to 0x119- Total Charging Amp-Hours - 24/25    
    //Registers 0x11A to 0x11B- Total Discharging Amp-Hours - 26/27    
    //Registers 0x11C to 0x11D- Total Cumulative power generation (kWH) - 28/29    
    //Registers 0x11E to 0x11F- Total Cumulative power consumption (kWH) - 30/31    
    //Register 0x120 - Load Status, Load Brightness, Charging State - 32    
    //Registers 0x121 to 0x122 - Controller fault codes - 33/34
    //TODO: More registers    

    //Serial.println("Battery voltage: " + String(renology_data.battery_voltage));
    //Serial.println("Temperature = " + String(battery_temperatures_raw));

    Serial.println("---");
  }
  else if (result == 0xE2) {
    Serial.println("Timed out reading the data registers!");
  }
  else {
    Serial.print("Failed to read the data registers... ");
    Serial.println(result, HEX); // E2 is timeout
  }

}


void read_renogy_info_registers() {

  uint8_t j, result;
  uint16_t info_registers[num_info_registers];
  char buffer1[40], buffer2[40];
  uint8_t raw_data;
  
  //////////////////////////////
  // Read the 17 info registers
  //////////////////////////////
  result = node.readHoldingRegisters(0x00A, num_info_registers);
  if (result == node.ku8MBSuccess)
  {
    Serial.println("Successfully read the info registers!");
    for (j = 0; j < num_info_registers; j++)
    {
      info_registers[j] = node.getResponseBuffer(j);
      Serial.println(info_registers[j]);
    }

    // read and process each value
    //Register 0x0A - Controller voltage and Current Rating - 0
    raw_data = info_registers[0]; 
    renology_info.voltage_rating = raw_data/100;
    renology_info.current_rating = raw_data%100; 

    //Register 0x0B - Controller discharge current and type - 1
    raw_data = info_registers[1]; 
    renology_info.discharge_current_rating = raw_data/100;
    renology_info.type = raw_data%100; 

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
    Serial.println("Software version: " + String(renology_info.software_version));


    //Registers 0x016 to 0x017 - Hardware Version - 12-13
    itoa(info_registers[12],buffer1,10); 
    itoa(info_registers[13],buffer2,10);
    strcat(buffer1, buffer2); // should put a divider between the two strings?
    strcpy(renology_info.hardware_version, buffer1);
    Serial.println("Hardware version: " + String(renology_info.hardware_version));


    //Registers 0x018 to 0x019 - Product Serial Number - 14-15
    itoa(info_registers[14],buffer1,10); 
    itoa(info_registers[15],buffer2,10);
    strcat(buffer1, buffer2); // should put a divider between the two strings?
    strcpy(renology_info.serial_number, buffer1);
    Serial.println("Serial number: " + String(renology_info.serial_number));


    renology_info.modbus_address = info_registers[16];
    Serial.println("Modbus address: " + String(renology_info.modbus_address));
    renology_info.last_update_millis = millis();
  
    Serial.println("---");
    Serial.println();
  }
  else if (result == 0xE2) {
    Serial.println("Timed out reading the info registers!");
  }
  else {
    Serial.print("Failed to read the info registers... ");
    Serial.println(result, HEX); // E2 is timeout
  }
}
