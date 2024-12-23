#include <emscripten/bind.h>
#include <emscripten/webaudio.h>
#include <thread>
#include <future>
#include <chrono>

#define TSF_IMPLEMENTATION
#include "tsf/tsf.h"

namespace emscrijpten {
    typedef std::function<void()> callback_t;
    void async_call_callback(void * arg) {
        callback_t * callback = (callback_t *)arg;
        (*callback)();
        delete callback;
    }
    void async_call(callback_t callback, int millis = 0) {
        callback_t * _callback = new callback_t(callback);
        emscripten_async_call(async_call_callback, _callback, millis);
    }

}

static tsf* g_TinySoundFont;


static bool run;
static int volume = 100;

static bool renderAudio(int numInputs, const AudioSampleFrame *inputs,
                      int numOutputs, AudioSampleFrame *outputs,
                      int numParams, const AudioParamFrame *params,
                      void *userData)
{
	for(int i = 0; i < numOutputs; ++i) {
		tsf_render_float(g_TinySoundFont, outputs[i].data, outputs[i].samplesPerChannel*outputs[i].numberOfChannels, 0);
	}

	return true;
}

void load(uintptr_t data, int length) {
    printf("hello world %d\n", length);
    auto array = reinterpret_cast<uint8_t *>( data );
  
    g_TinySoundFont = tsf_load_memory(array, length);
}

void destroy() {
	tsf_close(g_TinySoundFont);
}

void end() {
	run = false;
}
void note_on(int presetId, int key, float velocity, int delay) {
  tsf_note_on(g_TinySoundFont, presetId, key, velocity);
}

void note_off(int presetId, int key, float velocity, int delay) {
  tsf_note_off(g_TinySoundFont, presetId, key);
}

void set_volume(int vol) {
  // nulled out
}

int get_volume() {
  // always 100
	return 100;
}

uint8_t audioThreadStack[4096];

bool OnClick(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
  printf("canvas clicked\n");

  EMSCRIPTEN_WEBAUDIO_T audioContext = (EMSCRIPTEN_WEBAUDIO_T)userData;
  if (emscripten_audio_context_state(audioContext) != AUDIO_CONTEXT_STATE_RUNNING) {
    emscripten_resume_audio_context_sync(audioContext);
  }
  return false;
}

void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData)
{
  if (!success) return; // Check browser console in a debug build for detailed errors
  printf("pressor created\n");

  int outputChannelCounts[1] = { 1 };
  EmscriptenAudioWorkletNodeCreateOptions options = {
    .numberOfInputs = 0,
    .numberOfOutputs = 1,
    .outputChannelCounts = outputChannelCounts
  };

  // Create node
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext,
                                                            "noise-generator", &options, &renderAudio, 0);

  // Connect it to audio context destination
  emscripten_audio_node_connect(wasmAudioWorklet, audioContext, 0, 0);
  emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, (void*)audioContext, 0, OnClick);
}

void AudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData)
{
  if (!success) return; // Check browser console in a debug build for detailed errors
  printf("thread initialized\n");

  WebAudioWorkletProcessorCreateOptions opts = {
    .name = "noise-generator",
  };
  emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, &AudioWorkletProcessorCreated, 0);
}

int start()
{
    printf("we started\n");
    
    tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, 22000, 0);
    tsf_set_max_voices(g_TinySoundFont, 40);
    EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(0);
    emscripten_start_wasm_audio_worklet_thread_async(context, audioThreadStack, sizeof(audioThreadStack),
                                                   &AudioThreadInitialized, 0);
												   
	return 0;
}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("start", start);
    emscripten::function("load", load, emscripten::allow_raw_pointers());
    emscripten::function("end", end);
    emscripten::function("destroy", destroy);
    emscripten::function("note_off", note_off);
    emscripten::function("note_on", note_on);
	emscripten::function("set_volume", set_volume);
	emscripten::function("get_volume", get_volume);
}