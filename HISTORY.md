## 0.12.0

New Features:
  * Internal send and return plugins
  * Stereo mixer internal plugin
  * Sample delay internal plugin
  * Multi channel support in peak meter plugin
  * Timing and transport notifications
  * Master limiter
  * Updated VST SDK to v3.7.3
  * Updated Twine to v0.2.1
  * Build support for GCC10

Fixes:
  * gRPC segfault on exit
  * Refactored plugin instantiation architecture
  * Track noise for tracks with no audio input
  * Bug where events outputted before the audio process callback were lost
  * Race condition in OSC path registration

## 0.11.0

New Features:
  * Expanded gRPC control interface, including push-style notifications
  * Bumped recommended gRPC version
  * Dynamic loading and routing of tracks and plugins
  * Configurable OSC parameter output
  * Wav writer plugin
  * Mono summing plugin
  * Improved peak meter plugin

Fixes:
  * Aftertouch messages not forwarded in Alsa midi frontend
  * Ensuring silence when track gain is 0
  * Fix for generate script when python2.7 is missing
  * Fix to accommodate v1.16 and v1.24 of gRPC libraries
  * Raspa frontend initialisation order fix
  * Internal event system refactor
  * Controller class refactor and split into sub-controllers
  * Logging library built statically - faster compile time
  * Fix for timing sensitive unit tests
  * Audio routing bug for mono tracks

## 0.10.3

Fixes:
  * Mono tracks can now be connected to stereo output busses
  * OSC paths for gain and pan controls on tracks work again

## 0.10.2

Fixes:
  * LV2 parameter handling fix for non-sequential parameter ids

## 0.10.1

New Features:
  * LV2 worker thread extension support
  * Parameter values are now always normalised

Fixes:
  * LV2 string parameter value fix

## 0.10.0

New Features:
  * LV2 plugin support
  * Ableton Link support
  * Multiple midi input and output ports
  * Extended OSC control API making it more similar to gRPC

Fixes:
  * Updated example configuration files

## 0.9.1

New Features:
  * Plugin information dump with CL switch
  * Updated VST 3 SDK to latest 3.14

Fixes:
  * Allows plugin parameters with duplicate names
  * Events section not required anymore with any frontend

## 0.9.0

New Features:
  * CV and Gate support + related example plugins
  * Step sequencer example plugin
  * gRPC listening address configurable through command line

Fixes:
  * Parameter Units now passed correctly
  * Faster Json file loading
  * Better Json parsing error printing
  * Removed Raspalib submodule
  * Unit test fixes

## 0.8.0

New Features:
  * gRPC control API
  * Automatic clip detection on master tracks
  * Smooth track parameter controls at audio rate
  * Simple MIDI transposer internal plugin
  * Support relative mode for MIDI CCs mapping
  * (x86 / XMOS) : support in RASPA for USB Midi gadget
  * VST 2.x support is now an optional build feature
  * Support VST 3 programs
  * Added an option to automatically flush the logs at a periodic interval

Fixes:
  * TWINE: Automatic denormal handling in all threads
  * Raspa: fix for affinity of non-RT threads after initialization
  * Raspa on x86/XMOS: fixes for spurious startup synchronization
  * MIDI: explicitly convert NoteON with zero velocity to NoteOFF messages
  * Various VST 3 fixes
  * Log file can now be specified at any location
  * VST 2.x : allow host callback functions to be called during plugin initialization

## 0.7.0

  * Multicore track rendering through Twine library
  * Processor performance timings statistics
  * Dummy frontend for investigating RT safety issues in plugins

## 0.6.2

  * Fix RASPA plugin initialization issue
  * Fix parameter output from VST 2.x plugins

## 0.6.1

  * Fix: handling of track pan for multichannel plugins
  * Fix: erroneous channel handling for offline frontend

## 0.6

  * Multichannel track/buses architecture and support for VST multichannel plugins
  * Handling of events and MIDI outputs from plugins
  * Time information handling
  * Scheduling of asynchronous tasks from internal plugins
  * Arpeggiator and peak meter internal plugins
  * Many fixes and improvements to RASPA frontend and VST wrappers

## 0.5.1

  * RASPA Xenomai frontend
  * Multichannel I/O from audio frontends

## 0.5

  * VST 3.6 wrapper
  * Dynamic plugin loading

## 0.4

  * VST 2.4 wrapper
  * Xenomai offline frontend
  * More advanced MIDI routing and mapping

## 0.3

  * JACK audio frontend
  * MIDI support
  * OSC control

## 0.2

  * Events handling
  * Internal plugins (gain, eq, sampler)

## 0.1

  * Initial version
  * Offline frontend with I/O with libsndfile
