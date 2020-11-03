# attiny-stuff

This is my repository of C code written for the DigiSpark development board.

It's highly disorganized and written primarily from an experimental point of view. Capabilities tested so far:

1. Ability to allocate arrays of varying sizes (some instability sets in at ~450 bytes due to stack overflow(?))
2. FP32 arithmetic (purely software based), chip is currently incompatible with FP64 math.
3. PWM signals to drive a passive buzzer.
4. Incorporating the V-USB library for communication with a host device over USB/1.1.
5. Timer, pin change and general interrupts.
