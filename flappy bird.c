#include "msp.h"
#include <stdio.h>

#define UART_TX_PIN BIT2
#define BAUD_RATE 9600

volatile unsigned char birdPosition = 8;
volatile unsigned char pipePosition = 16;
volatile unsigned char gameover = 0;
volatile unsigned int score = 0;

void UART_init(void);
void UART_putchar(char c);
void UART_puts(const char* str);
void UART_println(const char* str);

void handle_input(void);
void game_logic(void);
char buffer[20];
int main(void)
		
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // Stop watchdog timer

    UART_init();    // Initialize UART
		
		P1->SEL1 &= ~2;
		P1->SEL0 &= ~2;
		P1->DIR  |=  2;
		P1->REN  |=  2;
		P1->OUT  |=  2;

    while (1)
    {
        handle_input();
        game_logic();

        // Update the game state via UART
        
        sprintf(buffer, "B%d P%d S%d G%d\n", birdPosition, pipePosition, score, gameover);
        UART_puts(buffer);

        __delay_cycles(100000);
    }
}

void UART_init(void)
{
    P1->SEL0 |= UART_TX_PIN;
    P1->SEL1 &= ~UART_TX_PIN;
    P1->DIR |= UART_TX_PIN;

    // Configure UART for 9600 baud rate
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST;   // Put eUSCI in reset
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SSEL__SMCLK;  // SMCLK as clock source
    EUSCI_A0->BRW = 78;     // 3,000,000 / 9600 = 312.5
    EUSCI_A0->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS) | EUSCI_A_MCTLW_OS16;  // UCBRx = 312, UCBRFx = 2, UCBRSx = 0, UCOS16 = 1
    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;  // Initialize eUSCI

    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;   // Clear RX interrupt flag
}

void UART_putchar(char c)
{
    while (!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));  // Wait for transmit buffer to be ready
    EUSCI_A0->TXBUF = c;    // Send character
}

void UART_puts(const char* str)
{
    while (*str)
    {
        UART_putchar(*str++);
    }
}

void UART_println(const char* str)
{
    UART_puts(str);
    UART_putchar('\n');
}

void handle_input(void)
{
		
    // Simulate button press (change the condition based on your input mechanism)
    if (P1->IN & 2)
    {
        if (!gameover)
        {
            // Move the bird upwards
            if (birdPosition > 0)
                birdPosition--;
        }
        else
        {
            // Reset the game if it is already over
            gameover = 0;
            score = 0;
        }
    }
}

void game_logic(void)
{
    if (!gameover)
    {
        // Move the pipes to the left
        if (pipePosition > 0)
            pipePosition--;
        else
        {
            // If the pipes reach the left edge, reset their position and increment the score
            pipePosition = 16;
            score++;
        }

        // Check for collision
        if (pipePosition <= birdPosition && birdPosition <= pipePosition + 2)
            gameover = 1;  // Set game over flag
    }
}
