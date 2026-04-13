#include "VL53L0X_mbed.hpp"

using namespace std::chrono;

namespace {
constexpr uint8_t ADDRESS_DEFAULT = 0b0101001; // 0x29, same as Pololu Arduino lib
constexpr uint32_t calcMacroPeriod(uint8_t vcsel_period_pclks) {
    return ((((uint32_t)2304 * vcsel_period_pclks * 1655) + 500) / 1000);
}
}

VL53L0X_mbed::VL53L0X_mbed(I2C& i2c, uint8_t address7bit)
    : bus(&i2c),
      address(address7bit),
      io_timeout(0),
      did_timeout(false),
      timeout_start_ms(0),
      stop_variable(0),
      measurement_timing_budget_us(0),
      last_status(0) {}

uint32_t VL53L0X_mbed::millis32() const {
    return duration_cast<milliseconds>(Kernel::Clock::now().time_since_epoch()).count();
}

void VL53L0X_mbed::startTimeout() {
    timeout_start_ms = millis32();
}

bool VL53L0X_mbed::checkTimeoutExpired() const {
    return io_timeout > 0 && (millis32() - timeout_start_ms > io_timeout);
}

void VL53L0X_mbed::setAddress(uint8_t new_addr_7bit) {
    writeReg(I2C_SLAVE_DEVICE_ADDRESS, new_addr_7bit & 0x7F);
    address = new_addr_7bit;
}

bool VL53L0X_mbed::init(bool io_2v8) {
    if (readReg(IDENTIFICATION_MODEL_ID) != 0xEE) {
        return false;
    }

    if (io_2v8) {
        writeReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV,
                 readReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV) | 0x01);
    }

    writeReg(0x88, 0x00);

    writeReg(0x80, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);
    stop_variable = readReg(0x91);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);

    writeReg(MSRC_CONFIG_CONTROL, readReg(MSRC_CONFIG_CONTROL) | 0x12);
    setSignalRateLimit(0.25f);
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0xFF);

    uint8_t spad_count = 0;
    bool spad_type_is_aperture = false;
    if (!getSpadInfo(&spad_count, &spad_type_is_aperture)) {
        return false;
    }

    uint8_t ref_spad_map[6] = {};
    readMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

    writeReg(0xFF, 0x01);
    writeReg(DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
    writeReg(DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
    writeReg(0xFF, 0x00);
    writeReg(GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);

    uint8_t first_spad_to_enable = spad_type_is_aperture ? 12 : 0;
    uint8_t spads_enabled = 0;

    for (uint8_t i = 0; i < 48; i++) {
        if (i < first_spad_to_enable || spads_enabled == spad_count) {
            ref_spad_map[i / 8] &= ~(1 << (i % 8));
        } else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1) {
            spads_enabled++;
        }
    }

    writeMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);

    writeReg(0xFF, 0x00);
    writeReg(0x09, 0x00);
    writeReg(0x10, 0x00);
    writeReg(0x11, 0x00);

    writeReg(0x24, 0x01);
    writeReg(0x25, 0xFF);
    writeReg(0x75, 0x00);

    writeReg(0xFF, 0x01);
    writeReg(0x4E, 0x2C);
    writeReg(0x48, 0x00);
    writeReg(0x30, 0x20);

    writeReg(0xFF, 0x00);
    writeReg(0x30, 0x09);
    writeReg(0x54, 0x00);
    writeReg(0x31, 0x04);
    writeReg(0x32, 0x03);
    writeReg(0x40, 0x83);
    writeReg(0x46, 0x25);
    writeReg(0x60, 0x00);
    writeReg(0x27, 0x00);
    writeReg(0x50, 0x06);
    writeReg(0x51, 0x00);
    writeReg(0x52, 0x96);
    writeReg(0x56, 0x08);
    writeReg(0x57, 0x30);
    writeReg(0x61, 0x00);
    writeReg(0x62, 0x00);
    writeReg(0x64, 0x00);
    writeReg(0x65, 0x00);
    writeReg(0x66, 0xA0);

    writeReg(0xFF, 0x01);
    writeReg(0x22, 0x32);
    writeReg(0x47, 0x14);
    writeReg(0x49, 0xFF);
    writeReg(0x4A, 0x00);

    writeReg(0xFF, 0x00);
    writeReg(0x7A, 0x0A);
    writeReg(0x7B, 0x00);
    writeReg(0x78, 0x21);

    writeReg(0xFF, 0x01);
    writeReg(0x23, 0x34);
    writeReg(0x42, 0x00);
    writeReg(0x44, 0xFF);
    writeReg(0x45, 0x26);
    writeReg(0x46, 0x05);
    writeReg(0x40, 0x40);
    writeReg(0x0E, 0x06);
    writeReg(0x20, 0x1A);
    writeReg(0x43, 0x40);

    writeReg(0xFF, 0x00);
    writeReg(0x34, 0x03);
    writeReg(0x35, 0x44);

    writeReg(0xFF, 0x01);
    writeReg(0x31, 0x04);
    writeReg(0x4B, 0x09);
    writeReg(0x4C, 0x05);
    writeReg(0x4D, 0x04);

    writeReg(0xFF, 0x00);
    writeReg(0x44, 0x00);
    writeReg(0x45, 0x20);
    writeReg(0x47, 0x08);
    writeReg(0x48, 0x28);
    writeReg(0x67, 0x00);
    writeReg(0x70, 0x04);
    writeReg(0x71, 0x01);
    writeReg(0x72, 0xFE);
    writeReg(0x76, 0x00);
    writeReg(0x77, 0x00);

    writeReg(0xFF, 0x01);
    writeReg(0x0D, 0x01);

    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x01);
    writeReg(0x01, 0xF8);

    writeReg(0xFF, 0x01);
    writeReg(0x8E, 0x01);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);

    writeReg(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    writeReg(GPIO_HV_MUX_ACTIVE_HIGH, readReg(GPIO_HV_MUX_ACTIVE_HIGH) & ~0x10);
    writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

    measurement_timing_budget_us = getMeasurementTimingBudget();
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8);
    setMeasurementTimingBudget(measurement_timing_budget_us);

    writeReg(SYSTEM_SEQUENCE_CONFIG, 0x01);
    if (!performSingleRefCalibration(0x40)) {
        return false;
    }

    writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
    if (!performSingleRefCalibration(0x00)) {
        return false;
    }

    writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8);
    return true;
}

void VL53L0X_mbed::writeReg(uint8_t reg, uint8_t value) {
    char data[2] = {
        static_cast<char>(reg),
        static_cast<char>(value)
    };
    last_status = static_cast<uint8_t>(bus->write(address << 1, data, 2, false));
}

void VL53L0X_mbed::writeReg16Bit(uint8_t reg, uint16_t value) {
    char data[3] = {
        static_cast<char>(reg),
        static_cast<char>(value >> 8),
        static_cast<char>(value & 0xFF)
    };
    last_status = static_cast<uint8_t>(bus->write(address << 1, data, 3, false));
}

void VL53L0X_mbed::writeReg32Bit(uint8_t reg, uint32_t value) {
    char data[5] = {
        static_cast<char>(reg),
        static_cast<char>((value >> 24) & 0xFF),
        static_cast<char>((value >> 16) & 0xFF),
        static_cast<char>((value >> 8) & 0xFF),
        static_cast<char>(value & 0xFF)
    };
    last_status = static_cast<uint8_t>(bus->write(address << 1, data, 5, false));
}

uint8_t VL53L0X_mbed::readReg(uint8_t reg) {
    char r = static_cast<char>(reg);
    char v = 0;
    last_status = static_cast<uint8_t>(bus->write(address << 1, &r, 1, true));
    if (last_status != 0) return 0;
    last_status = static_cast<uint8_t>(bus->read(address << 1, &v, 1, false));
    return static_cast<uint8_t>(v);
}

uint16_t VL53L0X_mbed::readReg16Bit(uint8_t reg) {
    uint8_t buf[2] = {};
    readMulti(reg, buf, 2);
    return (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
}

uint32_t VL53L0X_mbed::readReg32Bit(uint8_t reg) {
    uint8_t buf[4] = {};
    readMulti(reg, buf, 4);
    return (static_cast<uint32_t>(buf[0]) << 24) |
           (static_cast<uint32_t>(buf[1]) << 16) |
           (static_cast<uint32_t>(buf[2]) << 8) |
           static_cast<uint32_t>(buf[3]);
}

void VL53L0X_mbed::writeMulti(uint8_t reg, const uint8_t* src, uint8_t count) {
    char buffer[32];
    if (count > 31) {
        last_status = 1;
        return;
    }
    buffer[0] = static_cast<char>(reg);
    for (uint8_t i = 0; i < count; i++) {
        buffer[i + 1] = static_cast<char>(src[i]);
    }
    last_status = static_cast<uint8_t>(bus->write(address << 1, buffer, count + 1, false));
}

void VL53L0X_mbed::readMulti(uint8_t reg, uint8_t* dst, uint8_t count) {
    char r = static_cast<char>(reg);
    last_status = static_cast<uint8_t>(bus->write(address << 1, &r, 1, true));
    if (last_status != 0) return;
    last_status = static_cast<uint8_t>(bus->read(address << 1, reinterpret_cast<char*>(dst), count, false));
}

bool VL53L0X_mbed::setSignalRateLimit(float limit_Mcps) {
    if (limit_Mcps < 0.0f || limit_Mcps > 511.99f) {
        return false;
    }
    writeReg16Bit(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT,
                  static_cast<uint16_t>(limit_Mcps * (1 << 7)));
    return true;
}

float VL53L0X_mbed::getSignalRateLimit() {
    return static_cast<float>(readReg16Bit(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT)) / (1 << 7);
}

bool VL53L0X_mbed::setMeasurementTimingBudget(uint32_t budget_us) {
    SequenceStepEnables enables{};
    SequenceStepTimeouts timeouts{};

    constexpr uint16_t StartOverhead     = 1910;
    constexpr uint16_t EndOverhead       = 960;
    constexpr uint16_t MsrcOverhead      = 660;
    constexpr uint16_t TccOverhead       = 590;
    constexpr uint16_t DssOverhead       = 690;
    constexpr uint16_t PreRangeOverhead  = 660;
    constexpr uint16_t FinalRangeOverhead = 550;

    uint32_t used_budget_us = StartOverhead + EndOverhead;

    getSequenceStepEnables(&enables);
    getSequenceStepTimeouts(&enables, &timeouts);

    if (enables.tcc) {
        used_budget_us += timeouts.msrc_dss_tcc_us + TccOverhead;
    }

    if (enables.dss) {
        used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
    } else if (enables.msrc) {
        used_budget_us += timeouts.msrc_dss_tcc_us + MsrcOverhead;
    }

    if (enables.pre_range) {
        used_budget_us += timeouts.pre_range_us + PreRangeOverhead;
    }

    if (enables.final_range) {
        used_budget_us += FinalRangeOverhead;

        if (used_budget_us > budget_us) {
            return false;
        }

        uint32_t final_range_timeout_us = budget_us - used_budget_us;
        uint32_t final_range_timeout_mclks =
            timeoutMicrosecondsToMclks(final_range_timeout_us, timeouts.final_range_vcsel_period_pclks);

        if (enables.pre_range) {
            final_range_timeout_mclks += timeouts.pre_range_mclks;
        }

        writeReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
                      encodeTimeout(final_range_timeout_mclks));

        measurement_timing_budget_us = budget_us;
    }

    return true;
}

uint32_t VL53L0X_mbed::getMeasurementTimingBudget() {
    SequenceStepEnables enables{};
    SequenceStepTimeouts timeouts{};

    constexpr uint16_t StartOverhead      = 1910;
    constexpr uint16_t EndOverhead        = 960;
    constexpr uint16_t MsrcOverhead       = 660;
    constexpr uint16_t TccOverhead        = 590;
    constexpr uint16_t DssOverhead        = 690;
    constexpr uint16_t PreRangeOverhead   = 660;
    constexpr uint16_t FinalRangeOverhead = 550;

    uint32_t budget_us = StartOverhead + EndOverhead;

    getSequenceStepEnables(&enables);
    getSequenceStepTimeouts(&enables, &timeouts);

    if (enables.tcc) {
        budget_us += timeouts.msrc_dss_tcc_us + TccOverhead;
    }

    if (enables.dss) {
        budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
    } else if (enables.msrc) {
        budget_us += timeouts.msrc_dss_tcc_us + MsrcOverhead;
    }

    if (enables.pre_range) {
        budget_us += timeouts.pre_range_us + PreRangeOverhead;
    }

    if (enables.final_range) {
        budget_us += timeouts.final_range_us + FinalRangeOverhead;
    }

    measurement_timing_budget_us = budget_us;
    return budget_us;
}

bool VL53L0X_mbed::setVcselPulsePeriod(vcselPeriodType type, uint8_t period_pclks) {
    uint8_t vcsel_period_reg = encodeVcselPeriod(period_pclks);
    SequenceStepEnables enables{};
    SequenceStepTimeouts timeouts{};

    getSequenceStepEnables(&enables);
    getSequenceStepTimeouts(&enables, &timeouts);

    if (type == VcselPeriodPreRange) {
        switch (period_pclks) {
            case 12: writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x18); break;
            case 14: writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x30); break;
            case 16: writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x40); break;
            case 18: writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x50); break;
            default: return false;
        }
        writeReg(PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
        writeReg(PRE_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);

        uint16_t new_pre_range_timeout_mclks =
            timeoutMicrosecondsToMclks(timeouts.pre_range_us, period_pclks);
        writeReg16Bit(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI,
                      encodeTimeout(new_pre_range_timeout_mclks));

        uint16_t new_msrc_timeout_mclks =
            timeoutMicrosecondsToMclks(timeouts.msrc_dss_tcc_us, period_pclks);
        writeReg(MSRC_CONFIG_TIMEOUT_MACROP,
                 (new_msrc_timeout_mclks > 256) ? 255 : (new_msrc_timeout_mclks - 1));
    } else if (type == VcselPeriodFinalRange) {
        switch (period_pclks) {
            case 8:
                writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x10);
                writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
                writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x02);
                writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x0C);
                writeReg(0xFF, 0x01);
                writeReg(ALGO_PHASECAL_LIM, 0x30);
                writeReg(0xFF, 0x00);
                break;
            case 10:
                writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x28);
                writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
                writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
                writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x09);
                writeReg(0xFF, 0x01);
                writeReg(ALGO_PHASECAL_LIM, 0x20);
                writeReg(0xFF, 0x00);
                break;
            case 12:
                writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x38);
                writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
                writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
                writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x08);
                writeReg(0xFF, 0x01);
                writeReg(ALGO_PHASECAL_LIM, 0x20);
                writeReg(0xFF, 0x00);
                break;
            case 14:
                writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x48);
                writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
                writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
                writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x07);
                writeReg(0xFF, 0x01);
                writeReg(ALGO_PHASECAL_LIM, 0x20);
                writeReg(0xFF, 0x00);
                break;
            default:
                return false;
        }

        writeReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);

        uint32_t new_final_range_timeout_mclks =
            timeoutMicrosecondsToMclks(timeouts.final_range_us, period_pclks);

        if (enables.pre_range) {
            new_final_range_timeout_mclks += timeouts.pre_range_mclks;
        }

        writeReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
                      encodeTimeout(new_final_range_timeout_mclks));
    } else {
        return false;
    }

    setMeasurementTimingBudget(measurement_timing_budget_us);

    uint8_t sequence_config = readReg(SYSTEM_SEQUENCE_CONFIG);
    writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
    if (!performSingleRefCalibration(0x0)) {
        return false;
    }
    writeReg(SYSTEM_SEQUENCE_CONFIG, sequence_config);

    return true;
}

uint8_t VL53L0X_mbed::getVcselPulsePeriod(vcselPeriodType type) {
    if (type == VcselPeriodPreRange) {
        return decodeVcselPeriod(readReg(PRE_RANGE_CONFIG_VCSEL_PERIOD));
    }
    if (type == VcselPeriodFinalRange) {
        return decodeVcselPeriod(readReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD));
    }
    return 255;
}

void VL53L0X_mbed::startContinuous(uint32_t period_ms) {
    writeReg(0x80, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);
    writeReg(0x91, stop_variable);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);

    if (period_ms != 0) {
        uint16_t osc_calibrate_val = readReg16Bit(OSC_CALIBRATE_VAL);
        if (osc_calibrate_val != 0) {
            period_ms *= osc_calibrate_val;
        }
        writeReg32Bit(SYSTEM_INTERMEASUREMENT_PERIOD, period_ms);
        writeReg(SYSRANGE_START, 0x04);
    } else {
        writeReg(SYSRANGE_START, 0x02);
    }
}

void VL53L0X_mbed::stopContinuous() {
    writeReg(SYSRANGE_START, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);
    writeReg(0x91, 0x00);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
}

uint16_t VL53L0X_mbed::readRangeContinuousMillimeters() {
    startTimeout();
    while ((readReg(RESULT_INTERRUPT_STATUS) & 0x07) == 0) {
        if (checkTimeoutExpired()) {
            did_timeout = true;
            return 65535;
        }
    }

    uint16_t range = readReg16Bit(RESULT_RANGE_STATUS + 10);
    writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
    return range;
}

uint16_t VL53L0X_mbed::readRangeSingleMillimeters() {
    writeReg(0x80, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);
    writeReg(0x91, stop_variable);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);

    writeReg(SYSRANGE_START, 0x01);

    startTimeout();
    while (readReg(SYSRANGE_START) & 0x01) {
        if (checkTimeoutExpired()) {
            did_timeout = true;
            return 65535;
        }
    }

    return readRangeContinuousMillimeters();
}

bool VL53L0X_mbed::timeoutOccurred() {
    bool tmp = did_timeout;
    did_timeout = false;
    return tmp;
}

bool VL53L0X_mbed::getSpadInfo(uint8_t* count, bool* type_is_aperture) {
    uint8_t tmp;

    writeReg(0x80, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);

    writeReg(0xFF, 0x06);
    writeReg(0x83, readReg(0x83) | 0x04);
    writeReg(0xFF, 0x07);
    writeReg(0x81, 0x01);

    writeReg(0x80, 0x01);

    writeReg(0x94, 0x6b);
    writeReg(0x83, 0x00);
    startTimeout();
    while (readReg(0x83) == 0x00) {
        if (checkTimeoutExpired()) {
            return false;
        }
    }
    writeReg(0x83, 0x01);
    tmp = readReg(0x92);

    *count = tmp & 0x7F;
    *type_is_aperture = (tmp >> 7) & 0x01;

    writeReg(0x81, 0x00);
    writeReg(0xFF, 0x06);
    writeReg(0x83, readReg(0x83) & ~0x04);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x01);

    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);

    return true;
}

void VL53L0X_mbed::getSequenceStepEnables(SequenceStepEnables* enables) {
    uint8_t sequence_config = readReg(SYSTEM_SEQUENCE_CONFIG);

    enables->tcc = (sequence_config >> 4) & 0x1;
    enables->dss = (sequence_config >> 3) & 0x1;
    enables->msrc = (sequence_config >> 2) & 0x1;
    enables->pre_range = (sequence_config >> 6) & 0x1;
    enables->final_range = (sequence_config >> 7) & 0x1;
}

void VL53L0X_mbed::getSequenceStepTimeouts(const SequenceStepEnables* enables,
                                           SequenceStepTimeouts* timeouts) {
    timeouts->pre_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodPreRange);

    timeouts->msrc_dss_tcc_mclks = readReg(MSRC_CONFIG_TIMEOUT_MACROP) + 1;
    timeouts->msrc_dss_tcc_us =
        timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
                                   timeouts->pre_range_vcsel_period_pclks);

    timeouts->pre_range_mclks =
        decodeTimeout(readReg16Bit(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI));
    timeouts->pre_range_us =
        timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
                                   timeouts->pre_range_vcsel_period_pclks);

    timeouts->final_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodFinalRange);

    timeouts->final_range_mclks =
        decodeTimeout(readReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI));

    if (enables->pre_range) {
        timeouts->final_range_mclks -= timeouts->pre_range_mclks;
    }

    timeouts->final_range_us =
        timeoutMclksToMicroseconds(timeouts->final_range_mclks,
                                   timeouts->final_range_vcsel_period_pclks);
}

bool VL53L0X_mbed::performSingleRefCalibration(uint8_t vhv_init_byte) {
    writeReg(SYSRANGE_START, 0x01 | vhv_init_byte);

    startTimeout();
    while ((readReg(RESULT_INTERRUPT_STATUS) & 0x07) == 0) {
        if (checkTimeoutExpired()) {
            return false;
        }
    }

    writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
    writeReg(SYSRANGE_START, 0x00);

    return true;
}

uint16_t VL53L0X_mbed::decodeTimeout(uint16_t value) {
    return static_cast<uint16_t>((value & 0x00FF) << ((value & 0xFF00) >> 8)) + 1;
}

uint16_t VL53L0X_mbed::encodeTimeout(uint32_t timeout_mclks) {
    uint32_t ls_byte = 0;
    uint16_t ms_byte = 0;

    if (timeout_mclks > 0) {
        ls_byte = timeout_mclks - 1;
        while ((ls_byte & 0xFFFFFF00UL) > 0) {
            ls_byte >>= 1;
            ms_byte++;
        }
        return static_cast<uint16_t>((ms_byte << 8) | (ls_byte & 0xFF));
    }
    return 0;
}

uint32_t VL53L0X_mbed::timeoutMclksToMicroseconds(uint16_t timeout_period_mclks,
                                                  uint8_t vcsel_period_pclks) {
    uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);
    return ((timeout_period_mclks * macro_period_ns) + 500) / 1000;
}

uint32_t VL53L0X_mbed::timeoutMicrosecondsToMclks(uint32_t timeout_period_us,
                                                  uint8_t vcsel_period_pclks) {
    uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);
    return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

uint8_t VL53L0X_mbed::decodeVcselPeriod(uint8_t reg_val) {
    return (reg_val + 1) << 1;
}

uint8_t VL53L0X_mbed::encodeVcselPeriod(uint8_t period_pclks) {
    return (period_pclks >> 1) - 1;
}