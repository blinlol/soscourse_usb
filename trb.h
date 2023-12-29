#define PACKED     __attribute__((packed))
#define ALIGNED(n) __attribute__((aligned(n)))

// Transfer TRBs (6.4.1)
struct NormalTRB{
    uint64_t data_buffer_ptr;
    uint32_t transfer_length: 17;
    uint8_t td_size: 5;
    uint16_t interrupter_target: 10;
    bool c: 1;
    bool ent: 1;
    bool isp: 1;
    bool ns: 1;
    bool ch: 1;
    bool ioc: 1;
    bool idt: 1;
    uint8_t rsvdz: 2;
    bool bei: 1;
    uint8_t type: 6;
    uint16_t rscdz;
} PACKED ;

struct SetupStageTRB {
    uint8_t bm_request_type;
    uint8_t b_request;
    uint16_t w_value;

    uint16_t w_index;
    uint16_t w_length;
    
    // always 8
    uint32_t trb_transfer_len: 17;
    uint8_t rsvdz0: 5;
    uint16_t interrupter_target: 10;
    
    bool c: 1;
    uint8_t rsvdz1: 4;
    bool ioc: 1;
    
    // set to 1 in setup stage
    bool idt: 1;
    uint8_t rsvdz2: 3;
    uint8_t type: 6
    uint8_t trt: 2;
    uint16_t rsvdz3: 14
} PACKED;

struct DataStageTRB {
    uint64_t data_buffer_ptr;
    uint32_t transfer_length: 17;
    uint8_t td_size: 5;
    uint8_t interrupter_target: 10;
    bool c: 1;
    bool ent: 1;
    bool isp: 1;
    bool ns: 1;
    bool ch: 1;
    bool ioc: 1;
    bool idt: 1;
    uint8_t rsvdz: 3;
    uint8_t type: 6;
    bool dir: 1;
    uint16_t rscdz: 15;
} PACKED;

struct StatusStageTRB {
    uint64_t rsvdz0;
    uint32_t rsvdz1: 21;
    uint8_t interrupter_target: 10;

    bool c: 1;
    bool ent: 1;
    uint8_t rsvdz2: 2;
    bool ch:1;
    bool ioc: 1;
    uint8_t rsvdz3: 4;
    uint8_t type: 6;
    bool dir: 1;
    uint16_t rscdz: 15;
} PACKED;

struct IsochTRB {
    uint64_t data_buffer_ptr;
    
    uint32_t transfer_length: 17;

    // if ETE == 0: td_size
    // elif ETE == 1: TBC
    uint8_t td_size_tbc: 5;
    uint16_t interrupter_target: 10;

    bool c: 1;
    bool ent: 1;
    bool isp: 1;
    bool ns: 1;
    bool ch: 1;
    bool ioc: 1;
    bool idt: 1;
    uint8_t transfer_burst_count: 2;
    bool bei: 1;
    uint8_t type: 6;
    uint8_t tlbpc: 4;
    uint16_t frame_id: 11
    bool sia: 1;   
} PACKED; 

struct NoOpTRB {
    uint64_t rsvdz0;
    uint32_t rsvdz1: 22;
    uint16_t interrupter_target: 10;

    bool c: 1;
    bool ent: 1;
    uint8_t rsvdz2: 2;
    bool ch:1;
    bool ioc: 1;
    uint8_t rsvdz3: 4;
    uint8_t type: 6;
    uint16_t rscdz4;
} PACKED;


// Event TRBs (6.4.2)
struct TransferEventTRB {
    uint64_t trb_ptr;

    uint32_t transfer_length: 24;
    uint8_t completion_code;

    bool c: 1;
    bool rsvdz0: 1;
    bool ed: 1;
    uint8_t rsvdz1: 7;
    uint8_t type: 6;
    uint8_t endpoint_id: 5;
    uint8_t rsvdz2: 3;
    uint8_t slot_id;
} PACKED;

struct CommandCompletionEventTRB {
    uint8_t rsvdz0: 4;
    uint64_t command_trb_ptr: 60;

    uint32_t command_completion_parameter: 24;
    uint8_t completino_code;

    bool c: 1;
    uint16_t rsvdz0: 9;
    uint8_t type: 6;
    uint8_t vf_id;
    uint8_t slot_id;
} PACKED;

struct PortStatusChangeEventTRB {
    uint32_t rsvdz0: 24;
    uint8_t port_id;
    uint32_t rsvdz1;

    uint32_t rsvdz1: 24;
    uint8_t completion_code;

    bool c: 1;
    uint16_t rsvdz2: 9;
    uint8_t type: 6;
    uint16_t rsvdz3;
} PACKED;

struct BandwidthRequestEventTRB {
    uint64_t rsvdz0;

    uint32_t rsvdz1: 24;
    uint8_t completion_code;

    bool c: 1;
    uint16_t rsvdz2: 9;
    uint8_t type: 6;
    uint8_t rsvdz3;
    uint8_t slot_id;
} PACKED;

struct DoorbellEventTRB {
    uint8_t db_reason: 5;
    uint32_t rsvdz0: 27;
    uint32_t rsvdz1;

    uint32_t rsvdz2: 24;
    uint8_t completion_code;

    bool c: 1;
    uint16_t rsvdz3: 9;
    uint8_t type: 6;
    uint8_t vf_id;
    uint8_t slot_id;
} PACKED;

struct HostControllerEventTRB {
    uint8_t rsvdz0[11];
    uint8_t completion_code;

    bool c: 1;
    uint16_t rsvdz1: 9;
    uint8_t type: 6;
    uint16_t rsvdz2;
} PACKED;

struct DeviceNotificationEventTRB {
    uint8_t rsvdz0: 4;
    uint8_t notification_type: 4;
    uint64_t device_notification_data: 56;

    uint32_ t rsvdz1: 24;
    uint8_t completion_code;

    bool c: 1;
    uint16_t rsvdz2: 9;
    uint8_t type: 6;
    uint8_t rsvdz3;
    uint8_t slot_id;
} PACKED;

struct MFINDEXWrapEventTRB {
    uint8_t rsvdz0[11];
    uint8_t completion_code;

    bool c: 1;
    uint16_t rsvdz1: 9;
    uint8_t type: 6;
    uint16_t rsvdz2;
} PACKED;


// Command TRBs (6.4.3)

struct NoOpCommandTRB {
    uint32_t rsvdz0[3];
    
    bool c: 1;
    uint16_t rsvdz1: 9;
    uint8_t type: 6;
    uint8_t slot_type: 5
    uint16_t rsvdz2: 11;
} PACKED;

struct EnableSlotCommandTRB {
    uint32_t rsvdz0[3];

    bool c: 1;
    uint16_t rsvdz1: 9;
    uint8_t type: 6;
    uint16_t rsvdz2;
} PACKED;

struct DisableSlotCommandTRB {
    uint32_t rsvdz[3];

    bool c: 1;
    uint16_t rsvdz1: 9;
    uint8_t type: 6;
    uint8_t rsvdz2;
    uint8_t slot_id;
} PACKED;

struct AddressDeviceCommandTRB {
    uint8_t rsvdz0: 4;
    uint64_t input_context_ptr: 60;
    
    uint32_t rsvdz1;

    bool c: 1;
    uint8_t rsvdz2;
    bool bsr: 1;
    uint8_t type: 6;
    uint8_t rsvdz3;
    uint8_t slot_id;
} PACKED;

struct ConfigureEndpointCommandTRB {
    uint8_t rsvdz0: 4;
    uint64_t input_context_ptr: 60;

    uint32_t rsvdz1;

    bool c: 1;
    uint8_t rsvdz2;
    bool dc: 1;
    uint8_t type: 6;
    uint8_t rsvdz3;
    uint8_t slot_id;
} PACKED;

struct EvaluateContextCommandTRB {
    uint8_t rsvdz0: 4;
    uint64_t input_context_ptr: 60;
    
    uint32_t rsvdz1;

    bool c: 1;
    uint8_t rsvdz2;
    bool bsr: 1;
    uint8_t type: 6;
    uint8_t rsvdz3;
    uint8_t slot_id;
} PACKED;

struct ResetEndpointCommandTRB {
    uint32_t rsvdz0[3];

    bool c: 1;
    uint8_t rsvdz1;
    bool tsp: 1;
    uint8_t type: 6;
    uint8_t endpoint_id: 5;
    uint8_t rsvdz2: 3;
    uint8_t slot_id;
} PACKED;

struct StopEndpointCommandTRB {
    uint32_t rsvdz0[3];

    bool c: 1;
    uint16_t rsvdz1: 9;
    uint8_t type: 6;
    uint8_t endpoint_id: 5;
    uint8_t rsvdz2: 2;
    bool sp: 1;
    uint8_t slot_id;
} PACKED;

struct SetTRDequeuePointerCommandTRB {
    bool dcs: 1;
    uint8_t sct: 3;
    uint64_t new_tr_dequeue_ptr: 60;

    uint16_t rsvdz0;
    uint16_t stream_id;

    bool c: 1;
    uint16_t rsvdz1: 9;
    uint8_t type: 6;
    uint8_t endpoint_id: 5;
    uint8_t rsvdz2: 3;
    uint8_t slot_id;
} PACKED;

struct ResetDeviceCommandTRB {
    uint32_t rsvdz0[3];

    bool c: 1;
    uint16_t rsvdz1: 9;
    uint8_t type: 6;
    uint8_t rsvdz2;
    uint8_t slot_id;
} PACKED;

struct ForceEventCommandTRB {
    uint8_t rsvdz0: 4;
    uint64_t event_trb_ptr: 60;

    uint32_t rsvdp1: 22;
    uint16_t vf_interrupter_target: 10;

    bool c: 1;
    uint16_t rsvdz1: 9;
    uint8_t type: 6;
    uint8_t vf_id;
    uint8_t rsvdz2;
} PACKED;

struct NegotiateBandwidthCommandTRB{
    // Optional
} PACKED;

struct SetLTVCommandTRB{
    // Optional
} PACKED;

struct GetPortBandwidthCommandTRB {
    uint8_t rsvdz0: 4;
    uint64_t port_bandwidth_context_ptr: 60;

    uint32_t rsvdz1;

    bool c: 1;
    uint16_t rsvdz2: 9;
    uint8_t type: 6;
    uint8_t dev_speed: 4;
    uint8_t rsvdz3: 4;;
    uint8_t hub_slot_id;
} PACKED;

struct ForceHeaderCommandTRB {
    uint8_t packet_type: 5;
    uint32_t header_info_lo: 27;
    uint32_t header_info_mid
    uint32_t header_info_hi;

    bool c: 1;
    uint16_t rsvdz: 9;
    uint8_t type: 6;
    uint8_t rsvdz2;
    uint8_t root_hub_port_number;
} PACKED;

struct GetExtendedProperyCommandTRB {
    uint8_t rsvdz0: 4;
    uint64_t extended_property_context_ptr: 60;

    uint32_t eci;
    uint32_t rsvdz1;

    bool c: 1;
    uint16_t rsvdz2: 9;
    uint8_t type: 6;
    uint8_t command_sub_type: 3;
    uint8_t endpoint_id: 5;
    uint8_t slot_id;
} PACKED;

struct SetExtendedProperyCommandTRB {
    uint32_t rsvdz0[2];

    uint16_t eci;
    uint16_t capability_parameter: 9;
    uint8_t: reserved;

    bool c: 1;
    uint16_t rsvdz1: 9;
    uint8_t type: 6;
    uint8_t command_sub_type: 3;
    uint8_t endpoint_id: 5;
    uint8_t slot_id;
} PACKED;


// Other TRB's (6.4.4)

struct LinkTRB {
    uint8_t rsvdz0: 4;
    uint64_t ring_segment_ptr: 60;

    uint32_t rsvdz1: 22;
    uint16_t interrupter_target: 10;

    bool c: 1;
    bool tc: 1;
    uint8_t rsvdz2: 2;;
    bool ch: 1;
    bool ioc: 1;
    uint8_t rsvdz3: 4;
    uint8_t type: 6;
    uint16_t rsvdz4;
} PACKED;

struct EventDataTRB {
    uint64_t event_data;

    uint32_t rsvdz0: 22;
    uint16_t interrupter_target: 10;

    bool c: 1;
    bool ent: 1;
    uint8_t rsvdz1: 2;;
    bool ch: 1;
    bool ioc: 1;
    uint8_t rsvdz2: 3;
    bool bei: 1;
    uint8_t type: 6;
    uint16_t rsvdz3;
} PACKED;



// TRB Completion Codes (6.4.5)

// TRB Types (6.4.6)