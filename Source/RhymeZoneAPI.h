#pragma once
#include <JuceHeader.h>
#include <functional>

struct RhymeResult
{
    juce::String word;
    int score;
    int numSyllables;
};

class RhymeZoneAPI : private juce::Thread
{
public:
    enum class SearchType { PerfectRhymes, NearRhymes, SoundAlike, RelatedWords };

    using ResultCallback = std::function<void(juce::Array<RhymeResult>)>;

    RhymeZoneAPI();
    ~RhymeZoneAPI() override;

    void search(const juce::String& word, SearchType type, ResultCallback callback);

private:
    void run() override;

    struct Request
    {
        juce::String word;
        SearchType type;
        ResultCallback callback;
    };

    juce::CriticalSection lock;
    juce::OwnedArray<Request> queue;
    juce::WaitableEvent trigger;

    juce::Array<RhymeResult> fetchResults(const juce::String& word, SearchType type);
    static juce::String buildURL(const juce::String& word, SearchType type);
    static juce::Array<RhymeResult> parseJSON(const juce::String& json);
};
