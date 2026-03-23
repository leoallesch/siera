#define SIERA_DS_KEYS(KEY) \
    KEY(COUNTER,     uint32_t, SIERA_DS_RAM, 0,    SIERA_DS_NONE) \
    KEY(MESSAGE,     uint8_t,  SIERA_DS_RAM, 0,    SIERA_DS_NONE) \
    KEY(ARMED,       bool,     SIERA_DS_RAM, false, SIERA_DS_NONE)
