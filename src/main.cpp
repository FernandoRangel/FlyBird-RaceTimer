#include "debug.h"
#include "led.h"
#include "webserver.h"
#include <esp_now.h>

static RX5808 rx(PIN_RX5808_RSSI, PIN_RX5808_DATA, PIN_RX5808_SELECT, PIN_RX5808_CLOCK);
static Config config;
static Webserver ws;
static Buzzer buzzer;
static Led led;
static LapTimer timer;
static BatteryMonitor monitor;

static TaskHandle_t xTimerTask = NULL;

static void parallelTask(void *pvArgs) {
    for (;;) {
        uint32_t currentTimeMs = millis();
        buzzer.handleBuzzer(currentTimeMs);
        led.handleLed(currentTimeMs);
        ws.handleWebUpdate(currentTimeMs);
        config.handleEeprom(currentTimeMs);
        rx.handleFrequencyChange(currentTimeMs, config.getFrequency());
        monitor.checkBatteryState(currentTimeMs, config.getAlarmThreshold());
        buzzer.handleBuzzer(currentTimeMs);
        led.handleLed(currentTimeMs);
    }
}

static void initParallelTask() {
    disableCore0WDT();
    xTaskCreatePinnedToCore(parallelTask, "parallelTask", 3000, NULL, 0, &xTimerTask, 0);
}

void onReceive(const uint8_t *macAddr, const uint8_t *data, int len) {
    DEBUG("Dispositivo encontrado: ");
    for (int i = 0; i < 6; i++) {
        DEBUG("%02X", macAddr[i]);
        if (i < 5) DEBUG(":");
    }
    DEBUG("\n");
}
void onSent(const uint8_t *macAddr, esp_now_send_status_t status) {
    if(status != ESP_NOW_SEND_SUCCESS) {
        DEBUG("Falha no envio de pacote\n");
    }
}
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
typedef struct {
  char message[32];
} Packet;

Packet dataToSend;

void setup() {
    DEBUG_INIT;
    config.init();
    rx.init();
    buzzer.init(PIN_BUZZER, BUZZER_INVERTED);
    led.init(PIN_LED, false);
    timer.init(&config, &rx, &buzzer, &led);
    monitor.init(PIN_VBAT, VBAT_SCALE, VBAT_ADD, &buzzer, &led);
    ws.init(&config, &timer, &monitor, &buzzer, &led);
    led.on(400);
    buzzer.beep(200);

    // Certifique-se de desativar o Wi-Fi antes de reconfigurá-lo
    WiFi.disconnect();
    // Inicializa o ESP-NOW
    if (esp_now_init() == ESP_OK) {
        DEBUG("ESP-NOW inicializado com sucesso\n");
    } else {
        DEBUG("Erro ao inicializar ESP-NOW\n");
    }

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (!esp_now_is_peer_exist(broadcastAddress)) {
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            DEBUG("Erro ao adicionar peer\n");
        }
    }

    esp_now_register_recv_cb(onReceive);
    esp_now_register_send_cb(onSent);

    //initParallelTask();
}

void loop() {
    uint32_t currentTimeMs = millis();
    timer.handleLapTimerUpdate(currentTimeMs);

    strcpy(dataToSend.message, "Hello, ESP-NOW!\n");
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&dataToSend, sizeof(dataToSend));
    if (result != ESP_OK) {
        DEBUG("Erro ao enviar pacote: %d\n", result);
    }
    
    delay(2000);
}
