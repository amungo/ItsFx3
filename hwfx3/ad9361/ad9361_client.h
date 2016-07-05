#ifndef INCLUDED_AD9361_CLIENT_H
#define INCLUDED_AD9361_CLIENT_H

#include <memory>
#include <cstdint>

/*!
 * Frequency band settings
 */
typedef enum {
    AD9361_RX_BAND0,
    AD9361_RX_BAND1,
    AD9361_TX_BAND0
} frequency_band_t;

/*!
 * Clocking mode
 */
typedef enum {
    AD9361_XTAL_P_CLK_PATH,
    AD9361_XTAL_N_CLK_PATH
} clocking_mode_t;

/*!
 * Digital interface specific
 */
typedef enum {
    AD9361_DDR_FDD_LVCMOS,
    AD9361_DDR_FDD_LVDS
} digital_interface_mode_t;

/*!
 * Interface timing
 */
typedef struct {
    uint8_t rx_clk_delay;
    uint8_t rx_data_delay;
    uint8_t tx_clk_delay;
    uint8_t tx_data_delay;
} digital_interface_delays_t;

class ad9361_params {
public:
    virtual ~ad9361_params() {}

    virtual digital_interface_delays_t get_digital_interface_timing() {
        digital_interface_delays_t delays;
        delays.rx_clk_delay  = 0;
        delays.rx_data_delay = 0;
        delays.tx_clk_delay  = 0;
        delays.tx_data_delay = 0;
        return delays;
    }

    virtual digital_interface_mode_t get_digital_interface_mode() {
        return AD9361_DDR_FDD_LVCMOS;
    }

    virtual clocking_mode_t get_clocking_mode() {
        return AD9361_XTAL_P_CLK_PATH;
    }

    virtual double get_band_edge(frequency_band_t band) {
        switch (band) {
            case AD9361_RX_BAND0:   return 4.0e9;
            case AD9361_RX_BAND1:   return 4.0e9;
            case AD9361_TX_BAND0:   return 2.5e9;
            default:                return 0;
        }
    }

};


#endif /* INCLUDED_AD9361_CLIENT_H */
