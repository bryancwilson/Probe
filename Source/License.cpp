
#include "License.h"

using json = nlohmann::json;
using namespace std;

void ChainBuilderAudioProcessorEditor::updateLicenseState()
{
    switch (state)
    {
    case LicenseState::Uninitialized:
    {
        returned_licenseID = loadLicenseID();

        if (returned_licenseID.empty())
        {
            b_text = "Please enter your license";
            set_textbox(3);
            showText();
            state = LicenseState::AwaitingInput;
        }
        else
        {
            state = LicenseState::ValidatingLicense;
        }
        break;
    }

    case LicenseState::ValidatingLicense:
    {
        valid = validate_license(returned_licenseID);
        if (!valid)
        {
            b_text = "Uh oh, something went wrong!";
            saveLicenseID("");
            set_textbox(3);
            showText();
            state = LicenseState::AwaitingInput;
        }
        else
        {
            state = LicenseState::RegisteringMachine;
        }
        break;
    }

    case LicenseState::RegisteringMachine:
    {
        std::string machineID = juce::SystemStats::getUniqueDeviceID().toStdString();
        bool machineRegistered = validate_machine(returned_licenseID, machineID); // Check if machine is registered

        if (!machineRegistered)
        {
            if (!register_machine(returned_licenseID, machineID))
            {
                b_text = "This license cannot be used on this machine.";
                set_textbox(3);
                showText();
                state = LicenseState::Invalid;
            }
            else
            {
                state = LicenseState::Licensed;
                textBox.setVisible(false);
            }
        }
        else
        {
            state = LicenseState::Licensed;
            textBox.setVisible(false);
        }
        break;
    }

    case LicenseState::AwaitingInput:
        break;
    case LicenseState::Licensed:
        break;
    case LicenseState::Invalid:
        // No-op unless user triggers something
        break;
    }
}

bool validate_license(std::string returned_licenseID) {
    const std::string url = "https://mydb-api-rpyo.onrender.com/val";

    json payload;
    payload = {
        {"account_id", "95c86568-aa50-4a83-a8b4-84f4da5a20b1"},
        {"license_id", returned_licenseID},
        {"fingerprint", "test"}
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

        if (res == CURLE_COULDNT_RESOLVE_HOST) {
            // return "Uh oh! Something went wrong! Please check your internet connection.";
            return false;
        }
        else if (res == CURLE_COULDNT_CONNECT) {
            // return "The server may be down. Come back and try again next time!";
            return false;
        }
        else if (res == CURLE_OPERATION_TIMEDOUT)
        {
            // return "Hmm, things are taking a little while. Maybe check your network strength and stability.";
            return false;
        }
        if (res == CURLE_OK) {
            try {
                json responseJson = json::parse(response);

                if (responseJson.contains("detail")) {
                    // std::cout << "❌ License validation failed: "
                    //    << responseJson["detail"] << std::endl;
                    return false;
                }
                else {
                    // std::cout << "✅ License is valid." << std::endl;
                    return true;
                }
            }
            catch (std::exception& e) {
                // std::cerr << "JSON parse error: " << e.what()
                //    << "\nRaw response: " << response << std::endl;
                return false;
            }
        }
        else {
            std::cerr << "Curl error: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

    }
}

bool validate_machine(const std::string& license_id, const std::string& fingerprint) {
    CURL* curl;
    CURLcode res;
    long http_code = 0;
    std::string response;

    // Keygen API endpoint
    std::string url = "https://api.keygen.sh/v1/accounts/95c86568-aa50-4a83-a8b4-84f4da5a20b1/machines?"
        "fingerprint=" + fingerprint +
        "&license=" + license_id;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Accept: application/vnd.api+json");
        headers = curl_slist_append(headers, "Authorization: Bearer admin-880dc64881021be20df334527238bba8ccf684f44c01de9c0129566028a5f388v3");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Capture response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Disable SSL verification (for testing only)
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

            std::cout << "HTTP Response Code: " << http_code << std::endl;
            std::cout << "Response Body: " << response << std::endl;

            // ✅ Check if machine exists in response
            if (http_code == 200 && response.find("\"data\":[]") == std::string::npos) {
                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
                curl_global_cleanup();
                return true;  // Machine exists
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return false; // Machine not found or error
}


bool register_machine(const std::string& license_id, const std::string& fingerprint) {
    CURL* curl;
    CURLcode res;
    long http_code = 0;
    string url = "https://api.keygen.sh/v1/accounts/95c86568-aa50-4a83-a8b4-84f4da5a20b1/machines";

    juce::String osName = juce::SystemStats::getOperatingSystemName();
    std::string osName_str = osName.toStdString();

    std::string post_data = R"({
        "data": {
            "type": "machines",
            "attributes": {
                "fingerprint": ")" + fingerprint + R"(",
                "platform": ")" + osName_str + R"(",
                "name": ")" + osName_str + R"("
            },
            "relationships": {
                "license": {
                    "data": {
                        "type": "licenses",
                        "id": ")" + license_id + R"("
                    }
                }
            }
        }
    })";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    std::string response;

    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/vnd.api+json");
        headers = curl_slist_append(headers, "Accept: application/vnd.api+json");
        headers = curl_slist_append(headers, "Authorization: Bearer prod-0b8cca1b37ff663730b6971dba822aea5e3d51e6b465ef79b817de5827bbac06v3");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

        // Optional: capture response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Disable SSL verification (not for production)
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            cout << "HTTP Response Code: " << http_code << endl;
            cout << "Response Body: " << response << endl;
            return true;

        }
        else {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return false;
}

// To save the license ID
void ChainBuilderAudioProcessorEditor::saveLicenseID(const std::string& licenseID)
{
    auto* props = appProperties.getUserSettings();
    props->setValue("licenseID", juce::String(licenseID));
    props->saveIfNeeded();
    first_pass_valid = true;
}

// To load the license ID
std::string ChainBuilderAudioProcessorEditor::loadLicenseID()
{
    auto* props = appProperties.getUserSettings();
    return props->getValue("licenseID", "").toStdString();;
}