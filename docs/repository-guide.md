# Repository Guide

## `src/mobile_detection_node`

Firmware for the sensing/alert node. Configure `NODE_ID` from 1 to 6 when deploying more than one node. Each ID maps to the radio address `1NODE`…`6NODE`; the receiver uses the same list.

## `src/central_receiver`

Firmware for the central ESP32 receiver. It listens on six nRF24L01 reading pipes and prints alert packets to the Serial Monitor.

## `hardware`

Bill of materials and pin mappings. Always verify the exact module pinout before powering hardware.

## `docs`

System architecture and implementation notes.

## `results`

Reported experimental results from the associated project paper. These are not presented as newly reproduced measurements by this repository.
