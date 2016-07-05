#include "HexParser.h"

#include <fstream>
#include <string>

int parse_hex_file(const char* fname, std::vector<unsigned int>& addresses, std::vector<unsigned int>& hexdatas) {
    addresses.clear();
    hexdatas.clear();
    std::ifstream file;
    file.open( fname );
    while(!file.eof()){
        unsigned int addr;
        unsigned int data;
        std::string str;
        
        file >> str;
        //fprintf( stderr, "%s\n", str.c_str() );
        if ( str[0]==';' ) {
            continue;
        }
        sscanf( str.c_str(), "Reg%d", &addr );
        
        file >> str;
        //fprintf( stderr, "%s\n", str.c_str() );
        sscanf( str.c_str(), "%02X", &data );
        
        //fprintf( stderr, "Reg[%u] 0x%02X\n", addr, data );
        addresses.push_back(addr);
        hexdatas.push_back(data);
    }
    return 0;
}
