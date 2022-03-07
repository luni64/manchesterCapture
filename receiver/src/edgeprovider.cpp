#include "edgeprovider.h"

void EdgeProvider::init()
{
    *(portConfigRegister(11)) = 1;                           // ALT1, use pin 11 as input to TMR1_2
                                                             //
    ch->CTRL  = 0;                                           // stop timer
    ch->SCTRL = TMR_SCTRL_CAPTURE_MODE(3) | TMR_SCTRL_IEFIE; // both edges, enable edge interrupt
    ch->SCTRL |= TMR_SCTRL_IEFIE;                            // enable input edge flag interrupt
    ch->LOAD = 0;

    attachInterruptVector(IRQ_QTIMER1, onCapture);
    NVIC_ENABLE_IRQ(IRQ_QTIMER1);
    NVIC_SET_PRIORITY(IRQ_QTIMER1, 32); 

    ch->CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + 3) | TMR_CTRL_SCS(2); // start, source: peripheral clock, prescaler 3 (=> dt = 1/150Mhz * 8 = 53ns resolution, 2^15 * 53ns = 3.5ms max), use counter 2 input pin for capture
}

//------------------------------------------------------------------------------------------------
// called on each detected input pin edge
//
void EdgeProvider::onCapture()
{
    digitalWriteFast(9, HIGH);

    ch->SCTRL = TMR_SCTRL_CAPTURE_MODE(3) | TMR_SCTRL_IEFIE; // faster than selectively clearing interrupt flag

    uint16_t capture = ch->CAPT;                     // read out captured counter value
    uint32_t dt      = (uint16_t)(capture - oldCap); // time since last edge
    oldCap           = capture;                      // prepare next capture
    buff.push(dt);                                   // ringbuffer is very simple, make sure to read out before it overflows

    digitalWriteFast(9, LOW);
}

RingBuffer EdgeProvider::buff;
uint16_t EdgeProvider::oldCap = 0;