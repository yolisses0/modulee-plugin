/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <mutex>

void ModuleeAudioProcessor::setGraph(juce::String graphDataString) {
  graph_mutex.lock();
  set_graph(&**&graph, graphDataString.toStdString().c_str());
  graph_mutex.unlock();
  lastGraphData = graphDataString;
}

void ModuleeAudioProcessor::setNoteOn(int pitch) {
  graph_mutex.lock();
  set_note_on(&**&graph, pitch);
  graph_mutex.unlock();
}

void ModuleeAudioProcessor::setNoteOff(int pitch) {
  graph_mutex.lock();
  set_note_off(&**&graph, pitch);
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
  isMuted = false;
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
      graph_mutex.lock();
      set_note_on(&**&graph, (float)message.getNoteNumber());
      graph_mutex.unlock();
    } else if (message.isNoteOff()) {
      graph_mutex.lock();
      set_note_off(&**&graph, (float)message.getNoteNumber());
      graph_mutex.unlock();
    }
  }

  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  auto numSamples = buffer.getNumSamples();
  auto *channelData = buffer.getWritePointer(0);

  // TODO find a more elegant way to process block without writing in the buffer

  // Fill the buffer with generated audio from the graph
  graph_mutex.lock();
  process_block(&**&graph, channelData, numSamples);
  graph_mutex.unlock();

  if (isMuted) {
    // Clear all the outputs
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
      buffer.clear(i, 0, buffer.getNumSamples());
  } else {
    // Copy the output to other channels. This will be replaced once the app
    // allow stereo output
    for (int channel = 1; channel < totalNumOutputChannels; ++channel) {
      buffer.copyFrom(channel, 0, buffer, 0, 0, numSamples);
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

  auto state = std::make_unique<juce::XmlElement>("PluginState");

  state->setAttribute("lastPath", lastPath);
  state->setAttribute("lastGraphData", lastGraphData);

  copyXmlToBinary(*state, destData);
}

void ModuleeAudioProcessor::setStateInformation(const void *data,
                                                int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.

  auto state = getXmlFromBinary(data, sizeInBytes);

  if (state != nullptr) {
    if (state->hasAttribute("lastGraphData")) {
      lastGraphData = state->getStringAttribute("lastGraphData");
      setGraph(lastGraphData);
    }

    if (state->hasAttribute("lastPath")) {
      lastPath = state->getStringAttribute("lastPath");
    }
  }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new ModuleeAudioProcessor();
}
