#include "PluginEditor.h"

// ============================================================
//  ResultList
// ============================================================

ResultList::ResultList()
{
    addAndMakeVisible(listBox);
    listBox.setModel(this);
    listBox.setRowHeight(26);
    listBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff1e1e2e));
    listBox.setColour(juce::ListBox::outlineColourId,    juce::Colour(0xff44475a));
    listBox.setOutlineThickness(1);
}

void ResultList::setResults(const juce::Array<RhymeResult>& results)
{
    items = results;
    listBox.updateContent();
    listBox.repaint();
}

void ResultList::clear()
{
    items.clear();
    listBox.updateContent();
    listBox.repaint();
}

int ResultList::getNumRows() { return items.size(); }

void ResultList::paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected)
{
    if (row < 0 || row >= items.size()) return;

    if (selected)
        g.fillAll(juce::Colour(0xff6272a4));
    else if (row % 2 == 0)
        g.fillAll(juce::Colour(0xff1e1e2e));
    else
        g.fillAll(juce::Colour(0xff282a36));

    auto& r = items.getReference(row);

    g.setColour(juce::Colour(0xfff8f8f2));
    g.setFont(juce::FontOptions(15.0f));
    g.drawText(r.word, 10, 0, w - 80, h, juce::Justification::centredLeft);

    if (r.numSyllables > 0)
    {
        g.setColour(juce::Colour(0xff6272a4));
        g.setFont(juce::FontOptions(11.0f));
        juce::String syl = juce::String(r.numSyllables) + (r.numSyllables == 1 ? " syl" : " syls");
        g.drawText(syl, w - 75, 0, 65, h, juce::Justification::centredRight);
    }
}

void ResultList::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    if (row >= 0 && row < items.size())
    {
        juce::SystemClipboard::copyTextToClipboard(items[row].word);
    }
}

void ResultList::listBoxItemDoubleClicked(int row, const juce::MouseEvent& e)
{
    listBoxItemClicked(row, e);
}

void ResultList::resized() { listBox.setBounds(getLocalBounds()); }
void ResultList::paint(juce::Graphics&) {}

// ============================================================
//  RhymeZoneEditor
// ============================================================

RhymeZoneEditor::RhymeZoneEditor(RhymeZoneProcessor& p)
    : AudioProcessorEditor(&p)
{
    setSize(W, H);
    setResizable(true, true);
    setResizeLimits(320, 440, 800, 900);

    // Title
    titleLabel.setText("RhymeZone", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(26.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffbd93f9));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Search label
    searchLabel.setText("Enter a word:", juce::dontSendNotification);
    searchLabel.setFont(juce::FontOptions(13.0f));
    searchLabel.setColour(juce::Label::textColourId, juce::Colour(0xff6272a4));
    addAndMakeVisible(searchLabel);

    // Search box
    searchBox.setFont(juce::FontOptions(16.0f));
    searchBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff282a36));
    searchBox.setColour(juce::TextEditor::textColourId,       juce::Colour(0xfff8f8f2));
    searchBox.setColour(juce::TextEditor::outlineColourId,    juce::Colour(0xff6272a4));
    searchBox.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xffbd93f9));
    searchBox.setTextToShowWhenEmpty("e.g.  night", juce::Colour(0xff44475a));
    searchBox.addListener(this);
    addAndMakeVisible(searchBox);

    // Search button
    searchButton.setButtonText("Search");
    searchButton.setColour(juce::TextButton::buttonColourId,    juce::Colour(0xff6272a4));
    searchButton.setColour(juce::TextButton::buttonOnColourId,  juce::Colour(0xffbd93f9));
    searchButton.setColour(juce::TextButton::textColourOffId,   juce::Colour(0xfff8f8f2));
    searchButton.addListener(this);
    addAndMakeVisible(searchButton);

    // Tab buttons
    auto setupTab = [this](juce::TextButton& btn, const juce::String& label)
    {
        btn.setButtonText(label);
        btn.setClickingTogglesState(true);
        btn.setRadioGroupId(1);
        btn.setColour(juce::TextButton::buttonColourId,   juce::Colour(0xff282a36));
        btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffbd93f9));
        btn.setColour(juce::TextButton::textColourOffId,  juce::Colour(0xff6272a4));
        btn.setColour(juce::TextButton::textColourOnId,   juce::Colour(0xfff8f8f2));
        btn.addListener(this);
        addAndMakeVisible(btn);
    };

    setupTab(tabPerfect, "Perfect");
    setupTab(tabNear,    "Near");
    setupTab(tabSound,   "Sounds Like");
    setupTab(tabRelated, "Related");
    tabPerfect.setToggleState(true, juce::dontSendNotification);

    // Results list
    addAndMakeVisible(resultList);

    // Status
    statusLabel.setText("Type a word and press Search or Enter", juce::dontSendNotification);
    statusLabel.setFont(juce::FontOptions(12.0f));
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff6272a4));
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    // Copy hint
    copyButton.setButtonText("Click a word to copy it");
    copyButton.setColour(juce::TextButton::buttonColourId,  juce::Colour(0xff1e1e2e));
    copyButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff44475a));
    copyButton.setEnabled(false);
    addAndMakeVisible(copyButton);
}

RhymeZoneEditor::~RhymeZoneEditor() {}

void RhymeZoneEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e2e));

    // Decorative gradient band at top
    juce::ColourGradient grad(juce::Colour(0xff44475a), 0, 0,
                              juce::Colour(0xff1e1e2e), 0, 55, false);
    g.setGradientFill(grad);
    g.fillRect(0, 0, getWidth(), 55);
}

void RhymeZoneEditor::resized()
{
    auto area = getLocalBounds().reduced(12);

    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(6);

    searchLabel.setBounds(area.removeFromTop(18));
    area.removeFromTop(2);

    auto row = area.removeFromTop(32);
    searchButton.setBounds(row.removeFromRight(80));
    row.removeFromRight(6);
    searchBox.setBounds(row);

    area.removeFromTop(10);

    // Tab bar
    auto tabRow = area.removeFromTop(30);
    int tabW = tabRow.getWidth() / 4;
    tabPerfect.setBounds(tabRow.removeFromLeft(tabW));
    tabNear.setBounds(tabRow.removeFromLeft(tabW));
    tabSound.setBounds(tabRow.removeFromLeft(tabW));
    tabRelated.setBounds(tabRow);

    area.removeFromTop(4);

    statusLabel.setBounds(area.removeFromBottom(20));
    area.removeFromBottom(2);
    copyButton.setBounds(area.removeFromBottom(22));
    area.removeFromBottom(4);

    resultList.setBounds(area);
}

void RhymeZoneEditor::textEditorReturnKeyPressed(juce::TextEditor&)
{
    doSearch();
}

void RhymeZoneEditor::buttonClicked(juce::Button* btn)
{
    if (btn == &searchButton)
    {
        doSearch();
        return;
    }

    // Tab switched — re-run search with new type
    if (btn == &tabPerfect)       currentTab = RhymeZoneAPI::SearchType::PerfectRhymes;
    else if (btn == &tabNear)     currentTab = RhymeZoneAPI::SearchType::NearRhymes;
    else if (btn == &tabSound)    currentTab = RhymeZoneAPI::SearchType::SoundAlike;
    else if (btn == &tabRelated)  currentTab = RhymeZoneAPI::SearchType::RelatedWords;

    if (searchBox.getText().trim().isNotEmpty())
        doSearch();
}

void RhymeZoneEditor::doSearch()
{
    juce::String word = searchBox.getText().trim().toLowerCase();
    if (word.isEmpty()) return;

    resultList.clear();
    showStatus("Searching...");
    searching = true;

    api.search(word, currentTab, [this, word](juce::Array<RhymeResult> results)
    {
        searching = false;

        if (results.isEmpty())
        {
            showStatus("No results found for \"" + word + "\"");
        }
        else
        {
            showStatus(juce::String(results.size()) + " results for \"" + word + "\"");
            resultList.setResults(results);
        }
    });
}

void RhymeZoneEditor::showStatus(const juce::String& msg)
{
    statusLabel.setText(msg, juce::dontSendNotification);
}

void RhymeZoneEditor::timerCallback() {}
