#include <inc/x86.h>
#include <inc/string.h>
#include <inc/lib.h>
#include "fs/pci.h"

#include "usb/context.h"
#include "usb/registers.h"


// ПОТОМ ПЕРЕНЕСТИ в pci.h строка 53
#define XHCI_DATA_STRUCT_VADDR 0x7010000000
#define XHCI_DATA_STRUCT_SIZE (1024*1024) // 2 Mib

//used already
#define XHCI_MAX_MAP_MEM 0x4000
#define XHCI_RING_DEQUE_MEM 4096 //why 4096?
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
    volatile uint8_t *buffer;
};
static struct XhciController xhci;
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



void print_usb_memory_region() {
    // cprintf("Capability Registers:\n");
    // cprintf("CAPLENGTH: %d\n", cap_regs->caplength);
    // cprintf("HCIVERSION: %x\n", cap_regs->hciversion);
    // cprintf("HCSPARAMS1: %x\n", cap_regs->hcsparams1);
    // cprintf("HCSPARAMS2: %x\n", cap_regs->hcsparams2);
    // cprintf("HCSPARAMS3: %x\n", cap_regs->hcsparams3);
    // cprintf("HCCPARAMS1: %x\n", cap_regs->hccparams1);
    // cprintf("DBOFF: %x\n", cap_regs->dboff);
    // cprintf("RTSOFF: %x\n", cap_regs->rtsoff);
    // cprintf("HCCPARAMS2: %x\n", cap_regs->hccparams2);
    // cprintf("\n");
    // cprintf("Operational Registers:\n");
    // cprintf("USBCMD: %x\n", oper_regs->usbcmd);
    // cprintf("USBSTS: %x\n", oper_regs->usbsts);
    // cprintf("PAGESIZE: %x\n", oper_regs->pagesize);
    // cprintf("DNCTRL: %x\n", oper_regs->dnctrl);
    // cprintf("CRCR: %lx\n", oper_regs->crcr);
    // cprintf("DCBAAP: %lx\n", oper_regs->dcbaap);
    // cprintf("CONFIG: %x\n", oper_regs->config);
    // cprintf("\n");
//     for (int i = 0; i < 8; i++) {
//         volatile uint32_t portsc = get_portsc(i+1);
//         if ((portsc & (1 << 0)) >> 0) {
//             cprintf("PORTSC %d  - %08x\n", i+1, portsc);
//             cprintf("  CCS(connect status): %x\n", (portsc & (1 << 0)) >> 0);
//             cprintf("  PED(port en/dis): %x\n", (portsc & (1 << 1)) >> 1);
//             cprintf("  PLS(link state): %x\n", (portsc & (0xF << 5)) >> 5);
//             cprintf("  PP(port power): %x\n", (portsc & (1 << 9)) >> 9);
//             cprintf("  Port speed: %x\n", (portsc & (0xF << 10)) >> 10);
//             cprintf("  Port indicator control: %x\n", (portsc & (0x3 << 14)) >> 14);
//             cprintf("  LWS(link state write strobe): %x\n", (portsc & (1 << 16)) >> 16);
//             cprintf("  CSC(connect status change): %x\n", (portsc & (1 << 17)) >> 17);
//             cprintf("  PEC(port en/dis change): %x\n", (portsc & (1 << 18)) >> 18);
//         }
//     }
//     cprintf("\n");
//     for (int i = 0; i < 8; i++) {
//         cprintf("Doorbell register %d: %08x\n", i, doorbells->dbreg[i]);
//     }
//     volatile uint16_t xecp_offset = (cap_regs->hccparams1 >> 16);
//     //cprintf("hccparams1 - %08x\n", cap_regs->hccparams1);
//     cprintf("xecp offset - %08x\n", xecp_offset);
//     volatile uint8_t * xecp_pointer = ((uint8_t *)cap_regs + (xecp_offset << 2));
//     cprintf("\nExtended capability:\n");
//     cprintf("    ID: %02x\n", xecp_pointer[0]);
//     cprintf("    next cap ptr: %02x\n", xecp_pointer[1]);
//     cprintf("    BIOS semaphore + rsvd: %02x\n", xecp_pointer[2]);
//     cprintf("    OS semaphore + rsvd: %02x\n", xecp_pointer[3]);
//     cprintf("\n");
//     cprintf("Command ring dequeue ptr: %016lx\n", ((uint64_t)command_ring_deque));
//     cprintf("Command ring dequeue: ");
//     for (int i = 0; i < 16; i++) {
//         cprintf("%02x", command_ring_deque[i]);
//     }
//     cprintf("\n");
//     cprintf("Device context base address array:\n");
//     for (int i = 0; i < 10; i++) {
//         cprintf("  %02x -- %08x\n", i * 4, ((uint32_t *)dcbaap)[i]);
//     }
//     cprintf("\n");
//     cprintf("Runtime registers:\n");
//     cprintf("MFINDEX: %08x\n", run_regs->mfindex);
//     for (int i = 0; i < 1; i++) {
//         cprintf("  Int reg set: %d\n", i);
//         cprintf("    inter manag %08x\n", run_regs->int_reg_set[i].iman);
//         cprintf("    inter moder %08x\n", run_regs->int_reg_set[i].imod);
//         cprintf("    tab size %08x\n", run_regs->int_reg_set[i].erstsz);
//         cprintf("    tab addr %016lx\n", run_regs->int_reg_set[i].erstba);
//         cprintf("    deque ptr %016lx\n", run_regs->int_reg_set[i].erdp);
//     }
//     cprintf("Event ring segment table:\n");
//     for (int i = 0; i < event_ring_segment_table_size; i++) {
//         cprintf("  %016lx - %d\n", event_ring_segment_table[i].ring_segment_base_address, event_ring_segment_table[i].ring_segment_size);
//         //for (int j = 0; j < event_ring_segment_table[i].ring_segment_size; j++) {
//         //    cprintf("    ")
//         //}
//     }
//     //struct CommandCompletionTRB cc_trb = ((struct CommandCompletionTRB *)event_ring_segment_base_address)[1];
//     cprintf("Event ring deque ptr: %016lx\n", (uint64_t)event_ring_deque);
//     cprintf("\nCommand Completion:\n");
//     for (int i = 0; i < 128; i++) {
//         struct CommandCompletionTRB cc_trb = ((struct CommandCompletionTRB *)event_ring_segment_base_address)[i];
//         if (cc_trb.command_trb_ptr != 0) {
//             cprintf("TRB #%d\n", i);
//             cprintf("Address: %016lx\n", (uint64_t)((struct CommandCompletionTRB *)event_ring_segment_base_address + i));
//             cprintf("CommandTRB ptr: %016lx\n", cc_trb.command_trb_ptr);
//             cprintf("Compl code part: %08x\n", cc_trb.completion_code);
//             cprintf("TRB type: %08x\n", cc_trb.trb_type / 4);
//             cprintf("VF ID: %08x\n", cc_trb.vf_id);
//             cprintf("Slot ID: %08x\n", cc_trb.slot_id);
//         }
//     }
//     cprintf("\n");
//     cprintf("Device context:\n");
//     struct DeviceContext * dev_cont_ptr = (void *)device_context[1];
//     cprintf("  Slot context:\n");
//     cprintf("    Route string: %x\n", dev_cont_ptr->slot_context.first_row & 0xFFFFF);
//     cprintf("    Speed: %x\n", (dev_cont_ptr->slot_context.first_row & (0xF << 20)) >> 20);
//     cprintf("    MTT: %x\n", (dev_cont_ptr->slot_context.first_row & (0x1 << 25)) >> 25);
//     cprintf("    Hub: %x\n", (dev_cont_ptr->slot_context.first_row & (0x1 << 26)) >> 26);
//     cprintf("    Context entries: %x\n", (dev_cont_ptr->slot_context.first_row & (0x1F << 27)) >> 27);
//     cprintf("    Max exit latency: %x\n", dev_cont_ptr->slot_context.max_exit_latency);
//     cprintf("    Root hub port number: %x\n", dev_cont_ptr->slot_context.root_hub_port_number);
//     cprintf("    Number of ports: %x\n", dev_cont_ptr->slot_context.number_of_ports);
//     cprintf("    USB device address: %x\n", dev_cont_ptr->slot_context.usb_device_address);
//     cprintf("    Slot state: %x\n", dev_cont_ptr->slot_context.slot_state >> 4);
//     for (int i = 0; i < 6; i++) {
//         cprintf("  Endpoint context #%d\n", i);
//         cprintf("    EP state: %08x\n", dev_cont_ptr->endpoint_context[i].ep_state);
//         cprintf("    Mult: %08x\n", dev_cont_ptr->endpoint_context[i].mult_and_max_p_streams & 0x3);
//         cprintf("    MaxPStreams: %08x\n", dev_cont_ptr->endpoint_context[i].mult_and_max_p_streams & (0x1F << 2) >> 2);
//         cprintf("    LSA: %x\n", dev_cont_ptr->endpoint_context[i].mult_and_max_p_streams & (0x1 << 7) >> 7);
//         cprintf("    Interval: %08x\n", dev_cont_ptr->endpoint_context[i].interval);
//         cprintf("    Transfer ring deque ptr: %016lx\n", dev_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr);
//         if (dev_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr) {
//             cprintf("      ");
//             print_transfer_trb_structure((void *)dev_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr + 0x8040000000 - (dev_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr & 1));
//         }
//     }
//     cprintf("Input slot context:\n");
//     volatile struct InputContext * inp_cont_ptr = &(input_context);
//     cprintf("  Slot context:\n");
//     cprintf("    Route string: %x\n", inp_cont_ptr->slot_context.first_row & 0xFFFFF);
//     cprintf("    Speed: %x\n", (inp_cont_ptr->slot_context.first_row & (0xF << 20)) >> 20);
//     cprintf("    MTT: %x\n", (inp_cont_ptr->slot_context.first_row & (0x1 << 25)) >> 25);
//     cprintf("    Hub: %x\n", (inp_cont_ptr->slot_context.first_row & (0x1 << 26)) >> 26);
//     cprintf("    Context entries: %x\n", (inp_cont_ptr->slot_context.first_row & (0x1F << 27)) >> 27);
//     cprintf("    Max exit latency: %x\n", inp_cont_ptr->slot_context.max_exit_latency);
//     cprintf("    Root hub port number: %x\n", inp_cont_ptr->slot_context.root_hub_port_number);
//     cprintf("    Number of ports: %x\n", inp_cont_ptr->slot_context.number_of_ports);
//     cprintf("    USB device address: %x\n", inp_cont_ptr->slot_context.usb_device_address);
//     cprintf("    Slot state: %x\n", inp_cont_ptr->slot_context.slot_state >> 4);
//     for (int i = 0; i < 6; i++) {
//         cprintf("  Endpoint context #%d\n", i);
//         cprintf("    EP state: %08x\n", inp_cont_ptr->endpoint_context[i].ep_state);
//         cprintf("    Mult: %08x\n", inp_cont_ptr->endpoint_context[i].mult_and_max_p_streams & 0x3);
//         cprintf("    MaxPStreams: %08x\n", inp_cont_ptr->endpoint_context[i].mult_and_max_p_streams & (0x1F << 2) >> 2);
//         cprintf("    LSA: %x\n", inp_cont_ptr->endpoint_context[i].mult_and_max_p_streams & (0x1 << 7) >> 7);
//         cprintf("    Interval: %08x\n", inp_cont_ptr->endpoint_context[i].interval);
//         cprintf("    Transfer ring deque ptr: %016lx\n", inp_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr);
//         if (inp_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr) {
//             cprintf("      ");
//             print_transfer_trb_structure((void *)inp_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr + 0x8040000000 - (inp_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr & 1));
//         }
//     }
}

int xhci_map(struct XhciController *ctl) {
    //ПЕРЕДЕЛАН МАП ПАМЯТИ
    ctl->mmio_base_addr = (uint8_t *)get_bar_address(ctl->pcidev,0);
    size_t size = get_bar_size(ctl->pcidev, 0);
//    cprintf("SIZE AND ADDR: %p %d\n",ctl->mmio_base_addr,size);
    size = size > XHCI_MAX_MAP_MEM ? XHCI_MAX_MAP_MEM : size;
    int res = sys_map_physical_region(get_bar_address(ctl->pcidev, 0), CURENVID, (void *)ctl->mmio_base_addr, size, PROT_RW | PROT_CD);
    // добавить коды ошибок
    if (res)
        return 1;
    return 0;
}

void xhci_register_init(struct XhciController *ctl) {
    // ДАЛЕЕ переделать
    cprintf("^%p\n",ctl->mmio_base_addr);
    uint8_t *memory_space_ptr = (uint8_t*)ctl->mmio_base_addr;
    cap_regs = (struct CapabilityRegisters *)memory_space_ptr;
    oper_regs = ((void *)memory_space_ptr + cap_regs->caplength);
    cprintf("^%p\n",oper_regs);
    run_regs = ((void *)memory_space_ptr + (cap_regs->rtsoff & RTSOFF_MASK) );
    cprintf("^%p\n",run_regs);
    doorbells = ((void *)memory_space_ptr + (cap_regs->dboff & DBOFF_MASK));
    cprintf("^%p\n",doorbells);
    while(controller_not_ready()) {}
    
    //int res = sys_map_physical_region(, CURENVID, (void *)ctl->mmio_base_addr, size, PROT_RW | PROT_CD);
    //if (res)
    //    return 1;

    //этому 2048 с выравниванием по 6
    cprintf("^%lx\n",(oper_regs->dcbaap) & ZERO_MASK_64(0b111111) );
    //этому
    cprintf("^%lx\n",(oper_regs->crcr) & ZERO_MASK_64(0b111111) );
    //этому
    cprintf("^%lx\n",(run_regs->int_reg_set[0].erdp) & ZERO_MASK_64(0b111111) );
    // это ещё interrupts СОМНЕНИЕ в структуре
    for (int i = 0; i < 10; i++) {
        cprintf("^%ld %ld\n",
        run_regs->int_reg_set[i].erstba & ZERO_MASK_64(0b111111),
        run_regs->int_reg_set[i].erdp & ZERO_MASK_64(0b1111) );
    }
    
    // device_context[1] = (void *)(((uint64_t *)dcbaap)[1] + 0x8040000000);

    // command_ring_deque = (void *)(oper_regs->crcr & (~((uint64_t)0x3f)));
    // if (command_ring_deque != 0) {
    //     command_ring_deque = mmio_map_region((uint64_t)command_ring_deque, COMMAND_RING_DEQUE_SIZE);
    // }

    // event_ring_deque = (void *)run_regs->int_reg_set[0].erdp + 0x8040000000;
    // event_ring_segment_table = mmio_map_region((uint64_t)run_regs->int_reg_set[0].erstba, EVENT_RING_TABLE_SIZE);
    // event_ring_segment_table_size = run_regs->int_reg_set[0].erstsz;
    // event_ring_segment_base_address = mmio_map_region(event_ring_segment_table[0].ring_segment_base_address, sizeof(struct TRBTemplate) * EVENT_RING_SEGMENT_SIZE);
    // for (int i = 0; i < sizeof(struct TRBTemplate) * EVENT_RING_SEGMENT_SIZE; i++) {
    //     event_ring_segment_base_address[i] = 0;
    // }
    // event_ring_segment_table[0].ring_segment_size = EVENT_RING_SEGMENT_SIZE;
    // event_ring_deque = event_ring_segment_base_address;
    // run_regs->int_reg_set[0].erstba = (uint64_t)event_ring_segment_table;
}

static int
xhci_alloc_structures(struct XhciController *ctl) {
    return 0;
}

void xhci_init() {
    struct XhciController *ctl = &xhci;
    struct PciDevice *pcidevice = find_pci_dev(0x0C, 0x03);
    //int err;
    if (pcidevice == NULL)
        panic("NVMe device not found\n");
    ctl->pcidev = pcidevice;
    if (xhci_map(ctl) )
        panic("XHCI device not found\n");
    xhci_register_init(ctl);
    print_usb_memory_region();
    if (xhci_alloc_structures(ctl))
        panic("Unable to allocate XHCI structures\n");
    // ПРОДОЛЖИТЬ см nvme_init
}

extern volatile int pci_initial;

void
umain(int argc, char **argv) {
    cprintf("***********************INIT USB***********************\n");
    // уже написана в fs/pci.c

    //TO DO: исправить то что сделано в pmap.c
    pci_init(argv);
    // делаем похожей на nvme
    pci_init(argv);
    xhci_init();
    cprintf("***********************END USB***********************\n");
}

