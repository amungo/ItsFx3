#ifndef __LFE5U_HADWARE_H__
#define __LFE5U_HADWARE_H__

#include <memory>
#include <vector>
#include <cstdint>

class FX3DevIfce;
class DataLattice;


class LFE5U_Hardware
{
public:
    LFE5U_Hardware(FX3DevIfce* _device_cmd_io, std::shared_ptr<DataLattice>& _data_lattice);

    // Hardware methods
    int SPI_init(); // this function is responsible to initialize SSPI port.
    int SPI_final(); // this function is responsible to turn off SSPI port.
    int Wait(int32_t _ms); // this function take the number of millisecond and wait for the time specified.

    // SPI transmission functions
    int TRANS_starttranx(uint8_t _channel); // this function initiates a transmission by pulling chip-select low.
    int TRANS_endtranx(); // this function terminates the transmission by pulling chip-select high.
    int TRANS_cstoggle(uint8_t _channel); // this function pulls chip-select low, then pulls it high.
    int TRANS_trsttoggle(uint8_t _toggle); // this function pulls CRESET low or high.
    int TRANS_runClk(); // this function is responsible to drive at least 3 extra clocks after chip-select is pulled high.
    int TRANS_transmitBytes(uint8_t* _trBuffer, int32_t _trCount); // this function is responsible to transmit data over SSPI port.
    int TRANS_receiveBytes(uint8_t* _rcBuffer, int32_t _rcCount); // this function is responsible to receive data through SSPI port.

    int TRANS_transceive_stream(int32_t _trCount, uint8_t* _trBuffer,
                                int32_t _trCount2, int32_t _flag, uint8_t* _trBuffer2,
                                int32_t _flag_mask, uint8_t* _mask_buffer);
    friend class SSPICore;
protected:
    FX3DevIfce* m_io_iface;
    std::shared_ptr<DataLattice> m_pDataLattice;

    unsigned int m_uiCheckFailedRow = 0;
    unsigned int m_uiRowCount		= 0;
    std::vector<uint8_t> m_dataBuffer;
};

#endif // __LFE5U_HADWARE_H__

