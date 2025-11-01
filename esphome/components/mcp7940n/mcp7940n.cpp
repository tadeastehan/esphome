#include "mcp7940n.h"
#include "esphome/core/log.h"

// Datasheet:
// https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/MCP7940N-Battery-Backed-I2C-RTCC-with-SRAM-20005010J.pdf

namespace esphome {
namespace mcp7940n {

static const char *const TAG = "mcp7940n";

void MCP7940NComponent::setup() {
  if (!this->read_rtc_()) {
    this->mark_failed();
  }
}

void MCP7940NComponent::update() { this->read_time(); }

void MCP7940NComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "MCP7940N:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
  ESP_LOGCONFIG(TAG, "  Timezone: '%s'", this->timezone_.c_str());
}

float MCP7940NComponent::get_setup_priority() const { return setup_priority::DATA; }

void MCP7940NComponent::read_time() {
  if (!this->read_rtc_()) {
    return;
  }
  if (!mcp7940n_.reg.oscrun) {
    ESP_LOGW(TAG, "RTC halted, not syncing to system clock.");
    return;
  }
  ESPTime rtc_time{
      .second = uint8_t(mcp7940n_.reg.second + 10 * mcp7940n_.reg.second_10),
      .minute = uint8_t(mcp7940n_.reg.minute + 10u * mcp7940n_.reg.minute_10),
      .hour = uint8_t(mcp7940n_.reg.hour + 10u * mcp7940n_.reg.hour_10),
      .day_of_week = uint8_t(mcp7940n_.reg.weekday),
      .day_of_month = uint8_t(mcp7940n_.reg.date + 10u * mcp7940n_.reg.date_10),
      .day_of_year = 1,  // ignored by recalc_timestamp_utc(false)
      .month = uint8_t(mcp7940n_.reg.month + 10u * mcp7940n_.reg.month_10),
      .year = uint16_t(mcp7940n_.reg.year + 10u * mcp7940n_.reg.year_10 + 2000),
      .is_dst = false,  // not used
      .timestamp = 0    // overwritten by recalc_timestamp_utc(false)
  };
  rtc_time.recalc_timestamp_utc(false);
  if (!rtc_time.is_valid()) {
    ESP_LOGE(TAG, "Invalid RTC time, not syncing to system clock.");
    return;
  }
  time::RealTimeClock::synchronize_epoch_(rtc_time.timestamp);
}

void MCP7940NComponent::write_time() {
  auto now = time::RealTimeClock::utcnow();
  if (!now.is_valid()) {
    ESP_LOGE(TAG, "Invalid system time, not syncing to RTC.");
    return;
  }
  mcp7940n_.reg.year = (now.year - 2000) % 10;
  mcp7940n_.reg.year_10 = (now.year - 2000) / 10 % 10;
  mcp7940n_.reg.month = now.month % 10;
  mcp7940n_.reg.month_10 = now.month / 10;
  mcp7940n_.reg.date = now.day_of_month % 10;
  mcp7940n_.reg.date_10 = now.day_of_month / 10;
  mcp7940n_.reg.weekday = now.day_of_week;
  mcp7940n_.reg.hour = now.hour % 10;
  mcp7940n_.reg.hour_10 = now.hour / 10;
  mcp7940n_.reg.minute = now.minute % 10;
  mcp7940n_.reg.minute_10 = now.minute / 10;
  mcp7940n_.reg.second = now.second % 10;
  mcp7940n_.reg.second_10 = now.second / 10;

  this->write_rtc_();
}

bool MCP7940NComponent::read_rtc_() {
  if (!this->read_bytes(0, this->mcp7940n_.raw, sizeof(this->mcp7940n_.raw))) {
    ESP_LOGE(TAG, "Can't read I2C data.");
    return false;
  }
  ESP_LOGD(TAG, "Read  %0u%0u:%0u%0u:%0u%0u 20%0u%0u-%0u%0u-%0u%0u  OSCRUN:%s ST:%s",
           mcp7940n_.reg.hour_10, mcp7940n_.reg.hour, mcp7940n_.reg.minute_10, mcp7940n_.reg.minute,
           mcp7940n_.reg.second_10, mcp7940n_.reg.second, mcp7940n_.reg.year_10, mcp7940n_.reg.year,
           mcp7940n_.reg.month_10, mcp7940n_.reg.month, mcp7940n_.reg.date_10, mcp7940n_.reg.date,
           ONOFF(mcp7940n_.reg.oscrun), ONOFF(mcp7940n_.reg.st));

  return true;
}

bool MCP7940NComponent::write_rtc_() {
  if (!this->write_bytes(0, this->mcp7940n_.raw, sizeof(this->mcp7940n_.raw))) {
    ESP_LOGE(TAG, "Can't write I2C data.");
    return false;
  }
  ESP_LOGD(TAG, "Write %0u%0u:%0u%0u:%0u%0u 20%0u%0u-%0u%0u-%0u%0u  OSCRUN:%s RS:%0u SQWE:%s OUT:%s",
           mcp7940n_.reg.hour_10, mcp7940n_.reg.hour, mcp7940n_.reg.minute_10, mcp7940n_.reg.minute,
           mcp7940n_.reg.second_10, mcp7940n_.reg.second, mcp7940n_.reg.year_10, mcp7940n_.reg.year,
           mcp7940n_.reg.month_10, mcp7940n_.reg.month, mcp7940n_.reg.date_10, mcp7940n_.reg.date,
           ONOFF(mcp7940n_.reg.oscrun));
  return true;
}
}  // namespace mcp7940n
}  // namespace esphome
