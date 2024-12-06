#include <Arduino.h>
#include <led.h>
#include <esp_now.h>

class EspNow {
   public:
    void init(Led *l);
    void onReceive(const uint8_t *macAddr, const uint8_t *data, int len);
    void onSent(const uint8_t *macAddr, esp_now_send_status_t status);

    private:
      Led *led;
      Packet dataToSend;
      uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
};

typedef struct {
  char message[32];
} Packet;