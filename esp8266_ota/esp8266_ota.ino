#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
BearSSL::CertStore certStore;
#include <time.h>
 
const String FirmwareVer={"3.0"}; 
#define URL_fw_Version "/Ainqa-Kg/AQ-OTA/blob/main/esp32_ota/version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/Ainqa-Kg/AQ-OTA/blob/main/esp32_ota/fw.bin"

const char* host = "raw.githubusercontent.com";
const int httpsPort = 443;

// DigiCert High Assurance EV Root CA
const char trustRoot[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDcDCCAlgCCQDsrbkGko2aeTANBgkqhkiG9w0BAQsFADB6MQswCQYDVQQGEwJJ
TjELMAkGA1UECAwCVE4xDDAKBgNVBAcMA05HVDELMAkGA1UECgwCQVExCzAJBgNV
BAsMAkFRMQ4wDAYDVQQDDAVyYWtpbjEmMCQGCSqGSIb3DQEJARYXc2F0aGlzaC5y
YWtpbkBhaW5xYS5jb20wHhcNMjEwODEwMTY0MTE4WhcNMjIwODEwMTY0MTE4WjB6
MQswCQYDVQQGEwJJTjELMAkGA1UECAwCVE4xDDAKBgNVBAcMA05HVDELMAkGA1UE
CgwCQVExCzAJBgNVBAsMAkFRMQ4wDAYDVQQDDAVyYWtpbjEmMCQGCSqGSIb3DQEJ
ARYXc2F0aGlzaC5yYWtpbkBhaW5xYS5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IB
DwAwggEKAoIBAQC2YLma1zLTSF17ggoMZ39r/bXVdu7q5YnRj7S0fz7jTBLV2/m6
cDeNAKmuEajmaSca2k7+VpOm9ZGeQyVUGWj6rWzeAYBIZby71xVM4tFLOsS0D7Qw
vQqedy78QyPWsqBoIxiIzaEIP8LmYqQZ6Wa6gqYE3WpPuPIhnjOQklaZFrR43yvy
TELzSFWZ2vvV2FZxxZ7pChe5JIBcNMmr3x5P04BETd+U/XFTn3tv9dcMZZnNHsHH
JJMJ0vQCUkgnu6JggYcnKYLjtl5/fZCdLK+PDy9k/9WJTeM39qy1ilDlqF5qIl6T
Sj4CBq1wA5VL/H42sZmfScQ1BG5Iup+k4MxPAgMBAAEwDQYJKoZIhvcNAQELBQAD
ggEBACoUlpyMltrcot/2FYCIzk9q7dETyuibSpB1CdlKhOpDEdo3hRirgSv98SU2
HHO2iw23+pCNqgzWfI5pRxpQS0GpRPVcOCZ0Q87i2iS+CldOV4eRIAp+DzKziIe6
3N30xG61RfQpfmEXemrCR29yQ63OQ8W94g/NMyvt4Pu4nmxhWncajQR1E4RpTMsU
5pcqZjZIgEwzVpTXKc4KRopx1sNm9y3T1fz7/sDrx2q9jAPrHuBf8JItGuU1Pwzj
1Fco63QmmYxdrvvP3i0MVdDv1f6dLP2LZraweTyB1ihDZ5AkttlRF0uXltu2kJHJ
QG2PzdEffQOqZUvckxI2XhPjAp4=
-----END CERTIFICATE-----
)EOF";
X509List cert(trustRoot);


extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

const char* ssid = "AVE_X_NETWORK";
const char* password = "AVEX1000";

void setClock() {
   // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
/*
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
*/
}
  
void FirmwareUpdate()
{  
  WiFiClientSecure client;
  client.setTrustAnchors(&cert);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }
  client.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      //Serial.println("Headers received");
      break;
    }
  }
  String payload = client.readStringUntil('\n');

  payload.trim();
  if(payload.equals(FirmwareVer) )
  {   
     Serial.println("Device already on latest firmware version"); 
  }
  else
  {
    Serial.println("New firmware detected");
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW); 
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, URL_fw_Bin);
        
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    } 
  }
 }  
void connect_wifi();
unsigned long previousMillis_2 = 0;
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 60000;
const long mini_interval = 1000;
 void repeatedCall(){
    unsigned long currentMillis = millis();
    if ((currentMillis - previousMillis) >= interval) 
    {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      setClock();
      FirmwareUpdate();
    }

    if ((currentMillis - previousMillis_2) >= mini_interval) {
      static int idle_counter=0;
      previousMillis_2 = currentMillis;    
      Serial.print(" Active fw version:");
      Serial.println(FirmwareVer);
      Serial.print("Idle Loop....");
      Serial.println(idle_counter++);
     if(idle_counter%2==0)
      digitalWrite(LED_BUILTIN, HIGH);
     else 
      digitalWrite(LED_BUILTIN, LOW);
     if(WiFi.status() == !WL_CONNECTED) 
          connect_wifi();
   }
 }

  
void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Start");
  WiFi.mode(WIFI_STA);
  connect_wifi();  
  setClock();
  pinMode(LED_BUILTIN, OUTPUT);
  
}
void loop()
{
  repeatedCall();    
}

void connect_wifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("O");
  }                                   
  Serial.println("Connected to WiFi");
}
