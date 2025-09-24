
#include "PluginEditor.h"

#include <string>
#include <iostream>


size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

// Validate if License is Valid
bool validate_license(std::string returned_licenseID);

// Validate it Unique Code from System is associated to a machine
bool validate_machine(const std::string& license_id, const std::string& fingerprint);

// Register Current System to Machine
bool register_machine(const std::string& license_id, const std::string& fingerprint);