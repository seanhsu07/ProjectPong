# ProjectPong
Project Pong is a project that utilizes the microccontroller STM32 to create a mini Pong video game on the ILI9341 LCD display. 

# Components
The components used are the ILI9341 diaplay, STM32F0 discovery board, CFAL LCD diaplay, and a speaker. Both displays uses SPI communication interface and the CFAL device also uses DMA transfer to diaplay messages. 

# Software
The game runs in an infinite loop and only breaks after one player loses. After the system is powered, the game eneters mode selection and only exits when a mode is selected. Modes are selected by four push buttons (same buttons used for playing the game). The game has four modes: easy, normal, hard, and harder?. The game modes are based on the speed of the ball. In the moder Harder?, a pad is implemented in the middle to increase the difficulty for both players. The DAC output of the microcontroller produces a short beep with the speaker after one player scores, and a series of sound after the game ends. The game stays in gameover state until the system is reset. 

# Peripherals Used
- GPIO
- Timers
- DAC
- DMA
- SPI
