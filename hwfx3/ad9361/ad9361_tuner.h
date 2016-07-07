#ifndef AD9361TUNER_H
#define AD9361TUNER_H

#include <vector>
#include <map>
#include <mutex>
#include <functional>
#include <hwfx3/devioifce.h>

#include "ad9361_client.h"
#include "types/filters.h"

class ad9361_device_t {
public:
    ad9361_device_t( DeviceControlIOIfce* device_cmd_io, ad9361_params* params  );
    ~ad9361_device_t();
    enum direction_t { RX, TX };
    enum gain_mode_t {GAIN_MODE_MANUAL, GAIN_MODE_SLOW_AGC, GAIN_MODE_FAST_AGC};
    enum chain_t { CHAIN_1, CHAIN_2, CHAIN_BOTH };

    /* Initialize the AD9361 codec. */
    void initialize();

    /* This function sets the RX / TX rate between AD9361 and the FPGA, and
     * thus determines the interpolation / decimation required in the FPGA to
     * achieve the user's requested rate.
     */
    double set_clock_rate(const double req_rate);

    /* Set which of the four TX / RX chains provided by AD9361 are active.
     *
     * AD9361 provides two sets of chains, Side A and Side B. Each side
     * provides one TX antenna, and one RX antenna. The B200 maintains the USRP
     * standard of providing one antenna connection that is both TX & RX, and
     * one that is RX-only - for each chain. Thus, the possible antenna and
     * chain selections are:
     *
     */
    void set_active_chains(bool tx1, bool tx2, bool rx1, bool rx2);

    /* Tune the RX or TX frequency.
     *
     * This is the publicly-accessible tune function. It makes sure the tune
     * isn't a redundant request, and if not, passes it on to the class's
     * internal tune function.
     *
     * After tuning, it runs any appropriate calibrations. */
    double tune(direction_t direction, const double value);

    /* Get the current RX or TX frequency. */
    double get_freq(direction_t direction);

    /* Set the gain of RX1, RX2, TX1, or TX2.
     *
     * Note that the 'value' passed to this function is the actual gain value,
     * _not_ the gain index. This is the opposite of the eval software's GUI!
     * Also note that the RX chains are done in terms of gain, and the TX chains
     * are done in terms of attenuation. */
    double set_gain(direction_t direction, chain_t chain, const double value);

    /* Make AD9361 output its test tone. */
    void output_test_tone();

    void digital_test_tone(bool enb); // Digital output

    /* Turn on/off AD9361's TX port --> RX port loopback. */
    void data_port_loopback(const bool loopback_enabled);

    /* Read back the internal RSSI measurement data. */
    double get_rssi(chain_t chain);

    /*! Read the internal temperature sensor
     *\param calibrate return raw sensor readings or apply calibration factor.
     *\param num_samples number of measurements to average over
     */
    double get_average_temperature(const double cal_offset = -30.0, const size_t num_samples = 3);

    /* Turn on/off AD9361's RX DC offset correction */
    void set_dc_offset_auto(direction_t direction, const bool on);

    /* Turn on/off AD9361's RX IQ imbalance correction */
    void set_iq_balance_auto(direction_t direction, const bool on);

    /* Configure AD9361's AGC module to use either fast or slow AGC mode. */
    void set_agc_mode(chain_t chain, gain_mode_t gain_mode);

    /* Enable AD9361's AGC gain mode. */
    void set_agc(chain_t chain, bool enable);

    /* Set bandwidth of AD9361's analog LP filters.
     * Bandwidth should be RF bandwidth */
    double set_bw_filter(direction_t direction, const double rf_bw);

    /*
     * Filter API implementation
     * */
    filter_info_base::sptr get_filter(direction_t direction, chain_t chain, const std::string &name);

    void set_filter(direction_t direction, chain_t chain, const std::string &name, filter_info_base::sptr filter);

    std::vector<std::string> get_filter_names(direction_t direction);

    //Constants
    static const double AD9361_MAX_GAIN;
    static const double AD9361_MAX_CLOCK_RATE;
    static const double AD9361_MIN_CLOCK_RATE;
    static const double AD9361_CAL_VALID_WINDOW;
    static const double AD9361_RECOMMENDED_MAX_BANDWIDTH;
    static const double DEFAULT_RX_FREQ;
    static const double DEFAULT_TX_FREQ;

private:    //Methods
    void _program_fir_filter(direction_t direction, int num_taps, uint16_t *coeffs);
    void _setup_tx_fir(size_t num_taps, int32_t interpolation);
    void _setup_rx_fir(size_t num_taps, int32_t decimation);
    void _program_fir_filter(direction_t direction, chain_t chain, int num_taps, uint16_t *coeffs);
    void _setup_tx_fir(size_t num_taps);
    void _setup_rx_fir(size_t num_taps);
    void _calibrate_lock_bbpll();
    void _calibrate_synth_charge_pumps();
    double _calibrate_baseband_rx_analog_filter(double rfbw);
    double _calibrate_baseband_tx_analog_filter(double rfbw);
    double _calibrate_secondary_tx_filter(double rfbw);
    double _calibrate_rx_TIAs(double rfbw);
    void _setup_adc();
    void _calibrate_baseband_dc_offset();
    void _calibrate_rf_dc_offset();
    void _calibrate_rx_quadrature();
    void _tx_quadrature_cal_routine();
    void _calibrate_tx_quadrature();
    void _program_mixer_gm_subtable();
    void _program_gain_table();
    void _setup_gain_control(bool use_agc);
    void _setup_synth(direction_t direction, double vcorate);
    double _tune_bbvco(const double rate);
    void _reprogram_gains();
    double _tune_helper(direction_t direction, const double value);
    double _setup_rates(const double rate);
    double _get_temperature(const double cal_offset, const double timeout = 0.1);
    void _configure_bb_dc_tracking();
    void _configure_rx_iq_tracking();
    void _setup_agc(chain_t chain, gain_mode_t gain_mode);
    void _set_fir_taps(direction_t direction, chain_t chain, const std::vector<int16_t>& taps);
    std::vector<int16_t> _get_fir_taps(direction_t direction, chain_t chain);
    size_t _get_num_fir_taps(direction_t direction);
    size_t _get_fir_dec_int(direction_t direction);
    filter_info_base::sptr _get_filter_lp_tia_sec(direction_t direction);
    filter_info_base::sptr _get_filter_lp_bb(direction_t direction);
    filter_info_base::sptr _get_filter_dec_int_3(direction_t direction);
    filter_info_base::sptr _get_filter_hb_3(direction_t direction);
    filter_info_base::sptr _get_filter_hb_2(direction_t direction);
    filter_info_base::sptr _get_filter_hb_1(direction_t direction);
    filter_info_base::sptr _get_filter_fir(direction_t direction, chain_t chain);
    void _set_filter_fir(direction_t direction, chain_t channel, filter_info_base::sptr filter);
    void _set_filter_lp_bb(direction_t direction, filter_info_base::sptr filter);
    void _set_filter_lp_tia_sec(direction_t direction, filter_info_base::sptr filter);
    void _set_filter_dummy(direction_t direction, chain_t channel, filter_info_base::sptr filter);

private:    //Members
    typedef struct {
        uint8_t vcodivs;
        uint8_t inputsel;
        uint8_t rxfilt;
        uint8_t txfilt;
        uint8_t bbpll;
        uint8_t bbftune_config;
        uint8_t bbftune_mode;
    } chip_regs_t;


    struct filter_query_helper
    {
        filter_query_helper(
                std::function<filter_info_base::sptr (direction_t, chain_t)> p_get,
                std::function<void (direction_t, chain_t, filter_info_base::sptr)> p_set
                ) : get(p_get), set(p_set) {  }

        filter_query_helper(){ }

        std::function<filter_info_base::sptr (direction_t, chain_t)> get;
        std::function<void (direction_t, chain_t, filter_info_base::sptr)> set;
    };

    std::map<std::string, filter_query_helper> _rx_filters;
    std::map<std::string, filter_query_helper> _tx_filters;

    //Interfaces
    ad9361_params* _client_params;
    DeviceControlIOIfce* _io_iface;
    //Intermediate state
    double              _rx_freq, _tx_freq, _req_rx_freq, _req_tx_freq;
    double              _last_rx_cal_freq, _last_tx_cal_freq;
    double              _rx_analog_bw, _tx_analog_bw, _rx_bb_lp_bw, _tx_bb_lp_bw;
    double              _rx_tia_lp_bw, _tx_sec_lp_bw;
    //! Current baseband sampling rate (this is the actual rate the device is
    //  is running at)
    double              _baseband_bw;
    double              _bbpll_freq, _adcclock_freq;
    //! This was the last clock rate value that was requested.
    //  It is cached so we don't need to re-set the clock rate
    //  if another call to set_clock_rate() actually has the same value.
    double              _req_clock_rate;
    double              _req_coreclk;
    uint16_t     _rx_bbf_tunediv;
    uint8_t      _curr_gain_table;
    double              _rx1_gain, _rx2_gain, _tx1_gain, _tx2_gain;
    int32_t      _tfir_factor;
    int32_t      _rfir_factor;
    gain_mode_t         _rx1_agc_mode, _rx2_agc_mode;
    bool                _rx1_agc_enable, _rx2_agc_enable;
    //Register soft-copies
    chip_regs_t         _regs;
    //Synchronization
    std::recursive_mutex  _mutex;
    bool _use_dc_offset_tracking;
    bool _use_iq_balance_tracking;




};

#endif // AD9361TUNER_H
