#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdint.h>
#include <stdio.h>

int score1;
int score2;


// These are function pointers.  They can be called like functions
// after you set them to point to other functions.
// e.g.  cmd = bitbang_cmd;
// They will be set by the stepX() subroutines to point to the new
// subroutines you write below.
void (*cmd)(char b);
void (*data)(char b);
void (*display1)(const char *);
void (*display2)(const char *);

// Prototypes for subroutines in support.c
void generic_lcd_startup(void);
void clock(void);
void step1(void);
void step2(void);
void step3(void);
void step4(int wl);
void step6(void);
void step4_start(void);
void step4_select(int select);

#define SPI_DELAY 1337

// Extern declarations of function pointers in main.c.
extern void (*cmd)(char b);
extern void (*data)(char b);
extern void (*display1)(const char *);
extern void (*display2)(const char *);

void spi_cmd(char);
void spi_data(char);

//void spi_display1(const char *);
//void spi_display2(const char *);
void dma_display1(const char *);
void circdma_display1(const char *);
void circdma_display2(const char *);

void spi_init_lcd(void);
void dma_spi_init_lcd(void);

void init_tim2(void);


