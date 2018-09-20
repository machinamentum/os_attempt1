
#ifndef PCI_H
#define PCI_H

#define PCI_VENDOR_ID_VMWARE 0x15AD

#define PCI_DEVICE_ID_VMWARE_SVGA2 0x0405

#define PCI_MAX_BUSES  256
#define PCI_MAX_DEVICES_PER_BUS 32
#define PCI_MAX_FUNCTIONS_PER_DEVICE 8

#define PCI_CLASS_MASS_STORAGE_CONTROLLER 0x01

#define PCI_SUBCLASS_IDE_CONTROLLER       0x01

#define PCI_HEADER_MULTIFUNCTION_BIT (1 << 7)

#define PCI_CONFIG_ADDRESS 0x0CF8
#define PCI_CONFIG_DATA    0x0CFC

#define PCI_CONFIG_ENABLE_BIT     (1 << 31)
#define PCI_CONFIG_BUS_NUMBER(x) ((x) << 16) // maybe we should mask these?
#define PCI_CONFIG_DEVICE_NUMBER(x) ((x) << 11)
#define PCI_CONFIG_FUNCTION_NUMBER(x) ((x) << 8)
#define PCI_CONFIG_REGISTER_NUMBER(x) ((x) & 0xFC) // the osdev docs use a mask here, I guess what you really want is to always have a 4-byte aligned register access
#define PCI_CONFIG_GET_ADDRESS(b, s, f, o)    (PCI_CONFIG_BUS_NUMBER(b) | PCI_CONFIG_DEVICE_NUMBER(s) | PCI_CONFIG_FUNCTION_NUMBER(f) | PCI_CONFIG_REGISTER_NUMBER(o))

struct Pci_Device_Config {
    u16 vendor_id;
    u16 device_id;
    
    u16 command;
    u16 status;
    
    u8 revision_id;
    u8 prog_if;
    u8 subclass_code;
    u8 class_code;

    u8 cache_line_size;
    u8 latency_timer;
    u8 header_type;
    u8 bist;

    union {
        // header_type = 0x00
        struct {
            u32 bar0;
            u32 bar1;
            u32 bar2;
            u32 bar3;
            u32 bar4;
            u32 bar5;
            u32 cardbus_cis_pointer;
            u16 subsystem_vendor_id;
            u16 subsystem_id;
            u32 expansion_rom_base_addr;
            u8 capabilities_pointer;
            u8 pad0;
            u16 pad1;
            u32 pad2;
            u8 interrupt_line;
            u8 interrupt_pin;
            u8 min_grant;
            u8 max_latency;
        } type_00;

        // header_type = 0x01
        struct {
            u32 bar0;
            u32 bar1;
            u8 primary_bus_number;
            u8 secondary_bus_number;
            u8 subordinate_bus_number;
            u8 secondary_latency_timer;
            u16 memory_base;
            u16 memory_limit;

            // should this be a u64 ?
            u32 prefetch_base_upper32;
            u32 prefetch_base_lower32;

            u16 io_base_upper16;
            u16 io_limit_upper16;
            u8 capabilities_pointer;
            u32 expansion_rom_base_addr;
            u8 interrupt_line;
            u8 interrupt_pin;
            u16 bridge_controller;
        } pci_to_pci_bridge;

        // header_type = 0x02
        struct {
            u32 exca_base_addr;
            u8 capability_list_offset;
            u8 pad0;
            u16 secondary_status;
            u8 pci_bus_number;
            u8 cardbus_number;
            u8 subordinate_bus_number;
            u8 cardbus_latency_timer;
            u32 memory_base_addr0;
            u32 memory_limit0;
            u32 memory_base_addr1;
            u32 memory_limit1;
            u32 io_base_addr0;
            u32 io_limit0;
            u32 io_base_addr1;
            u32 io_limit1;
            u8 interrupt_line;
            u8 interrupt_pin;
            u16 bridge_controller;
            u16 subsystem_device_id;
            u16 subsystem_vendor_id;
            u32 pc_card_legacy_mode_base_addr; // @TODO osdev states this is 16 bits, but which 16 ???
        } pci_to_cardbus_bridge;
    };

    // the configuration space is 256 bytes
    u8 pad_to_256[256 - 0x48];

    u8 bus;
    u8 slot;
    u8 function;
};

void pci_enable_memory(Pci_Device_Config *config);


#endif
