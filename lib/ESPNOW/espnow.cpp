#include "espnow.h"
#include <esp_now.h>
#include <debug.h>

void EspNow::init() {
    DEBUG("EspNow::init\n");
}

void EspNow::sendDiscoveryPacket() {
    DEBUG("sendDiscoveryPacket\n");

    uint8_t myMac[6];
    esp_read_mac(myMac, ESP_MAC_WIFI_STA); // Pega o MAC do dispositivo atual
    
    // Prepara o pacote com o MAC do dispositivo
    DiscoveryPacket packet;
    memcpy(packet.mac, myMac, 6);

    // Envia o pacote para todos os dispositivos
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].discovered) {
            esp_now_send(devices[i].macAddress, (uint8_t*)&packet, sizeof(packet));
        }
    }
}

void EspNow::onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
    DEBUG("onDataRecv\n");
    DiscoveryPacket packet;
    memcpy(&packet, data, sizeof(packet));

    // Verifica se o dispositivo já foi descoberto
    bool exists = false;
    for (int i = 0; i < numDevices; i++) {
        if (memcmp(devices[i].macAddress, packet.mac, 6) == 0) {
        exists = true;
        break;
        }
    }

    // Se não encontrou, adiciona o dispositivo à lista
    if (!exists && numDevices < MAX_DEVICES) {
        memcpy(devices[numDevices].macAddress, packet.mac, 6);
        devices[numDevices].discovered = true;
        numDevices++;
    }

    // Exibe os dispositivos conhecidos
    Serial.println("Dispositivos conhecidos:");
    for (int i = 0; i < numDevices; i++) {
        for (int j = 0; j < 6; j++) {
        Serial.print(devices[i].macAddress[j], HEX);
        Serial.print(":");
        }
        Serial.println();
    }
}