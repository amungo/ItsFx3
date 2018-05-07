#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include "FX3Dev.h"
#include "HexParser.h"

#include "lattice/lfe5u_core.h"
#include "lattice/lfe5u_opcode.h"

#ifdef WIN32
#include <windows.h>
#endif


FX3Dev::FX3Dev( size_t one_block_size8, uint32_t dev_buffers_count ) :
    ctx( NULL ),
    device_handle( NULL ),
    endpoint_from_hst_num( endpoint_invalid ),
    endpoint_from_dev_num( endpoint_invalid ),
    event_loop_running( false ),
    buffers_count( dev_buffers_count ),
    half_full_size8( one_block_size8 ),
    write_transfer( NULL ),
    write_buffer( NULL ),
    data_handler( NULL ),
    last_overflow_count( 0 ),
    size_tx_mb( 0.0 )
    
{
    if ( one_block_size8 % 1024 ) {
        half_full_size8 = ( 1 + one_block_size8 / 1024 ) * 1024;
        fprintf( stderr, "FX3Dev::FX3Dev __warning__ one_block_size (%u) changed to %d\n", one_block_size8, half_full_size8 );
    }
    
    if ( dev_buffers_count < 2 ) {
        buffers_count = 2;
        fprintf( stderr, "FX3Dev::FX3Dev __warning__ dev_buffers_count (%u) changed to %d\n", dev_buffers_count, buffers_count );
    }
    transfers.resize( buffers_count );
    
    half_full_buffers.resize( buffers_count );
    for( uint32_t i = 0; i < buffers_count; i++ ) {
        half_full_buffers[ i ] = new uint8_t[ half_full_size8 ];
    }
    write_buffer = new uint8_t[ half_full_size8 ];

    m_SSPICore = std::shared_ptr<SSPICore>(new SSPICore(this));
}

FX3Dev::~FX3Dev() {
    if ( write_transfer ) {
        libusb_cancel_transfer(write_transfer);
        libusb_free_transfer(write_transfer);
    }
    
    if ( device_handle ) {
        int ires = libusb_release_interface(device_handle, 0);
        if( ires != 0 ) {
            fprintf( stderr,"FX3Dev::~FX3Dev(): libusb_release_interface failed with code %d %s\n", ires, libusb_error_name( ires ) );
        }
        libusb_close( device_handle );
        device_handle = NULL;
    }
    if ( ctx ) {
        libusb_exit( ctx );
    }
    
    for( uint32_t i = 0; i < buffers_count; i++ ) {
        delete [] half_full_buffers[ i ];
    }
    if ( write_buffer ) {
        delete[] write_buffer;
    }
}

fx3_dev_err_t FX3Dev::init(const char* firmwareFileName /* = NULL */, const char* additionalFirmwareFileName /* = NULL */ ) {
    int ires = libusb_init( &ctx );
    if ( ires != 0 ) {
        fprintf( stderr, "FX3Dev::Init(): __error__ libusb_init, code %d %s\n", ires, libusb_error_name( ires ) );
        return FX3_ERR_USB_INIT_FAIL;
    }

    libusb_set_debug( ctx, FX3_DEBUG_LEVEL_DEFAULT );

    scan();

    fx3_dev_err_t eres = FX3_ERR_OK;

    if ( endpoint_from_dev_num == endpoint_invalid ) {
        fprintf( stderr, "FX3Dev::Init() asked to flash firmware. Looking for device without firmware (pid = 0x%04x)\n", DEV_PID_FOR_FW_LOAD );
        device_handle = libusb_open_device_with_vid_pid( ctx, VENDOR_ID, DEV_PID_FOR_FW_LOAD );
        if( device_handle != NULL ) {
            fprintf( stderr, "FX3Dev::Init() Found device. Will flash it with firmware from file '%s'\n", firmwareFileName );
            eres = firmwareFlashFromFile( firmwareFileName );
            if ( eres == FX3_ERR_OK ) {
                fprintf( stderr, "FX3Dev::Init() flash completed!\nPlease wait for %d seconds\n", PAUSE_AFTER_FLASH_SECONDS );
                for ( int i = 0; i < PAUSE_AFTER_FLASH_SECONDS * 2; i++ ) {
                    #ifdef WIN32
                    Sleep( 500 );
                    #else
                    usleep( 500000 );
                    #endif
                    fprintf( stderr, "*" );
                }
                fprintf( stderr, "\n" );
                libusb_close(device_handle);
                device_handle = NULL;
                
                scan();
                
            } else {
                fprintf( stderr, "FX3Dev::Init() __error__ flash failed, error %d %s\n", eres, fx3_get_error_string( eres ) );
                return eres;
            }
        } else {
            fprintf( stderr, "FX3Dev::Init() no device without firmware found. Maybe already flashed?\n" );
        }
        last_overflow_count = 0;
    }
    
    if ( endpoint_from_dev_num == endpoint_invalid ) {
        fprintf( stderr, "FX3Dev::Init() Device don't have endpoint with stream data to host!\n" );
        return FX3_ERR_BAD_DEVICE;
    }
    
    fprintf( stderr, "FX3Dev::Init() Proceed to init flashed device (0x%04x)\n", DEV_PID_NO_FW_NEEDED );
    
    device_handle = libusb_open_device_with_vid_pid( ctx, VENDOR_ID, DEV_PID_NO_FW_NEEDED );
    
    if ( device_handle == NULL ) {
        fprintf( stderr, "FX3Dev::Init() __error__ no device with vid = 0x%04x and pid = 0x%04X found!\n", VENDOR_ID, DEV_PID_NO_FW_NEEDED );
        return FX3_ERR_NO_DEVICE_FOUND;
    }
    
    if ( additionalFirmwareFileName != NULL ) {
        if ( additionalFirmwareFileName[ 0 ] != 0 ) {
            eres = loadAdditionalFirmware( additionalFirmwareFileName, 48 );
            if ( eres != FX3_ERR_OK ) {
                fprintf( stderr, "FX3Dev::Init() __error__ loadAdditionalFirmware %d %s\n", eres, fx3_get_error_string( eres ) );
                return eres;
            }
        }
    }

#if 0
    readFwVersion();
#endif

    ires = libusb_claim_interface(device_handle, 0);
    if ( ires < 0 ) {
        fprintf( stderr, "FX3Dev::Init() __error__ libusb_claim_interface failed %d %s\n", ires, libusb_error_name( ires ) );
        return FX3_ERR_USB_INIT_FAIL;
    }
    
    return FX3_ERR_OK;
}

fx3_dev_err_t FX3Dev::init_fpga(const char* algoFileName, const char* dataFileName)
{
    fx3_dev_err_t retCode = FX3_ERR_OK;
    int siRetCode = m_SSPICore->SSPIEm_preset( algoFileName, dataFileName);
    siRetCode = m_SSPICore->SSPIEm(0xFFFFFFFF);

    retCode = (siRetCode == PROC_OVER) ? FX3_ERR_OK : FX3_ERR_FPGA_DATA_FILE_IO_ERROR;

    if(retCode == FX3_ERR_OK)
    {
        // Set DAC
        retCode = send24bitSPI8bit(0x000AFFFF<<4);
        retCode = device_stop();
        retCode = reset_nt1065();
    }

    return retCode;
}

fx3_dev_err_t FX3Dev::scan() {
    fprintf( stderr, "FX3Dev::scan()\n" );
    int ires;
    libusb_device **devs;
    ssize_t dev_cnt = libusb_get_device_list( ctx, &devs );
    if( dev_cnt < 0 ) {
        fprintf( stderr, "FX3Dev::Init(): __error__ libusb_get_device_list() error %d %s\n", dev_cnt, libusb_error_name( dev_cnt ) );
        return FX3_ERR_USB_INIT_FAIL;
    }
    fprintf( stderr, "FX3Dev::scan() libusb_get_device_list returned %d\n", dev_cnt );
    for ( int i = 0; devs[ i ] != NULL; i++ ) {
        libusb_device_descriptor desc;
        ires = libusb_get_device_descriptor( devs[ i ], &desc );
        if ( ires < 0 ) {
            fprintf( stderr, "libusb_get_device_descriptor %d %s\n", ires, libusb_error_name( ires ) );
        } else {
            uint8_t bus  = libusb_get_bus_number(devs[i]);
            uint8_t port = libusb_get_port_number(devs[i]);
            fprintf( stderr, "[%2d] bus:%u port:%u 0x%04x, 0x%04x", i, bus, port, desc.idVendor, desc.idProduct );
            if ( desc.idVendor == VENDOR_ID ) {
                if ( desc.idProduct == DEV_PID_FOR_FW_LOAD ) {
                    fprintf( stderr, " *** firmware needed\n" );
                } else if ( desc.idProduct == DEV_PID_NO_FW_NEEDED ) {
                    fprintf( stderr, " *** ready adc device\n" );
                } else {
                    fprintf( stderr, " *** driver compatible device" );
                }
                
                
                libusb_config_descriptor *config;
                libusb_get_config_descriptor(devs[ i ], 0, &config);
                const libusb_interface *inter;
                const libusb_interface_descriptor *interdesc;
                for(int i=0; i<(int)config->bNumInterfaces; i++) {
                    inter = &config->interface[i];
                    for(int j=0; j<inter->num_altsetting; j++) {
                        interdesc = &inter->altsetting[j];
                        fprintf( stderr, "%d endpoints\n", interdesc->bNumEndpoints );
                        for(int k=0; k<interdesc->bNumEndpoints; k++) {
                            uint8_t num = ( interdesc->endpoint[k].bEndpointAddress & ( 0x0F ) );
                            libusb_endpoint_direction dir = ( libusb_endpoint_direction )( interdesc->endpoint[k].bEndpointAddress & ( 0x80 ) );
                            uint16_t maxSize = interdesc->endpoint[k].wMaxPacketSize;
                            fprintf( stderr, "    [%d] %s - %u bytes max\n",
                                     num,
                                     dir == LIBUSB_ENDPOINT_IN ? "IN  (dev-to-host)" : "OUT (host-to-dev)",
                                     maxSize);
                            if ( dir == LIBUSB_ENDPOINT_IN ) {
                                if(desc.idProduct != DEV_PID_FOR_FW_LOAD)
                                    endpoint_from_dev_num = num;
                            } else {
                                endpoint_from_hst_num = num;
                            }
                        }
                    }
                }
                libusb_free_config_descriptor(config);
            }
            fprintf( stderr, "\n" );
            
        }
    }
    libusb_free_device_list( devs, 1 );
    return FX3_ERR_OK;
}


fx3_dev_err_t FX3Dev::firmwareFlashFromFile( const char* fw_fname ) {
    SectionList_t fw_sections;
    fx3_dev_err_t res = fw_parser.fx3_parse_firmware( fw_fname, fw_sections );
    if ( res == FX3_ERR_OK ) {
        // Download to device
        if ( firmwareUploadToDevice( fw_sections ) == FX3_ERR_OK ) {
            res = FX3_ERR_OK;
        }
    }
    return res;
}

fx3_dev_err_t FX3Dev::firmwareUploadToDevice(SectionList_t& sections) {
    SectionListIterator_t sect_it = sections.begin();
    
    // Split big blocks into small ones
    while ( sect_it != sections.end() ) {
        while ( sect_it->section_size8 > MAX_UPLOAD_BLOCK_SIZE8 ) {
            FX3_FW_Section subsect;
            subsect.dev_dest_addr      = sect_it->dev_dest_addr;
            subsect.host_source_buffer = sect_it->host_source_buffer;
            subsect.section_size8 = MAX_UPLOAD_BLOCK_SIZE8;
            sections.insert( sect_it, subsect );
            
            sect_it->dev_dest_addr      += MAX_UPLOAD_BLOCK_SIZE8;
            sect_it->host_source_buffer += MAX_UPLOAD_BLOCK_SIZE8;
            sect_it->section_size8      -= MAX_UPLOAD_BLOCK_SIZE8;
        }
        sect_it++;
    }
    
    // Transfer firmware block by block to device
    // Last transfer is special transfer with zero length - it is setting of 'program entry' point
    uint32_t section_num = 0;
    sect_it = sections.begin();
    int sec_size = sections.size(); // !!!
    int sec_cnt = 0; // !!!
    while ( sect_it != sections.end() ) {
        bool load_ok = false;
        uint32_t try_num  = 0;
        uint32_t bad_pos  = 0;
        uint8_t  bad_byte = 0;
        
        while ( !load_ok && try_num++ < FW_LOAD_TRY_COUNT ) {
            fx3_dev_err_t res = txFwToRAM(sect_it->host_source_buffer, sect_it->dev_dest_addr, sect_it->section_size8);
            if ( res != FX3_ERR_OK ) {
                return res;
            }
            if ( sect_it->section_size8 != 0 ) {
                uint8_t* checkbuf = new uint8_t[ sect_it->section_size8 ];
                res = txCheckFwFromRAM(checkbuf, sect_it->dev_dest_addr, sect_it->section_size8);
                if ( res != FX3_ERR_OK ) {
                    delete [] checkbuf;
                    return res;
                }
                
                load_ok = true;
                for ( bad_pos = 0; bad_pos < sect_it->section_size8 && load_ok; bad_pos++ ) {
                    if ( checkbuf[ bad_pos ] != sect_it->host_source_buffer[ bad_pos ] ) {
                        bad_byte = checkbuf[ bad_pos ];
                        load_ok = false;
                    }
                }
                
                delete [] checkbuf;
                
            } else {
                load_ok = true;
            }
            
            if ( !load_ok ) {
                fprintf( stderr, "FX3Dev::firmwareUploadToDevice() check failed on section %u. Last fail on byte [%u] 0x%02X != 0x%02X\n",
                         section_num, bad_pos, bad_byte, sect_it->host_source_buffer[ bad_pos ] );
            }
        }
        
        if ( !load_ok ) {
            fprintf( stderr, "FX3Dev::firmwareUploadToDevice() check failed on section %u. Last fail on byte [%u] 0x%02X != 0x%02X\n",
                     section_num, bad_pos, bad_byte, sect_it->host_source_buffer[ bad_pos ] );
            fprintf( stderr, "FX3Dev::firmwareUploadToDevice() no more tries left\n" );
            return FX3_ERR_FW_TOO_MANY_ERRORS;
        }
        
        sect_it++;
        section_num++;
        sec_cnt++;
    }
    return FX3_ERR_OK;
}

fx3_dev_err_t FX3Dev::txFwToRAM(uint8_t* src, uint32_t dev_ram_addr, uint32_t size8) {
    return txControlToDevice( src, size8, CMD_FW_LOAD, (dev_ram_addr & 0xFFFF), ((dev_ram_addr >> 16) & 0xFFFF) );
}

fx3_dev_err_t FX3Dev::txCheckFwFromRAM(uint8_t* dest, uint32_t dev_ram_addr, uint32_t size8) {
    return txControlFromDevice( dest, size8, CMD_FW_LOAD, (dev_ram_addr & 0xFFFF), ((dev_ram_addr >> 16) & 0xFFFF) );
}

fx3_dev_err_t FX3Dev::txControlToDevice(uint8_t* src, uint32_t size8, uint8_t cmd, uint16_t wValue_user, uint16_t wIndex_user) {
    /* From libusb-1.0 documentation:
     * Bits 0:4 determine recipient, see libusb_request_recipient.
     * Bits 5:6 determine type, see libusb_request_type.
     * Bit 7 determines data transfer direction, see libusb_endpoint_direction. */
    uint8_t bmRequestType = LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT;
    
    /* From libusb-1.0 documentation:
     * If the type bits of bmRequestType are equal to LIBUSB_REQUEST_TYPE_STANDARD
     * then this field refers to libusb_standard_request.
     * For other cases, use of this field is application-specific. */     
    uint8_t bRequest = cmd;
    
    /* From libusb-1.0 documentation:
     * Varies according to request */
    uint16_t wValue = wValue_user;
    
    /* From libusb-1.0 documentation:
     * Varies according to request */
    uint16_t wIndex = wIndex_user;
    
    /* From libusb-1.0 documentation:
     * timeout (in millseconds) that this function should wait before giving up
     * due to no response being received.
     * For an unlimited timeout, use value 0. */
    uint32_t timeout_ms = DEV_UPLOAD_TIMEOUT_MS;
    
    int res = libusb_control_transfer( device_handle, bmRequestType, bRequest, wValue, wIndex, src, size8, timeout_ms );
    if ( res != ( int ) size8 ) {
        fprintf( stderr, "FX3Dev::txControlToDevice() error %d %s\n", res, libusb_error_name(res) );
        return FX3_ERR_CTRL_TX_FAIL;
    }
    return FX3_ERR_OK;
}

fx3_dev_err_t FX3Dev::txControlFromDevice(uint8_t* dest, uint32_t size8 , uint8_t cmd, uint16_t wValue, uint16_t wIndex) {
    /* From libusb-1.0 documentation:
     * Bits 0:4 determine recipient, see libusb_request_recipient.
     * Bits 5:6 determine type, see libusb_request_type.
     * Bit 7 determines data transfer direction, see libusb_endpoint_direction. */
    uint8_t bmRequestType = LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN;
    uint8_t bRequest = cmd;
    uint32_t timeout_ms = DEV_UPLOAD_TIMEOUT_MS;
    
    int res = libusb_control_transfer( device_handle, bmRequestType, bRequest, wValue, wIndex, dest, size8, timeout_ms );
    if ( res != ( int ) size8 ) {
        fprintf( stderr, "FX3Dev::transferDataFromDevice() error %d %s\n", res, libusb_error_name(res) );
        return FX3_ERR_CTRL_TX_FAIL;
    }
    return FX3_ERR_OK;
}


fx3_dev_err_t FX3Dev::ctrlToDevice(uint8_t cmd, uint16_t value, uint16_t index, void *data, size_t data_len)
{
    uint8_t  dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;
    uint8_t* buf = dummybuf;

    if ( data && data_len != 0 ) {
        buf = (uint8_t*)data;
        len = data_len;
    }

    return txControlToDevice( buf, len, cmd, value, index );
}

fx3_dev_err_t FX3Dev::ctrlFromDevice(uint8_t cmd, uint16_t value, uint16_t index, void *dest, size_t data_len)
{
    uint8_t  dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;
    uint8_t* buf = dummybuf;

    if ( dest && data_len != 0 ) {
        buf = (uint8_t*)dest;
        len = data_len;
    }
    return txControlFromDevice( buf, len, cmd, value, index );
}

fx3_dev_err_t FX3Dev::loadAdditionalFirmware( const char* fw_name, uint32_t stop_addr ) {
    if ( !fw_name ) {
        return FX3_ERR_ADDFIRMWARE_FILE_IO_ERROR;
    }
    
    FILE* f = fopen( fw_name, "r" );
    if ( !f ) {
        fprintf( stderr, "FX3Dev::loadAdditionalFirmware __error__ can't open '%s'\n", fw_name );
        return FX3_ERR_ADDFIRMWARE_FILE_IO_ERROR;
    } else {
        fclose( f );
    }
    
    std::vector<uint32_t> addr;
    std::vector<uint32_t> data;
    
    parse_hex_file( fw_name, addr, data );
    
    for ( uint32_t i = 0; i < addr.size(); i++ ) {
        fx3_dev_err_t eres = send16bitToDeviceSynch(data[i], addr[i]);
        if ( eres != FX3_ERR_OK ) {
            return eres;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds((uint64_t)ADD_FW_LOAD_PAUSE_MS) );
        
        if ( addr[i] == stop_addr ) {
            break;
        }
    }
    return FX3_ERR_OK;
}


void FX3Dev::sendAttCommand5bits( uint32_t bits ) {
    if ( endpoint_from_hst_num == endpoint_invalid ) {
        uint8_t hi_bits = 0;
        uint8_t lo_bits = ( bits & 0x1F );
        send16bitToDeviceSynch(lo_bits, hi_bits);
    } else {
        uint8_t lep[4];
        lep[0] = ( bits & 0x1F );
        lep[1] = 0;
        lep[2] = 0;
        lep[3] = 0;
        sendDataToDeviceASynch( lep, 4 );
    }
}

fx3_dev_debug_info_t FX3Dev::getDebugInfoFromBoard(bool ask_speed_only) {
    if ( ask_speed_only ) {
        fx3_dev_debug_info_t info;
        info.status = FX3_ERR_OK;
        info.size_tx_mb_inc = size_tx_mb;
        info.speed_only = true;
        size_tx_mb = 0.0;
        return info;
    } else {
        uint32_t len32 = 8;
        uint32_t* ans = new uint32_t[ len32 ];
        fx3_dev_err_t eres = txControlFromDevice( (uint8_t*)ans, len32 * sizeof( uint32_t ), CMD_READ_DBG, 0, 1 );
        if ( eres != FX3_ERR_OK ) {
            fprintf( stderr, "FX3Dev::getDebugInfoFromBoard() __error__  %d %s\n", eres, fx3_get_error_string( eres ) );
        }
        fx3_dev_debug_info_t info;
        info.status = eres;
        info.transfers   = ans[ 0 ];
        info.overflows   = ans[ 1 ];
        info.phy_err_inc = ans[ 2 ];
        info.lnk_err_inc = ans[ 3 ];
        info.err_reg_hex = ans[ 4 ];
        info.phy_errs    = ans[ 5 ];
        info.lnk_errs    = ans[ 6 ];

        info.size_tx_mb_inc = size_tx_mb;
        info.speed_only = false;
        size_tx_mb = 0.0;

        info.overflows_inc = info.overflows - last_overflow_count;
        last_overflow_count = info.overflows;
        delete [] ans;
        return info;
    }
}


//---------------------- Lattice  ----------------------------

fx3_dev_err_t FX3Dev::send16bitSPI_ECP5(uint8_t _addr, uint8_t _data)
{
    uint8_t  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;

    buf[0] = (uint8_t)_addr;
    buf[1] = (uint8_t)_data;

    uint8_t cmd = 0xD6;
    uint16_t value = 0;
    uint16_t index = 1;

    fprintf( stderr, "Reg:%u 0x%04x \n", _addr, _data);

    return txControlToDevice( buf, len, cmd, value, index);
}

fx3_dev_err_t FX3Dev::sendECP5(uint8_t* _data, long _data_len)
{
    uint8_t  dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;
    uint8_t* buf = dummybuf;

    if(_data && _data_len != 0 ) {
        buf = (uint8_t*)_data;
        len = _data_len;
    }

    uint8_t cmd = ECP5_WRITE;
    uint16_t value = 0;
    uint16_t index = 1;

    return txControlToDevice(buf, len, cmd, value, index);
}

fx3_dev_err_t FX3Dev::recvECP5(uint8_t* _data, long _data_len)
{
    uint8_t  dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;
    uint8_t* buf = dummybuf;

    if(_data && _data_len != 0 ) {
        buf = (uint8_t*)_data;
        len = _data_len;
    }
    uint8_t cmd = ECP5_READ;
    uint16_t value = 0;
    uint16_t index = 1;

    return txControlFromDevice(buf, len, cmd, value, index );
}

fx3_dev_err_t FX3Dev::resetECP5()
{
    uint8_t  dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;

    uint8_t cmd = 0xD0;
    uint16_t value = 0;
    uint16_t index = 1;

    int success = (txControlFromDevice(dummybuf, len, cmd, value, index) == FX3_ERR_OK) ? 1 : 0;

    return (success & dummybuf[0]) ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3Dev::checkECP5()
{
    uint8_t  dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    memset(&dummybuf[0], 0xff, 16);
    uint32_t len = 16;

    uint8_t cmd = ECP5_CHECK;
    uint16_t value = 0;
    uint16_t index = 1;

    int success = (txControlFromDevice(dummybuf, len, cmd, value, index) == FX3_ERR_OK) ? 1 : 0;

    return (success & dummybuf[0]) ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3Dev::csonECP5()
{
    uint8_t  dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    ::memset(&dummybuf[0], 0xff, 16);
    uint32_t len = 16;

    uint8_t cmd = ECP5_CSON;
    uint16_t value = 0;
    uint16_t index = 1;

    int success = (txControlToDevice(dummybuf, len, cmd, value, index) == FX3_ERR_OK) ? 1 : 0;

    return (success & dummybuf[0]) ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3Dev::csoffECP5()
{
    uint8_t  dummybuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    ::memset(&dummybuf[0], 0xff, 16);
    uint32_t len = 16;

    uint8_t cmd = ECP5_CSOFF;
    uint16_t value = 0;
    uint16_t index = 1;

    int success = (txControlToDevice(dummybuf, len, cmd, value, index) == FX3_ERR_OK) ? 1 : 0;

    return (success & dummybuf[0]) ? FX3_ERR_OK : FX3_ERR_CTRL_TX_FAIL;
}

fx3_dev_err_t FX3Dev::send24bitSPI8bit(unsigned int _data)
{
    uint8_t  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;

    buf[0] = (uint8_t)(_data>16);
    buf[1] = (uint8_t)(_data>>8);
    buf[2] = (uint8_t)_data;

    uint8_t cmd = 0xD8;
    uint16_t value = 0;
    uint16_t index = 1;

    return txControlToDevice( buf, len, cmd, value, index);
}

fx3_dev_err_t FX3Dev::device_start()
{
    uint8_t  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;

    buf[2] = (uint8_t)(0xFF);

    uint8_t cmd = 0xBA;
    uint16_t value = 0;
    uint16_t index = 1;

    return txControlToDevice(buf, len, cmd, value, index);
}

fx3_dev_err_t FX3Dev::device_stop()
{
    uint8_t  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;

    buf[2] = (uint8_t)(0xFF);

    uint8_t cmd = 0xBB;
    uint16_t value = 0;
    uint16_t index = 1;

    return txControlToDevice(buf, len, cmd, value, index);
}

fx3_dev_err_t FX3Dev::device_reset()
{
    uint8_t  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;

    buf[2] = (uint8_t)(0xFF);

    uint8_t cmd = 0xB3;
    uint16_t value = 0;
    uint16_t index = 1;

    return txControlToDevice(buf, len, cmd, value, index);
}

fx3_dev_err_t FX3Dev::reset_nt1065()
{
    uint8_t  buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = 16;

    buf[2] = (uint8_t)(0xFF);

    uint8_t cmd = 0xD7;
    uint16_t value = 0;
    uint16_t index = 1;

    return txControlToDevice(buf, len, cmd, value, index);
}

fx3_dev_err_t FX3Dev::load1065Ctrlfile(const char* fwFileName, int lastaddr)
{
    fx3_dev_err_t retCode = FX3_ERR_OK;
    char line[128];

    FILE* pFile = fopen( fwFileName, "r" );
    if(!pFile) {
        return FX3_ERR_FIRMWARE_FILE_IO_ERROR;
    }

    while(fgets(line, 128, pFile) != NULL)
    {
        const std::string sline(line);
        if(sline[0] != ';')
        {
            size_t regpos = sline.find("Reg");
            if(regpos != std::string::npos)
            {
                std::string buf;
                std::stringstream ss(sline); // Insert the string into a stream
                std::vector<std::string> tokens;
                while(ss >> buf)
                    tokens.push_back(buf);
                if(tokens.size() == 2) // addr & value
                {
                    int addr = std::stoi(tokens[0].erase(0,3), nullptr, 10);
                    int value = std::stoi(tokens.at(1), nullptr, 16);
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    retCode = send16bitSPI_ECP5(addr, value);
                    if(retCode != FX3_ERR_OK || addr == lastaddr)
                        break;
                }
            }
        }
    }

    fclose(pFile);

    return retCode;
}

//------------------------------------------------------------


fx3_dev_err_t FX3Dev::send16bitToDeviceSynch( uint8_t byte0, uint8_t byte1 ) {
    //fprintf( stderr, "FX3Dev::send16bitToDevice( 0x%02X, 0x%02X )\n", byte0, byte1 );
    // see transferDataToDevice() for more information abput this specific values
    uint8_t bmRequestType = LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT;
    uint8_t bRequest = CMD_REG_WRITE;
    uint16_t wValue = 0;
    uint16_t wIndex = 1;
    uint32_t timeout_ms = DEV_UPLOAD_TIMEOUT_MS;
    
    uint8_t data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    data[0] = byte0;
    data[1] = byte1;
    
    int res = libusb_control_transfer( device_handle, bmRequestType, bRequest, wValue, wIndex, data, 16, timeout_ms );
    if ( res != 16 ) {
        fprintf( stderr, "FX3Dev::send16bitToDevice() error %d %s\n", res, libusb_error_name(res) );
        return FX3_ERR_REG_WRITE_FAIL;
    }
    return FX3_ERR_OK;
}

fx3_dev_err_t FX3Dev::sendDataToDeviceASynch(uint8_t* data, uint32_t size8) {

    if ( write_transfer == NULL ) {
        fprintf( stderr, "FX3Dev::sendDataToDeviceASynch() endpoint wasn't init yet. Let's do it now\n" );
        write_transfer = libusb_alloc_transfer(0);
        if ( write_transfer == NULL ) {
            fprintf( stderr, "FX3Dev::sendDataToDeviceASynch() __error__ libusb_alloc_transfer(0) returned NULL for write_transfer\n" );
            abort();
        } else {
            libusb_fill_bulk_transfer(write_transfer, device_handle, (endpoint_from_hst_num | LIBUSB_ENDPOINT_OUT), write_buffer,
                                      16, FX3Dev::onWriteReady, this, DEV_DOWNLOAD_TIMEOUT_MS);
        }
    }

    fprintf( stderr, "FX3Dev::sendDataToDeviceASynch() " );
    
    
    for ( int i = 0; i < 16; i++ ) {
        write_buffer[ i ] = 0;
    }
    
    for ( uint32_t i = 0; i < size8; i++ ) {
        write_buffer[ i ] = data[ i ];
    }
    
    int res;
    do {
        res= libusb_submit_transfer(write_transfer);
        if ( res == LIBUSB_ERROR_BUSY ) {
            fprintf( stderr, "FX3Dev::sendDataToDeviceASynch() previous transfer isn't complete!\n" );
            continue;
        }
        
        if(res != 0) {
            fprintf(stderr,"FX3Dev::sendDataToDeviceASynch() __error__ libusb_submit_transfer %d %s\n", res, libusb_error_name(res));
            abort();
        }
    } while ( res != FX3_ERR_OK );
    
    return FX3_ERR_OK;
}

void FX3Dev::startRead( DeviceDataHandlerIfce* handler ) {
    fprintf(stderr,"FX3Dev::startRead()\n");
    size_tx_mb = 0.0;
    
    int res;
    for(uint32_t i = 0;i<buffers_count;i++) {
        transfers[i] = libusb_alloc_transfer(0);
        if ( transfers[i] == NULL ) {
            fprintf( stderr, "FX3Dev::startRead() __error__ libusb_alloc_transfer(0) returned NULL\n" );
            abort();
        }
    }

    for(uint32_t i = 0;i<buffers_count;i++) {
        libusb_fill_bulk_transfer(transfers[i], device_handle, (endpoint_from_dev_num | LIBUSB_ENDPOINT_IN), half_full_buffers[i],
                                  half_full_size8, FX3Dev::onDataReady, this, DEV_DOWNLOAD_TIMEOUT_MS);
    }
    
    data_handler = handler;
    event_loop_running = true;
    event_thread = std::thread(&FX3Dev::event_loop, this);
    
    for(uint32_t i = 0;i<buffers_count;i++) {
        res = libusb_submit_transfer(transfers[i]);
        if(res != 0) {
            fprintf(stderr,"FX3Dev::startRead() __error__ libusb_submit_transfer%d %d %s\n", i, res, libusb_error_name(res));
            abort();
        }
    }
    device_start();
}

void FX3Dev::event_loop( void ) {
    fprintf( stderr, "FX3Dev::read_loop() started\n" );
    while(event_loop_running) {
        struct timeval tv  = { 0, DEV_DOWNLOAD_TIMEOUT_MS * 1000 };
        int res = libusb_handle_events_timeout_completed( NULL, &tv, NULL );
        if(res != 0) {
            fprintf(stderr,"FX3Dev::read_loop() __error__ %d, read error: %s\n", res, libusb_error_name(res));
            abort();
        }
    }
    fprintf( stderr, "FX3Dev::read_loop() finished\n" );
}


void FX3Dev::stopRead() {
    if ( event_loop_running ) {
        for(uint32_t i = 0;i<buffers_count;i++) {
            libusb_cancel_transfer(transfers[i]);
        }
        fprintf( stderr, "FX3Dev::stopRead() stopping read_thread...\n" );
        #ifdef WIN32
        Sleep( DEV_DOWNLOAD_TIMEOUT_MS * buffers_count );
        #else
        usleep( DEV_DOWNLOAD_TIMEOUT_MS * buffers_count );
        #endif
        
        event_loop_running = false;
        event_thread.join();
        
        for(uint32_t i = 0;i<buffers_count;i++) {
            libusb_free_transfer(transfers[i]);
        }
        
        data_handler = NULL;
        fprintf( stderr, "FX3Dev::stopRead() all done!\n" );    
    }

    device_stop();
}

void LIBUSB_CALL FX3Dev::onDataReady( libusb_transfer* xfr ) {
    FX3Dev *self = (FX3Dev *) xfr->user_data;
    switch(xfr->status) {
        case LIBUSB_TRANSFER_COMPLETED:
            libusb_submit_transfer(xfr);
            self->size_tx_mb += ( double ) xfr->actual_length / ( 1024.0 * 1024.0 );
            if ( self->data_handler ) {
                self->data_handler->HandleDeviceData( (short*) xfr->buffer, xfr->actual_length );
            }
            break;
        case LIBUSB_TRANSFER_CANCELLED:
            fprintf( stderr,"FX3Dev::onDataReady(): __warning__ LIBUSB_TRANSFER_CANCELLED\n" );
            break;
        case LIBUSB_TRANSFER_NO_DEVICE:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ LIBUSB_TRANSFER_NO_DEVICE\n" );
            break;
        case LIBUSB_TRANSFER_TIMED_OUT:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ LIBUSB_TRANSFER_TIMED_OUT\n" );
            break;
        case LIBUSB_TRANSFER_ERROR:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ LIBUSB_TRANSFER_ERROR\n" );
            break;
        case LIBUSB_TRANSFER_STALL:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ LIBUSB_TRANSFER_STALL\n" );
            break;
        case LIBUSB_TRANSFER_OVERFLOW:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ LIBUSB_TRANSFER_OVERFLOW\n" );
            break;

        default:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ unknown xfr->status %d\n", xfr->status );
    }
    
}

void FX3Dev::onWriteReady(libusb_transfer* xfr) {
    switch(xfr->status) {
        case LIBUSB_TRANSFER_COMPLETED:
            fprintf( stderr,"FX3Dev::onWriteReady(): __log__ LIBUSB_TRANSFER_COMPLETED\n" );
            break;
        case LIBUSB_TRANSFER_CANCELLED:
            fprintf( stderr,"FX3Dev::onDataReady(): __warning__ LIBUSB_TRANSFER_CANCELLED\n" );
            break;
        case LIBUSB_TRANSFER_NO_DEVICE:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ LIBUSB_TRANSFER_NO_DEVICE\n" );
            break;
        case LIBUSB_TRANSFER_TIMED_OUT:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ LIBUSB_TRANSFER_TIMED_OUT\n" );
            break;
        case LIBUSB_TRANSFER_ERROR:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ LIBUSB_TRANSFER_ERROR\n" );
            break;
        case LIBUSB_TRANSFER_STALL:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ LIBUSB_TRANSFER_STALL\n" );
            break;
        case LIBUSB_TRANSFER_OVERFLOW:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ LIBUSB_TRANSFER_OVERFLOW\n" );
            break;

        default:
            fprintf( stderr,"FX3Dev::onDataReady(): __error__ unknown xfr->status %d\n", xfr->status );
    }    
}

