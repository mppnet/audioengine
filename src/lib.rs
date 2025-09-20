use heapless::spsc::Queue;
use js_sys::Uint8Array;
use rustysynth::{SoundFont, Synthesizer, SynthesizerSettings};
use std::cell::RefCell;
use std::io::Cursor;
use std::sync::Arc;
use tinyaudio::prelude::*;
use wasm_bindgen::prelude::*;

enum SynthCommand {
    NoteOn(i32, i32, i32),
    NoteOff(i32, i32),
    Volume(f32),
}

thread_local! {
    static COMMAND_QUEUE: RefCell<Queue<SynthCommand, 256>> = const { RefCell::new(Queue::new()) };
    static DEVICE: RefCell<Option<OutputDevice>> = const { RefCell::new(None) };
}

#[wasm_bindgen]
pub fn is_synth_active() -> bool {
    DEVICE.with(|d| d.borrow().is_some())
}

#[wasm_bindgen]
pub fn init_synth(sf2: Uint8Array, channel_sample_count: usize) {
    let params = OutputDeviceParameters {
        channels_count: 2,
        sample_rate: 44100,
        channel_sample_count,
    };

    let sound_font = Arc::new(SoundFont::new(&mut Cursor::new(sf2.to_vec())).unwrap());
    let mut settings = SynthesizerSettings::new(params.sample_rate as i32);
    settings.maximum_polyphony = 256;
    let mut synth = Synthesizer::new(&sound_font, &settings).unwrap();

    let mut left = vec![0.0f32; params.channel_sample_count];
    let mut right = vec![0.0f32; params.channel_sample_count];

    let device = run_output_device(params, move |data| {
        COMMAND_QUEUE.with(|qcell| {
            let mut q = qcell.borrow_mut();
            while let Some(cmd) = q.dequeue() {
                match cmd {
                    SynthCommand::NoteOn(ch, key, vel) => synth.note_on(ch, key, vel),
                    SynthCommand::NoteOff(ch, key) => synth.note_off(ch, key),
                    SynthCommand::Volume(vol) => synth.set_master_volume(vol),
                }
            }
        });

        synth.render(&mut left, &mut right);
        for (i, v) in left
            .iter()
            .zip(right.iter())
            .flat_map(|(l, r)| [*l, *r])
            .enumerate()
        {
            data[i] = v;
        }
    })
    .unwrap();

    DEVICE.with(|d| d.borrow_mut().replace(device));
}

#[wasm_bindgen]
pub fn set_volume(volume: f32) {
    COMMAND_QUEUE.with(|qcell| {
        let _ = qcell.borrow_mut().enqueue(SynthCommand::Volume(volume));
    });
}

#[wasm_bindgen]
pub fn note_on(channel: i32, key: i32, velocity: i32) {
    COMMAND_QUEUE.with(|qcell| {
        let _ = qcell
            .borrow_mut()
            .enqueue(SynthCommand::NoteOn(channel, key, velocity));
    });
}

#[wasm_bindgen]
pub fn note_off(channel: i32, key: i32) {
    COMMAND_QUEUE.with(|qcell| {
        let _ = qcell
            .borrow_mut()
            .enqueue(SynthCommand::NoteOff(channel, key));
    });
}
