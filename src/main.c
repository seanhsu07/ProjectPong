
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "block.h"
#include "potentiometer.h"
#include "stm32f0_discovery.h"
#include "stm32f0xx.h"
#include "display.h"
#include "lab8.h"
#include "wavetable.h"
#define N 1024

// array for push buttons status
int arr[4] = {0, 0, 0, 0};

// for sound
int s1 = 0;
int s2 = 0;

// Global variables for ball's movement
int x, y, dx, dy;
int pad1x, pad1dx;
int pad2x, pad2dx;
int pad3x, pad3dx;
int harder = 0;

// available x-axis
#define X_AVA_MAX ((uint16_t)(COLUMN_NUM))
// available y-axis
#define Y_AVA_MAX ((uint16_t)(ROW_NUM))

void __attribute__((optimize("O0"))) delay_cycles(uint32_t cyc) {
  uint32_t d_i;
  for (d_i = 0; d_i < cyc; ++d_i) {
    asm("NOP");
  }
}

void hspi_init(SPI_TypeDef *SPIx) {
  // Ensure that the peripheral is disabled, and reset it.
  SPIx->CR1 &= ~(SPI_CR1_SPE);
  if (SPIx == SPI1) {
    RCC->APB2RSTR |= (RCC_APB2RSTR_SPI1RST);
    RCC->APB2RSTR &= ~(RCC_APB2RSTR_SPI1RST);
  }
  // Use unidirectional simplex mode.
  // SPIx->CR1 &= ~(SPI_CR1_BIDIMODE |
  //               SPI_CR1_BIDIOE);
  // Set clock polarity/phase to 0/0?
  SPIx->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA);
  // SPIx->CR1 |=  (SPI_CR1_CPHA);
  // Or 1/1 seems to work...
  SPIx->CR1 |= (SPI_CR1_CPOL | SPI_CR1_CPHA);
  // Set the STM32 to act as a host device.
  SPIx->CR1 |= (SPI_CR1_MSTR);
  // Set software 'Chip Select' pin.
  SPIx->CR1 |= (SPI_CR1_SSM);
  // (Set the internal 'Chip Select' signal.)
  SPIx->CR1 |= (SPI_CR1_SSI);

  SPIx->CR1 |= (SPI_CR1_SPE);
}

/*
 * Send a byte of data over hardware SPI.
 * This method does not wait for the communication to finish.
 * (The STM32 has an onboard FIFO queue, so we can check
 *  whether that has space for writing instead.)
 */
void hspi_w8(SPI_TypeDef *SPIx, uint8_t dat) {
  // Wait for TXE.
  while (!(SPIx->SR & SPI_SR_TXE)) {
  };
  // Send the byte.
  *(uint8_t *)&(SPIx->DR) = dat;
}

/*
 * Send 16 bits of data over hardware SPI.
 * This method does not wait for the communication to finish.
 * It adds two bytes to the FIFO queue at once; that's one
 * pixel for a 16-bit color display.
 *
 * On L0 platforms, packing 2 data frames at once does
 * not appear to be supported.
 */
void hspi_w16(SPI_TypeDef *SPIx, uint16_t dat) {
  // Wait for TXE.
  while (!(SPIx->SR & SPI_SR_TXE)) {
  };
  // Send the data.
  // (Flip the bytes for the little-endian ARM core.)
  dat = (((dat & 0x00FF) << 8) | ((dat & 0xFF00) >> 8));
  *(uint16_t *)&(SPIx->DR) = dat;

  // hspi_w8(SPIx, (uint8_t)(dat >> 8));
  // hspi_w8(SPIx, (uint8_t)(dat & 0xFF));
}

/*
 * Send a 'command' byte over hardware SPI.
 * Pull the 'D/C' pin low, send the byte, then pull the pin high.
 * Wait for the transmission to finish before changing the
 * 'D/C' pin value.
 */
void hspi_cmd(SPI_TypeDef *SPIx, uint8_t cmd) {
  while ((SPIx->SR & SPI_SR_BSY)) {
  };
  GPIOB->ODR &= ~(1 << PB_DC);
  hspi_w8(SPIx, cmd);
  while ((SPIx->SR & SPI_SR_BSY)) {
  };
  GPIOB->ODR |= (1 << PB_DC);
}
void ili9341_hspi_init(SPI_TypeDef *SPIx) {
  // (Display off)
  // hspi_cmd(SPIx, 0x28);

  // Issue a series of initialization commands from the
  // Adafruit library for a simple 'known good' test.
  hspi_cmd(SPIx, 0xEF);
  hspi_w8(SPIx, 0x03);
  hspi_w8(SPIx, 0x80);
  hspi_w8(SPIx, 0x02);
  hspi_cmd(SPIx, 0xCF);
  hspi_w8(SPIx, 0x00);
  hspi_w8(SPIx, 0xC1);
  hspi_w8(SPIx, 0x30);
  hspi_cmd(SPIx, 0xED);
  hspi_w8(SPIx, 0x64);
  hspi_w8(SPIx, 0x03);
  hspi_w8(SPIx, 0x12);
  hspi_w8(SPIx, 0x81);
  hspi_cmd(SPIx, 0xE8);
  hspi_w8(SPIx, 0x85);
  hspi_w8(SPIx, 0x00);
  hspi_w8(SPIx, 0x78);
  hspi_cmd(SPIx, 0xCB);
  hspi_w8(SPIx, 0x39);
  hspi_w8(SPIx, 0x2C);
  hspi_w8(SPIx, 0x00);
  hspi_w8(SPIx, 0x34);
  hspi_w8(SPIx, 0x02);
  hspi_cmd(SPIx, 0xF7);
  hspi_w8(SPIx, 0x20);
  hspi_cmd(SPIx, 0xEA);
  hspi_w8(SPIx, 0x00);
  hspi_w8(SPIx, 0x00);
  // PWCTR1
  hspi_cmd(SPIx, 0xC0);
  hspi_w8(SPIx, 0x23);
  // PWCTR2
  hspi_cmd(SPIx, 0xC1);
  hspi_w8(SPIx, 0x10);
  // VMCTR1
  hspi_cmd(SPIx, 0xC5);
  hspi_w8(SPIx, 0x3E);
  hspi_w8(SPIx, 0x28);
  // VMCTR2
  hspi_cmd(SPIx, 0xC7);
  hspi_w8(SPIx, 0x86);
  // MADCTL
  hspi_cmd(SPIx, 0x36);
  hspi_w8(SPIx, 0x48);
  // VSCRSADD
  hspi_cmd(SPIx, 0x37);
  hspi_w8(SPIx, 0x00);
  // PIXFMT
  hspi_cmd(SPIx, 0x3A);
  hspi_w8(SPIx, 0x55);
  // FRMCTR1
  hspi_cmd(SPIx, 0xB1);
  hspi_w8(SPIx, 0x00);
  hspi_w8(SPIx, 0x18);
  // DFUNCTR
  hspi_cmd(SPIx, 0xB6);
  hspi_w8(SPIx, 0x08);
  hspi_w8(SPIx, 0x82);
  hspi_w8(SPIx, 0x27);
  hspi_cmd(SPIx, 0xF2);
  hspi_w8(SPIx, 0x00);
  // GAMMASET
  hspi_cmd(SPIx, 0x26);
  hspi_w8(SPIx, 0x01);
  // (Actual gamma settings)
  hspi_cmd(SPIx, 0xE0);
  hspi_w8(SPIx, 0x0F);
  hspi_w8(SPIx, 0x31);
  hspi_w8(SPIx, 0x2B);
  hspi_w8(SPIx, 0x0C);
  hspi_w8(SPIx, 0x0E);
  hspi_w8(SPIx, 0x08);
  hspi_w8(SPIx, 0x4E);
  hspi_w8(SPIx, 0xF1);
  hspi_w8(SPIx, 0x37);
  hspi_w8(SPIx, 0x07);
  hspi_w8(SPIx, 0x10);
  hspi_w8(SPIx, 0x03);
  hspi_w8(SPIx, 0x0E);
  hspi_w8(SPIx, 0x09);
  hspi_w8(SPIx, 0x00);
  hspi_cmd(SPIx, 0xE1);
  hspi_w8(SPIx, 0x00);
  hspi_w8(SPIx, 0x0E);
  hspi_w8(SPIx, 0x14);
  hspi_w8(SPIx, 0x03);
  hspi_w8(SPIx, 0x11);
  hspi_w8(SPIx, 0x07);
  hspi_w8(SPIx, 0x31);
  hspi_w8(SPIx, 0xC1);
  hspi_w8(SPIx, 0x48);
  hspi_w8(SPIx, 0x08);
  hspi_w8(SPIx, 0x0F);
  hspi_w8(SPIx, 0x0C);
  hspi_w8(SPIx, 0x31);
  hspi_w8(SPIx, 0x36);
  hspi_w8(SPIx, 0x0F);

  // Exit sleep mode.
  hspi_cmd(SPIx, 0x11);
  delay_cycles(2000000);
  // Display on.
  hspi_cmd(SPIx, 0x29);
  delay_cycles(2000000);
  // 'Normal' display mode.
  hspi_cmd(SPIx, 0x13);
}

void spi_gpio_init() {
  // Enable the GPIOB peripheral in 'RCC_AHBENR'.
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

  // Enable the SPI1 peripheral.
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

  //#define VVC_SWSPI (1)
  // Initialize the GPIOB pins.
  // Mode: Output
  GPIOB->MODER &=
      ~((0x3 << (PB_MOSI * 2)) | (0x3 << (PB_SCK * 2)) | (0x3 << (PB_DC * 2)));
  // Set the MOSI and SCK pins to alternate function mode 0.
  // Set D/C to normal output.

  GPIOB->AFR[0] &= ~(GPIO_AFRL_AFRL3 | GPIO_AFRL_AFRL5);

  GPIOB->MODER |=
      ((0x2 << (PB_MOSI * 2)) | (0x2 << (PB_SCK * 2)) | (0x1 << (PB_DC * 2)));
  // Use pull-down resistors for the SPI peripheral?
  // Or no pulling resistors?
  GPIOB->PUPDR &=
      ~((0x3 << (PB_MOSI * 2)) | (0x3 << (PB_SCK * 2)) | (0x3 << (PB_DC * 2)));
  GPIOB->PUPDR |= ((0x1 << (PB_MOSI * 2)) | (0x1 << (PB_SCK * 2)));
  // Output type: Push-pull
  GPIOB->OTYPER &= ~((0x1 << PB_MOSI) | (0x1 << PB_SCK) | (0x1 << PB_DC));
  // High-speed - 50MHz maximum
  // (Setting all '1's, so no need to clear bits first.)
  GPIOB->OSPEEDR |=
      ((0x3 << (PB_MOSI * 2)) | (0x3 << (PB_SCK * 2)) | (0x3 << (PB_DC * 2)));
  // Initialize the GPIOA pins; ditto.
  GPIOA->MODER &= ~((0x3 << (PA_CS * 2)) | (0x3 << (PA_RST * 2)));
  GPIOA->MODER |= ((0x1 << (PA_CS * 2)) | (0x1 << (PA_RST * 2)));
  GPIOA->OTYPER &= ~((0x1 << PA_CS) | (0x1 << PA_RST));
  GPIOA->PUPDR &= ~((0x3 << (PA_CS * 2)) | (0x3 << (PA_RST * 2)));
}

void spi1_init() {
  GPIOA->ODR |= (1 << PA_CS);
  //   (See the 'sspi_cmd' method for 'DC' pin info.)
  GPIOB->ODR |= (1 << PB_DC);
  // Set SCK high to start
  GPIOB->ODR |= (1 << PB_SCK);
  // Reset the display by pulling the reset pin low,
  // delaying a bit, then pulling it high.
  GPIOA->ODR &= ~(1 << PA_RST);
  // Delay at least 100ms; meh, call it 2 million no-ops.
  delay_cycles(2000000);
  GPIOA->ODR |= (1 << PA_RST);
  delay_cycles(2000000);
  hspi_init(SPI1);
  // Pull CS low.
  GPIOA->ODR &= ~(1 << PA_CS);
  // Initialize the display.
  ili9341_hspi_init(SPI1);
}

void SPI_INIT() {
  spi_gpio_init();
  spi1_init();
}

void FillScreen() {
  int tft_iter = 0;
  int tft_on = 0;
  // Set column range.
  hspi_cmd(SPI1, 0x2A);
  hspi_w16(SPI1, 0x0000);
  hspi_w16(SPI1, (uint16_t)(239));
  // Set row range.
  hspi_cmd(SPI1, 0x2B);
  hspi_w16(SPI1, 0x0000);
  hspi_w16(SPI1, (uint16_t)(319));
  // Set 'write to RAM'
  hspi_cmd(SPI1, 0x2C);
  while (1) {
    // Write 320 * 240 pixels.
    for (tft_iter = 0; tft_iter < (320 * 240); ++tft_iter) {
      // Write a 16-bit color.
      if (tft_on) {
        hspi_w16(SPI1, 0xF800);
      } else {
        hspi_w16(SPI1, 0x001F);
      }
    }
    tft_on = !tft_on;
  }
}

void UpdateScreen() {
  // Set column range.
  hspi_cmd(SPI1, 0x2A);
  hspi_w16(SPI1, 0x0000);
  hspi_w16(SPI1, (uint16_t)(COLUMN_NUM - 1));
  // Set row range.
  hspi_cmd(SPI1, 0x2B);
  hspi_w16(SPI1, 0x0000);
  hspi_w16(SPI1, (uint16_t)(ROW_NUM - 1));
  // Set 'write to RAM'
  hspi_cmd(SPI1, 0x2C);
  uint16_t i = 0, j = 0;
  for (i = 0; i < ROW_NUM; i++) {
    for (j = 0; j < COLUMN_NUM; j++) {
      if (i >= 54 && i < 58) {
        hspi_w16(SPI1, (uint16_t)0xFF00);
      } else {
        hspi_w16(SPI1, (uint16_t)0x0000);
      }
    }
  }
}

void UpdateScreen_XY_THICK(uint16_t x, uint16_t y, uint16_t color,
                           uint16_t thick) {
  assert(x >= 0);
  assert(x + thick < X_AVA_MAX);
  assert(y >= 0);
  assert(y + thick < Y_AVA_MAX);

  // Set column range.
  hspi_cmd(SPI1, 0x2A);
  hspi_w16(SPI1, (uint16_t)(x));
  hspi_w16(SPI1, (uint16_t)(x + thick - 1));
  // Set row range.
  hspi_cmd(SPI1, 0x2B);
  hspi_w16(SPI1, (uint16_t)(y));
  hspi_w16(SPI1, (uint16_t)(y + thick - 1));
  // Set 'write to RAM'
  hspi_cmd(SPI1, 0x2C);
  uint16_t i = 0;
  for (i = 0; i < thick * thick; i++) {
    hspi_w16(SPI1, color);
  }
}

#define BACKGROUD ((uint16_t) ILI9341_BLUE)
void CleanScreen() {
  // Set column range.
  hspi_cmd(SPI1, 0x2A);
  hspi_w16(SPI1, 0x0000);
  hspi_w16(SPI1, (uint16_t)(COLUMN_NUM));
  // Set row range.
  hspi_cmd(SPI1, 0x2B);
  hspi_w16(SPI1, (uint16_t)(0));
  hspi_w16(SPI1, (uint16_t)(ROW_NUM));
  // Set 'write to RAM'
  hspi_cmd(SPI1, 0x2C);
  uint16_t i = 0, j = 0;
  for (i = 0; i < COLUMN_NUM; i++) {
    for (j = 0; j < ROW_NUM; j++) {
      hspi_w16(SPI1, BACKGROUD);
    }
  }
}

uint16_t get_pin(uint8_t io) {
	return GPIOC->IDR & (GPIO_IDR_2 << io);
}

void setup_dac() {
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR &= ~DAC_CR_EN1;
    DAC->CR &= ~DAC_CR_BOFF1;
    DAC->CR |= DAC_CR_TEN1;
    DAC->CR |= DAC_CR_TSEL1;
    DAC->CR |= DAC_CR_EN1;
}


void enableButtons(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN;
	GPIOA->MODER &= ~(3<<0);
	GPIOA->MODER &= ~(3<<2);
	GPIOC->MODER &= ~(3<<0);
	GPIOC->MODER &= ~(3<<2);

}

void getButtonInput(){



	if (GPIOA->IDR & GPIO_IDR_0){

		arr[0] = 1;
	}

	if (GPIOA->IDR & GPIO_IDR_1){

			arr[1] = 1;

	}

	if (GPIOC->IDR & GPIO_IDR_0){

			arr[2] = 1;
	}

	if (GPIOC->IDR & GPIO_IDR_1){
				arr[3] = 1;

	}


}

void ball_collision_check(){
	if(y <= 10 && ( x >= pad1x && x <= (pad1x + 40))){
		y = 10;
		dy *= (-1);

	}
	else if(y >= 300 - 1 && ( x >= pad2x && x <= (pad2x + 40))){
		y = 300 - 1;
		dy *= (-1);
	}
	if(harder == 1){

	if(y == 160 && ( x >= pad3x && x <= (pad3x + 80))){
			y = 160;
			dy *= (-1);
		}}

  // vertical collision detection
	if (y <= 0) {
		y = 0;
		dy *= (-1);
		score1 -- ;

		setup_dac();
		 int mod_10 = 0;
		    while (mod_10 <= 100000){

		            int step1 = 2.61626 * (1<<16);
		            int step2 = 3.29628 * (1<<16);

		            s1 += step1;
		            s2 += step2;

		                           if (s1>>16 >= N){
		                               s1 -= N<<16;
		                           }
		                           if (s2>>16 >= N){
		                               s2 -= N<<16;
		                           }

		                           int sample = wavetable[s1>>16] /*+ wavetable[s2>>16]*/;

		                           if (sample > 4095){
		                               sample = 4095;
		                           }
		                           else if (sample < 0){
		                               sample = 0;
		                           }

		                           DAC->DHR12R1 = sample;
		                           DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;

		        mod_10 += 1;}
			step4(0);


	}else if (y >= 310 - 1) {
		y = 310 - 1;
		dy *= (-1);
		score2 --;
		setup_dac();
		 int mod_10 = 0;
				    while (mod_10 <= 100000){

				            int step1 = 2.61626 * (1<<16);
				            int step2 = 3.29628 * (1<<16);

				            s1 += step1;
				            s2 += step2;

				                           if (s1>>16 >= N){
				                               s1 -= N<<16;
				                           }
				                           if (s2>>16 >= N){
				                               s2 -= N<<16;
				                           }

				                           int sample = wavetable[s1>>16] /*+ wavetable[s2>>16]*/;

				                           if (sample > 4095){
				                               sample = 4095;
				                           }
				                           else if (sample < 0){
				                               sample = 0;
				                           }

				                           DAC->DHR12R1 = sample;
				                           DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;

				        mod_10 += 1;}
		step4(0);
	}
	// horizontal collision detection
	if (x <= 0) {
		// score for p1?
		//if (y < p1.y || ball.y > p1.y + PADDLE_HEIGHT - 1) { p2.score += 1; }
		x = 0;
		dx *= (-1);
	}else if (x >= 230 - 1) {
		// score for p1?
		//if (ball.y < p2.y || ball.y > p2.y + PADDLE_HEIGHT - 1) { p1.score += 1; }

		x = 230 - 1;
		dx *= (-1);
	}


}
void beep(void){
  tim2_init();
  dac_init();
  tim6_init();
  dma3_init();
  tim3_init();
}

void obs_collision_check(){
	if(pad3x < 2){
		pad3x = 3;
		pad3dx *= -1;
	}
	else if(pad3x >= 159){
		pad3x = 158;
		pad3dx *= -1;
	}
}

uint16_t back = ILI9341_BLUE;

int main(void) {
  SPI_INIT();
  CleanScreen();

  enableButtons();
  int wl =0;

  int mode = 20000000;



  while(1){
  		  step4_start();
  		  getButtonInput();
  		  if(arr[0] == 1){
  			  mode = 20000000;
  			  step4_select(0);
  			  break;
  		  }
  		  if(arr[1] == 1){
  		  			  mode = 10000000;
  		  			step4_select(1);
  		  			break;
  		  		  }
  		  if(arr[2] == 1){
  		  			  mode = 5000000;
  		  			step4_select(2);
  		  			break;
  		  		  }
  		  if(arr[3] == 1){
  		  			  mode = 2000000;
  		  			step4_select(3);
  		  			harder = 1;
  		  			break;
  		  		  }
  	  }

  Block *pad1_tmp = block_init(120, 1, 40, 10 , ILI9341_WHITE);
  show_block(pad1_tmp);
  free(pad1_tmp);
  Block *pad1 = block_init(120, 1, 40, 10 , back);
  show_block(pad1);


  Block *pad2_tmp = block_init(120, 309, 40, 10 , ILI9341_WHITE);
  show_block(pad2_tmp);
  free(pad2_tmp);
   Block *pad2 = block_init(120, 309, 40, 10 , back);
   show_block(pad2);

  dx = 1;
  dy = -1;
  x = 0;
  y = 160;

  int x1 = 230;
  int y1 = 160;

  pad1x = 120;
  pad2x = 120;
  pad3x = 120;

  pad1dx = 2;
  pad2dx = 2;
  pad3dx = 1;
  // Initialize scores
  score1 = 3;
  score2 = 3;

  step4(0);
  while (1){
	  arr[0] = 0;
	  arr[1] = 0;
	  arr[2] = 0;
	  arr[3] = 0;
	  getButtonInput();


	  // Paddle 1
	  free(pad1);
	  Block *pad11 = block_init(pad1x, 1, 40, 10 , ILI9341_WHITE);
	  show_block(pad11);


	  if (arr[0] == 1){  // Use PA0 to move pad1 leftward

		  free(pad11);
		  Block *pad1 = block_init(pad1x, 1, 40, 10 , back);
		  show_block(pad1);
		  free(pad1);
		  if (pad1x > 197){
			  pad1x = 199;
			  Block *pad1 = block_init(pad1x, 1, 40, 10 , ILI9341_WHITE);
			  show_block(pad1);
		  }
		  else {
			  pad1x += 2;
			  Block *pad1 = block_init(pad1x, 1, 40, 10 , ILI9341_WHITE);
			  show_block(pad1);
		  }


	  }

	  if (arr[1] == 1){  // Use PA1 to move pad1 rightward

		  free(pad1);
		  Block *pad1 = block_init(pad1x, 1, 40, 10 , back);
		  show_block(pad1);
		  free(pad1);

		  if (pad1x < 2){
			  pad1x = 0;
			  Block *pad1 = block_init(pad1x, 1, 40, 10 , ILI9341_WHITE);
			  show_block(pad1);
		  }
		  else{

			  pad1x -= 2;
			  Block *pad1 = block_init(pad1x, 1, 40, 10 , ILI9341_WHITE);
			  show_block(pad1);
		  }

	  }



	  // Paddle 2
	  free(pad2);
	  Block *pad22 = block_init(pad2x, 309, 40, 10 , ILI9341_WHITE);
	  show_block(pad2);

		  if (arr[2] == 1){  // Use PC0 to move pad2 leftward

			  free(pad22);
			 		  Block *pad2 = block_init(pad2x, 309, 40, 10 , back);
			 		  show_block(pad2);
			 		  free(pad2);

			 		  if (pad2x > 197){
			 			  pad2x = 199;
			 			  Block *pad2 = block_init(pad2x, 309, 40, 10 , ILI9341_WHITE);
			 			  show_block(pad2);
			 		  }
			 		  else{
			 			  pad2x += 2;
			 			  Block *pad2 = block_init(pad2x, 309, 40, 10 , ILI9341_WHITE);
			 			  show_block(pad2);
			 		  }
		  }

		  if (arr[3] == 1){  // Use PC1 to move pad2 rightward

			  free(pad2);
			 		  Block *pad2 = block_init(pad2x, 309, 40, 10 , back);
			 		  show_block(pad2);
			 		  free(pad2);

			 		  if (pad2x < 2){
			 			  pad2x = 0;
			 			  Block *pad2 = block_init(pad2x, 309, 40, 10 , ILI9341_WHITE);
			 			  show_block(pad2);
			 		  }
			 		  else{

			 			  pad2x -= 2;
			 			  Block *pad2 = block_init(pad2x, 309, 40, 10 , ILI9341_WHITE);
			 			  show_block(pad2);
			 		  }
		  }


	  // Ball
	  Block * ball = block_init(x, y, 10, 10 , ILI9341_WHITE);
	  show_ball(ball);
	  free(ball);
	  nano_wait(mode);
	  Block * ballless = block_init(x, y, 10, 10 , back);
	  show_ball(ballless);
	  free(ballless);

	  // Update ball's location
	  x += dx;
	  y += dy;


	  ball_collision_check();
	  //----------------------------------------------------
	  		  //paddle 3
	  if(harder == 1){
	  Block * obs = block_init(pad3x, 160, 80,3, ILI9341_BLACK);
	  show_block(obs);
	  free(obs);
	  nano_wait(mode);
	  Block * obsless = block_init(pad3x, 160, 80, 3, back);
	  show_block(obsless);
	  free(obsless);

	  pad3x += pad3dx;
	  obs_collision_check();}

	  //----------------------------------------------------


	  if(score1 == 0 || score2 == 0){
		  if(score1 == 0){
		  	  wl = 2;
		    }
		  if(score2 == 0){
		  	  wl = 1;
	    }
		  beep();

		    step4_end(wl);
		    break;
	  }

  }

  delay_cycles(20000);


  return 0;
}
