#include "../PluginEditor.h"

using json = nlohmann::json;

// Callback function to capture the response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;

}
std::string ChainBuilderAudioProcessorEditor::prompt_gen()
{
    const std::string url = "https://mydb-api-rpyo.onrender.com/generate_probe";

    juce::String ps = dropZone->param_list;
    json payload;
    payload = {
        {"plugin_name", "AI EQ"},
        {"plugin_type", "Equalizer"},
        {"available_parameters", ps.toStdString()},

        {"spectral_centroid", std::to_string(audioProcessor.spectral_centroid)},
        {"spectral_rolloff", std::to_string(audioProcessor.spectral_rolloff)},
        {"spectral_flatness", std::to_string(audioProcessor.spectral_flatness)},
        {"resonance_score", std::to_string(audioProcessor.resonance_score)},
        {"harmonic_to_noise", std::to_string(audioProcessor.harmonic_to_noise)},

        {"rms", std::to_string(audioProcessor.rms)},
        {"lufs", std::to_string(audioProcessor.lufs)},
        {"peak", std::to_string(audioProcessor.peak)},
        {"crest_factor", std::to_string(audioProcessor.crest_factor)},
        {"transient_sharpness", std::to_string(audioProcessor.transient_sharpness)},
        {"decay_time", std::to_string(audioProcessor.decay_time)},

        {"stereo_correlation", std::to_string(audioProcessor.stereo_correlation)},
        {"modulation_depth", std::to_string(audioProcessor.modulation_depth)},

        {"prompt", creative_text}
    };

    std::string jsonPayload = payload.dump();

    CURL* curl;
    CURLcode res;
    std::string response;

    // Initialize curl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());

        // was getting CURLE_PEER_FAILED_VERIFICATION, this is a temp fix but will need a more permanent one, ask chat-gpt
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);


        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);


        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        res = curl_easy_perform(curl);
        // wait_on_response = false;

        if (res == CURLE_COULDNT_RESOLVE_HOST) 
        {
            return "Uh oh! Something went wrong! Please check your internet connection.";
        }
        else if (res == CURLE_COULDNT_CONNECT) 
        {
            return "The server may be down. Come back and try again next time!";
        }
        else if (res == CURLE_OPERATION_TIMEDOUT)
        {
            return "Hmm, things are taking a little while. Maybe check your network strength and stability.";
        }
        else if (res == CURLE_OK)
        {
            json j = json::parse(response);
            std::string generatedText = j["response"];

            // --- find the RANGE block ---
            std::string plainText = generatedText;
            std::string rangeBlock;

            size_t rangePos = generatedText.find("RANGE:");
            if (rangePos != std::string::npos)
            {
                // split into text + range JSON
                plainText = generatedText.substr(0, rangePos);
                creative_response.setText(plainText, juce::dontSendNotification);
                
                rangeBlock = generatedText.substr(rangePos + 6); // skip "RANGE:"

                // trim whitespace
                auto trim = [](std::string& s) {
                    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                        return !std::isspace(ch);
                        }));
                    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                        return !std::isspace(ch);
                        }).base(), s.end());
                    };
                trim(rangeBlock);

                // parse JSON array
                try
                {
                    json rangeJson = json::parse(rangeBlock);

                    for (auto& item : rangeJson)
                    {
                        std::string param = item.value("parameter", "");
                        float target = item.value("target", 0.0f);

                        DBG("Parameter: " << param
                            << " Target: " << target);

                        //// Find the corresponding ParameterDisplay and set the target
                        for (auto* display : parameterDisplays)
                        {
                            if (display->getParameter()->getName(100).toStdString() == param)
                            {
                                display->setTargetValue(target);
                                break; // found it, no need to continue
                            }
                        }
                    }
                }
                catch (std::exception& e)
                {
                    DBG("Failed to parse RANGE JSON: " << e.what());
                }
            }

            return plainText; // return the suggestion text only
        }

        else
        {
            return "Uh oh! Something went wrong! Please comeback and try again in a few minutes.";
        }

    }

    
}

void ChainBuilderAudioProcessorEditor::api_func()
{
	
}
