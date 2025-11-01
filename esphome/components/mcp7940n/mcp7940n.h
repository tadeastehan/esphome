#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/time/real_time_clock.h"

namespace esphome {
namespace mcp7940n {

class MCP7940NComponent : public time::RealTimeClock, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void read_time();
  void write_time();

 protected:
  //
  // Internal state machine, used to split all the actions into
  // small steps in loop() to make sure we are not blocking execution
  //
  enum class State : uint8_t {
    INIT,
    IDLE,
    INIT_OSC_START,
    INIT_OSC_START_WAIT,
    INIT_SET_VBATEN,
    WRITE_OSC_START,
    WRITE_OSC_START_WAIT,
    WRITE_OSC_STOP,
    WRITE_OSC_STOP_WAIT,
    WRITE_TIME
  } state_ = State::INIT;

  bool read_rtc_();
  bool write_rtc_();
  union MCP7940NReg {
    struct {
      // Seconds register
      uint8_t second : 4;
      uint8_t second_10 : 3;
      bool st : 1;

      // Minutes register
      uint8_t minute : 4;
      uint8_t minute_10 : 3;
      uint8_t : 1;

      // Hours register 24-hour format (12/24 = 0)
      uint8_t hour : 4;
      uint8_t hour_10 : 2;
      bool format_12_24 : 1;
      uint8_t : 1;

      // Weekdays register
      uint8_t weekday : 3;
      bool vbat_en : 1;
      bool pwr_fail : 1;
      bool oscrun : 1;
      uint8_t : 2;

      // Date register
      uint8_t date : 4;
      uint8_t date_10 : 2;
      uint8_t : 2;

      // Months register
      uint8_t month : 4;
      uint8_t month_10 : 1;
      bool lpyr : 1;
      uint8_t : 2;

      // Years register
      uint8_t year : 4;
      uint8_t year_10 : 4;

      // Control Register
      uint8_t sqwfs : 2;
      bool crstrim : 1;
      bool extosc : 1;
      bool alarm0_en : 1;
      bool alarm1_en : 1;
      bool sqw_en : 1;
      bool out : 1;

      // Oscillator Digital Trim Register
      uint8_t trim : 7;
      bool sign : 1;

      // Reserved
      uint8_t : 8;

      // Seconds Alarm register
      uint8_t second_alarm0 : 4;
      uint8_t second_alarm0_10 : 3;
      uint8_t : 1;

      // Minutes Alarm register
      uint8_t minute_alarm0 : 4;
      uint8_t minute_alarm0_10 : 3;
      uint8_t : 1;

      // Hours Alarm register 24-hour format (12/24 = 0)
      uint8_t hour_alarm0 : 4;
      uint8_t hour_alarm0_10 : 2;
      bool format_12_24_alarm0 : 1;
      uint8_t : 1;

      // Weekdays Alarm register
      uint8_t weekday_alarm0 : 3;
      bool alarm0_int : 1;
      uint8_t alarm0_msk : 3;
      bool alarm0_int_pol : 1;

      // Date Alarm register
      uint8_t date_alarm0 : 4;
      uint8_t date_alarm0_10 : 2;
      uint8_t : 2;

      // Months Alarm register
      uint8_t month_alarm0 : 4;
      uint8_t month_alarm0_10 : 1;
      uint8_t : 3;

      // Reserved
      uint8_t : 8;

      // Seconds Alarm register
      uint8_t second_alarm1 : 4;
      uint8_t second_alarm1_10 : 3;
      uint8_t : 1;

      // Minutes Alarm register
      uint8_t minute_alarm1 : 4;
      uint8_t minute_alarm1_10 : 3;
      uint8_t : 1;

      // Hours Alarm register 24-hour format (12/24 = 0)
      uint8_t hour_alarm1 : 4;
      uint8_t hour_alarm1_10 : 2;
      bool format_12_24_alarm1 : 1;
      uint8_t : 1;

      // Weekdays Alarm register
      uint8_t weekday_alarm1 : 3;
      bool alarm1_int : 1;
      uint8_t alarm1_msk : 3;
      bool alarm1_int_pol : 1;

      // Date Alarm register
      uint8_t date_alarm1 : 4;
      uint8_t date_alarm1_10 : 2;
      uint8_t : 2;

      // Months Alarm register
      uint8_t month_alarm1 : 4;
      uint8_t month_alarm1_10 : 1;
      uint8_t : 3;

      // Reserved
      uint8_t : 8;

      // Power-Down Minutes register
      uint8_t minute_pd : 4;
      uint8_t minute_pd_10 : 3;
      uint8_t : 1;

      // Power-Down Hours register 24-hour format (12/24 = 0)
      uint8_t hour_pd : 4;
      uint8_t hour_pd_10 : 2;
      bool format_12_24_pd : 1;
      uint8_t : 1;

      // Power-Down Date register
      uint8_t date_pd : 4;
      uint8_t date_pd_10 : 2;
      uint8_t : 2;

      // Power-Down Months register
      uint8_t month_pd : 4;
      uint8_t month_pd_10 : 1;
      uint8_t weekday_pd : 3;

      // Power-Up Minutes register
      uint8_t minute_pu : 4;
      uint8_t minute_pu_10 : 3;
      uint8_t : 1;

      // Power-Up Hours register 24-hour format (12/24 = 0)
      uint8_t hour_pu : 4;
      uint8_t hour_pu_10 : 2;
      bool format_12_24_pu : 1;
      uint8_t : 1;

      // Power-Up Date register
      uint8_t date_pu : 4;
      uint8_t date_pu_10 : 2;
      uint8_t : 2;

      // Power-Up Months register
      uint8_t month_pu : 4;
      uint8_t month_pu_10 : 1;
      uint8_t weekday_pu : 3;

    } reg;
    mutable uint8_t raw[sizeof(reg)];
  } mcp7940n_;
};

template<typename... Ts> class WriteAction : public Action<Ts...>, public Parented<MCP7940NComponent> {
 public:
  void play(Ts... x) override { this->parent_->write_time(); }
};

template<typename... Ts> class ReadAction : public Action<Ts...>, public Parented<MCP7940NComponent> {
 public:
  void play(Ts... x) override { this->parent_->read_time(); }
};
}  // namespace mcp7940n
}  // namespace esphome
