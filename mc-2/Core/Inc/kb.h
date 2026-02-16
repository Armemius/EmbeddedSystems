// kb.h
#pragma once

#include "main.h"

#define KBRD_ADDR 0xE2

#define ROW1 0xFE
#define ROW2 0xFD
#define ROW3 0xFB
#define ROW4 0xF7

HAL_StatusTypeDef kb_init(void);

uint8_t kb_check_row(uint8_t row_mask);

uint8_t kb_get_key(void);

char kb_get_symb(void);
