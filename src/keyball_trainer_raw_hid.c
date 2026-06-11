/*
 * Keyball trainer raw HID telemetry.
 *
 * Packet format, 32 bytes:
 * 0..1  magic "KB"
 * 2     version
 * 3     type: 1=position
 * 4     state: 1=down, 0=up
 * 5     highest active layer index
 * 6..9  key position, little endian
 * 10    source
 */

#include <stdint.h>
#include <string.h>

#include <raw_hid/events.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/keymap.h>

#define KBTRAIN_MAGIC_0 'K'
#define KBTRAIN_MAGIC_1 'B'
#define KBTRAIN_VERSION 1
#define KBTRAIN_TYPE_POSITION 1
#define KBTRAIN_REPORT_SIZE 32

static void put_u32_le(uint8_t *buf, uint32_t value) {
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
}

static int keyball_trainer_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *pos_ev = as_zmk_position_state_changed(eh);
    if (pos_ev == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    static uint8_t report[KBTRAIN_REPORT_SIZE];
    memset(report, 0, sizeof(report));
    report[0] = KBTRAIN_MAGIC_0;
    report[1] = KBTRAIN_MAGIC_1;
    report[2] = KBTRAIN_VERSION;
    report[3] = KBTRAIN_TYPE_POSITION;
    report[4] = pos_ev->state ? 1 : 0;
    report[5] = zmk_keymap_highest_layer_active();
    put_u32_le(&report[6], pos_ev->position);
    report[10] = pos_ev->source;

    raise_raw_hid_sent_event((struct raw_hid_sent_event){
        .data = report,
        .length = sizeof(report),
    });

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(keyball_trainer_raw_hid, keyball_trainer_listener);
ZMK_SUBSCRIPTION(keyball_trainer_raw_hid, zmk_position_state_changed);
