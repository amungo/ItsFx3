#ifndef _hex_parser_h_
#define _hex_parser_h_

#include <vector>

int parse_hex_file( const char* fname,
                    std::vector< unsigned int >& addreses,
                    std::vector< unsigned int >& hexdatas );


#endif

