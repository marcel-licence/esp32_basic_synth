# esp32_basic_synth
ESP32 based simple synthesizer project

The project can be seen in my video https://www.youtube.com/watch?v=5XVK5MOKmZw

---
The project has been tested on 
- ESP32 Audio Kit V2.2 (using ESP32-A1S)
- ESP32 DEVKIT - DOIT (actually only supporting esp32\1.0.4 library)

# ESP32 Audio Kit V2.2
To compile set board to: ESP32 Dev Module
Ensure "#define ESP32_AUDIO_KIT" is set in config.h
IO22 will be used for MIDI in.

The ADC multiplexer can be used (other wise please remove #define ADC_TO_MIDI_ENABLED from config.h)
Connection of the ADC multplexer:
- EN -> Ground
- S0 -> IO23
- S1 -> IO18
- S2 -> IO14
- S3 -> Ground
- Sig -> IO12

# ESP32 DEVKIT - DOIT
To compile set the board to: DOIT ESP32 DEVKIT V1
Ensure that "#define ESP32_AUDIO_KIT" has been removed from config.h

ADC has not been tested with this configuration (please remove #define ADC_TO_MIDI_ENABLED)

An external audio DAC is recommended for this setup:
- BCLK -> IO25
- WLCK -> IO27
- DOUT -> IO26

Connection of the ADC multplexer:
- EN -> Ground
- S0 -> IO33
- S1 -> IO32
- S2 -> IO13
- S3 -> Ground
- Sig -> IO12

# ADC Mapping
The adc module has been only tested with the ESP32 Audio Kit V2.2.
In z_config.ino you can define your own mapping. Actually only 8 channels are read from the multiplexer.
The adc lookup is used to define a channel and cc per analog input (C0..C7).
By changing adc values a MIDI messages will generated internally. 
It should be also mapped int the MIDI mapping.

# MIDI Mapping
A controller mapping can be found in z_config.ino. 
You can define there your own controller mapping if your controller does support CC messages.

---
If you have questions or ideas please feel free to use the discussion area!

---