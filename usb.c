#include <inc/x86.h>
#include <inc/string.h>
#include "fs/pci.h"

// ПОТОМ ПЕРЕНЕСТИ в pci.h строка 53
#define XHCI_VADDR       0x7010000000
#define XHCI_DEQUE_VADDR 0x7011000000

//used already
#define XHCI_MAX_MAP_MEM 0x4000
//not used yet
#define INTERRUPTER_REGISTER_SET_COUNT 256
#define EVENT_RING_TABLE_SIZE 256
#define EVENT_RING_SEGMENT_SIZE 128
#define COMMAND_RING_DEQUE_SIZE 4096
#define DCBAAP_SIZE 128

//аналогично nvme
struct XhciController {
    struct PciDevice *pcidev;
    volatile uint8_t *mmio_base_addr;
};
static struct XhciController xhci;

// это сама работа с xhci
struct CapabilityRegisters {
    volatile uint8_t caplength;
    volatile uint8_t rsvd0;
    volatile uint16_t hciversion;
    volatile uint32_t hcsparams[3];
    volatile uint32_t hccparams1;
    volatile uint32_t dboff;
    volatile uint32_t rtsoff;
    volatile uint32_t hccparams2;
} * cap_regs;
struct OperationalRegisters {
    volatile uint32_t usbcmd;
    volatile uint32_t usbsts;
    volatile uint32_t pagesize;
    volatile uint32_t rsvdz0;
    volatile uint32_t dnctrl;
    volatile uint64_t crcr;
    volatile uint32_t rsvdz1[4];
    volatile uint64_t dcbaap;
    volatile uint32_t config;
    volatile uint32_t rsvdz2[241];
} * oper_regs;
struct InterrupterRegisterSet {
    volatile uint32_t iman;
    volatile uint32_t imod;
    volatile uint32_t erstsz;
    volatile uint32_t rsvd;
    volatile uint64_t erstba;
    volatile uint64_t erdp;
};
struct RuntimeRegisters {
    volatile uint32_t mfindex;
    volatile uint32_t rsvdz[7];
    volatile struct InterrupterRegisterSet int_reg_set[INTERRUPTER_REGISTER_SET_COUNT];
} * run_regs;
struct EventRingTableEntry {
    volatile uint64_t ring_segment_base_address;
    volatile uint8_t ring_segment_size;
    volatile uint8_t rsvdz[3];
};
struct Doorbell {
    volatile uint32_t dbreg[256];
} * doorbells;
// not used yet
int controller_not_ready() {
    return (oper_regs->usbsts & (1 << 11)) >> 11;
}
volatile uint8_t * memory_space_ptr;
volatile uint8_t * dcbaap;
volatile uint8_t * device_context[32];
volatile uint8_t * dcbaap;
volatile uint8_t * command_ring_deque;
volatile struct EventRingTableEntry * event_ring_segment_table;
volatile uint32_t event_ring_segment_table_size;
volatile uint8_t * event_ring_segment_base_address;
volatile uint8_t * event_ring_deque;


int xhci_map(XhciController *ctl) {
    //готово, не тестировано
    ctl->mmio_base_addr = (volatile uint8_t *)XHCI_VADDR;
    size_t size = get_bar_size(ctl->pcidev, 0);
    size = size > XHCI_MAX_MAP_MEM ? XHCI_MAX_MAP_MEM : size;
    int res = sys_map_physical_region(get_bar_address(ctl->pcidev, 0), CURENVID, (void *)ctl->mmio_base_addr, size, PROT_RW | PROT_CD);
    // добавить коды ошибок
    if (res)
        return 1;
    return 0;
}

void xhci_memory_init(XhciController *ctl) {
    // ДАЛЕЕ переделать
    cap_regs = (struct CapabilityRegisters *)memory_space_ptr;
    oper_regs = ((void *)memory_space_ptr + cap_regs->caplength);
    run_regs = ((void *)memory_space_ptr + (cap_regs->rtsoff & 0xFFFFFFF0));
    doorbells = ((void *)memory_space_ptr + cap_regs->dboff);
    while(controller_not_ready()) {}

    dcbaap = (void *)oper_regs->dcbaap + 0x8040000000;
    //dcbaap = mmio_map_region(oper_regs->dcbaap, XHCI_MEMORY_SPACE_BUFFER_SIZE);
    device_context[1] = (void *)(((uint64_t *)dcbaap)[1] + 0x8040000000);

    command_ring_deque = (void *)(oper_regs->crcr & (~((uint64_t)0x3f)));
    if (command_ring_deque != 0) {
        command_ring_deque = mmio_map_region((uint64_t)command_ring_deque, COMMAND_RING_DEQUE_SIZE);
    }

    event_ring_deque = (void *)run_regs->int_reg_set[0].erdp + 0x8040000000;
    event_ring_segment_table = mmio_map_region((uint64_t)run_regs->int_reg_set[0].erstba, EVENT_RING_TABLE_SIZE);
    event_ring_segment_table_size = run_regs->int_reg_set[0].erstsz;
    event_ring_segment_base_address = mmio_map_region(event_ring_segment_table[0].ring_segment_base_address, sizeof(struct TRBTemplate) * EVENT_RING_SEGMENT_SIZE);
    for (int i = 0; i < sizeof(struct TRBTemplate) * EVENT_RING_SEGMENT_SIZE; i++) {
        event_ring_segment_base_address[i] = 0;
    }
    event_ring_segment_table[0].ring_segment_size = EVENT_RING_SEGMENT_SIZE;
    event_ring_deque = event_ring_segment_base_address;
    //run_regs->int_reg_set[0].erstba = (uint64_t)event_ring_segment_table;
}


void xhci_init() {
    struct PciDevice *pcidevice = find_pci_dev(6, 1);
    int err;
    if (pcidevice == NULL)
        panic("NVMe device not found\n");
    xhci.pcidev = pcidevice;
    if ( xhci_map(ctl) )
        return 1;
    xhci_memory_init();
    // ПРОДОЛЖИТЬ см nvme_init
}

void
umain(int argc, char **argv) {
    cprintf("***********************INIT USB***********************\n");
    // уже написана в fs/pci.c
    pci_init(argv);
    // делаем похожей на nvme
    xhci_init();
    cprintf("***********************END USB***********************\n");
}

