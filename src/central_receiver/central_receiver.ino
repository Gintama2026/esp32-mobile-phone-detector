/*
  Mobile Phone Detection System - ESP32 Central Receiver

  Receives detection packets from up to six ESP32 detection nodes
  through nRF24L01. Each node is assigned a dedicated 5-byte radio
  address: 1NODE ... 6NODE (addresses differ in the first byte, as
  required by nRF24L01 reading pipes 2-5).

  Packet fields:
    node ID
    detection status
    node timestamp (milliseconds since boot)
    magnetic-field deviation

  Serial Monitor: 115200 baud
*/

#include <SPI.h>
#include <RF24.h>

// -------------------- Pins --------------------
#define NRF_CE  4
#define NRF_CSN 5

RF24 radio(NRF_CE, NRF_CSN);

// nRF24L01 supports up to six reading pipes.
const uint8_t MAX_NODES = 6;
const byte NODE_ADDRESSES[MAX_NODES][6] = {
  "1NODE", "2NODE", "3NODE", "4NODE", "5NODE", "6NODE"
};

struct __attribute__((packed)) AlertPacket {
  uint8_t nodeId;
  uint8_t detected;
  uint32_t timestampMs;
  float deltaB;
};

void setup() {
  Serial.begin(115200);
  delay(100);

  if (!radio.begin()) {
    Serial.println("nRF24L01 initialization failed.");
    while (true) {
      delay(1000);
    }
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setPayloadSize(sizeof(AlertPacket));

  for (uint8_t pipe = 0; pipe < MAX_NODES; ++pipe) {
    // Pipes 0-5 are all used: one address per node.
    radio.openReadingPipe(pipe, NODE_ADDRESSES[pipe]);
  }

  radio.startListening();

  Serial.println();
  Serial.println("====================================");
  Serial.println(" Mobile Phone Detection Receiver");
  Serial.println(" Listening for nodes 1NODE-6NODE");
  Serial.println("====================================");
}

void loop() {
  while (radio.available()) {
    AlertPacket packet;
    radio.read(&packet, sizeof(packet));

    Serial.println("----- ALERT RECEIVED -----");
    Serial.print("Node ID: ");
    Serial.println(packet.nodeId);

    Serial.print("Detection: ");
    Serial.println(packet.detected ? "MOBILE DETECTED" : "NO DETECTION");

    Serial.print("Node timestamp: ");
    Serial.print(packet.timestampMs);
    Serial.println(" ms");

    Serial.print("Delta B: ");
    Serial.print(packet.deltaB, 2);
    Serial.println(" mG");

    Serial.println("--------------------------");
  }
}
