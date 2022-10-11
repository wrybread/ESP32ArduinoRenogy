/*



See my post here for wiring diagram and notes: https://forum.arduino.cc/t/trying-to-read-registers-from-a-solar-charge-controller-over-rs232-solved/1039864/11


To do:
- make a "turn on load" function with the other charge controller, which I think supports it

*/

// https://github.com/syvic/ModbusMaster
#include <ModbusMaster.h>
ModbusMaster node;



// Pins used by ESP32 for the 2nd serial port. Not possible on all Arduinos.
#define RXD2 16
#define TXD2 17

// Number of registers to check. I think all Renogy controlls have 30
// data registers (not all of which are used) and 17 info registers.
int num_data_registers = 30;
int num_info_registers = 17;


struct Controller_data {
  float percent_battery_capacity;  // percent
  float battery_voltage;           // volts
  float battery_charge_current;    // amps
  float battery_temperature;       // celcius
  float controller_temperature;    // celcius
  float load_voltage;              // volts
  float load_current;              // amps
  float load_power;                // watts
  float solar_panel_voltage;       // volts
  float solar_panel_current;       // amps
  float solar_panel_power;         // watts
  float min_battery_voltage_today; // volts
  float max_battery_voltage_today; // volts
  float max_charge_current_today;  // amps
  float max_discharge_current_today; // amps
  float max_charge_power_today;    // watts
  float max_discharge_power_today; // watts
  float charge_amphours_today;     // amp hours
  float discharge_amphours_today;  // amp hours
  float charge_watthours_today;    // watt hours
  float discharge_watthours_today; // watt hours
  float controller_uptime;         // days
  float total_battery_overcharges; // count
  float total_battery_fullcharges; // count
  long last_update_millis;
};
Controller_data renology_data;

struct Controller_info {
  float controller_voltage_rating; // volts
  float controller_current_rating; // amps
  float controller_discharge_current_rating; // amps
  float controller_type;
  float controller_name;
  float controller_software_version;
  float controller_hardware_version;
  float controller_serial_number;
  float controller_modbus_address;  
  long last_update_millis;

};
Controller_info renology_info;







void setup()
{
  Serial.begin(115200);
  Serial.println("Started!");

  // create a second serial interface for modbus
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // is SERIAL_8N1 correct?

  int modbus_address = 255; // my Renogy Wanderer has an (slave) address of 255! Not in docs???
  node.begin(modbus_address, Serial2); 

}


void loop()
{
  
  static uint32_t i;
  uint8_t j, result;
  uint16_t data_registers[num_data_registers];
  uint16_t info_registers[num_info_registers];
  
  i++;
  
  // set word 0 of TX buffer to least-significant word of counter (bits 15..0)
  node.setTransmitBuffer(0, lowWord(i));  
  // set word 1 of TX buffer to most-significant word of counter (bits 31..16)
  node.setTransmitBuffer(1, highWord(i));
    

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

    // read the params into the struct
    renology_data.percent_battery_capacity = data_registers[0]; 
    renology_data.battery_voltage = data_registers[1] * .1; // will it crash if data_registers[1] doesn't exist?

    renology_data.battery_charge_current = data_registers[2] * .1;
    
    //0x103 returns two bytes, one for battery and one for controller temp in c
    // here's the javascript code:
    /*
    const buf = Buffer.alloc(2)
    buf.writeInt16BE(data_registers[3]);
    this.battT = buf[0];
    this.controlT = buf[1];
    */

    int battery_temperatures_raw =  data_registers[3];
    //renology_data.battery_temperatures = data_registers[3] * .01;
    //renology_data.controller_temperature;
    
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

    Serial.println("Battery voltage: " + String(renology_data.battery_voltage));
    Serial.println("Temperature = " + String(battery_temperatures_raw));

    Serial.println("---");
  }
  else if (result == 0xE2) {
    Serial.println("Timed out reading the data registers!");
  }
  else {
    Serial.print("Failed to read the data registers... ");
    Serial.println(result, HEX); // E2 is timeout
  }


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

  delay(5000);

}
