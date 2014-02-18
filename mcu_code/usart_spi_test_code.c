/* Author: Tuan Truong
 * Date: 2/12/14
 * Brief: use spi to read button input from shift register
 * display button inputs via corresponding LEDs
 * Send button input via USART to serial terminal
 * Goal is to prove SPI and USART interface works
 */

/*****************************************************/
/* Hardware description:
 * LEDs - P2[4-7]
 * SPI - UCB1
 *      TE - p4.0 (UCB1STE)
 *      MOSI - P4.1 (UCB1SIMO)
 *      MISO - p4.2 (UCB1SOMI)
 *      CLK - p4.3 (UCSB1CLK)
 * USART - UCA1
 *      TX - p4.4 (UCA1TXD)
 *      RX - p4.5 (UCA1RXD)
 * SHIFT REGISTER
 *      SHIFT LATCH - p1.0
 *      SHIFT_INH - p4.0
 */
/*****************************************************/

//includes
#include "msp430.h"

//macros
#define LED_PORT P2OUT
#define LED1 7
#define LED2 4
#define LED3 5
#define LED4 6

#define SPI_STE 0
#define SPI_MOSI 1
#define SPI_MISO 2
#define SPI_CLK 3

#define USART_TX 3
#define USART_RX 4

#define SHIFT_LATCH 0
#define SHIFT_INH 0


unsigned char temp;

void dir_setup(void){
  P2DIR |= 0xF0;
  P4DIR |= (1<<SPI_STE)|(1<<SPI_MOSI)|(1<<SPI_CLK);
  P4DIR &= ~(1<<SPI_MISO);
  P1DIR |= (1<<SHIFT_LATCH);
  P3DIR |= 1<<USART_TX;
  P3DIR &= ~(1<<USART_RX);
}

void delay(int time){
  unsigned int i;
  i = time * 1000;                          // SW Delay
  do i--;
  while (i != 0);
}

void led_startup(void){
  P2OUT &= 0x0F;
  delay(2);
  P2OUT = (1<<LED1);
  delay(2);
  P2OUT = (1<<LED2);
  delay(2);
  P2OUT = (1<<LED3);
  delay(2);
  P2OUT = (1<<LED4);
  delay(2);
  P2OUT = 0x00;
  delay(2);
  P2OUT = 0xF0;
  delay(2);
  P2OUT = 0x00;
  delay(2);
  P2OUT = 0xF0;
  delay(2);
  P2OUT = 0x00;
  
}

void spi_setup(void){
  P4SEL |= BIT1+BIT2+BIT3;
  //Enable software reset
  UCB1CTL1 |= UCSWRST;
  
  //sample on rising edge, MSB, 3 wire asynchronous SPI
  UCB1CTL0 |=  UCMSB | UCMST;
  //CLK select
  UCB1CTL1 |= UCSSEL_2;
  //bit prescale (TODO: look in to this)
  UCB1BR0 = 0xff;
  UCB1BR1 = 0;
  
  UCB1CTL1 &= ~UCSWRST;
  UCB1IE |= UCRXIE;
  
  
  
  return;
}

void spi_send(char data){
  UCB1TXBUF = data;
  LED_PORT ^=0xF0;
  LPM0;
  while((UCB1STAT & UCBUSY));
  return;
}

void usart_setup(void){
  P3SEL |= BIT3+BIT4;                       // P3.3,4 = USCI_A0 TXD/RXD
  UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 9;                              // 1MHz 115200 (see User's Guide)
  UCA0BR1 = 0;                              // 1MHz 115200
  UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interruptu
  
  return;
}


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
  dir_setup();
  led_startup();
  spi_setup();
  usart_setup();
  
  while (!(UCB1IFG&UCTXIFG));               // USCI_B1 TX buffer ready?
  UCB1TXBUF = 0xaa;  
  __bis_SR_register(LPM0_bits+GIE);
  /*
  while(1){
      spi_send(0xaa);
      delay(3);
  }
  */
  __no_operation();
}

#pragma vector=USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void){
  //UCB0IE &= ~UCRXIE;
  switch(__even_in_range(UCB1IV,4)){
  case 0:
    break;
  case 2:
    P1OUT &= ~(1<<SHIFT_LATCH);
    delay(1);
    P1OUT |= 1<<SHIFT_LATCH;
    P4OUT &= ~(1<<SHIFT_INH);
    while(!(UCB1IFG & UCTXIFG));
    UCB1TXBUF = 0x00;
    temp = UCB1RXBUF<<3;
    P2OUT = temp;
    delay(1);
    P1OUT |= 1<<SHIFT_LATCH;
    P4OUT |= 1<<SHIFT_INH;
    while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    UCA0TXBUF = temp;                  // TX -> RXed character
    //delay(1);
    break;
  case 4:
    break;
  default:
    break;
  //__bic_SR_register_on_exit(LPM0_bits);
  }
}


#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  //LED_PORT |= 0xF0;
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG

    while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    UCA0TXBUF = temp;                  // TX -> RXed character
    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}