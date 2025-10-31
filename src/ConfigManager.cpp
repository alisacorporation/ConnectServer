#include "ConfigManager.h"
#include <fstream>
#include <algorithm>
#include <cctype>

ConfigManager::ConfigManager() {
}

ConfigManager::~ConfigManager() {
}

void ConfigManager::trim(std::string& str) {
    // Trim leading whitespace
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    
    // Trim trailing whitespace
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

bool ConfigManager::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    config_.clear();
    std::string line;
    std::string current_section;

    while (std::getline(file, line)) {
        trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Check for section header [SectionName]
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            current_section = line.substr(1, line.length() - 2);
            trim(current_section);
            continue;
        }

        // Parse key=value pairs
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            trim(key);
            trim(value);

            // Store in config map
            config_[current_section][key] = value;
        }
    }

    file.close();
    return true;
}

int ConfigManager::get_int(const std::string& section, const std::string& key, int default_value) const {
    auto sec_it = config_.find(section);
    if (sec_it == config_.end()) {
        return default_value;
    }

    auto key_it = sec_it->second.find(key);
    if (key_it == sec_it->second.end()) {
        return default_value;
    }

    try {
        return std::stoi(key_it->second);
    } catch (...) {
        return default_value;
    }
}

std::string ConfigManager::get_string(const std::string& section, const std::string& key, 
                                     const std::string& default_value) const {
    auto sec_it = config_.find(section);
    if (sec_it == config_.end()) {
        return default_value;
    }

    auto key_it = sec_it->second.find(key);
    if (key_it == sec_it->second.end()) {
        return default_value;
    }

    return key_it->second;
}

bool ConfigManager::has_section(const std::string& section) const {
    return config_.find(section) != config_.end();
}

bool ConfigManager::has_key(const std::string& section, const std::string& key) const {
    auto sec_it = config_.find(section);
    if (sec_it == config_.end()) {
        return false;
    }
    return sec_it->second.find(key) != sec_it->second.end();
}
