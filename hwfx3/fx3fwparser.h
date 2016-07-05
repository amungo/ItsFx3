#ifndef FX3FWPARSER_H
#define FX3FWPARSER_H

#include <cstdint>

#include "fx3deverr.h"
#include <list>



// One FPGA section description. This stucture is used for firmware loading.
struct FX3_FW_Section {
    uint8_t* host_source_buffer; // Address of a block in host's memory
    uint32_t dev_dest_addr;      // Device's block address (destination)
    uint32_t section_size8;      // A block's size in bytes.
};
typedef std::list<FX3_FW_Section> SectionList_t; // List of sections/blocks of firmware
typedef std::list<FX3_FW_Section>::iterator SectionListIterator_t;

class FX3DevFwParser {
public:
    FX3DevFwParser();
    ~FX3DevFwParser();
    fx3_dev_err_t fx3_parse_firmware( const char* fw_file_name, SectionList_t& sections_output );
    void fx3_delete_fw_buf();
private:
    fx3_dev_err_t firmwareParse( const uint8_t* firmware_buffer, uint32_t fw_size8, SectionList_t& sections_output );
    uint8_t* fw_buf;

    static const uint8_t CY_SIGN_BYTE_0 = 0x43; // Cypress firmware file format specific: first byte in firmware file.
    static const uint8_t CY_SIGN_BYTE_1 = 0x59; // Cypress firmware file format specific: second byte in firmware file.

};

#endif // FX3FWPARSER_H
