#pragma once

#include <string>
#include <map>

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // Load configuration from INI file
    bool load(const std::string& filename);

    // Get integer value from section/key with default
    int get_int(const std::string& section, const std::string& key, int default_value) const;

    // Get string value from section/key with default
    std::string get_string(const std::string& section, const std::string& key, 
                          const std::string& default_value) const;

    // Check if section exists
    bool has_section(const std::string& section) const;

    // Check if key exists in section
    bool has_key(const std::string& section, const std::string& key) const;

private:
    std::map<std::string, std::map<std::string, std::string>> config_;

    // Helper function to trim whitespace
    void trim(std::string& str);
};
