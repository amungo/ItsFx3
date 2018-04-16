#ifndef __LFE5U_UTIL_H__
#define __LFE5U_UTIL_H__

#include <string>
#include <vector>
#include <fstream>

// ***********************************************************************
// Utility functions
// ***********************************************************************

class CSU
{
public:
    CSU() : m_csValue(0), m_csWidth(0), m_csChunkSize(0)
    {}

    CSU(short int _width, short int _chunkSize) : m_csValue(0), m_csWidth(_width),
                                                m_csChunkSize(_chunkSize)
    {}

    void init_CS(short int width, short int chunkSize);
    unsigned int getCheckSum();
    void putChunk(unsigned int chunk);

    friend class SSPICore;
protected:
    unsigned int m_csValue;
    short int m_csWidth;
    short int m_csChunkSize;
};


// ************************************************************************
// algorithm utility functions
// ************************************************************************

class AlgoLattice
{
public:
    AlgoLattice();
    AlgoLattice(const char* _algoFileName);
    ~AlgoLattice();

    int algoPreset(const char* _algoFileName);
    int algoInit();
    int algoGetByte(unsigned char* _byteOut);
    int algoFinal();

protected:
    std::string m_fileName;
    std::ifstream m_file;
};

// ************************************************************************
// Data utility functions
// ************************************************************************
struct DATA_TOC
{
    DATA_TOC() : ID(0), uncomp_size(0),
                compression(0), address(0)
    {}
    unsigned char ID;
    unsigned int uncomp_size;
    unsigned char compression;
    unsigned int address;
};

struct DATA_BUFFER
{
    DATA_BUFFER() : ID(0), address(0)
    {}
    unsigned char ID;
    unsigned int address;
};


#define DATA_BUFFER_SIZE	5
#define D_TOC_NUMBER		16

#define		SSPI_DATAUTIL_VERSION1		1
#define		SSPI_DATAUTIL_VERSION2		2
#define		SSPI_DATAUTIL_VERSION3		3

class DataLattice
{
public:
    DataLattice();
    DataLattice(const char* _dataFileName);
    ~DataLattice();

    int dataInit(); // initialize data
    int dataPreset(const char* _dataFileName);
    int dataReset(unsigned char _isResetBuffer); // reset data pointer
    int dataFinal();
    int HLDataGetByte(unsigned char _dataSet, unsigned char* _dataByte, unsigned int _uncomp_bitsize);

protected:
    int dataGetByte(unsigned char* _byteOut, short int _incCurrentAddr, CSU* _checksumUnit); // get one byte from current column

    // --------------- Decompression ----------------
    void set_compression(unsigned char _cmp);
    unsigned char get_compression() const;
    short int decomp_initFrame(int _bitSize);
    short int decomp_getByte(unsigned char* _byteOut);
    short int decomp_getNum();

    int dataReadthroughComment();
    int dataLoadTOC(short int _storeTOC);
    int dataRequestSet(unsigned char dataSet);

protected:
    std::string m_fileName;
    std::ifstream m_file;
    std::vector<DATA_BUFFER> m_dataBufferArray;
    std::vector<DATA_TOC> m_toc;
    unsigned short int  m_tocNumber;
    unsigned char		m_isDataInput;
    unsigned int		m_offset;
    unsigned int		m_currentAddress;
    unsigned char		m_requestNewData;
    unsigned int		m_currentSize;
    unsigned short int	m_currentDataSetIndex;
    CSU					m_CSU;
    short int			m_SSPIDatautilVersion;

    unsigned char       m_compression;
    unsigned char       m_compByte;
    short int           m_currentCounter;
    unsigned short int  m_frameSize;
    unsigned short int  m_frameCounter;

};

#endif // __LFE5U_UTIL_H__


