#include <inc/x86.h>
#include <inc/string.h>
#include <inc/lib.h>
#include "fs/pci.h"

#include "usb/context.h"
#include "usb/registers.h"


// ПОТОМ ПЕРЕНЕСТИ в pci.h строка 53
#define XHCI_BASE_ADDR         0x7010000000
uint8_t *__xhci_current_Va = (void*)XHCI_BASE_ADDR;
uintptr_t __xhci_current_Pa;
#define PAGESIZE 4096

//used already
#define XHCI_MAX_MAP_MEM 0x4000
#define XHCI_RING_DEQUE_MEM 4096 //why 4096?
#define DEVICE_NUMBRER 8 // вот я хочу 8 устройств, этого в целом достаточно
//not used yet
#define INTERRUPTER_REGISTER_SET_COUNT 256
#define EVENT_RING_TABLE_SIZE 256
#define EVENT_RING_SEGMENT_SIZE 128
#define COMMAND_RING_DEQUE_SIZE 4096

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

struct DoorbellRegister (*doorbells)[256];
// TODO: db_array[0] is allocated to host controller for command ring managment

int controller_not_ready() {
    return (oper_regs->usbsts & (1 << 11)) >> 11;
}
volatile uint8_t * memory_space_ptr;
volatile uint64_t * dcbaap;
volatile uint8_t * device_context[32];
volatile uint8_t * command_ring_deque;
volatile struct EventRingTableEntry * event_ring_segment_table;
volatile uint32_t event_ring_segment_table_size;
volatile uint8_t * event_ring_segment_base_address;
volatile uint8_t * event_ring_deque;
struct DeviceContext *dcbaa_table_clone[DEVICE_NUMBRER];



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

void print_ports(){
    uint8_t num_ports = cap_regs->hcsparams1 >> 24;
    for (uint8_t i=1; i <= num_ports; i++){
        cprintf("PORTSC[%u] = %x\n", i, *((uint32_t*)((uint8_t*)oper_regs + (0x400 + (0x10 * (i - 1))))));
    }
}

// "va" and "pa" returns values
int memory_map( void **va, uintptr_t *pa, size_t size) {
    // всегда мапим по границе страницы, иначе ERROR
    if (size % PAGESIZE != 0)
        size = size - (size % PAGESIZE) + PAGESIZE;
    if (va != NULL)
        *va = (void*)__xhci_current_Va;
    if (pa != NULL)
        *pa = __xhci_current_Pa;
    int res = sys_map_physical_region(__xhci_current_Pa, CURENVID, (void*)__xhci_current_Va, size, PROT_RW | PROT_CD);
    __xhci_current_Va += size;
    __xhci_current_Pa += size;
    return res;
}

int xhci_map(struct XhciController *ctl) {
    size_t size = get_bar_size(ctl->pcidev, 0);
    size = size > XHCI_MAX_MAP_MEM ? XHCI_MAX_MAP_MEM : size;
    // first time initialization of pa pointer
    __xhci_current_Pa = get_bar_address(ctl->pcidev, 0);
    return memory_map((void**)(&(ctl->mmio_base_addr)),NULL,size);
    //now, ctl->mmio_base_address is initialized
}

int xhci_register_init(struct XhciController *ctl) {
    __xhci_current_Pa = 0x1000000;
    uint8_t *memory_space_ptr = (uint8_t*)ctl->mmio_base_addr;
    cap_regs = (struct CapabilityRegisters *)memory_space_ptr;
    cprintf("^%p\n",cap_regs);
    oper_regs = ((void *)memory_space_ptr + cap_regs->caplength);
    cprintf("^%p\n",oper_regs);
    run_regs = ((void *)memory_space_ptr + (cap_regs->rtsoff & RTSOFF_MASK) );
    cprintf("^%p\n",run_regs);
    doorbells = ((void *)memory_space_ptr + (cap_regs->dboff & DBOFF_MASK));
    cprintf("^%p\n",doorbells);
    while(controller_not_ready()) {}

    // small check: read-only registers can't be rewrited
//    cprintf("!! %d\n",oper_regs->pagesize);
//    oper_regs->pagesize = 0xFFFFFFFF;
//    cprintf("!! %d\n",oper_regs->pagesize);


    // этому 2048 с выравниванием по 6
    // но мапится всё равно целое число страниц, поэтому так
    uintptr_t pa;
    int res;
    size_t size = PAGESIZE;
    if ((res = memory_map((void**)(&dcbaap),&pa,size)))
        return res;
    oper_regs->dcbaap = pa | (oper_regs->dcbaap & 0b111111);
    cprintf("^%p %lx\n",dcbaap,oper_regs->dcbaap & ZERO_MASK_64(0b111111));

    // !но каждому из DCBAA нужна своя DeviceContext
    memset((void*)dcbaap,0,PAGESIZE);
    for (int i = 1; i < DEVICE_NUMBRER; i++) {
        if ((res = memory_map((void**)(dcbaa_table_clone+i),&pa,PAGESIZE) ))
            return res;
        dcbaap[i] = pa;
        cprintf("  ^%p %lx %lx\n",dcbaa_table_clone[i],pa,dcbaap[i]);
    }
    uint32_t scratchpad = (cap_regs->hcsparams2 >> 21) & 0b11111;
    if ( scratchpad > 0) {
        // impossible in our case
        if (( res = memory_map((void**)dcbaa_table_clone, &pa, PAGESIZE*scratchpad) ))
            return res;
        dcbaap[0] = pa;
    }
    // TODO: zero fields of new pages, но не все, см 4.5.2

    // этому 4 Kib
    size = PAGESIZE;
    if ((res = memory_map((void**)(&command_ring_deque),&pa,size)))
        return res;
    oper_regs->crcr = pa | (oper_regs->crcr & 0b111111);
    cprintf("^%p %lx\n",command_ring_deque,oper_regs->crcr & ZERO_MASK_64(0b111111));    

    //этому 512 Kib:
    size = 512*1024;
    if ((res = memory_map((void**)(&event_ring_segment_table),&pa,size)))
        return res;
    cprintf("^%p %lx\n",event_ring_segment_table,pa);
    
    // THIS DONt WORKS
    // run_regs->int_reg_set[0].erdp = pa | (run_regs->int_reg_set[0].erdp & 0b111111);

    // struct Device Context needs: devicee context mapping

    for (int i = 0; i < EVENT_RING_TABLE_SIZE; i++)
        event_ring_segment_table[i].ring_segment_size = EVENT_RING_SEGMENT_SIZE;

    size = sizeof(struct TRBTemplate) * EVENT_RING_SEGMENT_SIZE;
    if ((res = memory_map((void**)(&event_ring_segment_base_address),&pa,size)))
        return res;

    for (int i = 0; i < sizeof(struct TRBTemplate) * EVENT_RING_SEGMENT_SIZE; i++) {
        event_ring_segment_base_address[i] = 0;
    }

    return 0;
}

void xhci_settings_init() {
    oper_regs->config |= DEVICE_NUMBRER;

    volatile uint32_t usbcmd = oper_regs->usbcmd;
    usbcmd = usbcmd | 0x1;
    oper_regs->usbcmd = usbcmd;         // setting Run/Stop register to Run state 

    //volatile uint16_t xecp_offset = (cap_regs->hccparams1 >> 16);
    //cprintf("hccparams1 - %x\n", cap_regs->hccparams1);
    //volatile uint32_t * xecp_pointer = (uint32_t *)((uint8_t *)cap_regs + (xecp_offset << 2)); 
    //volatile uint32_t xecp = xecp_pointer[0];
    //xecp = xecp | (1 << 24);
    //cprintf("xecp - %x\n",xecp);

    // здесь печать обнаруженных устройств. Если не видите здесь своего устройства - это проблема
    for (int i = 0; i < 8; i++) {
        uint32_t val = *(uint32_t*)(((uint8_t*)(oper_regs))+0x400 + i*16);
        if ( val != 0x2a0 )
            cprintf("FIND: ");
        cprintf("PORTSC[%d] = %x\n",i,val);
    }
    cprintf("\n");
}

void wait_for_command_ring_not_running() {
    while ((oper_regs->crcr & (1 << 3)) == 0) {}
}

void wait_for_command_ring_running() {
    while (oper_regs->crcr & (1 << 3)) { cprintf("-\n"); }
}

volatile struct InputContext *input_context;
volatile struct TransferTRB *transfer_ring;

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

int xhci_slots_init() {
    size_t size = sizeof(struct InputContext);
    uintptr_t pa_inp_cntx_ptr;
    int res;
    if ((res = memory_map((void**)(&input_context),&pa_inp_cntx_ptr,size)))
        return res;

//    volatile uint8_t * input_context_ptr = (void *)input_context;
//    for (int i = 0; i < sizeof(struct InputContext); i++) {
//        input_context_ptr[i] = 0;
//    }

    input_context->input_control_context.add_context_flags = 0x3;
    input_context->slot_context.root_hub_port_number = 0x5;
    input_context->slot_context.slot_state = 0x1 << 4;
    //input_context->slot_context.usb_device_address = 0;
    uint32_t buf = input_context->slot_context.slot_state;
    buf = buf & ZERO_MASK_32(0xFF);
    input_context->slot_context.slot_state = buf;
    volatile uint32_t first_row = (1 << 27) + (1 << 20);
    input_context->slot_context.first_row = first_row;

    //struct DeviceContext * dev_cont_ptr = (void *)device_context[1];

    size = (PAGESIZE)*32;
    uintptr_t pa;
    uint8_t va;
    if ((res = memory_map((void**)(&va),&pa,size)))
        return res;

    for (int i = 1; i < 31; i++) {
        input_context->endpoint_context[i].transfer_ring_deque_ptr = (uint64_t)(va) + ((uint64_t)i)*PAGESIZE;
        //dev_cont_ptr->endpoint_context[i].transfer_ring_deque_ptr = (uint64_t)((void *)&(transfer_ring[i][0]) - 0x8040000000);
    }

    size = sizeof(struct TransferTRB) * 16 * 128;
    if ((res = memory_map((void**)(&transfer_ring),&pa,size)))
        return res;

    input_context->endpoint_context[0].transfer_ring_deque_ptr = (uint64_t)transfer_ring;
    //dev_cont_ptr->endpoint_context[0].transfer_ring_deque_ptr = (uint64_t)((void *)&(transfer_ring[0][0]) - 0x8040000000 + 1);
    volatile uint8_t flags = 0x26;
    input_context->endpoint_context[0].flags_off = flags;
    input_context->endpoint_context[0].max_burst_size = 0;
    input_context->endpoint_context[0].interval = 0;
    input_context->endpoint_context[0].maxPStreams = 0;
    input_context->endpoint_context[0].max_packet_size = 0x8;

    struct AddressDeviceCommandTRB * adr_command_ring_adr = (struct AddressDeviceCommandTRB *)command_ring_deque;
    adr_command_ring_adr[0] = address_device_command(pa_inp_cntx_ptr, 1, 1);
    doorbells[0]->target = 0;
    doorbells[0]->stream_id = 0;

    //wait_for_command_ring_not_running();
    return 0;
}


void xhci_device_init(){

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
    print_usb_memory_region();
    if (xhci_register_init(ctl))
        panic("Unable to allocate XHCI structures\n");
    // ПРОДОЛЖИТЬ см nvme_init А ЛУЧШЕ дедовский usb
    xhci_settings_init();

    if (xhci_slots_init())
        panic("Unable to allocate XHCI structures\n");
}

extern volatile int pci_initial;

void
umain(int argc, char **argv) {
    cprintf("***********************INIT USB***********************\n");
    // уже написана в fs/pci.c
    //TO DO: исправить то что сделано в pmap.c
    // делаем похожей на nvme
    pci_init(argv);
    xhci_init();
    cprintf("***********************END USB***********************\n");
    while(1){}
}

