#include <Arduino.h>

class EspNow {
   public:
    void init();
    void sendDiscoveryPacket();
    void onDataRecv(const uint8_t *mac, const uint8_t *data, int len);
};


#define MAX_DEVICES 10 // Número máximo de dispositivos na rede

struct DeviceInfo {
  uint8_t macAddress[6]; // Endereço MAC do dispositivo
  bool discovered;       // Indica se o dispositivo foi descoberto
};

DeviceInfo devices[10];

int numDevices = 0; // Contador de dispositivos descobertos

struct DiscoveryPacket {
  uint8_t mac[6]; // Endereço MAC do dispositivo
};