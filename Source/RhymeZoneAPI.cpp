#include "RhymeZoneAPI.h"

RhymeZoneAPI::RhymeZoneAPI() : juce::Thread("RhymeZoneAPI")
{
    startThread();
}

RhymeZoneAPI::~RhymeZoneAPI()
{
    signalThreadShouldExit();
    trigger.signal();
    stopThread(2000);
}

void RhymeZoneAPI::search(const juce::String& word, SearchType type, ResultCallback callback)
{
    auto* req = new Request();
    req->word = word.trim().toLowerCase();
    req->type = type;
    req->callback = std::move(callback);

    {
        juce::ScopedLock sl(lock);
        queue.add(req);
    }

    trigger.signal();
}

void RhymeZoneAPI::run()
{
    while (!threadShouldExit())
    {
        trigger.wait(-1);

        while (true)
        {
            Request* req = nullptr;
            {
                juce::ScopedLock sl(lock);
                if (queue.isEmpty()) break;
                req = queue.removeAndReturn(0);
            }

            if (req == nullptr) break;

            std::unique_ptr<Request> owned(req);
            auto results = fetchResults(owned->word, owned->type);

            juce::MessageManager::callAsync([cb = std::move(owned->callback), res = std::move(results)]() mutable
            {
                cb(res);
            });
        }
    }
}

juce::Array<RhymeResult> RhymeZoneAPI::fetchResults(const juce::String& word, SearchType type)
{
    juce::String urlStr = buildURL(word, type);
    juce::URL url(urlStr);

    juce::StringPairArray responseHeaders;
    int statusCode = 0;

    auto stream = url.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(5000)
            .withResponseHeaders(&responseHeaders)
            .withStatusCode(&statusCode)
    );

    if (stream == nullptr || statusCode != 200)
        return {};

    juce::String json = stream->readEntireStreamAsString();
    return parseJSON(json);
}

juce::String RhymeZoneAPI::buildURL(const juce::String& word, SearchType type)
{
    // Datamuse API — the same engine that powers RhymeZone.com
    juce::String base = "https://api.datamuse.com/words?max=100&";
    juce::String encoded = juce::URL::addEscapeChars(word, true);

    switch (type)
    {
        case SearchType::PerfectRhymes:  return base + "rel_rhy=" + encoded;
        case SearchType::NearRhymes:     return base + "rel_nry=" + encoded;
        case SearchType::SoundAlike:     return base + "sl=" + encoded;
        case SearchType::RelatedWords:   return base + "ml=" + encoded;
        default:                         return base + "rel_rhy=" + encoded;
    }
}

juce::Array<RhymeResult> RhymeZoneAPI::parseJSON(const juce::String& json)
{
    juce::Array<RhymeResult> results;

    auto parsed = juce::JSON::parse(json);
    if (auto* arr = parsed.getArray())
    {
        for (auto& item : *arr)
        {
            if (auto* obj = item.getDynamicObject())
            {
                RhymeResult r;
                r.word = obj->getProperty("word").toString();
                r.score = (int)obj->getProperty("score");
                r.numSyllables = (int)obj->getProperty("numSyllables");
                if (r.word.isNotEmpty())
                    results.add(r);
            }
        }
    }

    return results;
}
