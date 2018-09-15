# LED FFT Music Analyzer

Software for a spectrum analyzer which is capable of sampling audio at a high frequency while also driving a string of 256 Neopixel LEDs. The SGTL5000 Teensy audio shield is used to do all of the A/D conversion, sampling, and audio in/out. Fourier transforms are then performed on the audio signal and the frequency values are used to drive a percentage of the LEDs in each column.

The LEDs are supposed to be wired in a continuous strand, so LEDs in the odd-numbered columns are lit from top to bottom, and LEDs in the even-numbered columns are lit from bottom to top.