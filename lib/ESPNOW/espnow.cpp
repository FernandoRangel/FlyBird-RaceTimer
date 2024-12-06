#include "espnow.h"
#include <esp_now.h>
#include <debug.h>
#include <led.h>
#include <WiFi.h>

void EspNow::init(Led *l) {
    DEBUG("EspNow::init\n");
    led = l;

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
}

void onReceive(const uint8_t *macAddr, const uint8_t *data, int len) {
    DEBUG("Dispositivo encontrado: ");
    for (int i = 0; i < 6; i++) {
        DEBUG("%02X", macAddr[i]);
        if (i < 5) DEBUG(":");
    }
    DEBUG("\n");

    DEBUG("Dados recebidos (string): ");
    for (int i = 0; i < len; i++) {
        DEBUG("%c", data[i]); // Converte cada byte para caractere
    }
    DEBUG("\n");

    led->on(400);
    delay(500);
    led->off();
}

void onSent(const uint8_t *macAddr, esp_now_send_status_t status) {
    if(status != ESP_NOW_SEND_SUCCESS) {
        DEBUG("Falha no envio de pacote\n");
    }
}