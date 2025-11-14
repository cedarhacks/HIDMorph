#include "hid_parsing.h"

uint8_t key_down[256] = {0};

char keycode_from_report(char report_val) {
    return report_val;
}

char keycode_to_ascii(uint8_t keycode, bool shift) {
    // HID usage IDs 0x04 to 0x1D: 'a' to 'z'
    if (keycode >= 0x04 && keycode <= 0x1D) {
        return (shift ? 'A' : 'a') + (keycode - 0x04);
    }

    // HID usage IDs 0x1E to 0x27: '1' to '0'
    if (keycode >= 0x1E && keycode <= 0x27) {
        const char unshifted[] = "1234567890";
        const char shifted[] = "!@#$%^&*()";
        return shift ? shifted[keycode - 0x1E] : unshifted[keycode - 0x1E];
    }

    // Basic punctuation and space
    switch (keycode) {
    case 0x2C:
        return ' '; // Space
    case 0x2D:
        return shift ? '_' : '-';
    case 0x2E:
        return shift ? '+' : '=';
    case 0x2F:
        return shift ? '{' : '[';
    case 0x30:
        return shift ? '}' : ']';
    case 0x31:
        return shift ? '|' : '\\';
    case 0x33:
        return shift ? ':' : ';';
    case 0x34:
        return shift ? '"' : '\'';
    case 0x35:
        return shift ? '~' : '`';
    case 0x36:
        return shift ? '<' : ',';
    case 0x37:
        return shift ? '>' : '.';
    case 0x38:
        return shift ? '?' : '/';
    }

    return 0; // Unknown or non-printable
}

int parse_hid_packet(HID_MESSAGE_PACKET_t *packet, Event_t *events, int max_events) {
    int num_events = 0;

    uint8_t *data = packet->message.new_report.data;
    int data_len = packet->message.new_report.length;

    char modifier = data[0];
    bool is_l_ctrl = (data[0] >> 0) & 0b1;
    bool is_l_shift = (data[0] >> 1) & 0b1;
    bool is_l_alt = (data[0] >> 2) & 0b1;
    bool is_l_cmd = (data[0] >> 3) & 0b1;

    bool is_r_ctrl = (data[0] >> 4) & 0b1;
    bool is_r_shift = (data[0] >> 5) & 0b1;
    bool is_r_alt = (data[0] >> 6) & 0b1;
    bool is_r_cmd = (data[0] >> 7) & 0b1;

    bool is_shift = is_l_shift || is_r_shift;
    bool is_ctrl = is_l_ctrl || is_r_ctrl;
    bool is_alt = is_l_alt || is_r_alt;
    bool is_cmd = is_l_cmd || is_r_cmd;

    uint8_t key_seen[256] = {0};

    for (int i = 2; i < data_len; i++) {

        uint8_t keycode = keycode_from_report(data[i]);
        char ascii = keycode_to_ascii(keycode, is_shift);

        key_seen[keycode] = 1;

        if (key_down[keycode] == 0x00 && num_events < max_events - 1) {
            // key pressed
            events[num_events].type = EVENT_KEY_PRESSED;
            events[num_events].keycode = keycode;
            events[num_events].ascii = ascii;
            num_events += 1;

            // mark as pressed
            key_down[keycode] = 1;
        }
    }

    for (int keycode = 1; keycode < 256 && num_events < max_events; keycode++) {
        if (key_down[keycode] && !key_seen[keycode]) {
            events[num_events].type = EVENT_KEY_RELEASED;
            events[num_events].keycode = keycode;
            events[num_events].ascii = keycode_to_ascii(keycode, false);
            num_events++;

            key_down[keycode] = 0;
        }
    }

    return num_events;
}

void print_hid_packet(const HID_MESSAGE_PACKET_t *packet) {
    switch (packet->type) {
    case DEVICE_CONNECTED:
        printf("HID Message: DEVICE_CONNECTED\n");
        printf("  dev_addr: %d\n", packet->message.connected.dev_addr);
        break;

    case DEVICE_DISCONNECTED:
        printf("HID Message: DEVICE_DISCONNECTED\n");
        printf("  dev_addr: %d\n", packet->message.disconnected.dev_addr);
        break;

    case NEW_REPORT:
        printf("HID Message: NEW_REPORT\n");
        printf("  dev_addr : %d\n", packet->message.new_report.dev_addr);
        printf("  report_id: %d\n", packet->message.new_report.report_id);
        printf("  length   : %d\n", packet->message.new_report.length);
        printf("  data     :");
        for (int i = 0; i < packet->message.new_report.length; i++) {
            printf(" %02X", packet->message.new_report.data[i]);
        }
        printf("\n");
        break;

    default:
        printf("HID Message: UNKNOWN TYPE (%d)\n", packet->type);
        break;
    }
}
