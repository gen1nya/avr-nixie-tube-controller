#include "avr/io.h"
#include "avr/interrupt.h"

#define F_CPU 16000000L

#define baudrate 57600L
#define bauddivider (F_CPU/(16*baudrate)-1)
#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)

#define ANODES_PORT PORTB
#define DECODER_PORT PORTC

#define COMMAND_MODE 0
#define SET_DATA_MODE 1
#define SET_BRIGHTNESS_MODE 2

#define DEFAULT_BRIGHTNESS 254
#define DIGITS 6

uint8_t uartDataBytesCounter = 0;
uint8_t mode = COMMAND_MODE;
uint8_t currentElement = 0;
uint8_t display[DIGITS];
uint8_t brightness[DIGITS];

int main() {

  DDRC = 0b00001111; // 0-3 for К155ИД1 bcd to decimal decoder
  DDRB = 0b11111111; // for anodes

  cli();

  for (uint8_t i = 0; i < DIGITS; i++) {
    display[i] = 0;
    brightness[i] = DEFAULT_BRIGHTNESS;
  }

  TCCR2B = 1<<CS20 | 1<<CS21;
  TIMSK2 = 1<<OCIE2A | 1<<TOIE2;
  OCR2A = DEFAULT_BRIGHTNESS;
  
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
        case SET_DATA_MODE: {
          mode = SET_DATA_MODE;
          uartDataBytesCounter = 0;
          break;
        }

        case SET_BRIGHTNESS_MODE: {
          mode = SET_BRIGHTNESS_MODE;
          uartDataBytesCounter = 0;
          break;
        }
        
        default:
          break;
      }
  } else {
    switch (mode) {
      case SET_DATA_MODE: {
        display[uartDataBytesCounter] = UDR0;
        uartDataBytesCounter++;
        if (uartDataBytesCounter >= DIGITS) {
          uartDataBytesCounter = 0;
          mode = COMMAND_MODE;
        }
        break;
      }

      case SET_BRIGHTNESS_MODE: {
        brightness[uartDataBytesCounter] = UDR0;
        uartDataBytesCounter++;
        if (uartDataBytesCounter >= DIGITS) {
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
  DECODER_PORT = display[currentElement];
  ANODES_PORT = 1 << currentElement;
  if (++currentElement >= DIGITS) currentElement = 0;
  OCR2A = brightness[currentElement];
}

ISR (TIMER2_COMPA_vect) {
   ANODES_PORT = 0x00;
   DECODER_PORT = 11;
}