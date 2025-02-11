/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <mutex>

void ModuleeAudioProcessor::setGroups(const char *groups_data) {
  graph_mutex.lock();
  set_groups(&**&graph, groups_data);
  graph_mutex.unlock();
}

void ModuleeAudioProcessor::setMainGroupId(int main_group_id) {
  graph_mutex.lock();
  set_main_group_id(&**&graph, main_group_id);
  graph_mutex.unlock();
}

//==============================================================================
ModuleeAudioProcessor::ModuleeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              )
      // I'm not sure how this initialization works. If something breaks
      // involving this scope, revise this thoroughly.
      ,
      graph(nullptr, &destroy_graph_pointer)
#endif
{
  graph.reset(create_graph_pointer());
}

ModuleeAudioProcessor::~ModuleeAudioProcessor() {}

//==============================================================================
const juce::String ModuleeAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool ModuleeAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool ModuleeAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool ModuleeAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double ModuleeAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int ModuleeAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int ModuleeAudioProcessor::getCurrentProgram() { return 0; }

void ModuleeAudioProcessor::setCurrentProgram(int index) {}

const juce::String ModuleeAudioProcessor::getProgramName(int index) {
  return {};
}

void ModuleeAudioProcessor::changeProgramName(int index,
                                              const juce::String &newName) {}

//==============================================================================
void ModuleeAudioProcessor::prepareToPlay(double sampleRate,
                                          int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
}

void ModuleeAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ModuleeAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void ModuleeAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                         juce::MidiBuffer &midiMessages) {

  for (const auto metadata : midiMessages) {
    auto message = metadata.getMessage();
    auto timeStamp = metadata.samplePosition;

    if (message.isNoteOn()) {
      DBG("Note On: " << message.getNoteNumber() << " Velocity: "
                      << message.getVelocity() << " Timestamp: " << timeStamp);
      set_note_on(&**&graph, (float)message.getNoteNumber());
    } else if (message.isNoteOff()) {
      DBG("Note Off: " << message.getNoteNumber()
                       << " Timestamp: " << timeStamp);
      set_note_off(&**&graph, (float)message.getNoteNumber());
    } else if (message.isController()) {
      DBG("Controller: " << message.getControllerNumber()
                         << " Value: " << message.getControllerValue()
                         << " Timestamp: " << timeStamp);
    }
  }

  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // This is the place where you'd normally do the guts of your plugin's
  // audio processing...
  // Make sure to reset the state if your inner loop is processing
  // the samples and the outer loop is handling the channels.
  // Alternatively, you can process the samples with the channels
  // interleaved by keeping the same state.

  auto numSamples = buffer.getNumSamples();
  auto *channelData = buffer.getWritePointer(0);
  // Fill the buffer with generated audio from Rust
  graph_mutex.lock();
  process_block(&**&graph, channelData, numSamples);
  graph_mutex.unlock();

  for (int channel = 1; channel < totalNumOutputChannels; ++channel) {
    auto *otherChannelData = buffer.getWritePointer(channel);

    for (int sample = 0; sample < numSamples; ++sample) {
      otherChannelData[sample] = channelData[sample];
    }
  }
}

//==============================================================================
bool ModuleeAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *ModuleeAudioProcessor::createEditor() {
  return new ModuleeAudioProcessorEditor(*this);
}

//==============================================================================
void ModuleeAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void ModuleeAudioProcessor::setStateInformation(const void *data,
                                                int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new ModuleeAudioProcessor();
}
