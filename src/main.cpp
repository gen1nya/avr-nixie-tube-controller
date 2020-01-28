#include "avr/io.h"
#include "avr/interrupt.h"

#define F_CPU 16000000L

#define baudrate 9600L
#define bauddivider (F_CPU/(16*baudrate)-1)
#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)

#define COMMAND_MODE 0

#define SET_DATA 1
#define SET_BRIGHTNESS 2

uint8_t uartDataBytesCounter = 0;
uint8_t mode = COMMAND_MODE;
uint8_t currentElement = 0;
uint8_t display[6] = {1, 2, 3, 4, 5, 6 };
uint8_t brightness[6] = { 245, 245, 245, 245, 245, 254 };

int main() {

  DDRB = 0b00001111; // 0-3 for К155ИД1 bcd to decimal decoder
  DDRD = 0b11111110; // 0 for uart rx, 1 for uart tx, 2-7 for anodes

  cli();

  TCCR2B = 1<<CS20 | 1<<CS21;
  TIMSK2 = 1<<OCIE2A | 1<<TOIE2;
  OCR2A = 245;
  
  UBRR0L = LO(bauddivider);
  UBRR0H = HI(bauddivider);
  UCSR0A = 0;
  UCSR0B = 1<<RXEN0 | 1<<RXCIE0 | 1<<TXEN0;
  UCSR0C = 1<<UCSZ00 | 1<<UCSZ01;
  
  sei();

  while(true) {
    // do nothing
  }
  return 0;
}

ISR (USART_RX_vect) {
  if (mode == COMMAND_MODE) {
      switch (UDR0) {
        case SET_DATA: {
          mode = SET_DATA;
          uartDataBytesCounter = 0;
          break;
        }

        case SET_BRIGHTNESS: {
          mode = SET_BRIGHTNESS;
          uartDataBytesCounter = 0;
          break;
        }
        
        default:
          break;
      }
  } else {
    switch (mode) {
      case SET_DATA: {
        display[uartDataBytesCounter] = UDR0;
        uartDataBytesCounter++;
        if (uartDataBytesCounter >= 6) {
          uartDataBytesCounter = 0;
          mode = COMMAND_MODE;
        }
        break;
      }

      case SET_BRIGHTNESS: {
        brightness[uartDataBytesCounter] = UDR0;
        uartDataBytesCounter++;
        if (uartDataBytesCounter >= 6) {
          uartDataBytesCounter = 0;
          mode = COMMAND_MODE;
        }
        break;
      }

      default: {
        mode = COMMAND_MODE;
        uartDataBytesCounter = 0;
        break;
      }
          
    }
  }
}

ISR (TIMER2_OVF_vect) {
  PORTB = display[currentElement];
  PORTD = (1<<(currentElement + 2));
  if (++currentElement > 6) currentElement = 0;
  OCR2A = brightness[currentElement];
}

ISR (TIMER2_COMPA_vect) {
   PORTD = 0x00;
   PORTB = 11;
}