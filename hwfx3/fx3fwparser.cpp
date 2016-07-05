#include "fx3fwparser.h"

#include <cstdio>

FX3DevFwParser::FX3DevFwParser() :
    fw_buf( NULL )
{
}

FX3DevFwParser::~FX3DevFwParser() {
    if ( fw_buf ) {
        delete [] fw_buf;
    }
}

fx3_dev_err_t FX3DevFwParser::firmwareParse( const uint8_t* firmware_buffer, uint32_t fw_size8, SectionList_t& sections_output ) {
    sections_output.clear();
    uint8_t* buf = ( uint8_t* ) firmware_buffer;
    int32_t rest_size8 = fw_size8;
    uint32_t check_sum = 0;

    // Check Cypress signature
    if ( buf[ 0 ] != CY_SIGN_BYTE_0 || buf[ 1 ] != CY_SIGN_BYTE_1 ) {
        fprintf( stderr, "FX3Dev::firmwareParse __error__ bad Cypress signature: 0x%02X 0x%02X\n", buf[ 0 ], buf[ 1 ] );
        return FX3_ERR_FIRMWARE_FILE_CORRUPTED;
    }
    // Skip 4 bytes (two bytes of signature and two next)
    buf += 4;
    rest_size8 -= 4;

    // Find sizes of sections:
    bool end_of_fw = false;
    while ( !end_of_fw ) {
        // Each section format is:
        // uint32_t sections_size_in_32bit_words;
        // uint32_t device_destination_address;
        // uint8_t data[]

        uint32_t section_size8 = 4 * (*(uint32_t*)buf);
        buf += sizeof(uint32_t);
        rest_size8 -= sizeof(uint32_t);

        if ( section_size8 != 0 ) {
            // case 1. Have one more section.

            uint32_t section_addr  = (*(uint32_t*)buf);
            buf += sizeof(uint32_t);
            rest_size8 -= sizeof(uint32_t);

            FX3_FW_Section section;
            section.host_source_buffer = buf;
            section.section_size8      = section_size8;
            section.dev_dest_addr      = section_addr;
            sections_output.push_back( section );

            // Calc general checksum (of whole firmware).
            uint32_t* ptr32 = (uint32_t*) buf;
            for ( uint32_t i = 0; i < section_size8 / sizeof( uint32_t ); i++ ) {
                check_sum += ptr32[ i ];
            }

            buf += section_size8;
            rest_size8 -= section_size8;

            fprintf( stderr, "SECTION[%2d]: addr8 0x%08X...0x%08X, length: 0x%08X (%6u) bytes, rest %6d bytes\n",
                     sections_output.size() - 1,
                     section_addr, section_addr + section_size8,
                     section_size8, section_size8,
                     rest_size8 );
        } else {
            // case 2 (final). No sections anymore in file.

            // Last section is special - it is setting of 'program entry' point
            end_of_fw = true;
            uint32_t program_entry_address = (*(uint32_t*)buf);
            buf += sizeof(uint32_t);
            rest_size8 -= sizeof(uint32_t);

            FX3_FW_Section section;
            section.host_source_buffer = ( uint8_t* ) firmware_buffer; // dummy buffer. It can be NULL, but someone is afraid of passing NULLs
            section.section_size8      = 0; // Special case, use zero length transfer
            section.dev_dest_addr      = program_entry_address;
            sections_output.push_back( section );

            fprintf( stderr, "PROGRAM_ENTRY: 0x%08X\n", program_entry_address );

            // Check checksum
            uint32_t expected_check_sum = (*(uint32_t*)buf);
            buf += sizeof(uint32_t);
            rest_size8 -= sizeof(uint32_t);

            if ( expected_check_sum != check_sum ) {
                fprintf( stderr, "FX3Dev::firmwareParse __error__ CHECK_SUM: expected 0x%08X, calculated 0x%08X\n", expected_check_sum, check_sum );
                return FX3_ERR_FIRMWARE_FILE_CORRUPTED;
            }

            break;
        }

        // Not all wants segmentation faults
        if ( rest_size8 < ( int32_t ) ( 2 * sizeof( uint32_t ) ) ) {
            fprintf( stderr, "FX3Dev::firmwareParse __error__ bad firmware section's sizes\n" );
            return FX3_ERR_FIRMWARE_FILE_CORRUPTED;
        }
    }
    fprintf( stderr, "FX3Dev::firmwareParse Found %d sections, rest space %d bytes\n", sections_output.size(), rest_size8 );
    return FX3_ERR_OK;
}

fx3_dev_err_t FX3DevFwParser::fx3_parse_firmware(const char *fw_fname, SectionList_t &sections_output) {

    fx3_dev_err_t res = FX3_ERR_OK;

    FILE* fwfile = fopen( fw_fname, "rb" );
    if ( fwfile == NULL ) {
        fprintf( stderr, "fx3_parse_firmware() __error__ io file '%s'\n", fw_fname );
        res = FX3_ERR_FIRMWARE_FILE_IO_ERROR;
    } else {
        // Find file szie
        fseek( fwfile, 0, SEEK_END );
        uint32_t fw_size8  = ftell( fwfile );
        fseek( fwfile, 0, SEEK_SET );

        // Read firmware
        if ( fw_buf ) {
            delete [] fw_buf;
            fw_buf = NULL;
        }
        fw_buf = new uint8_t[ fw_size8 ];
        fread( fw_buf, 1, fw_size8, fwfile );
        fclose( fwfile );

        // Parse firmware to fw_sections list
        res = firmwareParse( fw_buf, fw_size8, sections_output );
    }
    return res;
}
