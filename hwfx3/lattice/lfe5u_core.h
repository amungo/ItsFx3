#ifndef __LFE5U_CORE_H__
#define __LFE5U_CORE_H__

#include <vector>
#include <string>
#include <memory>

class CSU;

class FX3DevIfce;
class AlgoLattice;
class DataLattice;
class LFE5U_Hardware;

class SSPICore
{
public:
    SSPICore(FX3DevIfce* _device_cmd_io);
    ~SSPICore();

    int SSPIEm_preset(const std::string& _algoFileName, const std::string& _dataFileName);
    int SSPIEm(unsigned int _algoID);

protected:
    // *********************************************************************
    // Processing functions
    // *********************************************************************
    int SSPIEm_init(unsigned int _algoID);
    int SSPIEm_process(unsigned char* _bufAlgo, unsigned int _bufAlgoSize);
    int VME_getByte(unsigned char* _byteOut, unsigned char* _bufferedAlgo,
                    unsigned int _bufferedAlgoSize, unsigned int* _bufferedAlgoIndex);
    unsigned int VME_getNumber(unsigned char* _bufAlgo, unsigned int _bufAlgoSize,
                      unsigned int* _bufAlgoIndex, unsigned int* _byteCount);

    int proc_TRANS(unsigned char* _bufAlgo, unsigned int _bufAlgoSize,
                   unsigned int* _absbufAlgoIndex, unsigned char _channel, unsigned char _currentByte);
    int proc_REPEAT(unsigned char* _bufAlgo, unsigned int _bufAlgoSize,
                    unsigned int* _absBufAlgoIndex, unsigned int _LoopMax);
    int proc_LOOP(unsigned char* _bufAlgo, unsigned int _bufAlgoSize,
                  unsigned int* _absBufAlgoIndex, unsigned int _LoopMax);
    int proc_HCOMMENT(unsigned char* _bufferedAlgo, unsigned int _bufferedAlgoSize,
                   unsigned int* _absBufferedAlgoIndex, CSU* _headerCS);

protected:
    unsigned char getCurrentChannel()
    { return m_currentChannel; }

protected:
    std::shared_ptr<AlgoLattice> m_pAlgoLattice;
    std::shared_ptr<DataLattice> m_pDataLattice;
    std::shared_ptr<LFE5U_Hardware> m_pHardware;

    unsigned char m_currentChannel;

    std::vector<unsigned char> m_version;
    std::vector<unsigned char> m_algoBuffer;
    std::vector<unsigned char> m_stack;
};

#endif // __LFE5U_CORE_H__

