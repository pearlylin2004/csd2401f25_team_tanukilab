/* Start Header ************************************************************************/
/*!
\file       LeaderboardSystem.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       March, 11th, 2026
\brief      Implementation of the LeaderboardSystem singleton.
            Reads/writes leaderboard data from/to a local JSON file using RapidJSON.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "LeaderboardSystem.h"
#include "JsonIO.h"
#include <ctime>
#include <filesystem>

#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./assets"
#endif

// ---------------------------------------------------------------------------
// Path helper
// ---------------------------------------------------------------------------
std::string LeaderboardSystem::getFilePath() const {
    return std::string(RUNTIME_DIR_R) + "/localleaderboardscores.json";
}

// ---------------------------------------------------------------------------
// Date helper
// ---------------------------------------------------------------------------
std::string LeaderboardSystem::todayDateString() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return std::string(buf);
}

// ---------------------------------------------------------------------------
// Init — load from disk (or create an empty file)
// ---------------------------------------------------------------------------
void LeaderboardSystem::init() {
    load();
    m_initialized = true;
    std::cout << "[Leaderboard] Initialized with " << m_entries.size() << " entries\n";
}

// ---------------------------------------------------------------------------
// Load — parse the JSON file into m_entries
// ---------------------------------------------------------------------------
void LeaderboardSystem::load() {
    m_entries.clear();

    const std::string path = getFilePath();

    // If the file doesn't exist yet, just leave entries empty
    if (!std::filesystem::exists(path)) {
        std::cout << "[Leaderboard] No existing file at " << path << " — starting fresh\n";
        return;
    }

    rapidjson::Document doc;
    std::string err;
    if (!JsonIO::ReadFileToDocument(path, doc, &err)) {
        std::cerr << "[Leaderboard] Failed to read " << path << ": " << err << "\n";
        return;
    }

    if (!doc.HasMember("leaderboard") || !doc["leaderboard"].IsArray()) {
        std::cerr << "[Leaderboard] JSON missing 'leaderboard' array\n";
        return;
    }

    const auto& arr = doc["leaderboard"];
    for (rapidjson::SizeType i = 0; i < arr.Size(); ++i) {
        const auto& entry = arr[i];
        LeaderboardEntry e;
        if (entry.HasMember("name") && entry["name"].IsString())
            e.name = entry["name"].GetString();
        if (entry.HasMember("score") && entry["score"].IsInt())
            e.score = entry["score"].GetInt();
        if (entry.HasMember("date") && entry["date"].IsString())
            e.date = entry["date"].GetString();
        m_entries.push_back(e);
    }

    // Ensure sorted descending by score
    std::sort(m_entries.begin(), m_entries.end(),
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) { return a.score > b.score; });
}

// ---------------------------------------------------------------------------
// Save — serialize m_entries to the JSON file
// ---------------------------------------------------------------------------
void LeaderboardSystem::save() const {
    rapidjson::Document doc;
    doc.SetObject();
    auto& a = doc.GetAllocator();

    rapidjson::Value arr(rapidjson::kArrayType);
    for (const auto& e : m_entries) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("name", rapidjson::Value(e.name.c_str(), a), a);
        obj.AddMember("score", e.score, a);
        obj.AddMember("date", rapidjson::Value(e.date.c_str(), a), a);
        arr.PushBack(obj, a);
    }
    doc.AddMember("leaderboard", arr, a);

    std::string err;
    if (!JsonIO::WriteDocumentToFile(getFilePath(), doc, true, &err)) {
        std::cerr << "[Leaderboard] Failed to save: " << err << "\n";
    }
    else {
        std::cout << "[Leaderboard] Saved " << m_entries.size() << " entries to disk\n";
    }
}

// ---------------------------------------------------------------------------
// Add entry — insert, sort, trim, save
// ---------------------------------------------------------------------------
void LeaderboardSystem::addEntry(const std::string& name, int score) {
    LeaderboardEntry entry;
    entry.name = name;
    entry.score = score;
    entry.date = todayDateString();

    m_entries.push_back(entry);

    // Sort descending by score
    std::sort(m_entries.begin(), m_entries.end(),
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) { return a.score > b.score; });

    // Trim to MAX_ENTRIES
    if (static_cast<int>(m_entries.size()) > MAX_ENTRIES) {
        m_entries.resize(MAX_ENTRIES);
    }

    save();
    std::cout << "[Leaderboard] Added entry: " << name << " — " << score << "\n";
}

// ---------------------------------------------------------------------------
// Get top N
// ---------------------------------------------------------------------------
std::vector<LeaderboardEntry> LeaderboardSystem::getTopN(int n) const {
    if (n <= 0) return {};
    int count = std::min(n, static_cast<int>(m_entries.size()));
    return std::vector<LeaderboardEntry>(m_entries.begin(), m_entries.begin() + count);
}

// ---------------------------------------------------------------------------
// Clear all
// ---------------------------------------------------------------------------
void LeaderboardSystem::clearAll() {
    m_entries.clear();
    save();
    std::cout << "[Leaderboard] All entries cleared\n";
}
