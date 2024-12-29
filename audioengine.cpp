#include <emscripten/bind.h>
#include <emscripten/webaudio.h>
#include <thread>
#include <future>
#include <chrono>

#define TSF_IMPLEMENTATION
#include "tsf/tsf.h"

static tsf *g_TinySoundFont = NULL;

static bool run;
int volume = 100;

static bool renderAudio(int numInputs, const AudioSampleFrame *inputs,
                        int numOutputs, AudioSampleFrame *outputs,
                        int numParams, const AudioParamFrame *params,
                        void *userData)
{
  for (int i = 0; i < numOutputs; ++i)
  {
    tsf_render_float(g_TinySoundFont, outputs[i].data, outputs[i].samplesPerChannel * outputs[i].numberOfChannels, 0);
  }

  return true;
}

void load(uintptr_t data, int length)
{
  printf("A file of length %d has been loaded into AudioEngine.\n", length);
  auto array = reinterpret_cast<uint8_t *>(data);
  bool recreating = false;
  if (g_TinySoundFont != NULL)
  {
    tsf_close(g_TinySoundFont);
    recreating = true;
  }
  g_TinySoundFont = tsf_load_memory(array, length);
  if (recreating)
  {
    tsf_set_output(g_TinySoundFont, TSF_MONO, 44100, 0);
    tsf_set_max_voices(g_TinySoundFont, 200);
    tsf_set_volume(g_TinySoundFont, volume / 100.0f);
  }
}

void destroy()
{
  tsf_close(g_TinySoundFont);
}

void end()
{
  run = false;
}
void note_on(int presetId, int key, float velocity, int delay)
{
  tsf_note_on(g_TinySoundFont, presetId, key, velocity);
}

void note_off(int presetId, int key, float velocity, int delay)
{
  tsf_note_off(g_TinySoundFont, presetId, key);
}

uint8_t audioThreadStack[4096];

bool OnClick(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
  EMSCRIPTEN_WEBAUDIO_T audioContext = (EMSCRIPTEN_WEBAUDIO_T)userData;
  if (emscripten_audio_context_state(audioContext) != AUDIO_CONTEXT_STATE_RUNNING)
  {
    emscripten_resume_audio_context_sync(audioContext);
  }
  return false;
}

void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData)
{
  if (!success)
    return; // Check browser console in a debug build for detailed errors

  int outputChannelCounts[1] = {1};
  EmscriptenAudioWorkletNodeCreateOptions options = {
      .numberOfInputs = 0,
      .numberOfOutputs = 1,
      .outputChannelCounts = outputChannelCounts};

  // Create node
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext,
                                                                                               "audioengine", &options, &renderAudio, 0);

  // Connect it to audio context destination
  emscripten_audio_node_connect(wasmAudioWorklet, audioContext, 0, 0);
  emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, (void *)audioContext, 0, OnClick);
}

void AudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData)
{
  if (!success)
    return; // Check browser console in a debug build for detailed errors

  WebAudioWorkletProcessorCreateOptions opts = {
      .name = "audioengine",
  };
  emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, &AudioWorkletProcessorCreated, 0);
}

int get_volume()
{
  return volume;
}

void set_volume(int vol)
{
  volume = vol;
  tsf_set_volume(g_TinySoundFont, vol / 100.0f);
}

int start()
{
  printf("AudioEngine has been started.\n");

  tsf_set_output(g_TinySoundFont, TSF_MONO, 44100, 0);
  tsf_set_max_voices(g_TinySoundFont, 200);
  EmscriptenWebAudioCreateAttributes attr = {
      .latencyHint = "playback",
      .sampleRate = 44100};
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(&attr);
  emscripten_start_wasm_audio_worklet_thread_async(context, audioThreadStack, sizeof(audioThreadStack), &AudioThreadInitialized, 0);
  return 0;
}

EMSCRIPTEN_BINDINGS(my_module)
{
  emscripten::function("start", start);
  emscripten::function("load", load, emscripten::allow_raw_pointers());
  emscripten::function("end", end);
  emscripten::function("destroy", destroy);
  emscripten::function("note_off", note_off);
  emscripten::function("note_on", note_on);
  emscripten::function("set_volume", set_volume);
  emscripten::function("get_volume", get_volume);
}