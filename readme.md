# audioengine for mppnet

this is a c++ (compiled to wasm) application which is a supposed replacement for the current AudioEngineWeb that's in multiplayerpiano.  

it can reliably play most black midis and normal midis with resonable performance with absolutely no  artifacting.

this is very heavily in development still, and is not a final version at all. 

uses [TinySoundFont](https://github.com/schellingb/TinySoundFont) and [emscripten](https://emscripten.org).

[Video - freedom dive black midi](https://files.sad.ovh/public/2024-12-28%2017-40-26.mp4)

## bugs
1. The only major bug is the fact that it's still mono! There's a ongoing TSF issue, which I've reported is still happening.
2. Also, higher than 100-ish voice count crackles (we're using 200 voices for better blackmidi playback).. I have no clue how to fix this, maybe it's just a issue with how small the sample size for webaudio is (128 samples??).

## build
use (with emsdk activated and sourced)
```
mkdir -p build

emcc -std=c++11 -lembind -s ALLOW_MEMORY_GROWTH -s EXPORTED_FUNCTIONS="['_malloc', '_free']" -s AUDIO_WORKLET=1 -s WASM_WORKERS=1 -O3 --emit-tsd audioengine.ts.d -o build/audioengine.js audioengine.cpp
```

to compile into `build`