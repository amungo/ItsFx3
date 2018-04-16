#include "lfe5u_hardware.h"
#include "lfe5u_debug.h"
#include "lfe5u_util.h"
#include "lfe5u_opcode.h"
#include "hwfx3/fx3devifce.h"
#include <thread>
#include <chrono>
#include <cassert>


// In order to use stream transmission, dataBuffer[] is required to
// buffer the data.  Please refer to devices' specification for the
// number of bytes required.  For XP2-40, minimum is 423 bytes.
// Declare a little bit more than the minimum, just to be safe.
static const int BUFFER_SIZE = 1024;

LFE5U_Hardware::LFE5U_Hardware(FX3DevIfce* _device_cmd_io, std::shared_ptr<DataLattice>& _data_lattice) : m_io_iface(_device_cmd_io), m_pDataLattice(_data_lattice)
{
    assert(_device_cmd_io);
    m_dataBuffer.resize(BUFFER_SIZE);
}

//--------------  Hardware methods  --------------------

// this function is responsible to initialize SSPI port.
int LFE5U_Hardware::SPI_init()
{
    return m_io_iface->resetECP5();
}

// this function is responsible to turn off SSPI port.
int LFE5U_Hardware::SPI_final()
{
    return m_io_iface->checkECP5();
}

// this function take the number of millisecond and wait for the time specified.
int LFE5U_Hardware::Wait(int32_t _ms)
{
    std::this_thread::sleep_for( std::chrono::milliseconds(_ms) );
    return PROC_COMPLETE;
}

//----------------   SPI transmission functions    --------------------

// this function initiates a transmission by pulling chip-select low.
int LFE5U_Hardware::TRANS_starttranx(uint8_t _channel)
{
    _channel = _channel;

    return m_io_iface->csoffECP5();
}

// this function terminates the transmission by pulling chip-select high.
int LFE5U_Hardware::TRANS_endtranx()
{
    return m_io_iface->csonECP5();
}

// *******************************************************************
// Function TRANS_cstoggle(unsigned char channel)
// Purpose: To toggle chip select (CS) of specific channel
//
// Return:		0 - succeed
//				1 - fail
//
// If your embedded system has a dedicated SPI port that does not
// allow bit banging, simply transmit a byte of 0xFF to the device,
// and the device will ignore that.
// *******************************************************************
int LFE5U_Hardware::TRANS_cstoggle(uint8_t _channel)
{
    // ********************************************************
    // here you should implement toggling CRESET signal.
    // Currently it prints message on screen and in log file.
    //*********************************************************
    if(_channel != 0x00)
        return PROC_FAIL;
    else{
        // *******************************************************
        // here you should implement toggling CS.
        // Currently it prints message on screen and in log file.
        // *******************************************************
        m_io_iface->csonECP5();
        m_io_iface->csoffECP5();
        return m_io_iface->csonECP5();
    }
}

// *************************************************************
// Function TRANS_trsttoggle(unsigned char toggle)
// Purpose: To toggle CRESET (TRST) signal
//
// Return:		0 - succeed
//				1 - fail
// ************************************************************
int LFE5U_Hardware::TRANS_trsttoggle(uint8_t _toggle)
{
    // ********************************************************
    // here you should implement toggling CRESET signal.
    // Currently it prints message on screen and in log file.
    // ********************************************************
    _toggle = _toggle;

    return PROC_COMPLETE;
}

// **********************************************************************
// Function TRANS_runClk()
// Purpose: To drive extra clock.
//
// Return:		1 - succeed
//				0 - fail
// If your embedded system has a dedicated SPI port that does not
// allow bit banging, simply transmit a byte of 0xFF on another channel
// that is not being used, so the device will only see the clock.
// **********************************************************************

// *******************************************************************
// here you should implement running free clock
// *******************************************************************
int LFE5U_Hardware::TRANS_runClk()
{
    uint8_t OXFF = 0xFF;
    m_io_iface->sendECP5(&OXFF, 1);
    m_io_iface->sendECP5(&OXFF, 1);
    return m_io_iface->sendECP5(&OXFF, 1);
}

// this function is responsible to transmit data over SSPI port.
int LFE5U_Hardware::TRANS_transmitBytes(uint8_t* _trBuffer, int32_t _trCount)
{
    int32_t n_bytes = _trCount >> 3;
    return m_io_iface->sendECP5(_trBuffer, n_bytes);
}

// this function is responsible to receive data through SSPI port.
int LFE5U_Hardware::TRANS_receiveBytes(uint8_t* _rcBuffer, int32_t _rcCount)
{
    int32_t n_bytes = _rcCount >> 3;
    return m_io_iface->recvECP5(_rcBuffer, n_bytes);
}

// *************************************************************************
// Function TRANS_transceive_stream(int trCount, unsigned char *trBuffer,
// 					int trCount2, int flag, unsigned char *trBuffer2
// Purpose: Transmits opcode and transceive data
//
// It will have the following operations depending on the flag:
//
//		NO_DATA: end of transmission.  trCount2 and trBuffer2 are discarded
//		BUFFER_TX: transmit data from trBuffer2
//		BUFFER_RX: receive data and compare it with trBuffer2
//		DATA_TX: transmit data from external source
//		DATA_RX: receive data and compare it with data from external source
//
// If the data is not byte bounded and your SPI port only transmit/ receive
// byte bounded data, you need to have padding to make it byte-bounded.
// If you are transmit non-byte-bounded data, put the padding at the beginning
// of the data.  If you are receiving data, do not compare the padding,
// which is at the end of the transfer.
// *************************************************************************

#define NO_DATA		0
#define BUFFER_TX	1
#define BUFFER_RX	2
#define DATA_TX		3
#define DATA_RX		4

int LFE5U_Hardware::TRANS_transceive_stream(int32_t _trCount, uint8_t* _trBuffer,
                            int32_t _trCount2, int32_t _flag, uint8_t* _trBuffer2,
                            int32_t _flag_mask, uint8_t* _mask_buffer)
{
    int32_t i               = 0;
    uint16_t tranxByte      = 0;
    uint8_t trByte          = 0;
    uint8_t dataByte        = 0;
    int32_t mismatch        = 0;
    uint8_t dataID          = 0;
    uint8_t* dataBuffer     = m_dataBuffer.data();

    if(_trCount > 0)
    {
        // calculate # of bytes being transmitted
        tranxByte = (uint16_t)(_trCount / 8);
        if(_trCount % 8 != 0){
            tranxByte ++;
            _trCount += (8 - (_trCount % 8));
        }
        if(TRANS_transmitBytes(_trBuffer, _trCount) != PROC_COMPLETE)
            return ERROR_PROC_HARDWARE;
    }
    switch(_flag)
    {
    case NO_DATA:
        return PROC_COMPLETE;
        break;
    case BUFFER_TX:
        tranxByte = (uint16_t)(_trCount2 / 8);
        if(_trCount2 % 8 != 0)
        {
            tranxByte ++;
            _trCount2 += (8 - (_trCount2 % 8));
        }
        if(TRANS_transmitBytes(_trBuffer2, _trCount2) != PROC_COMPLETE)
            return ERROR_PROC_HARDWARE;

        return PROC_COMPLETE;
        break;
    case BUFFER_RX:
        tranxByte = (uint16_t)(_trCount2 / 8);
        if(_trCount2 % 8 != 0)
        {
            tranxByte ++;
            _trCount2 += (8 - (_trCount2 % 8));
        }
        if(TRANS_receiveBytes(_trBuffer2, _trCount2) != PROC_COMPLETE)
            return ERROR_PROC_HARDWARE;
        return PROC_COMPLETE;
        break;
    case DATA_TX:
        tranxByte = (uint16_t)((_trCount2 + 7) / 8);
        if(_trCount2 % 8 != 0) {
            trByte = (uint8_t)(0xFF << (_trCount2 % 8));
        }
        else
            trByte = 0;

        if(_trBuffer2 != 0)
            dataID = *_trBuffer2;
        else
            dataID = 0x04;

        for(i=0; i < tranxByte; i++)
        {
            if(i == 0){
                if(m_pDataLattice->HLDataGetByte(dataID, &dataByte, _trCount2) != PROC_COMPLETE)
                    return ERROR_INIT_DATA;
            }
            else {
                if(m_pDataLattice->HLDataGetByte(dataID, &dataByte, 0) != PROC_COMPLETE)
                    return ERROR_INIT_DATA;
            }
            if(_trCount2 % 8 != 0)
                trByte += (uint8_t)(dataByte >> (8- (_trCount2 % 8)));
            else
                trByte = dataByte;

            dataBuffer[i] = trByte;
            // do not remove the line below!  It handles the padding for
            // non-byte-bounded data
            if(_trCount2 % 8 != 0)
                trByte = (uint8_t)(dataByte << (_trCount2 % 8));
        }
        if(_trCount2 % 8 != 0){
            _trCount2 += (8 - (_trCount2 % 8));
        }
        if(TRANS_transmitBytes(dataBuffer, _trCount2) != PROC_COMPLETE)
            return ERROR_PROC_HARDWARE;
        return PROC_COMPLETE;
        break;
    case DATA_RX:
        tranxByte = (uint16_t)(_trCount2 / 8);
        if(_trCount2 % 8 != 0){
            tranxByte ++;
        }
        if(_trBuffer2 != 0)
            dataID = *_trBuffer2;
        else
            dataID = 0x04;
        if(TRANS_receiveBytes(dataBuffer, (tranxByte * 8)) != PROC_COMPLETE)
            return ERROR_PROC_HARDWARE;
        for(i=0; i < tranxByte; i++)
        {
            if(i == 0) {
                if(m_pDataLattice->HLDataGetByte(dataID, &dataByte, _trCount2) != PROC_COMPLETE)
                    return ERROR_INIT_DATA;
            }
            else {
                if(m_pDataLattice->HLDataGetByte(dataID, &dataByte, 0) != PROC_COMPLETE)
                    return ERROR_INIT_DATA;
            }

            trByte = dataBuffer[i];
            if(_flag_mask)
            {
                trByte = trByte & _mask_buffer[i];
                dataByte = dataByte & _mask_buffer[i];
            }
            if(i == tranxByte - 1){
                trByte = (uint8_t)(trByte ^ dataByte) &
                    (uint8_t)(0xFF << (8 - (_trCount2 % 8)));
            }
            else
                trByte = (uint8_t)(trByte ^ dataByte);

            if(trByte)
                mismatch ++;
        }
        if(mismatch == 0)
        {
            if(m_uiCheckFailedRow)
            {
                m_uiRowCount++;
            }
            return PROC_COMPLETE;
        }
        else
        {
            if(dataID == 0x01 && m_uiRowCount == 0)
            {
                return ERROR_IDCODE;
            }
            else if(dataID == 0x05)
            {
                return ERROR_USERCODE;
            }
            else if(dataID == 0x06)
            {
                return ERROR_SED;
            }
            else if(dataID == 0x07)
            {
                return ERROR_TAG;
            }
            return ERROR_VERIFICATION;
        }
        break;
    default:
        return ERROR_INIT_ALGO;
    }

    return PROC_COMPLETE;
}
