# audioengine for mppnet

this is a c++ (compiled to wasm) application which is a supposed replacement for the current AudioEngineWeb that's in multiplayerpiano.  

it can reliably play most black midis and normal midis with resonable performance with absolutely no cracking or artifacting.

this is very heavily in development still, and is not a final version at all.

we are mainly missing all Synth stuff, and audio playback is only mono.

uses [TinySoundFont](https://github.com/schellingb/TinySoundFont) and [emscripten](https://emscripten.org).


## build
use (with emsdk activated and sourced)
```
mkdir -p build

emcc -std=c++11 -lembind -s ALLOW_MEMORY_GROWTH -s EXPORTED_FUNCTIONS="['_malloc', '_free']" -s AUDIO_WORKLET=1 -s WASM_WORKERS=1 -O3 -o build/audioengine.js audioengine.cpp 
```

to compile into `build`