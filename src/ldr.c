#include "ldr.h"
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

#define LDR_ADC_UNIT     ADC_UNIT_1
#define LDR_ADC_CHANNEL  ADC_CHANNEL_6
#define ADC_MAX_RAW      4095

static adc_oneshot_unit_handle_t s_adc_handle;

void ldr_init(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = LDR_ADC_UNIT,
    };
    adc_oneshot_new_unit(&unit_cfg, &s_adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(s_adc_handle, LDR_ADC_CHANNEL, &chan_cfg);
}

int ldr_read_percent(void)
{
    int raw = 0;
    if (adc_oneshot_read(s_adc_handle, LDR_ADC_CHANNEL, &raw) != ESP_OK) {
        return -1;
    }
    return (raw * 100) / ADC_MAX_RAW;
}