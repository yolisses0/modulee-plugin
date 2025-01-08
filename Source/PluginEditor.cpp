/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ModuleeAudioProcessorEditor::ModuleeAudioProcessorEditor(ModuleeAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{

  // create the actual browser component
  webView.reset(new juce::WebBrowserComponent());
  addAndMakeVisible(webView.get());

  // send the browser to a start page..
  webView->goToURL("https://modulee.yolisses.com");

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(600, 400);
  setResizable(true, true);
}

ModuleeAudioProcessorEditor::~ModuleeAudioProcessorEditor()
{
}

//==============================================================================
void ModuleeAudioProcessorEditor::paint(juce::Graphics &g)
{
  // (Our component is opaque, so we must completely fill the background with a solid colour)
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ModuleeAudioProcessorEditor::resized()
{
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
  webView->setBounds(0, 0, getWidth(), getHeight());
}
