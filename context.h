#define PACKED     __attribute__((packed))
#define ALIGNED(n) __attribute__((aligned(n)))



/*struct SlotContext{
    uint32_t route_string: 20;
    uint8_t speed: 4;
    uint8_t rzvdz0: 1;
    bool mtt: 1;
    bool hub: 1;
    uint8_t context_entries: 5;

    uint16_t max_exit_latency;
    uint8_t root_hub_port_number;
    uint8_t number_of_ports;

    uint8_t parent_hub_slot_id;
    uint8_t parent_port_number;
    uint8_t ttt: 2;
    uint8_t rzvdz1: 4;
    uint16_t interrupter_target: 10;

    uint8_t usb_device_address;
    uint32_t rsvdz2: 19;
    uint8_t slot_state: 5;

    uint32_t rsvdo[4]; 
} PACKED;




struct EndpointContext {
    uint8_t ep_state: 3;
    uint8_t rsvdz0: 5;
    uint8_t mult: 2;
    uint8_t maxPStreams: 5;
    bool lsa:1;
    uint8_t interval;
    uint8_t max_esit_payload_hi;

    uint8_t rsvdz1: 1;
    uint8_t CErr: 2;
    uint8_t EP_type: 3;
    uint8_t rsvdz2: 1;
    bool hid: 1;
    uint8_t max_burst_size;
    uint16_t max_packet_size;
    
    bool dcs: 1;
    uint8_t rsvdz3: 3;
    uint64_t tr_dequeue_pointer:60;

    uint16_t avg_trb_length;
    uint16_t max_esit_payload_lo;

    uint32_t rsvdo[3];
} PACKED;

// 6.2.1
struct EndpointContext DeviceContext[32];



struct StreamContext {
    bool dcs: 1;
    uint8_t sct: 3;
    uint64_t tr_dequeue_pointer: 60;
    uint32_t stopped_edtla: 24;
    uint8_t rsvdo[5];
} PACKED;


struct InputControlContext {
    // 1:0 - rsvdz0
    uint32_t drop_context_flags;
    uint32_t add_context_flags;
    uint32_t reserved[5];
    uint8_t config_value;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t rsvdz0;
};

// 6.2.5
// 0's element is InputControlContext
// remaining same as in DeviceContext
struct EndpointContext InputContext[33];*/

#define SLOT_CONTEXT_ROUTE_STRING_OFFSET 0x0         // ALL OFFSETS IN BITS FROM START OF 32BIT ALIGNED ADDRESS, NOT BYTES FROM START OF STRUCT
#define SLOT_CONTEXT_SPEED_OFFSET 0x14
#define SLOT_CONTEXT_MTT_OFFSET 0x1a
#define SLOT_CONTEXT_HUB_OFFSET 0x1b
#define SLOT_CONTEXT_CONTEXT_ENTRIES_OFFSET 0x1c

#define SLOT_CONTEXT_TTT_OFFSET 0x10
#define SLOT_CONTEXT_INTERRUPTER_TARGET_OFFSET 0x16

struct SlotContext {
    uint32_t first_row; // defined like this because of shifted data. Contains route_string(0-19 bytes), speed(20-23), MTT(25), HUB(26), context_entries(27-31)
    
    uint16_t max_exit_latency;
    uint8_t root_hub_port_number;
    uint8_t number_of_ports;

    uint8_t tt_hub_slot_id;
    uint8_t tt_port_number;
    uint16_t ttt_and_interrupter_target;

    uint8_t usb_device_address;
    uint8_t reserved0[2];
    uint8_t slot_state;            // ALERT HERE MUST BE 4BIT OFFSET TO START OF SLOT STATE
    uint32_t reserved1[4]; 
};

struct EndpointContext {
    uint8_t ep_state;
    uint8_t mult_and_max_p_streams;
    uint8_t interval;
    uint8_t max_esit_payload_hi;

    uint8_t flags_off_08;
    uint8_t max_burst_size;
    uint16_t max_packet_size;

    uint64_t transfer_ring_deque_ptr;

    uint16_t avg_trb_length;
    uint16_t max_esit_payload_lo;

    uint32_t reserved[3];
};

struct DeviceContext {
    struct SlotContext slot_context;
    struct EndpointContext endpoint_context[31];
};

struct InputControlContext {
    uint32_t drop_context;
    uint32_t add_context;
    uint32_t reserved[5];
    uint8_t config_value;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t rsvd;
};

struct InputContext {
    struct InputControlContext input_control_context;
    struct SlotContext slot_context;
    struct EndpointContext endpoint_context[31];
};

struct TransferTRB {
    volatile uint64_t trb_pointer;

    volatile uint16_t trb_transfer_len_lo;
    volatile uint8_t trb_transfer_len_hi;
    volatile uint8_t completion_code;

    volatile uint16_t flags;
    volatile uint8_t endpoint_id;
    volatile uint8_t slot_id;
};

struct AddressDeviceCommandTRB {
    volatile uint64_t input_context_ptr;

    volatile uint32_t reserved0;

    volatile uint16_t flags;
    volatile uint8_t reserved1;
    volatile uint8_t slot_id;
};

struct AddressDeviceCommandTRB address_device_command(uint64_t input_context_ptr, uint8_t slot_id, bool cycle_bit) {
    struct AddressDeviceCommandTRB adr_trb;
    adr_trb.slot_id = slot_id;
    adr_trb.input_context_ptr = input_context_ptr;
    adr_trb.reserved0 = 0;
    adr_trb.flags = 11 << 10;
    adr_trb.reserved1 = 0;
    if (cycle_bit) {
        adr_trb.flags += 1;
    }

    return adr_trb;
}
