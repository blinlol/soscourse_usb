#define PACKED     __attribute__((packed))
#define ALIGNED(n) __attribute__((aligned(n)))



struct SlotContext{
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
struct EndpointContext InputContext[33];
