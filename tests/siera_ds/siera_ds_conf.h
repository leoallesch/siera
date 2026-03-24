/* Keys used by the siera_ds unit tests */
#define SIERA_DS_KEYS(KEY) \
    KEY(SYSTEM_STATE,  uint8_t,  SIERA_DS_RAM,     0,     SIERA_DS_NONE)     \
    KEY(ALARM_FIRING,  bool,     SIERA_DS_RAM,     false, SIERA_DS_NONE)     \
    KEY(ALARM_HOUR,    uint8_t,  SIERA_DS_NVS, 7,     SIERA_DS_NONE)     \
    KEY(ALARM_MIN,     uint8_t,  SIERA_DS_NVS, 0,     SIERA_DS_NONE)     \
    KEY(ALARM_ENABLED, bool,     SIERA_DS_NVS, false, SIERA_DS_NONE)     \
    KEY(BRIGHTNESS,    uint8_t,  SIERA_DS_NVS, 128,   SIERA_DS_NONE)     \
    KEY(LED_STATUS,    bool,     SIERA_DS_GPIO,    false, SIERA_DS_NONE)     \
    KEY(BUTTON_SET,    bool,     SIERA_DS_GPIO,    false, SIERA_DS_READONLY) \
    KEY(BATTERY_MV,    uint16_t, SIERA_DS_ADC,     0,     SIERA_DS_READONLY) \
    KEY(LIGHT_SENSOR,  uint16_t, SIERA_DS_ADC,     0,     SIERA_DS_READONLY)
