#include "lfe5u_core.h"
#include "lfe5u_util.h"
#include "lfe5u_hardware.h"
#include "lfe5u_debug.h"
#include "lfe5u_opcode.h"
#include "../devioifce.h"

using namespace std;

// **************************************************************************
// Definition of System properties
//
// This section defines properties of the processing system.  This
// part need to be configured when generating algorithm byte stream.
//
// MAXBUF		- maximum buffer allowed.
// MAXTRANSBUF	- maximum transmission buffer allowed.
//	HOLDAF		- time (millisecond) hold after fail, must be positive
//				  0:		not continue, exit.
//				  Other:	milliseconds
// MAXSTACK	- maximum stack allowed, indicating maximum nested loop
//				  allowed in a loop / repeat.
// MAX_MASKSIZE - maximum mask size allowed in bytes.  4 or more is required.
//
// **************************************************************************

#define MAXBUF			200
#define MAXTRANSBUF		500
#define HOLDAF			0
#define MAXSTACK		3
#define MAX_MASKSIZE	32
#define MAX_DEBUGSTR	80
#define HEADERCRCSIZE	2


SSPICore::SSPICore(FX3DevIfce* _device_cmd_io) : m_currentChannel(0)
{
    m_pAlgoLattice = std::shared_ptr<AlgoLattice>(new AlgoLattice());
    m_pDataLattice = std::shared_ptr<DataLattice>(new DataLattice());
    m_pHardware = std::shared_ptr<LFE5U_Hardware>(new LFE5U_Hardware(_device_cmd_io, m_pDataLattice));

    m_version = {4, 0, 0};
    m_algoBuffer.resize(MAXBUF);
    m_stack.resize(MAXSTACK);
}

SSPICore::~SSPICore()
{

}

int SSPICore::SSPIEm_preset(const std::string& _algoFileName, const std::string& _dataFileName)
{
    int retVal = m_pAlgoLattice->algoPreset(_algoFileName.c_str());
    fprintf( stderr, "m_pAlgoLattice->algoPreset code %d\n", retVal );

    if(retVal == PROC_COMPLETE) {
        retVal = m_pDataLattice->dataPreset(_dataFileName.c_str());
        fprintf( stderr, "m_pAlgoLattice->dataPreset code %d\n", retVal );

    }

    return retVal;
}

int SSPICore::SSPIEm(unsigned int _algoID)
{
    int retVal = 0;
    retVal =  SSPIEm_init(_algoID);
    fprintf( stderr, "SSPIEm_init code %d\n", retVal );
    if(retVal != PROC_COMPLETE)
        return retVal;
    retVal = SSPIEm_process(0,0);
    fprintf( stderr, "SSPIEm_process code %d\n", retVal );

    return retVal;
}


// ********************************************************************
// Function SSPIEm_init()
// Start initialization
// ********************************************************************
int SSPICore::SSPIEm_init(unsigned int _algoID)
{
    unsigned char currentByte = 0;
    int i					  = 0;
    unsigned int mask         = 0;
    CSU headerCS;
    // initialize header check sum unit
    headerCS.init_CS(HEADERCRCSIZE * 8, 8);
    // initialize debug
    #ifdef	DEBUG_LEVEL_1
    dbgu_init();
    #endif
    // initialize SPI
    if(m_pHardware->SPI_init() != PROC_COMPLETE)
    {
        #ifdef	DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_ALGO_INIT, INIT_SPI_FAIL);
        #endif
        return ERROR_INIT_SPI;
    }
    #ifdef	DEBUG_LEVEL_2
    dbgu_putint(DBGU_L2_INIT, INIT_BEGIN);//"Initialization begin"
    #endif
    // initialize algorithm utility
    if(m_pAlgoLattice->algoInit() != PROC_COMPLETE)
    {
        #ifdef	DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_ALGO_INIT, INIT_ALGO_FAIL);
        #endif
        return ERROR_INIT_ALGO;
    }
    // discard comments, if available
    do{
        if(VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE)
        {
            #ifdef	DEBUG_LEVEL_1
            dbgu_putint(DBGU_L1_ALGO_INIT, NO_ALGOID);
            #endif
            return ERROR_INIT;
        }
        headerCS.putChunk((unsigned int)currentByte);
        if(currentByte == HCOMMENT)
        {
            if(proc_HCOMMENT(0, 0, 0, &headerCS) == PROC_FAIL){
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_INIT, COMMENT_END_UNEXPECTED);
                #endif
                return ERROR_INIT;
            }
        }
    }while(currentByte == HCOMMENT);
    // check ALGOID
    if(currentByte != ALGOID){
        #ifdef	DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_ALGO_INIT, NO_ALGOID);
        #endif
        return ERROR_INIT;
    }
    else
    {
        for(i=0; i<4; i++)
        {
            if(VME_getByte(&currentByte, 0, 0, 0) == PROC_COMPLETE)
            {
                headerCS.putChunk((unsigned int)currentByte);
                if (currentByte != (unsigned char)(_algoID >> ((3-i) * 8))){
                    if(_algoID != 0xFFFFFFFF){
                        #ifdef	DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_MISMATCH, NO_ALGOID);
                        #endif
                        return ERROR_INIT;
                    }
                }
            }
            // no algorithm ID byte available
            else
            {
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_INIT, NO_ALGOID);
                #endif
                return ERROR_INIT;
            }
        }
    }
    // check VERSION
    if((VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE) || (currentByte != VERSION))
    {
        #ifdef	DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_ALGO_INIT, NO_VERSION);
        #endif
        return ERROR_INIT;
    }
    else
    {
        headerCS.putChunk((unsigned int)currentByte);
        for(i=0; i<3; i++)
        {
            if(VME_getByte(&currentByte, 0, 0, 0) == PROC_COMPLETE)
            {
                headerCS.putChunk((unsigned int)currentByte);
                if(currentByte > m_version[i])
                {
                    #ifdef	DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_MISMATCH, NO_VERSION);
                    #endif
                    return ERROR_INIT_VERSION;
                }
            }
            // no version byte available
            else
            {
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_INIT, NO_VERSION);
                #endif
                return ERROR_INIT;
            }
        }
    }
    // check BUFFERREQ
    if((VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE) || currentByte != BUFFERREQ)
    {
        #ifdef	DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_BUFFERREQ);
        #endif
            return ERROR_INIT;
    }
    else
    {
        headerCS.putChunk((unsigned int)currentByte);
        if((VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE) || currentByte > MAXBUF)
        {
            #ifdef	DEBUG_LEVEL_1
            dbgu_putint(DBGU_L1_MISMATCH, NO_BUFFERREQ);
            #endif
            return ERROR_INIT;
        }
        else
            headerCS.putChunk((unsigned int)currentByte);
    }
    // check STACKREQ
    if((VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE) || currentByte != STACKREQ)
    {
        #ifdef	DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_ALGO_INIT, NO_STACKREQ);
        #endif
        return ERROR_INIT;
    }
    else
    {
        headerCS.putChunk((unsigned int)currentByte);
        // check STACKREQ
        if((VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE) || currentByte > MAXSTACK)
        {
            #ifdef	DEBUG_LEVEL_1
            dbgu_putint(DBGU_L1_MISMATCH, NO_STACKREQ);
            #endif
            return ERROR_INIT;
        }
        // no STACKREQ byte available
        else{
            headerCS.putChunk((unsigned int)currentByte);
        }
    }
    // check MASKBUFREQ
    if((VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE) || currentByte != MASKBUFREQ){
        #ifdef DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_ALGO_INIT, NO_MASKBUFREQ);
        #endif
        return ERROR_INIT;
    }
    else {
        headerCS.putChunk((unsigned int)currentByte);
        if((VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE) || currentByte > MAX_MASKSIZE)
        {
            #ifdef DEBUG_LEVEL_1
            dbgu_putint(DBGU_L1_MISMATCH, NO_MASKBUFREQ);
            #endif
            return ERROR_INIT;
        }
        else
            headerCS.putChunk((unsigned int)currentByte);
    }
    // store Channel
    if((VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE) || currentByte != HCHANNEL)
    {
        #ifdef	DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_ALGO_INIT, NO_CHANNEL);
        #endif
        return ERROR_INIT;
    }
    else
    {
        headerCS.putChunk((unsigned int)currentByte);
        if(VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE)
        {
            #ifdef	DEBUG_LEVEL_1
            dbgu_putint(DBGU_L1_ALGO_INIT, NO_CHANNEL);
            #endif
            return ERROR_INIT;
        }
        else{
            headerCS.putChunk((unsigned int)currentByte);
            m_currentChannel = currentByte;
        }
    }
    // check COMPRESSION
    if(m_pDataLattice->dataInit() != PROC_COMPLETE) {
        return ERROR_INIT_DATA;
    }
    if(VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE)
    {
        #ifdef	DEBUG_LEVEL_1
        dbgu_putint( DBGU_L1_ALGO_INIT, NO_COMPRESSION);
        #endif
            return ERROR_INIT;
    }
    else
    {
        headerCS.putChunk((unsigned int)currentByte);
        if( currentByte == COMPRESSION || currentByte == HCOMMENT )
        {
            if(VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE)
            {
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_INIT, NO_COMPRESSION);
                #endif
                return ERROR_INIT;
            }
            headerCS.putChunk(currentByte);
        }
        else{
            #ifdef	DEBUG_LEVEL_1
            dbgu_putint(DBGU_L1_ALGO_INIT, NO_COMPRESSION);
            #endif
            return ERROR_INIT;
        }
    }
    // check HEADERCS (done)
    if((VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE) || currentByte != HEADERCRC)
    {
        #ifdef DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_ALGO_INIT, NO_HEADERCS);
        #endif
        return ERROR_INIT;
    }
    else
    {
        for(i=0; i<HEADERCRCSIZE; i++)
        {
            if(VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE)
            {
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_INIT, NO_HEADERCS);
                #endif
                return ERROR_INIT;
            }
            mask = 0xFF << (8*(HEADERCRCSIZE - i - 1));
            if(	currentByte != ((headerCS.m_csValue & mask ) >> (8*(HEADERCRCSIZE - i - 1))) &&
                currentByte != 0xFF)
            {
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_MISMATCH, NO_HEADERCS);
                #endif
                return ERROR_INIT_CHECKSUM;
            }
        }
    }
    // get STARTOFALGO byte
    if((VME_getByte(&currentByte, 0, 0, 0) != PROC_COMPLETE) || currentByte != STARTOFALGO)
    {
        #ifdef	DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_ALGO_INIT, NO_STARTOFALGO);
        #endif
        return ERROR_INIT;
    }
    else
    {
        #ifdef	DEBUG_LEVEL_2
        dbgu_putint(DBGU_L2_INIT, INIT_COMPLETE);
        #endif
    }
    m_pHardware->m_uiCheckFailedRow = 0;
    m_pHardware->m_uiRowCount       = 0;

    return PROC_COMPLETE;
}

// **************************************************************************
// Function SSPI_process
// The main function of the processing engine.  During regular time,
// it automatically gets byte from external storage.  However, this
// function requires an array of buffered algorithm during
// loop / repeat operations. Input bufferedAlgo must be 0 to indicate regular operation.
//
// To call the VME, simply call SSPI_processVME(int debug, 0, 0, 0);
// Having input for this function is better suited for internal use.
// We highly recommend not to use any input in external environment.
//
// Input:
// debug				- 0 for normal mode, 1 for debug mode
// Internal Input:
// *bufferedAlgo		- array of algorithm during loop / repeat operation
// bufferedAlgoSize	- max size of buffer
// repeatVar			- special input for repeat only
//
// Output (procReturn value):
// 0	- Process fail
// 1	- Process complete
// 2	- Process successfully over
// *************************************************************************
int SSPICore::SSPIEm_process(unsigned char* _bufAlgo, unsigned int _bufAlgoSize)
{
    unsigned int	bufAlgoIndex = 0;
    short int		procReturn   = PROC_COMPLETE;
    unsigned char	currentByte  = 0;
    unsigned int	temp         = 0;
    #ifdef	DEBUG_LEVEL_2
    if(_bufAlgo == 0)
        dbgu_putint(DBGU_L2_PROC, START_PROC);
    else
        dbgu_putint(DBGU_L2_PROC, START_PROC_BUFFER);
    #endif
    while(procReturn == PROC_COMPLETE)
    {
        // ***********************************************************************
        //	Under STANDBY state, it allows opcode STARTTRAN, WAIT, LOOP, REPEAT
        //	If it is in LOOP or REPEAT, it also allows CONDITION
        // ***********************************************************************
        if(VME_getByte(&currentByte, _bufAlgo, _bufAlgoSize, &bufAlgoIndex) != PROC_COMPLETE)
        {
            if(_bufAlgo != 0)
            {
                #ifdef	DEBUG_LEVEL_2
                dbgu_putint(DBGU_L2_PROC, END_PROC_BUFFER);
                #endif
                return PROC_OVER;
            }
            else
            {
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_PROC, UNABLE_TO_GET_BYTE);
                #endif
                return ERROR_PROC_ALGO;
            }
        }
        switch(currentByte)
        {
        case HCOMMENT:
            if(proc_HCOMMENT(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0) == PROC_FAIL)
            {
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_PROC, COMMENT_END_UNEXPECTED);
                #endif
                return ERROR_PROC_ALGO;
            }
            break;
        case STARTTRAN:	// starts transmission
            #ifdef	DEBUG_LEVEL_2
            dbgu_putint(DBGU_L2_PROC, ENTER_STARTTRAN);
            #endif

            if(m_pHardware->TRANS_starttranx(getCurrentChannel()) == PROC_FAIL)
            {
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_TRANX_PROC, STARTTRAN_FAIL);
                #endif
                return ERROR_PROC_HARDWARE;
            }
            break;
        case TRANSIN:
        case TRANSOUT:
            //**********************************************************************
            // Under STARTTRAN, opcode TRANSOUT, TRANSIN are allowed.  Since the
            // SSPI Embedded system operates under Master SPI mode, it always does
            // TRANSOUT first.
            //**********************************************************************
            if(_bufAlgo == 0)
                procReturn = proc_TRANS(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, getCurrentChannel(),currentByte);
            else
                procReturn = proc_TRANS(&(_bufAlgo[bufAlgoIndex]), _bufAlgoSize, &bufAlgoIndex, getCurrentChannel(),currentByte);
            if(procReturn != PROC_COMPLETE){
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_PROCESS, TRANX_FAIL);//"Transmission fail",
                #endif
            }
            break;
        case RUNCLOCK:
            #ifdef	DEBUG_LEVEL_2
            dbgu_putint(DBGU_L2_PROC, ENTER_RUNCLOCK);
            #endif
            if(m_pHardware->TRANS_runClk() != PROC_COMPLETE)
            {
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_PROCESS, RUNCLOCK_FAIL);
                #endif
                procReturn = ERROR_PROC_HARDWARE;
            }
            break;
        case REPEAT:
            #ifdef	DEBUG_LEVEL_2
            dbgu_putint(DBGU_L2_PROC, ENTER_REPEAT);
            #endif

            // ***********************************************************************
            // REPEAT opcode is followed by the number of repeats.
            // Then it start processing the transmission by calling proc_REPEAT().
            // ***********************************************************************
            temp = VME_getNumber(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0);
            if(temp == 0){
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_PROC, NO_NUMBER_OF_REPEAT);
                #endif
                procReturn = ERROR_PROC_ALGO;
            }
            else{
                m_pHardware->m_uiCheckFailedRow = 1;
                m_pHardware->m_uiRowCount = 1;
                if(_bufAlgo == 0)
                    procReturn = proc_REPEAT(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, temp);
                else
                    procReturn = proc_REPEAT(&(_bufAlgo[bufAlgoIndex]), _bufAlgoSize, &bufAlgoIndex, temp);
                m_pHardware->m_uiCheckFailedRow = 0;
                if(procReturn != PROC_COMPLETE)
                {
                    #ifdef	DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_PROCESS, REPEAT_FAIL);
                    #endif
                }
            }
            break;
        case LOOP:
            #ifdef	DEBUG_LEVEL_2
            dbgu_putint(DBGU_L2_PROC, ENTER_LOOP);
            #endif

            // ************************************************************************
            // LOOP opcode is followed by the max number of looping, then it process
            // the transmission by calling proc_LOOP().
            // ************************************************************************

            temp = VME_getNumber(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0);
            if(temp == 0)
            {
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_PROC, NO_NUMBER_OF_LOOP);
                #endif
                procReturn = ERROR_PROC_ALGO;
            }
            else
            {
                if(_bufAlgo == 0){
                    procReturn = proc_LOOP(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, temp);
                }
                else {
                    procReturn = proc_LOOP(&(_bufAlgo[bufAlgoIndex]), _bufAlgoSize, &bufAlgoIndex, temp);
                }
                if(procReturn != PROC_COMPLETE){
                    #ifdef	DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_PROCESS, LOOP_FAIL);
                    #endif
                    procReturn = ERROR_LOOP_COND;
                }
            }
            break;
        case WAIT:		// process WAIT
            #ifdef	DEBUG_LEVEL_2
            dbgu_putint(DBGU_L2_PROC, ENTER_WAIT); "Enter WAIT",
            #endif

            // ***********************************************************************
            // WAIT opcode is followed by wait time in millisecond, then it process
            // the wait by calling proc_WAIT().
            // ***********************************************************************

            temp = VME_getNumber(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0);
            if(temp == 0){
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_PROC, NO_NUMBER_OF_WAIT);
                #endif
                procReturn = ERROR_PROC_ALGO;
            }
            else{
                procReturn = m_pHardware->Wait(temp);
            }
            break;
        case RESETDATA:
            if(m_pDataLattice->dataReset(1) != PROC_COMPLETE)
            {
                #ifdef	DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_PROCESS, RESETDATA_FAIL);
                #endif
                procReturn = ERROR_PROC_DATA;
            }
            break;
        case ENDTRAN:
            if(m_pHardware->TRANS_endtranx() != PROC_COMPLETE)
            {
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_TRANX_PROC, ENDTRAN_FAIL);
                #endif
                procReturn = ERROR_PROC_HARDWARE;
            }
            break;
        case ENDOFALGO:
            #ifdef	DEBUG_LEVEL_2
            if(bufAlgo != 0)
                dbgu_putint(DBGU_L2_PROC, END_PROC_BUFFER);
            else
                dbgu_putint(DBGU_L2_PROC, END_PROC);
            #endif
            procReturn = PROC_OVER;
            break;
        //case TRSTTOGGLE:
            //temp = VME_getNumber(bufAlgo, bufAlgoSize, &bufAlgoIndex, 0);
            //procReturn = TRANS_trsttoggle((unsigned char)temp);
            //break;
        default:
            #ifdef	DEBUG_LEVEL_1
            dbgu_putint(DBGU_L1_ALGO_PROC, UNRECOGNIZED_OPCODE);/*"Unrecognized opcode" */
            #endif
            return ERROR_PROC_ALGO;
            break;
        }
    }
    if(_bufAlgo == 0)
    {
        if(m_pAlgoLattice->algoFinal() != PROC_COMPLETE)
            procReturn = ERROR_PROC_ALGO;
        if(m_pDataLattice->dataFinal() != PROC_COMPLETE)
            procReturn = ERROR_PROC_DATA;
        if(m_pHardware->SPI_final() != PROC_COMPLETE)
            procReturn = ERROR_PROC_HARDWARE;
    }

    return procReturn;
}

// **************************************************************************
// VME_getByte
// Get a byte for algorithm
//
// Input:
// byteOut		- the address of output byte
// bufAlgo		- pointer to the buffered algorithm, if needed
// bufAlgoSize	- the size of buffered algorithm.
//				  this field is ignored if bufAlgo is 0
// bufAlgoIndex - the address of current index of the buffered algorithm
// 			  this field is ignored if bufAlgo is 0
// *************************************************************************
int SSPICore::VME_getByte(unsigned char* _byteOut, unsigned char* _bufAlgo,
                unsigned int _bufAlgoSize, unsigned int* _bufAlgoIndex)
{
    if(_bufAlgo == 0)
    {
        #ifdef DEBUG_LEVEL_3
        dbgu_putint(12,1);
        #endif
        m_pAlgoLattice->algoGetByte(_byteOut);
        return PROC_COMPLETE;
    }
    else
    {
        #ifdef DEBUG_LEVEL_3
        dbgu_putint(12,2);
        #endif
        if(*_bufAlgoIndex < _bufAlgoSize)
        {
            *_byteOut = _bufAlgo[(*_bufAlgoIndex)];
            (*_bufAlgoIndex)++;
            return PROC_COMPLETE;
        }
        else
            return PROC_FAIL;
    }
}

// *************************************************************************
// VME_getByte
// Get a number for algorithm
// *************************************************************************
unsigned int SSPICore::VME_getNumber(unsigned char* _bufAlgo, unsigned int _bufAlgoSize,
                  unsigned int* _bufAlgoIndex, unsigned int* _byteCount)
{
    unsigned char byteIn = 0x80;
    unsigned int output  = 0;
    short int i          = 0;
    do
    {
        if(VME_getByte(&byteIn, _bufAlgo, _bufAlgoSize, _bufAlgoIndex) != PROC_COMPLETE){
            return PROC_FAIL;
        }
        else{
            output += (unsigned long) ((byteIn & 0x7F) << (7 * i));
            i++;
        }
    }while(byteIn & 0x80);
    if(_byteCount)
        *_byteCount += i;

    return output;
}

// *************************************************************************
// Function proc_TRANS
// Start processing transmissions
//
// Input:
// bufAlgo			- the pointer to buffered algorithm if available
// bufAlgoSize		- the size of buffered algorithm
//					  this field is discarded if bufAlgo is 0
//	absBufAlgoIndex	- the pointer to the absolute Algorithm Index from
//					  the caller so proc_TRANS accumulates the absolute
//					  index while processing the transmission
//
// Return:
// PROC_FAIL		- Transmission fail or mismatch appears
// PROC_COMPLETE	- Transmission complete
// *************************************************************************

#define NO_DATA		0
#define BUFFER_TX	1
#define BUFFER_RX	2
#define DATA_TX		3
#define DATA_RX		4


int SSPICore::proc_TRANS(unsigned char* _bufAlgo, unsigned int _bufAlgoSize,
                         unsigned int* _absbufAlgoIndex, unsigned char _channel, unsigned char _currentByte)
{
    unsigned char trBuffer[MAXTRANSBUF];
    unsigned char maskBuffer[MAX_MASKSIZE / 8];
    int trCount = 0;
    unsigned int bufAlgoIndex = 0;
    short int flag_mask = 0;
    short int flag_transin = 0;
    int byteNum;
    short int retVal = 0;
    unsigned int mismatch = 0;
    int temp;
    int i;

    while(retVal != PROC_OVER)
    {
        switch(_currentByte)
        {
            case HCOMMENT:
                if(proc_HCOMMENT(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0) == PROC_FAIL)
                {
                    #ifdef	DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_ALGO_PROC, COMMENT_END_UNEXPECTED);
                    #endif
                    return ERROR_PROC_ALGO;
                }
                break;
            case WAIT:
                //************************************************************************
                //* WAIT opcode is followed by wait time in millisecond, then it process
                //* the wait by calling proc_WAIT().
                //************************************************************************

                temp = VME_getNumber(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0);
                if(temp == 0)
                {
                    #ifdef	DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_ALGO_PROC, NO_NUMBER_OF_WAIT);//"No millisecond after WAIT",
                    #endif
                    return ERROR_PROC_ALGO;
                }
                else{
                    m_pHardware->Wait(temp);
                }
                break;
            // since the proc system is Master SPI, it always transmit data out first
            case TRANSOUT:
                #ifdef DEBUG_LEVEL_2
                dbgu_putint(8,2);//"Enter TRANSOUT",
                #endif
                // get transmit size in bits, whether the data is compressed or not
                trCount = VME_getNumber(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0);
                if(trCount == 0){
                    #ifdef DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_ALGO_TRANX, NO_TRANSOUT_SIZE);//"No byte available at Size",
                    #endif
                    return ERROR_PROC_ALGO;		// no byte available at Size
                }
                byteNum = trCount / 8;
                if(trCount % 8 != 0)
                    byteNum ++;
                // check if the next Byte is DATA or DATAM
                if(VME_getByte(&_currentByte, _bufAlgo, _bufAlgoSize, &bufAlgoIndex) != PROC_COMPLETE){
                        #ifdef DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_ALGO_TRANX,NO_TRANSOUT_TYPE);//"Algo error: no byte available at DATA type",
                        #endif
                    return ERROR_PROC_ALGO;		// no byte available at DATA type
                }
                if(_currentByte == ALGODATA)
                {
                    // buffer transmit bytes
                    for(i=0; i< byteNum; i++)
                    {
                        if(VME_getByte(&(trBuffer[i]), _bufAlgo, _bufAlgoSize, &bufAlgoIndex) != PROC_COMPLETE){
                            #ifdef DEBUG_LEVEL_1
                            dbgu_putint(DBGU_L1_ALGO_TRANX, NO_TRANSOUT_DATA);//"Algo error: no byte available for transmit",
                            #endif
                            return ERROR_PROC_ALGO;
                        }
                    }
                    retVal = m_pHardware->TRANS_transceive_stream(trCount, trBuffer, 0, NO_DATA, 0, flag_mask, maskBuffer);
                    if((retVal != PROC_COMPLETE) && retVal != ERROR_VERIFICATION )
                    {
                        #ifdef DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_TRANX_PROC, TRANX_OPCODE_FAIL);
                        #endif
                        return retVal;
                    }
                }
                else if(_currentByte == PROGDATAEH)
                {
                    retVal = m_pHardware->TRANS_transceive_stream(0, trBuffer, trCount, DATA_TX, &_currentByte, flag_mask, maskBuffer);
                    if(retVal != PROC_COMPLETE){
                        #ifdef DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_TRANX_PROC, TRANX_OUT_PROG_FAIL);
                        #endif
                        return retVal;
                    }

                }
                else
                {
                    #ifdef DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_ALGO_TRANX,NO_TRANSOUT_TYPE);//"Algo error: no byte available at DATA type",
                    #endif
                    return ERROR_PROC_ALGO;		// no byte available at DATA type
                }
                flag_transin = 0;
                break;
            case ALGODATA:
                if(!flag_transin)
                {
                    retVal = m_pHardware->TRANS_transceive_stream(0, 0, trCount, BUFFER_TX, trBuffer, flag_mask, maskBuffer);
                    if(retVal != PROC_COMPLETE){
                        #ifdef DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_TRANX_PROC, TRANX_OUT_ALGO_FAIL);
                        #endif
                        return retVal;
                    }
                }
                else
                {
                    retVal = m_pHardware->TRANS_transceive_stream(0, 0, trCount, BUFFER_RX, trBuffer, flag_mask, maskBuffer);
                    if(retVal != PROC_COMPLETE && retVal != ERROR_VERIFICATION){
                        #ifdef DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_TRANX_PROC, TRANX_IN_ALGO_FAIL);//"Transmit error: unable to transmit",
                        #endif
                        return retVal;
                    }

                    for(i=0; i< byteNum; i++)
                    {
                        if(VME_getByte(&_currentByte, _bufAlgo, _bufAlgoSize, &bufAlgoIndex) != PROC_COMPLETE)
                        {
                            #ifdef DEBUG_LEVEL_1
                            dbgu_putint(DBGU_L1_ALGO_TRANX, NO_TRANSIN_DATA);//"Algo error: no byte available for transmit",
                            #endif
                            return ERROR_PROC_ALGO;
                        }

                        if(flag_mask)
                        {
                            trBuffer[i] = trBuffer[i] & maskBuffer[i];
                            _currentByte  = _currentByte & maskBuffer[i];
                        }
                        else if(i == byteNum - 1 && trCount % 8 != 0)
                            trBuffer[i] = trBuffer[i] & (~((unsigned char) (0xFF >> (trCount % 8))));

                        if(trBuffer[i] != _currentByte)
                            mismatch ++;
                    }
                }
                break;
            case PROGDATA:
                if(!flag_transin)
                {
                    retVal = m_pHardware->TRANS_transceive_stream(0, trBuffer, trCount, DATA_TX, 0, flag_mask, maskBuffer);
                    if(retVal != PROC_COMPLETE)
                    {
                        #ifdef DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_TRANX_PROC, TRANX_OUT_PROG_FAIL);
                        #endif
                        return retVal;
                    }
                }
                else
                {
                    retVal = m_pHardware->TRANS_transceive_stream(0, trBuffer, trCount, DATA_RX, 0, flag_mask, maskBuffer);
                    if((retVal != PROC_COMPLETE) && retVal != ERROR_VERIFICATION)
                    {
                        #ifdef DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_TRANX_PROC, TRANX_IN_PROG_FAIL);//"Transmit error: unable to transmit",
                        #endif
                        return retVal;
                    }
                }
                break;
            case PROGDATAEH:
                if(!flag_transin)
                {
                    retVal = m_pHardware->TRANS_transceive_stream(0, trBuffer, trCount, DATA_TX, &_currentByte, flag_mask, maskBuffer);
                    if(retVal != PROC_COMPLETE)
                    {
                        #ifdef DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_TRANX_PROC, TRANX_OUT_PROG_FAIL);
                        #endif
                        return retVal;
                    }
                }
                else
                {
                    retVal = m_pHardware->TRANS_transceive_stream(0, trBuffer, trCount, DATA_RX, &_currentByte, flag_mask, maskBuffer);
                    if((retVal != PROC_COMPLETE) && retVal != ERROR_VERIFICATION)
                    {
                        #ifdef DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_TRANX_PROC, TRANX_IN_PROG_FAIL);//"Transmit error: unable to transmit",
                        #endif
                        return retVal;
                    }
                    else if((retVal != PROC_COMPLETE) && retVal == ERROR_VERIFICATION)
                    {
                        #ifdef DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_TRANX_PROC, TRANX_IN_PROG_FAIL);//"Transmit error: unable to transmit",
                        #endif
                        mismatch = 1;
                    }
                }
                break;
            case TRANSIN:
                trCount = VME_getNumber(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0);
                if(trCount == 0){
                    #ifdef DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_ALGO_TRANX, NO_TRANSIN_SIZE);//"No byte available at Size",
                    #endif
                    return ERROR_PROC_ALGO;		// no byte available at Size
                }

                byteNum = trCount / 8;
                if(trCount % 8 != 0)
                    byteNum ++;
                flag_transin = 1;
                break;
            case MASK:
                if(trCount <= MAX_MASKSIZE){
                    for(i = 0; i< byteNum; i++)
                    {
                        if(VME_getByte(&(maskBuffer[i]), _bufAlgo, _bufAlgoSize, &bufAlgoIndex) != PROC_COMPLETE){
                            #ifdef DEBUG_LEVEL_1
                            dbgu_putint(DBGU_L1_ALGO_TRANX, NO_TRANSIN_MASK);//"No byte available at Mask",
                            #endif
                            return ERROR_PROC_ALGO;		// no byte available at Mask
                        }
                    }
                    flag_mask = 1;
                }
                break;
            case ENDTRAN:
                if(m_pHardware->TRANS_endtranx() != PROC_COMPLETE){
                    #ifdef DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_TRANX_PROC, ENDTRAN_FAIL);
                    #endif
                    return ERROR_PROC_HARDWARE;
                }
                if(_bufAlgo != 0)
                    (*_absbufAlgoIndex) += bufAlgoIndex;
                if(mismatch){
                    return ERROR_VERIFICATION;
                }
                else
                    return PROC_COMPLETE;
                break;
            case REPEAT:
                // starts repeat. Buffer whatever in the repeat loop.
                //************************************************************************
                //* REPEAT opcode is followed by the number of repeats.
                //* Then it start processing the transmission by calling proc_REPEAT().
                //************************************************************************
                temp = VME_getNumber(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0);
                if(temp == 0){
                    #ifdef	DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_ALGO_PROC, NO_NUMBER_OF_REPEAT);
                    #endif
                    return ERROR_PROC_ALGO;
                }
                else{
                    if(_bufAlgo == 0)
                        retVal = proc_REPEAT(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, temp);
                    else
                        retVal = proc_REPEAT(&(_bufAlgo[bufAlgoIndex]), _bufAlgoSize, &bufAlgoIndex, temp);
                    if(retVal != PROC_COMPLETE){
                        #ifdef	DEBUG_LEVEL_1
                        dbgu_putint(DBGU_L1_PROCESS, REPEAT_FAIL);
                        #endif
                        return ERROR_PROC_ALGO;
                    }
                }
                break;
            case RESETDATA:
                if(m_pDataLattice->dataReset(1) != PROC_COMPLETE) {
                    #ifdef	DEBUG_LEVEL_1
                    dbgu_putint(DBGU_L1_PROCESS, RESETDATA_FAIL);// fail to reset data
                    #endif
                    retVal = ERROR_PROC_DATA;
                }
                break;
            default:
                if(_bufAlgo != 0)
                {
                    if(_bufAlgo != 0)
                        (*_absbufAlgoIndex) += bufAlgoIndex;
                    if(mismatch){
                        return ERROR_VERIFICATION;
                    }
                    else
                        return PROC_COMPLETE;
                }
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_TRANX, UNRECOGNIZED_OPCODE);
                #endif
                return ERROR_PROC_ALGO;
                break;
        }
        if(VME_getByte(&_currentByte, _bufAlgo, _bufAlgoSize, &bufAlgoIndex) != PROC_COMPLETE)
        {
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_ALGO_TRANX, NO_TRANX_OPCODE);
                #endif
                return ERROR_PROC_ALGO;
        }
    }

    if(_bufAlgo != 0)
        (*_absbufAlgoIndex) += bufAlgoIndex;

    if(retVal != PROC_COMPLETE)
        return retVal;

    if(mismatch)
    {
        #ifdef DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_TRANX_PROC, COMPARE_FAIL);
        // internal debugging
        dbgu_putint(8,mismatch);
        #ifdef DEBUG_LEVEL_3
        for (i=0; i<4; i++){
            dbgu_putint(2,(unsigned int)(trBuffer[i] >> 4));//"Mismatch occurs",
            dbgu_putint(2,(unsigned int)(trBuffer[i] & 0x0f));//"Mismatch occurs",
        }
        #endif
        #endif
        return ERROR_VERIFICATION;
    }
    else
        return PROC_COMPLETE;
}

// *******************************************************************************
// Function proc_REPEAT
// Process Repeat block
//
// Input:
// bufAlgo		- the pointer to buffered algorithm if available
// bufAlgoSize	- the size of buffered algorithm
//				  this field is discarded if bufAlgo is 0
//	absBufAlgoIndex	- the pointer to the absolute Algorithm Index from the caller
//					  so proc_REPEAT accumulates the absolute index while
//					  processing the transmission
// LoopMax		- Max number of repeat
//
// Return:
// PROC_FAIL		- loop condition not met
// PROC_COMPLETE	- loop condition met
// ******************************************************************************
int SSPICore::proc_REPEAT(unsigned char* _bufAlgo, unsigned int _bufAlgoSize,
                          unsigned int* _absBufAlgoIndex, unsigned int _LoopMax)
{
    unsigned char*		bufferPtr   = 0;	// loop / repeat buffer
    unsigned int		bufferSize   = 0;	// size of algorithm within loop / repeat
    unsigned int		loopCount    = 0;
    unsigned int		stackIndex   = 0;
    unsigned char		currentByte  = 0;
    unsigned int		bufAlgoIndex = 0;
    int					flag         = 1;	// initialize to 1 for counting / buffering algo
                                            // when processing loop, flag becomes the condition
                                            // whether the loop succeed or fail.


    int i = 0;
    // initialize bufferPtr.  If processed is not buffered,
    // it points to the beginning of buffer.  Else it points to where bufAlgo points to
    if(_bufAlgo == 0)
    {
        for(i = 0; i < MAXBUF; i++)
        {
            m_algoBuffer[i] = 0;
        }
        bufferPtr = m_algoBuffer.data();	// points to the global buffer
    }
    else
        bufferPtr = _bufAlgo;
    #ifdef DEBUG_LEVEL_2
    dbgu_putint(DBGU_L2_REPEAT, PREPARE_BUFFER);
    #endif
    do{
        if(VME_getByte(&currentByte, _bufAlgo, _bufAlgoSize, &bufAlgoIndex) != PROC_COMPLETE){
            #ifdef DEBUG_LEVEL_1
            dbgu_putint(DBGU_L1_REPEAT, BUFFER_FAIL);
            #endif
            return ERROR_PROC_ALGO;
        }
        if(currentByte == HCOMMENT)
        {
            if(proc_HCOMMENT(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0) == PROC_FAIL){
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_REPEAT, REPEAT_COMMENT_FAIL);
                #endif
                return ERROR_PROC_ALGO;
            }
        }
        // nested LOOP / REPEAT check
        // if currentByte is LOOP or REPEAT, nested loop, put in stack
        else if(currentByte == LOOP || currentByte == REPEAT)
        {
            m_stack[stackIndex] = currentByte;
            stackIndex++;
        }
        // if currentByte is ENDREPEAT, pop from stack
        else if(currentByte == ENDREPEAT)
        {
            if(stackIndex == 0)
                flag = 0;		// end of loop
            else if(m_stack[stackIndex-1] != REPEAT)
            {
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_REPEAT, STACK_MISMATCH); // Stack mismatch when buffering REPEAT
                #endif
                return ERROR_PROC_ALGO;
            }
            else
                stackIndex--;
        }
        // if currentByte is ENDLOOP, check if its end of loop,
        //   or pop from stack if its end of nested loop
        else if(currentByte == ENDLOOP)
        {
            if(stackIndex == 0 || m_stack[stackIndex-1] != LOOP){
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_REPEAT, STACK_MISMATCH);
                #endif
                return ERROR_PROC_ALGO;
            }
            else
                stackIndex--;
        }
        // discard comment for buffering
        if(flag == 1 && currentByte != HCOMMENT)
        {
            if(bufferSize > MAXBUF)
            {
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_REPEAT , REPEAT_SIZE_EXCEED);
                #endif
                return ERROR_PROC_ALGO;
            }
            else{
                if(_bufAlgo == 0)
                    m_algoBuffer[bufferSize] = currentByte;
                bufferSize++;
            }
        }
    }while(flag == 1);

    #ifdef DEBUG_LEVEL_2
    dbgu_putint(DBGU_L2_REPEAT, FINISH_BUFFER); // End calculate REPEAT size, and buffer REPEAT
    #endif
    bufAlgoIndex = 0;
    // process REPEAT
    flag = 0;
    loopCount = 0;
    #ifdef DEBUG_LEVEL_2
    dbgu_putint(DBGU_L2_REPEAT, START_PROC_REPEAT);
    #endif
    do{
        flag = SSPIEm_process(bufferPtr, bufferSize);
        loopCount ++;
    }while(flag == PROC_OVER && loopCount < _LoopMax);

    if(flag != PROC_COMPLETE){
        #ifdef DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_REPEAT, REPEAT_COND_FAIL); // REPEAT condition fails
        #endif
        (*_absBufAlgoIndex) += bufferSize + 1;
        return flag;
    }
    else{
        #ifdef DEBUG_LEVEL_2
        dbgu_putint(DBGU_L2_REPEAT, END_PROC_REPEAT); // End processing REPEAT
        #endif
        (*_absBufAlgoIndex) += bufferSize + 1;
        return PROC_COMPLETE;
    }
}

// **************************************************************************
// Function proc_LOOP
// Process Loop block
//
// The function will see if the processes is being buffered.  If the
// processes is not buffered, it buffers the processes.  Else it will
// calculate the maximum of the buffer between LOOP - ENDLOOP block.
//
// Note that the format of the loop requires the condition check to be
// the end of the loop block.  Once the last process succeed, the
// loop is completed.
//
// The loop will break if all the processes within the loop succeed.
// It will continue if any step in the loop fails.  Therefore it is
// better to put the condition process, such as TRANS with TRANSIN, as
// the last process in a loop so it will go through all the processes
// before deciding whether the loop will continue or break.
//
// Input:
// bufAlgo			- the pointer to buffered algorithm if available
// bufAlgoSize		- the size of buffered algorithm
//					  this field is discarded if bufAlgo is 0
//	absBufAlgoIndex	- the pointer to the absolute Algorithm Index from the caller
//					  so proc_LOOP accumulates the absolute index while
//					  processing the transmission
// LoopMax			- Max number of loop allowed
//
// Return:
// PROC_FAIL		- loop condition not met
// PROC_COMPLETE	- loop condition met
// *****************************************************************************
int SSPICore::proc_LOOP(unsigned char* _bufAlgo, unsigned int _bufAlgoSize, unsigned int* _absBufAlgoIndex, unsigned int _LoopMax)
{
    unsigned char*		bufferPtr      = 0;	// loop buffer
    unsigned int		bufferSize     = 0;	// size of algo within loop
    unsigned int		loopCount      = 0;
    unsigned int		stackIndex     = 0;
    unsigned char		currentByte    = 0;
    unsigned int		bufAlgoIndex   = 0;
    int		            flag           = 1; // initialize to 1 for counting / buffering algo
                                            // when processing loop, flag becomes the condition
                                            // whether the loop succeed or fail.

    int i = 0;
    // initialize bufferPtr.  If processed is not buffered,
    // it points to the beginning of buffer.  Else it points to where bufAlgo points to
    if(_bufAlgo == 0)
    {
        for(i = 0; i < MAXBUF; i++)
        {
            m_algoBuffer[i] = 0;
        }
        bufferPtr = m_algoBuffer.data();	// points to the global buffer
    }
    else
        bufferPtr = _bufAlgo;
    #ifdef DEBUG_LEVEL_2
    dbgu_putint(DBGU_L2_LOOP, PREPARE_BUFFER);
    #endif
    do
    {
        if(VME_getByte(&currentByte, _bufAlgo, _bufAlgoSize, &bufAlgoIndex) != PROC_COMPLETE){
            #ifdef DEBUG_LEVEL_1
            dbgu_putint(DBGU_L1_LOOP, BUFFER_FAIL);
            #endif
            return ERROR_PROC_ALGO;
        }
        if(currentByte == HCOMMENT)
        {
            if(proc_HCOMMENT(_bufAlgo, _bufAlgoSize, &bufAlgoIndex, 0) == PROC_FAIL){
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_REPEAT, LOOP_COMMENT_FAIL);
                #endif
                return ERROR_PROC_ALGO;
            }
        }
        else if(currentByte == LOOP || currentByte == REPEAT){
            m_stack[stackIndex] = currentByte;
            stackIndex++;
        }
        else if(currentByte == ENDREPEAT)
        {
            if(stackIndex == 0 || m_stack[stackIndex-1] != REPEAT){
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_LOOP, STACK_MISMATCH);
                #endif
                return ERROR_PROC_ALGO;
            }
            else
                stackIndex--;
        }
        // if currentByte is ENDLOOP, check if its end of loop,
        //   or pop from stack if its end of nested loop
        else if(currentByte == ENDLOOP)
        {
            if(stackIndex == 0)
                flag = 0;
            else if(m_stack[stackIndex-1] != LOOP){
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_LOOP, STACK_MISMATCH);
                #endif
                return ERROR_PROC_ALGO;
            }
            else
                stackIndex--;
        }
        // discard comment for buffering
        if(flag == 1 && currentByte != HCOMMENT)
        {
            if(bufferSize > MAXBUF)
            {
                #ifdef DEBUG_LEVEL_1
                dbgu_putint(DBGU_L1_LOOP, LOOP_SIZE_EXCEED); // Loop size exceed Maximum Buffer size
                #endif
                return ERROR_PROC_ALGO;
            }
            else
            {
                if(_bufAlgo == 0)
                    m_algoBuffer[bufferSize] = currentByte;
                bufferSize++;
            }
        }
    }while(flag == 1);
    #ifdef DEBUG_LEVEL_2
    dbgu_putint(DBGU_L2_LOOP, FINISH_BUFFER); // End calculate LOOP size, and buffer LOOP
    #endif
    bufAlgoIndex = 0;
    // process loop
    flag = 0;
    loopCount = 0;
    #ifdef DEBUG_LEVEL_2
    dbgu_putint(DBGU_L2_LOOP, START_PROC_LOOP);
    #endif
    do{
        flag = SSPIEm_process(bufferPtr, bufferSize);
        loopCount ++;
    }while(flag != PROC_COMPLETE && loopCount < _LoopMax);

    if(flag != PROC_COMPLETE)
    {
        #ifdef DEBUG_LEVEL_1
        dbgu_putint(DBGU_L1_LOOP, LOOP_COND_FAIL); // LOOP condition not met
        #endif
        (*_absBufAlgoIndex) += bufferSize + 1;
        return flag;
    }
    else
    {
        #ifdef DEBUG_LEVEL_2
        dbgu_putint(DBGU_L2_LOOP, END_PROC_LOOP); // End processing LOOP
        #endif
        (*_absBufAlgoIndex) += bufferSize + 1;
        return PROC_COMPLETE;
    }
}

int SSPICore::proc_HCOMMENT(unsigned char* _bufferedAlgo, unsigned int _bufferedAlgoSize,
               unsigned int* _absBufferedAlgoIndex, CSU* _headerCS)
{
    unsigned char currentByte  = 0;
    do{
        if(VME_getByte(&currentByte, _bufferedAlgo, _bufferedAlgoSize, _absBufferedAlgoIndex) != PROC_COMPLETE){
            return PROC_FAIL;
        }
        if(_headerCS != 0)
            _headerCS->putChunk((unsigned int) currentByte); //putChunk(headerCS, (unsigned int) currentByte);
    }
    while(currentByte != HENDCOMMENT);

    return PROC_COMPLETE;
}

