# manchesterCapture
Experimental Manchester Decoder,

2022-03-07:
Changed the sender to generate TS5643 data fields at 2MHz. The receiver code is running stably at 2MHz. Some high level interrupts can still delay the edgeProvider interrupt so that an edge might be missed. This happens mainly when Serial printing in the foreground. The error recovery is able to detect this. CRC not yet implemented.
DMA to be tested in next version

2022-03-08:
Implemented a first **DMA based version** . This version runs absolutely stable. No errors detected so far. The DMA controller writes the 16bit timestamps into a 512 byte circular buffer. This buffer is a bit small and needs very frequent calls to decoder.tick() to avoid overruns. Increased buffer size in next version.



