#pragma once

#include "hid_event.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

char keycode_from_report(char report_val);
char keycode_to_ascii(uint8_t keycode, bool shift);
int parse_hid_packet(HID_MESSAGE_PACKET_t *packet, Event_t *events, int max_events);
void print_hid_packet(const HID_MESSAGE_PACKET_t *packet);