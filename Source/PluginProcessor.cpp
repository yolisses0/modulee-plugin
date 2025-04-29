/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void ModuleeAudioProcessor::setGraph(juce::String graphData) {
  pendingGraphString = graphData.toStdString();
  lastGraphData = graphData;
  graphDataIsPending = true;
}

void ModuleeAudioProcessor::setNoteOn(int pitch) {
  // TODO check if there's some way to pass it to the default MidiBuffer
  // (wherever is it)
  //
  // TODO pass velocity data too
  //
  // TODO check if the channel makes sense
  auto midiEvent = juce::MidiMessage::noteOn(1, pitch, 1.0f);
  // TODO check if the sampleNumber makes sense
  uiMidiBuffer.addEvent(midiEvent, 0);
}

void ModuleeAudioProcessor::setNoteOff(int pitch) {
  // TODO check if the channel makes sense
  auto midiEvent = juce::MidiMessage::noteOff(1, pitch, 1.0f);
  // TODO check if the sampleNumber makes sense
  uiMidiBuffer.addEvent(midiEvent, 0);
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

void ModuleeAudioProcessor::processMidiMessagesFrom(
    juce::MidiBuffer &midiMessages) {
  // Process MIDI events
  for (const auto metadata : midiMessages) {
    auto message = metadata.getMessage();
    if (message.isNoteOn()) {
      set_note_on(graph.get(), (float)message.getNoteNumber());
    } else if (message.isNoteOff()) {
      set_note_off(graph.get(), (float)message.getNoteNumber());
    }
  }
}

void ModuleeAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                         juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumOutputChannels = getTotalNumOutputChannels();
  auto numSamples = buffer.getNumSamples();
  auto *channelData = buffer.getWritePointer(0);

  if (graphDataIsPending) {
    set_graph(graph.get(), pendingGraphString.c_str());
    graphDataIsPending = false;
  }

  processMidiMessagesFrom(midiMessages);
  processMidiMessagesFrom(uiMidiBuffer);
  uiMidiBuffer.clear();

  // Process audio
  process_block(graph.get(), channelData, numSamples);

  if (isMuted) {
    for (auto i = 0; i < totalNumOutputChannels; ++i)
      buffer.clear(i, 0, numSamples);
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
