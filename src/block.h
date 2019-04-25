#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f0_discovery.h"
#include "stm32f0xx.h"

#ifndef _INIT_SUCCESS_
#define _INIT_SUCCESS_ 1

#define LEFT 3
#define RIGHT 2
#define UP 1
#define DOWN 0

#define PB_MOSI (5)
#define PB_SCK (3)
#define PB_DC (4)
#define PA_CS (12)
#define PA_RST (15)

#define ROW_NUM ((uint16_t)320)
#define COLUMN_NUM ((uint16_t)240)

// available x-axis
#define X_AVA_MAX ((uint16_t)(COLUMN_NUM))

// available y-axis
#define Y_AVA_MAX ((uint16_t)(ROW_NUM))

typedef struct Block {
  uint16_t block_x, block_y, thick_x, thick_y, color;
} Block;
#endif

void block_left_right(Block *block, uint16_t dist, uint8_t direction);

void block_up_down(Block *block, uint16_t dist, uint8_t direction);

void show_block(Block *block);

void show_block_x_overflow(Block *block);

void show_block_y_overflow(Block *block);

void show_block_valid(Block *block);

void show_block_x_y_overflow(Block *block);
