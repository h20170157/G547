
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "usbdrv.h"

typedef enum _command    // user defined request used for various pin function 
{
   BOARD_INIT, // This does the init of board
   PINMODE_INPUT, // Set GPIO as input
   PINMODE_OUTPUT, // Set GPIO as output
   PIN_READ,   // Read GPIO
   PIN_WRITE, // Write to GPIO
} command;
static uint8_t replybuf[6];

static inline uint8_t gpio_setting(uint8_t no)   // setting to adjust PD2 AND PD4 as they are connected to D+ and D-
{
   if (no == 3) no = 4;
   else if (no == 4) no = 6;
   else if (no == 5) no = 7;
   else if (no == 6) no = 8;
   return no;
}
 // FUNCTION FOR SETTING PIN DIRECTION AS INPUT OR OUTPUT
static void gpio_init(uint8_t no, uint8_t input)
{
	// setting pin of PORTD
	if (no > 2)
	   no = gpio_setting(no);
   
     switch (no)
     {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
         if (!input)
           DDRD |= (1 << (no - 1));  // SET AS OUTPUT
         else
           DDRD &= ~(1 << (no - 1)); //SET AS INPUT
         break;
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
         if (!input)
           DDRB |= (1 << (no - 7)); // SET AS OUTPUT
         else
           DDRB &= ~(1 << (no - 7)); // SET AS INPUT
         break;
        case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
 	 if (!input)
           DDRC |= (1 << (no - 13)); // SET AS OUTPUT
         else
           DDRC &= ~(1 << (no - 13)); // SET AS INPUT
         break;
      default:
         break;
     }
}

// FUNCTION FOR SETTING AND READING THE PIN STATUS (ARG1: GPIO PIN NUMBER, ARG2:READ/WRITE,HIGH/LOW)
static void gpio_high_low(uint8_t no, uint8_t write, uint8_t *val)
{ 
	if (no > 2)
           no = gpio_setting(no);
   switch(no)
     {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
         if (write)   // SETTING THE PIN
           {
              if (*val)
                PORTD |= (1 << (no - 1));
              else
                PORTD &= ~(1 << (no - 1));
           }
         else  // READING THE PIN
           {
              if (bit_is_clear(PIND, (no - 1)))
              {  *val = 0;
         	 replybuf[3]=0;
		}     
	 else
	{
                *val = 1;
		 replybuf[3]=1;	
	}         
	  }
         break;
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
         if (write)
           {
              if  (*val)
                PORTB |= (1 << (no - 7));
              else
                PORTB &= ~(1 << (no - 7));
           }
         else
           {
              if (bit_is_clear(PINB, (no - 7)))
                {
                *val = 0;
		 replybuf[3]=0;	
	}
              else
               {
                *val = 1;
		 replybuf[3]=1;	
	}
           }
         break;
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
         if (write)
           {
              if  (*val)
                PORTC |= (1 << (no - 13));
              else
                PORTC &= ~(1 << (no - 13));
           }
         else
           {
              if (bit_is_clear(PINC, (no - 13)))
               {
                *val = 0;
		 replybuf[3]=0;	
	}
              else
               {
                *val = 1;
		 replybuf[3]=1;	
	}
           }
         break;
      default:
         break;
     }
}


usbMsgLen_t usbFunctionSetup(uchar data[8])
{
   usbRequest_t *rq = (void *)data;
   uint8_t len = 0;

   replybuf[0] = rq->bRequest;

   switch(rq->bRequest)
     {
// BOARD_INIT CALLED WHEN USB DEVICE IS CONNECTED
      case BOARD_INIT:

         
         len = 1;
         
         break;      
// SETTING PIN AS INPUT
      case PINMODE_INPUT:
         replybuf[1] = rq->wValue.bytes[0]; // gpio no
         gpio_init(replybuf[1], 1);

         len = 2;
         break;
// SETTING PIN AS OUTPUT
      case PINMODE_OUTPUT:
         replybuf[1] = rq->wValue.bytes[0]; //gpio no
         gpio_init(replybuf[1], 0);

         len = 2;
         break;
// READING PIN AS low
      case PIN_READ:
         replybuf[1] = rq->wValue.bytes[0]; // gpio no
	 //replybuf[2] = rq->wValue.bytes[1]; // gpio value
         gpio_high_low(replybuf[1], 0, &replybuf[2]); 
         len = 3;
         break;
//SETTING PIN AS high
      case PIN_WRITE:
         replybuf[1] = rq->wValue.bytes[0]; //gpio no.
         replybuf[2] = rq->wValue.bytes[1]; // gpio value
         gpio_high_low(replybuf[1], 1, &replybuf[2]);

         len = 3;
         break;

      default:
         break;
     }

   usbMsgPtr = (unsigned char *) replybuf;

   return len; 
}

int __attribute__((noreturn))
main(void)
{
   uchar i = 0;

   wdt_enable(WDTO_2S);
   usbInit();
   usbDeviceDisconnect();

   while(--i)
     {
        wdt_reset();
        _delay_ms(1);
     }

   usbDeviceConnect();
   sei();

   while(1)
     {
        wdt_reset();
        usbPoll();
     }
}
