#define PACKED     __attribute__((packed))
#define ALIGNED(n) __attribute__((aligned(n)))

// это сама работа с xhci
struct CapabilityRegisters {
    volatile uint8_t caplength;
    volatile uint8_t rsvd0;
    volatile uint16_t hciversion;
    volatile uint32_t hcsparams1;
    // volatile union {
    //     uint32_t all;
    //     struct {
    //     uint8_t MaxSlots;
    //     uint16_t MaxIntrs: 11;
    //     uint8_t rsvd: 5;
    //     uint8_t MaxPorts;
    //     } PACKED;
    // } hcsparams1;
    volatile uint32_t hcsparams2;
    volatile uint32_t hcsparams3;
    volatile uint32_t hccparams1;
    uint8_t rsvd1 : 2;
    volatile uint32_t dboff: 30;
    uint8_t rsvd2 : 5;
    volatile uint32_t rtsoff: 27;
    volatile uint32_t hccparams2;
} PACKED * cap_regs;



struct OperationalRegisters {
    volatile uint32_t usbcmd;
    volatile uint32_t usbsts;
    volatile uint32_t pagesize;
    volatile uint32_t rsvdz0[2];
    volatile uint32_t dnctrl;
    volatile uint64_t crcr;
    volatile uint32_t rsvdz1[4];
    volatile uint64_t dcbaap;
    volatile uint32_t config;
    volatile uint8_t rsvdz2[964];
    // see table 5-18
    // ...

} * oper_regs;

struct PortRegisterSet {
    uint32_t portsc;
    uint32_t portpmsc;
    uint32_t portli;
    uint32_t porthlpmc;
};

#define USBCMD_RS 0
#define USBCMD_HCRST 1
#define USBCMD_INTE 2
#define USBCMD_HSEE 3
#define USBCMD_LHCRST 7
#define USBCMD_CSS 8
#define USBCMD_CRS 9
#define USBCMD_EWE 10
#define USBCMD_EU3S 11
#define USBCMD_CME 13
#define USBCMD_ETE 14
#define USBCMD_TSC_EN 15
#define USBCMD_VTIOE 16

// TODO: add other constants (5.4.2 - 5.4.11)


struct InterrupterRegisterSet {
    volatile uint32_t iman;
    volatile struct {
        uint16_t interval;
        uint16_t counter;
    } imod;
    volatile struct{
        uint16_t erstsz;
        uint16_t rsvd;
    } erstsz;
    volatile uint32_t rsvd;
    volatile struct {
        uint8_t rsvd: 6;
        uint64_t erstba: 58;
    } PACKED erstba;
    volatile struct {
        uint8_t desi: 3;
        bool ehb: 1;
        uint64_t erdp: 60;
    } PACKED erdp;
};

#define IMAN_IP 0
#define IMAN_IE 1

struct DoorbellRegistter {
    uint8_t target;
    uint8_t rsvd;
    uint16_t stream_id;
};


#define INTERRUPTER_REGISTER_SET_MAXCOUNT 1024

struct RuntimeRegisters {
    volatile uint32_t mfindex;
    volatile uint32_t rsvdz[7];
    volatile struct InterrupterRegisterSet int_reg_set[INTERRUPTER_REGISTER_SET_MAXCOUNT];
} * run_regs;
