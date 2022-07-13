#include <HX711_ADC.h>
#include <ArduinoBLE.h>

//Initializie Bluetooth Sevice
BLEService gaslevelService("1813");

//Initialize Bluetooth Char
BLEUnsignedCharCharacteristic gaslevelChar("2A98",BLERead | BLENotify);
BLEUnsignedCharCharacteristic percentChar("2A01",BLERead | BLENotify);//2A01

//pins:
const int HX711_dout = 3; //mcu > HX711 dout pin
const int HX711_sck = 4; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
int calibrationValue = -51616.78;

void setup(){
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);  
  unsigned long stabilizingtime = 2000;
  boolean _tare = true;
  
  LoadCell.begin();
  LoadCell.start(stabilizingtime, _tare);
  LoadCell.setCalFactor(calibrationValue);


  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("Gasflasche");
  BLE.setAdvertisedService(gaslevelService); // add the service UUID
  gaslevelService.addCharacteristic(gaslevelChar); // add the battery level characteristic
  gaslevelService.addCharacteristic(percentChar);
  BLE.addService(gaslevelService);
  
  BLE.advertise();
  Serial.println("Bluetooth® device active, waiting for connections...");
}

void loop(){
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    // print the central's BT address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    while (central.connected()){
      LoadCell.update();
      int result = LoadCell.getData();

      int percentage = map(result,5.2,11,0,100);
      percentage = max(percentage,5.2);
      
      gaslevelChar.writeValue(result);
      percentChar.writeValue(percentage);
      Serial.println(percentage);
      
      delay(200);
    }

    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
}
}
