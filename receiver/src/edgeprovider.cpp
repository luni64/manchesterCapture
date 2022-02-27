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
    NVIC_SET_PRIORITY(IRQ_QTIMER1, 16);

    ch->CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + 3) | TMR_CTRL_SCS(2); // start, source: peripheral clock, prescaler 3 (=> dt = 1/150Mhz * 8 = 53ns resolution, 2^15 * 53ns = 3.5ms max), use counter 2 input pin for capture
}

//------------------------------------------------------------------------------------------------
// called on each detected input pin edge
//
void EdgeProvider::onCapture()
{
    digitalWriteFast(9, HIGH);

    ch->SCTRL &= ~TMR_SCTRL_IEF; // clear interrupt flag
    uint16_t curCap = ch->CAPT;  // read out captured counter value
    uint16_t input  = ch->SCTRL & TMR_SCTRL_INPUT;
    uint16_t delta  = curCap - oldCap;
    uint32_t edge   = delta | (input << 16); // read out current input pin value and store in HIGH WORD
    buffer.push(edge);                       //
    oldCap = curCap;
    // prepare next capture
    digitalWriteFast(9, LOW);
}

EdgeBuffer EdgeProvider::buffer;
uint16_t EdgeProvider::oldCap = 0;