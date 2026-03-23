/* Start Header ************************************************************************/
/*!
\file       LeaderboardSystem.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       March, 11th, 2026
\brief      Singleton leaderboard system that persists player names and scores
            to a local JSON file. Supports adding entries, querying top-N scores,
            and automatic file I/O.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

struct LeaderboardEntry {
    std::string name;
    int score = 0;
    std::string date; // ISO 8601 date string (e.g. "2026-03-11")
};

class LeaderboardSystem {
public:
    static LeaderboardSystem& getInstance() {
        static LeaderboardSystem instance;
        return instance;
    }

    // Load leaderboard data from the JSON file (creates file if missing)
    void init();

    // Add a new entry, re-sort, trim to max entries, and save to disk
    void addEntry(const std::string& name, int score);

    // Get the top N entries (sorted descending by score)
    std::vector<LeaderboardEntry> getTopN(int n) const;

    // Get all entries
    const std::vector<LeaderboardEntry>& getAll() const { return m_entries; }

    // Clear all entries and save
    void clearAll();

    // Force a save to disk
    void save() const;

    // Force a reload from disk
    void load();

    // Maximum number of entries stored
    static constexpr int MAX_ENTRIES = 10;

private:
    LeaderboardSystem() = default;
    ~LeaderboardSystem() = default;
    LeaderboardSystem(const LeaderboardSystem&) = delete;
    LeaderboardSystem& operator=(const LeaderboardSystem&) = delete;

    // Returns the full path to the leaderboard JSON file
    std::string getFilePath() const;

    // Returns today's date as an ISO 8601 string
    static std::string todayDateString();

    std::vector<LeaderboardEntry> m_entries;
    bool m_initialized = false;
};
