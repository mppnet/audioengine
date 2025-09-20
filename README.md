# AudioEngine for MPP
This is a Rust (compiled to wasm via wasm-pack) application which is a supposed replacement for the current AudioEngineWeb that's in MPP.

It can reasonably play medium sized black midis, and sounds great on less-intensive midis aswell.

Uses [RustySynth](https://github.com/sinshu/rustysynth) and [tinyaudio](https://github.com/mrDIMAS/tinyaudio).

This is a deep rewrite of the original AudioEngine (that was C++). That version had so many different issues, and the main two ones were the fact that there was no Stereo output, and the fact that audio artifacting happened at no matter what volume. You can hear, even in the original example video there were lots of pops/crackles. That no longer exists in this version..

## Issues
We're very limited by polyphony on RustySynth. This can handle high NPS midis, but high polyphony midis (256+) will start to sound horrible.

## Build & package
```
wasm-pack --target web
./package.sh
```
