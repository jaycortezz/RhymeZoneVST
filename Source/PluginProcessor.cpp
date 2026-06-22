#include "PluginProcessor.h"
#include "PluginEditor.h"

RhymeZoneProcessor::RhymeZoneProcessor()
    : AudioProcessor(BusesProperties())
{}

juce::AudioProcessorEditor* RhymeZoneProcessor::createEditor()
{
    return new RhymeZoneEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhymeZoneProcessor();
}
