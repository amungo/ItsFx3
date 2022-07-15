#ifndef DEVIOIFCE_H
#define DEVIOIFCE_H

#include <cstdint>

class DeviceControlIOIfce {
public:
    virtual uint8_t peek8( uint32_t register_address24 ) = 0;
    virtual void poke8( uint32_t register_address24, uint8_t value ) = 0;
};

#endif // DEVIOIFCE_H
