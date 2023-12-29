#define PACKED __attribute__((packed))


struct EventRingSegment {
    uint8_t rvsdz0: 6;
    uint64_t ring_segment_base_address: 58;

    uint16_t ring_segment_size;
    uint16_t rsvdz1[3];
} PACKED;

