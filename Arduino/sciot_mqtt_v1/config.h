#define DEV_ID  123



#define DHTPIN A0
#define MOISTURE_PIN A1
#define DHTTYPE DHT11  
#define VALVE 2

const char ssid[] = "SmartPlant";
const char pass[] = "**********";

String sensorTopic = "sensor/";
String actuatorTopic = "actuator/";
 

const char mqttServer[] = "192.168.19.1";
int mqttPort = 1883;

char username[] = "arduino";
char password[] = "mw_arduino";

int interval = 1000;
int colorR = 0;
int colorG = 0;
int colorB = 200;
