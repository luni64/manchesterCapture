#include "edgeproviderDMA.h"
#include "pins_Arduino.h"
#include <cstdlib>

namespace Manchester
{
    constexpr size_t bufSize = 256;                                         // DMA buffer for edge timestamps (512 bytes, 256 timestamps)
    uint16_t buf[bufSize] __attribute__((aligned(512)));                    // The DMA controller will replace the lowest n-bits of the address by a counter
                                                                            // to implement the circular buffer -> we need to align the start address of the buffer
    void EdgeProviderDMA::init()                                            // such that it corresponds to a countervalue of 0
    {                                                                       //
        *(portConfigRegister(11)) = 1;                                      // ALT1, use pin 11 as input to TMR1_2
                                                                            //
        ch->CTRL  = 0;                                                      // stop timer
        ch->SCTRL = TMR_SCTRL_CAPTURE_MODE(3);                              // both edges, enable edge interrupt
        ch->LOAD  = 0;                                                      // reload the counter with 0 at rollover (doesn't work without setting this explicitely)
        ch->DMA   = TMR_DMA_IEFDE;                                          // DMA on capture events
        ch->CTRL  = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + 3) | TMR_CTRL_SCS(2); // start, source: peripheral clock, prescaler 3 (=> dt = 1/150Mhz * 8 = 53ns resolution, 2^15 * 53ns = 3.5ms max), use counter 2 input pin for capture
                                                                            //
        dmaChannel.begin();                                                 //
        dmaChannel.triggerAtHardwareEvent(DMAMUX_SOURCE_QTIMER1_READ2);     // trigger DMA by capture event on channel 2
        dmaChannel.source(ch->CAPT);                                        // DMA source = capture register (16 bit)
        dmaChannel.destinationCircular(buf, bufSize * sizeof(uint16_t));    // use a circular buffer as destination. Buffer size in bytes
        dmaChannel.enable();
    }

    uint16_t EdgeProviderDMA::popTimestamp()
    {
        return buf[tail++];
    }

    bool EdgeProviderDMA::hasElements()
    {
        return dmaChannel.destinationAddress() != (buf + tail);
    }

    DMAChannel EdgeProviderDMA::dmaChannel;
    uint8_t EdgeProviderDMA::tail = 0;

}