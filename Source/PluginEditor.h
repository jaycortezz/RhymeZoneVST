#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RhymeZoneAPI.h"

class ResultList : public juce::Component,
                   public juce::ListBoxModel
{
public:
    ResultList();

    void setResults(const juce::Array<RhymeResult>& results);
    void clear();

    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::ListBox listBox;
    juce::Array<RhymeResult> items;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResultList)
};

class RhymeZoneEditor : public juce::AudioProcessorEditor,
                        private juce::TextEditor::Listener,
                        private juce::Button::Listener,
                        private juce::Timer
{
public:
    explicit RhymeZoneEditor(RhymeZoneProcessor&);
    ~RhymeZoneEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void textEditorReturnKeyPressed(juce::TextEditor&) override;
    void textEditorTextChanged(juce::TextEditor&) override {}
    void buttonClicked(juce::Button*) override;
    void timerCallback() override;

    void doSearch();
    void showStatus(const juce::String& msg);

    RhymeZoneAPI api;

    juce::Label titleLabel;
    juce::Label searchLabel;
    juce::TextEditor searchBox;
    juce::TextButton searchButton;

    juce::Label tabLabel;
    juce::TextButton tabPerfect, tabNear, tabSound, tabRelated;
    RhymeZoneAPI::SearchType currentTab { RhymeZoneAPI::SearchType::PerfectRhymes };

    ResultList resultList;
    juce::Label statusLabel;
    juce::TextButton copyButton;

    juce::String pendingWord;
    bool searching { false };

    static constexpr int W = 400;
    static constexpr int H = 560;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RhymeZoneEditor)
};
