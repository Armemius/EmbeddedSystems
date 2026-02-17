#include "kb.h"
#include "pca9538.h"
#include "main.h"

HAL_StatusTypeDef kb_init(void)
{
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t buf;

    // POLARITY_INVERSION = 0
    buf = 0;
    ret = PCA9538_Write_Register(KBRD_ADDR, POLARITY_INVERSION, &buf);
    if (ret != HAL_OK) {
        return ret;
    }

    // OUTPUT_PORT = 0
    buf = 0;
    ret = PCA9538_Write_Register(KBRD_ADDR, OUTPUT_PORT, &buf);
    if (ret != HAL_OK) {
        return ret;
    }

    return HAL_OK;
}

uint8_t kb_check_row(uint8_t row_mask)
{
    uint8_t key = 0x00;
    HAL_StatusTypeDef ret;
    uint8_t buf;
    uint8_t kbd_in;

    ret = kb_init();
    if (ret != HAL_OK) {
        return 0;
    }

    buf = row_mask;
    ret = PCA9538_Write_Register(KBRD_ADDR, CONFIG, &buf);
    if (ret != HAL_OK) {
        return 0;
    }

    ret = PCA9538_Read_Inputs(KBRD_ADDR, &buf);
    if (ret != HAL_OK) {
        return 0;
    }

    kbd_in = (uint8_t)(buf & 0x70);
    if (kbd_in == 0x70) return 0;

    if (!(kbd_in & 0x10)) key = 1;
    else if (!(kbd_in & 0x20)) key = 2;
    else if (!(kbd_in & 0x40)) key = 3;
    else key = 0;

    return key;
}

uint8_t kb_get_key(void)
{
    const uint8_t rows[4] = { ROW1, ROW2, ROW3, ROW4 };

    for (int i = 0; i < 4; i++) {
    	for (int j = 0; j < 5; ++j) {
    		uint8_t k = kb_check_row(rows[i]);
    		if (k != 0) return 3 * i + k;
    	}
    }
    return 0;
}

char kb_get_symb(void)
{
	const char symb[13] = {0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#'};
	return symb[kb_get_key()];
}
