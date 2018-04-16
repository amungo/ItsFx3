#include "lfe5u_util.h"
#include "lfe5u_opcode.h"
#include <cinttypes>

using namespace std;

//------------------    --------------------------------

void CSU::init_CS(short int _width, short int _chunkSize)
{
    m_csWidth = _width;
    m_csChunkSize = _chunkSize;
    m_csValue = 0;
}

unsigned int CSU::getCheckSum()
{
    unsigned int mask = 0xFFFFFFFF;
    mask >>= (32 - m_csWidth);

    return (m_csValue & mask);
}

void CSU::putChunk(unsigned int _chunk)
{
    unsigned int mask = 0xFFFFFFFF;
    mask >>= (32 - m_csChunkSize);

    m_csValue += (_chunk & mask);
}

//-----------------    -----------------------------
AlgoLattice::AlgoLattice()
{
}

AlgoLattice::AlgoLattice(const char* _algoFileName) : m_fileName(_algoFileName)
{
}

AlgoLattice::~AlgoLattice()
{
    algoFinal();
}

int AlgoLattice::algoPreset(const char* _algoFileName)
{
    m_fileName = _algoFileName;
    return PROC_COMPLETE;
}

int AlgoLattice::algoInit()
{
    if(m_file.is_open())
        m_file.close();
    m_file.open(m_fileName.c_str(), ios_base::out | ios_base::binary);

    return m_file.is_open() ? PROC_COMPLETE : PROC_FAIL;

    // **********************************************************************
    // After this initialization, the processing engine need to be able
    // to read algorithm byte by using algoGetByte()
    // **********************************************************************
}

int AlgoLattice::algoGetByte(unsigned char* _byteOut)
{
    if(!m_file.eof())
        m_file.read((char*)_byteOut, 1);

    return PROC_COMPLETE;
}

int AlgoLattice::algoFinal()
{
    if(m_file.is_open())
        m_file.close();

    return PROC_COMPLETE;
}

//--------------------------   DataLattice   -----------------------------------
DataLattice::DataLattice() : m_tocNumber(0), m_isDataInput(0),
                            m_offset(0), m_currentAddress(0),
                            m_requestNewData(0), m_currentSize(0),
                            m_currentDataSetIndex(0), m_SSPIDatautilVersion(0),
                            m_compression(0), m_compByte(0), m_currentCounter(0),
                            m_frameSize(0), m_frameCounter(0)
{
    m_toc.resize(D_TOC_NUMBER);
    m_dataBufferArray.resize(DATA_BUFFER_SIZE);
}

DataLattice::DataLattice(const char* _dataFileName) : m_fileName(_dataFileName),
                                                    m_tocNumber(0), m_isDataInput(0),
                                                    m_offset(0), m_currentAddress(0),
                                                    m_requestNewData(0), m_currentSize(0),
                                                    m_currentDataSetIndex(0), m_SSPIDatautilVersion(0),
                                                    m_compression(0), m_compByte(0), m_currentCounter(0),
                                                    m_frameSize(0), m_frameCounter(0)
{
    if(m_fileName.empty())
        m_isDataInput = 0;
    else
        m_isDataInput = 1;

    m_toc.resize(D_TOC_NUMBER);
    m_dataBufferArray.resize(DATA_BUFFER_SIZE);
}

DataLattice::~DataLattice()
{
    dataFinal();
}

int DataLattice::dataPreset(const char* _dataFileName)
{
    m_fileName = _dataFileName;
    if(m_fileName.empty())
        m_isDataInput = 0;
    else
        m_isDataInput = 1;

    return PROC_COMPLETE;
}

// initialize data
int DataLattice::dataInit()
{
    unsigned char currentByte = 0;
    int temp                  = 0;
    int i                     = 0;
    m_offset                  = 0;
    m_currentDataSetIndex     = 0;
    if(m_isDataInput == 0)
        return PROC_COMPLETE;

    m_file.open(m_fileName.c_str(), ios_base::out | ios_base::binary);
    if(!m_file.is_open())
        return PROC_FAIL;

    // ********************************************************************
    // End of design-dependent implementation
    // After this initialization, the processing engine need to be able
    // to read data by using dataGetByte()
    // ********************************************************************
    for(i = 0; i < DATA_BUFFER_SIZE; i ++)
    {
        m_dataBufferArray[i].ID = 0x00;
        m_dataBufferArray[i].address = 0;
    }
    if(dataGetByte(&currentByte, 0, NULL) != PROC_COMPLETE)
        return PROC_FAIL;
    m_offset ++;
    if(currentByte == HCOMMENT)
    {
        temp = dataReadthroughComment();
        if(!temp )
            return PROC_FAIL;
        m_offset += temp;
        if(dataGetByte(&currentByte, 0, NULL) != PROC_COMPLETE)
            return PROC_FAIL;
        m_offset ++;
    }
    if(currentByte == HDATASET_NUM)
    {
        m_SSPIDatautilVersion = SSPI_DATAUTIL_VERSION3;
        temp = dataLoadTOC(1);
        if(!temp )
            return PROC_FAIL;
        m_offset += temp;
        m_currentAddress = 0x00000000;
        m_requestNewData = 1;
        return PROC_COMPLETE;
    }
    else if(currentByte == 0x00 || currentByte == 0x01)
    {
        m_SSPIDatautilVersion = SSPI_DATAUTIL_VERSION1;
        set_compression(currentByte);
        return PROC_COMPLETE;
    }
    else
        return PROC_FAIL;
}

// get one byte from current column
int DataLattice::dataGetByte(unsigned char* _byteOut, short int _incCurrentAddr, CSU* _checksumUnit)
{
    if(!m_file.is_open() || m_file.eof())
    {
        *_byteOut = 0xFF;
        return PROC_FAIL;
    }
    m_file.read((char*)_byteOut, 1);

    if(_checksumUnit)
        _checksumUnit->putChunk((unsigned int)(*_byteOut));
    if(_incCurrentAddr)
        m_currentAddress++;
    return PROC_COMPLETE;
}

// reset data pointer
int DataLattice::dataReset(unsigned char _isResetBuffer)
{
    unsigned char currentByte   = 0;
    int i                       = 0;

    if(!m_file.is_open())
        return PROC_FAIL;

    //--- rewind ---
    m_file.clear();
    m_file.seekg(0, ios::beg);

    // *******************************************************************
    // End of design-dependent implementation
    //
    // After this, the data stream should be in the same condition as
    // it was initialized.  What dataGetByte() function would get must be
    // the same as what it got when being called in dataInit().
    // *******************************************************************

    if(_isResetBuffer)
    {
        for(i = 0; i < DATA_BUFFER_SIZE; i++)
        {
            m_dataBufferArray[i].ID = 0x00;
            m_dataBufferArray[i].address = 0;
        }
    }
    if(dataGetByte( &currentByte, 0, NULL ) != PROC_COMPLETE)
        return PROC_FAIL;
    if(currentByte == HCOMMENT){
        if(!dataReadthroughComment())
            return PROC_FAIL;
        if(dataGetByte(&currentByte, 0, NULL) != PROC_COMPLETE)
            return PROC_FAIL;
    }

    if(m_SSPIDatautilVersion == SSPI_DATAUTIL_VERSION3)
    {
        dataLoadTOC(0);
        m_currentAddress = 0x00000000;
        m_currentDataSetIndex = 0;
    }

    return PROC_COMPLETE;
}

int DataLattice::dataFinal()
{
    if(m_file.is_open())
        m_file.close();

    return PROC_COMPLETE;
}

int DataLattice::HLDataGetByte(unsigned char _dataSet, unsigned char* _dataByte, unsigned int _uncomp_bitsize)
{
    int retVal              = 0;
    unsigned char tempChar  = 0;
    unsigned int bufferSize = 0;
    unsigned int i          = 0;

    if(m_SSPIDatautilVersion == SSPI_DATAUTIL_VERSION1)
    {
        if(get_compression())
        {
            if(_uncomp_bitsize != 0 && (decomp_initFrame(_uncomp_bitsize) != PROC_COMPLETE))
                return PROC_FAIL;
            retVal = decomp_getByte(_dataByte);
        }
        else
            retVal = dataGetByte(_dataByte, 1, &m_CSU);
        return retVal;
    }
    else
    {
        if(m_requestNewData || _dataSet != m_toc[m_currentDataSetIndex].ID)
        {
            if(dataRequestSet(_dataSet) != PROC_COMPLETE)
                return PROC_FAIL;
            m_currentSize = 0;
            // check if buffer has any address
            for(i = 0; i < DATA_BUFFER_SIZE; i ++)
            {
                if(m_dataBufferArray[i].ID == _dataSet)
                {
                    bufferSize = m_dataBufferArray[i].address;
                    break;
                }
            }
            for(i = 0; i < bufferSize; i ++)
                HLDataGetByte(_dataSet, &tempChar, _uncomp_bitsize);
        }

        if(m_toc[m_currentDataSetIndex].uncomp_size == 0)
        {
            *_dataByte = 0xFF;
            return PROC_FAIL;
        }
        else if(m_currentSize < m_toc[m_currentDataSetIndex].uncomp_size)
        {
            if(get_compression())
            {
                if(_uncomp_bitsize != 0 && !decomp_initFrame(_uncomp_bitsize) )
                    return PROC_FAIL;
                retVal = decomp_getByte(_dataByte);
            }
            else
                retVal = dataGetByte(_dataByte, 1, &m_CSU);
            m_currentSize ++;
            // store data buffer
            for(i = 0; i < DATA_BUFFER_SIZE; i ++)
            {
                if(m_dataBufferArray[i].ID == _dataSet)
                {
                    if(m_currentSize != m_toc[m_currentDataSetIndex].uncomp_size)
                        m_dataBufferArray[i].address = m_currentSize;
                    else
                    {
                        m_dataBufferArray[i].ID = 0x00;
                        m_dataBufferArray[i].address = 0;
                    }
                    break;
                }
            }
            if(i == DATA_BUFFER_SIZE)
            {
                for(i = 0; i < DATA_BUFFER_SIZE; i ++)
                {
                    if(m_dataBufferArray[i].ID == 0x00)
                    {
                        m_dataBufferArray[i].ID = _dataSet;
                        m_dataBufferArray[i].address = m_currentSize;
                        break;
                    }
                }
            }
            // check 16 bit check sum
            if(m_currentSize == m_toc[m_currentDataSetIndex].uncomp_size)
            {
                m_currentDataSetIndex = 0;
                m_requestNewData = 1;
                if( dataGetByte(&tempChar, 1, &m_CSU) != PROC_COMPLETE)
                    return PROC_FAIL;	// read upper check sum
                if( dataGetByte(&tempChar, 1, &m_CSU) != PROC_COMPLETE )
                    return PROC_FAIL;	// read lower check sum
                if( dataGetByte(&tempChar, 1, &m_CSU) != PROC_COMPLETE )
                    return PROC_FAIL;	// read 0xB9
                if( dataGetByte(&tempChar, 1, &m_CSU) != PROC_COMPLETE )
                    return PROC_FAIL;	// read 0xB2
            }
            return retVal;
        }
        else
        {
            m_requestNewData = 1;
            return HLDataGetByte(_dataSet, _dataByte, _uncomp_bitsize);
        }
    }

    return retVal;
}

int DataLattice::dataReadthroughComment()
{
    unsigned char currentByte = 0;
    int retVal                = 0;
    m_CSU.init_CS(16, 8);
    do
    {
        if(dataGetByte(&currentByte, 0, NULL ) != PROC_COMPLETE)
            break;
        retVal++;
    }
    while(currentByte != HENDCOMMENT);

    if(currentByte != HENDCOMMENT)
        return 0;

    return retVal;
}

int DataLattice::dataLoadTOC(short int _storeTOC)
{
    unsigned char currentByte = 0;
    int i                     = 0;
    int j                     = 0;
    int retVal                = 0;
    if(_storeTOC)
    {
        for(i = 0; i < D_TOC_NUMBER; i++)
        {
            m_toc[i].ID = 0;
            m_toc[i].uncomp_size = 0;
            m_toc[i].compression = 0;
            m_toc[i].address = 0x00000000;
        }
    }
    if(dataGetByte( &currentByte, 0, NULL ) != PROC_COMPLETE)
        return PROC_FAIL;
    retVal ++;
    if(_storeTOC)
        m_tocNumber = currentByte;
    for(i = 0; i < m_tocNumber; i++)
    {
        // read HTOC
        if((dataGetByte(&currentByte, 0, NULL) != PROC_COMPLETE) || currentByte != HTOC )
            return PROC_FAIL;
        retVal++;
        // read ID
        if(dataGetByte(&currentByte, 0, NULL) != PROC_COMPLETE)
            return PROC_FAIL;
        if(_storeTOC)
            m_toc[i].ID = currentByte;
        retVal++;
        // read status
        if(dataGetByte(&currentByte, 0, NULL ) != PROC_COMPLETE)
            return PROC_FAIL;
        retVal++;
        // read uncompressed data set size
        if(_storeTOC)
            m_toc[i].uncomp_size = 0;
        j = 0;
        do
        {
            if(dataGetByte(&currentByte, 0, NULL ) != PROC_COMPLETE)
                return PROC_FAIL;
            else{
                retVal++;
                if(_storeTOC)
                    m_toc[i].uncomp_size += (unsigned long)((currentByte & 0x7F) << (7 * j));
                j++;
            }
        }while(currentByte & 0x80);

        // read compression
        if(dataGetByte(&currentByte, 0, NULL) != PROC_COMPLETE)
            return PROC_FAIL;
        if(_storeTOC)
            m_toc[i].compression = currentByte;
        retVal++;
        // read address
        if(_storeTOC)
            m_toc[i].address = 0x00000000;
        for(j = 0; j < 4; j++)
        {
            if(dataGetByte(&currentByte, 0, NULL) != PROC_COMPLETE)
                return PROC_FAIL;
            retVal++;
            if(_storeTOC)
            {
                m_toc[i].address <<= 8;
                m_toc[i].address += currentByte;
            }
        }
    }
    return retVal;
}

int DataLattice::dataRequestSet(unsigned char _dataSet)
{
    int i                      = 0;
    unsigned char currentByte  = 0;
    for(i = 0; i < m_tocNumber; i++)
    {
        if(m_toc[i].ID == _dataSet)
        {
            m_currentDataSetIndex = i;
            break;
        }
    }
    if(i == m_tocNumber)
        return PROC_FAIL;

    // *************************************************************************
    // prepare data for reading
    // for streaming data, ignore data prior to the address
    // if the current address is bigger than requested address, reset the stream
    // *************************************************************************
    if(m_currentAddress > m_toc[m_currentDataSetIndex].address)
    {
        i = m_currentDataSetIndex;
        dataReset(0);
        m_currentDataSetIndex = i;
    }
    set_compression(m_toc[m_currentDataSetIndex].compression);
    // move currentAddress to requestAddress
    while(m_currentAddress < m_toc[m_currentDataSetIndex].address)
    {
        if(dataGetByte(&currentByte, 1, NULL ) != PROC_COMPLETE)
            return PROC_FAIL;
    }
    // read BEGIN_OF_DATA
    if(dataGetByte(&currentByte, 1, &m_CSU) != PROC_COMPLETE)
        return PROC_FAIL;
    if(dataGetByte(&currentByte, 1, &m_CSU) != PROC_COMPLETE)
        return PROC_FAIL;
    m_requestNewData = 0;
    return PROC_COMPLETE;
}


//-----------------  Decompression  --------------------

void DataLattice::set_compression(unsigned char _cmp)
{
    m_compression =  _cmp;
}

unsigned char DataLattice::get_compression() const
{
    return m_compression;
}

short int DataLattice::decomp_initFrame(int _bitSize)
{
    unsigned char compressMethod = 0;
    if(dataGetByte(&compressMethod, 1, &m_CSU) != PROC_COMPLETE)
    {
        return PROC_FAIL;
    }
    m_frameSize = (unsigned short int)(_bitSize / 8);
    if(_bitSize % 8 != 0)
        m_frameSize++;

    m_frameCounter = 0;

    switch(compressMethod)
    {
    case 0x00:
        m_currentCounter = -1;
        break;
    case 0x01:
        m_currentCounter = 0;
        m_compByte = 0xFF;
        break;
    case 0x02:
        m_currentCounter = 0;
        m_compByte = 0x00;
        break;
    default:
        return PROC_FAIL;
    }

    return PROC_COMPLETE;
}

short int DataLattice::decomp_getByte(unsigned char* _byteOut)
{
    if(m_frameCounter >= m_frameSize)
        return PROC_FAIL;
    switch(m_currentCounter)
    {
    case -1:
        if(dataGetByte(_byteOut, 1, &m_CSU) != PROC_COMPLETE)
            return PROC_FAIL;
        else
        {
            m_frameCounter++;
            return PROC_COMPLETE;
        }
        break;

    case 0:
        if(dataGetByte(_byteOut, 1, &m_CSU) != PROC_COMPLETE)
            return PROC_FAIL;
        if(*_byteOut == m_compByte)
        {
            if(decomp_getNum() != PROC_COMPLETE)
                return PROC_FAIL;

            m_currentCounter--;
            m_frameCounter++;
            return PROC_COMPLETE;
        }
        else{
            m_frameCounter++;
            return PROC_COMPLETE;
        }
        break;

    default:
        *_byteOut = m_compByte;
        m_currentCounter--;
        m_frameCounter++;
        return PROC_COMPLETE;
    }
}

short int DataLattice::decomp_getNum()
{
    unsigned char byteIn = 0x80;

    if(dataGetByte(&byteIn, 1, &m_CSU) != PROC_COMPLETE){
        return PROC_FAIL;
    }
    else{
        m_currentCounter = (short int)byteIn;
    }

    return PROC_COMPLETE;
}

