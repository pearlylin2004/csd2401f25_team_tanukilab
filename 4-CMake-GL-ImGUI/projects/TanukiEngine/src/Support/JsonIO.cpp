/* Start Header ************************************************************************/
/*!
\file		JsonIO.cpp
\author     Varick, v.teo, 2403417 - 70%
			Tan Chee Qing, cheeqing.tan, 2401486 - 30%
\par        v.teo@digipen.edu
\par        cheeqing.tan@digipen.edu
\date		November, 5th, 2025
\brief      Implementation of JSON helpers and scene serialization for the WaterBound engine.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/	

#include "JsonIO.h"
#include "GameObjectManager.h"
#include "renderer.h"
#include "PathManager.h"

#ifndef RUNTIME_SCENE_DIR_R
#define RUNTIME_SCENE_DIR_R "./assets/Scene"
#endif
//#ifndef SOURCE_ASSETS_DIR_R
//#define SOURCE_ASSETS_DIR_R "./projects/WaterBound/assets"
//#endif
#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./assets"
#endif
//#ifndef SOURCE_DIR_R
//#define SOURCE_DIR_R "./projects/WaterBound/assets"
//#endif
//==============================================================================
// Paths
//==============================================================================

/**
 * \namespace Paths
 * \brief OS-aware helpers to resolve folders relative to the executable.
 *
 * These functions let the engine run regardless of where the EXE lives by
 * computing paths at runtime. Use these for loading configs, scripts, and
 * other resources that may be bundled alongside the executable.
 */
namespace Paths {

    /**
     * \brief Returns the directory that contains the running executable.
     * \return Executable folder as std::filesystem::path.
     *
     * \note On Windows, uses \c GetModuleFileNameW .
     *       On Linux, resolves \c /proc/self/exe .
     */
    std::filesystem::path exe_dir() {
#ifdef _WIN32
        wchar_t buf[MAX_PATH];
        GetModuleFileNameW(nullptr, buf, MAX_PATH);
        return std::filesystem::path(buf).parent_path();
#else
        return std::filesystem::canonical("/proc/self/exe").parent_path();
#endif
    }

    /**
     * \brief Convenience: path to the \c shaders directory next to the EXE.
     */
    std::filesystem::path shaders() { return exe_dir() / "shaders"; }

    /**
     * \brief Convenience: path to the \c shaders/Scene directory next to the EXE.
     */
    std::filesystem::path scenes() { return shaders() / "Scene"; }

    /**
     * \brief Resolves the configuration file location.
     * \return \c config.json next to the EXE if present; otherwise \c resources/config.json .
     */
    std::filesystem::path config() {
        auto direct = exe_dir() / "config.json";
        if (std::filesystem::exists(direct))
            return direct;
        return exe_dir() / "resources" / "config.json";
    }

    /**
     * \brief Convenience: path to the \c scripting directory next to the EXE.
     */
    std::filesystem::path scripts() { return exe_dir() / "scripting"; }

    /**
     * \brief Ensures a directory exists (creates it if needed).
     * \param p Directory path to create.
     *
     * \note Failures are swallowed via \c std::error_code; it is safe to call
     *       even if the directory already exists.
     */
    void ensure_dir(const std::filesystem::path& p) {
        std::error_code ec;
        std::filesystem::create_directories(p, ec);
    }

} // namespace Paths


//==============================================================================
// JsonIO
//==============================================================================

/**
 * \namespace JsonIO
 * \brief Thin abstraction layer over RapidJSON plus engine-specific helpers.
 *
 * Responsibilities:
 * - Safe JSON read/write with good error messages
 * - Atomic file replacement for saves
 * - Small value helpers (bool/int/float/vec/color)
 * - String <-> enum conversions for engine types (e.g., shape)
 * - Scene path indirections for live-edit workflows
 */
namespace JsonIO {

    /// \name Scene path helpers
    /// @{

    /**
     * \brief Resolves runtime scene file path.
     * \param name Scene file name (e.g., \c level01.json ).
     * \return Absolute/relative path into \c RUNTIME_SCENE_DIR_R .
     */
    std::string JsonIO::runtimeScenePath(const std::string& name) {
        return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
    }

    /**
     * \brief Resolves source (project) scene file path.
     * \param name Scene file name (e.g., \c level01.json ).
     * \return Absolute/relative path into \c SOURCE_SCENE_DIR_R .
     */
    std::string JsonIO::sourceScenePath(const std::string& name) {
        //return std::string(SOURCE_ASSETS_DIR_R) + "/Scene/" + name;
        return PathManager::Instance().getSourceScenePath() + name;
    }

    /**
     * \brief Returns the project source root for shaders-related assets.
     * \return Value of \c SOURCE_DIR_R .
     */
    std::string projectSourceRoot() {
        //return std::string(SOURCE_DIR_R);
        return PathManager::Instance().getSourcePath() + "/assets";
    }

    /// @}

    /**
     * \brief Copies a scene from the project source folder into the runtime folder.
     * \param name Scene filename (e.g., \c level01.json ).
     * \return \c true on success, \c false otherwise.
     *
     * Creates the destination directory if needed and overwrites existing files.
     * Useful in editor workflows so content creators don't need to manually copy.
     */
    bool syncSceneToRuntime(const std::string& name) {
        const std::string src = sourceScenePath(name);
        const std::string dst = runtimeScenePath(name);

        std::error_code ec;
        const auto parent = std::filesystem::path(dst).parent_path();
        if (!parent.empty())
            std::filesystem::create_directories(parent, ec);

        std::filesystem::copy_file(
            src, dst,
            std::filesystem::copy_options::overwrite_existing, ec
        );

        if (ec) {
            std::cerr << "Scene sync failed: " << src << " -> " << dst
                << " (" << ec.message() << ")\n";
            return false;
        }
        std::cout << "Scene synced: " << dst << "\n";
        return true;
    }


    /// \name Shape conversion helpers
    /// @{

    /**
     * \brief Converts a \c shape enum to its JSON string representation.
     * \param s Engine shape enum.
     * \return \c "square" , \c "circle" , or \c "triangle" (defaults to \c "square" ).
     */
    const char* ShapeToStr(shape s) {
        switch (s) {
        case shape::square:   return "square";
        case shape::circle:   return "circle";
        case shape::triangle: return "triangle";
        default:              return "square";
        }
    }

    /**
     * \brief Converts a JSON string into a \c shape enum.
     * \param s JSON string (\c "square" , \c "circle" , \c "triangle" ).
     * \return Parsed enum; defaults to \c shape::square .
     */
    shape StrToShape(const std::string& s) {
        if (s == "circle")   return shape::circle;
        if (s == "triangle") return shape::triangle;
        return shape::square;
    }

    /// @}


    /**
     * \brief Converts a CollisionResponseMode enum to a string.
     * \param mode The collision response mode enum.
     * \return String representation ("StopWhenCollide" or "MoveWhenCollide").
     */
    const char* CollisionResponseModeToStr(CollisionResponseMode mode) {
        switch (mode) {
        case CollisionResponseMode::StopWhenCollide: return "StopWhenCollide";
        case CollisionResponseMode::MoveWhenCollide: return "MoveWhenCollide";
        default:                                    return "NoResponse";
        }
    }

    /**
     * \brief Converts a JSON string into a CollisionResponseMode enum.
     * \param s JSON string ("StopWhenCollide" or "MoveWhenCollide").
     * \return Parsed enum; defaults to CollisionResponseMode::StopWhenCollide.
     */
    CollisionResponseMode StrToCollisionResponseMode(const std::string& s) {
        if (s == "MoveWhenCollide") return CollisionResponseMode::MoveWhenCollide;
        if (s == "StopWhenCollide") return CollisionResponseMode::StopWhenCollide;
        return CollisionResponseMode::NoResponse;
    }

    //for button
    const char* ButtonActionToStr(ButtonAction action) {
        switch (action) {
        case ButtonAction::LOAD_SCENE:    return "load_scene";
        case ButtonAction::EXIT_GAME:     return "exit_game";
        case ButtonAction::RESUME_GAME:   return "resume_game";
        case ButtonAction::YES_EXIT:      return "yes_exit";
        case ButtonAction::NO_EXIT:       return "no_exit";
        case ButtonAction::BACK:          return "back";

        case ButtonAction::MUSIC_VOL_UP:   return "music_vol_up";
        case ButtonAction::MUSIC_VOL_DOWN: return "music_vol_down";
        case ButtonAction::SFX_VOL_UP:     return "sfx_vol_up";
        case ButtonAction::SFX_VOL_DOWN:   return "sfx_vol_down";

        case ButtonAction::MUTE:   return "mute";
        case ButtonAction::UNMUTE: return "unmute";

        default: return "none";
        }
    }

    //for button
    ButtonAction StrToButtonAction(const std::string& s) {
        if (s == "load_scene") return ButtonAction::LOAD_SCENE;

        if (s == "exit_game") return ButtonAction::EXIT_GAME;
        if (s == "resume_game") return ButtonAction::RESUME_GAME;
        if (s == "yes_exit") return ButtonAction::YES_EXIT;
        if (s == "no_exit") return ButtonAction::NO_EXIT;
        if (s == "back") return ButtonAction::BACK;

        if (s == "music_vol_up") return ButtonAction::MUSIC_VOL_UP;
        if (s == "music_vol_down") return ButtonAction::MUSIC_VOL_DOWN;
        if (s == "sfx_vol_up") return ButtonAction::SFX_VOL_UP;
        if (s == "sfx_vol_down") return ButtonAction::SFX_VOL_DOWN;

        if (s == "mute") return ButtonAction::MUTE;
        if (s == "unmute") return ButtonAction::UNMUTE;

        return ButtonAction::NONE;
	}

    //player state
    PlayerState parsePlayerState(const std::string& s) {
        if (s == "Idle")     return PlayerState::Idle;
        if (s == "Walking")  return PlayerState::Walking;
        if (s == "Jumping")  return PlayerState::Jumping;
        if (s == "Falling")  return PlayerState::Falling;
        if (s == "Shooting") return PlayerState::Shooting;
        if (s == "Dead")     return PlayerState::Dead;
        return PlayerState::Idle; // default/fallback
    }


    NodeImplementation stringToNodeImpl(const std::string& str) {
        if (str == "On Start") return NodeImplementation::ON_START;
        if (str == "On Update") return NodeImplementation::ON_UPDATE;
		if (str == "On Collision Enter") return NodeImplementation::ON_COLLISION_ENTER;
		if (str == "While Colliding") return NodeImplementation::WHILE_COLLIDING;
		if (str == "On Collision Exit") return NodeImplementation::ON_COLLISION_EXIT;
		if (str == "On Hover Enter") return NodeImplementation::ON_HOVER_ENTER;
		if (str == "While Hovering") return NodeImplementation::WHILE_HOVERING;
		if (str == "On Hover Exit") return NodeImplementation::ON_HOVER_EXIT;
        if (str == "On Click Enter") return NodeImplementation::ON_CLICK_ENTER;
        if (str == "While Clicking") return NodeImplementation::WHILE_CLICKING;
        if (str == "On Click Exit") return NodeImplementation::ON_CLICK_EXIT;
        if (str == "On Bullet Shoot") return NodeImplementation::ON_BULLET_SHOOT;
		if (str == "On Bullet Refill") return NodeImplementation::ON_BULLET_REFILL;

        if (str == "Print Message") return NodeImplementation::PRINT_MESSAGE;
        if (str == "Move Object") return NodeImplementation::MOVE_OBJECT;
        if (str == "Rotate Object") return NodeImplementation::ROTATE_OBJECT;
        if (str == "Set Rotation") return NodeImplementation::SET_ROTATION;
		if (str == "Set Rotation Rate of Change") return NodeImplementation::SET_ROTATION_RATE_OF_CHANGE;
		//if (str == "Rotate To") return NodeImplementation::ROTATE_TO;
        if (str == "Set Position") return NodeImplementation::SET_POSITION;
        if (str == "Set Scale") return NodeImplementation::SET_SCALE;
        if (str == "Scale Proportional") return NodeImplementation::SCALE_PROPORTIONAL;
		if (str == "Set Float") return NodeImplementation::SET_FLOAT;
        if (str == "Modify Float Variable") return NodeImplementation::MODIFY_FLOAT;
		if (str == "Play Sound") return NodeImplementation::PLAY_SOUND;
		if (str == "Set State") return NodeImplementation::SET_STATE;
		if (str == "Reset Timer") return NodeImplementation::RESET_TIMER;
        if (str == "Set Texture") return NodeImplementation::SET_TEXTURE;
		if (str == "Set Position On Collided") return NodeImplementation::SET_POSITION_ON_COLLIDED;
		if (str == "Set State On Collided") return NodeImplementation::SET_STATE_ON_COLLIDED;
		if (str == "Apply Force On Collided") return NodeImplementation::APPLY_FORCE_ON_COLLIDED;
        if (str == "Set Global Float") return NodeImplementation::SET_GLOBAL_FLOAT;
        if (str == "Modify Global Float") return NodeImplementation::MODIFY_GLOBAL_FLOAT;
		if (str == "Trigger Bullet Refill Event") return NodeImplementation::TRIGGER_BULLET_REFILL_EVENT;
        if (str == "Restart Level") return NodeImplementation::RESTART_LEVEL;
        if (str == "Load Level") return NodeImplementation::LOAD_LEVEL;
        if (str == "Disable Collision") return NodeImplementation::DISABLE_COLLISION;
		if (str == "Is Sinle Physics") return NodeImplementation::IS_SINLE_PHYSICS;
		if (str == "Enable Physics") return NodeImplementation::ENABLE_PHYSICS;
		if (str == "Disable Physics") return NodeImplementation::DISABLE_PHYSICS;
		if (str == "Enter Water") return NodeImplementation::ENTER_WATER;
		if (str == "Exit Water") return NodeImplementation::EXIT_WATER;
		if (str == "Hit Ground") return NodeImplementation::HIT_GROUND;
        if (str == "Not On Ground") return NodeImplementation::NOT_ON_GROUND;
        if (str == "Change Acceleration Direction") return NodeImplementation::CHANGE_ACCEL_DIRECTION;
        if (str == "Set Sinle Acceleration") return NodeImplementation::SET_SINLE_ACCEL;
        if (str == "Play Random Sound") return NodeImplementation::PLAY_RANDOM_SOUND;
        if (str == "Check Animation End") return NodeImplementation::CHECK_ANIMATION_END;

		if (str == "Check Key Held") return NodeImplementation::CHECK_KEY_HELD;
		if (str == "Compare Value") return NodeImplementation::COMPARE_VALUE;
		if (str == "Compare Variable") return NodeImplementation::COMPARE_VAR;
		if (str == "Check State") return NodeImplementation::CHECK_STATE;
		if (str == "Check Timer") return NodeImplementation::CHECK_TIMER;
        if (str == "Compare Global Float") return NodeImplementation::COMPARE_GLOBAL_FLOAT;
		if (str == "Check Collided State") return NodeImplementation::CHECK_COLLIDED_STATE;

        return NodeImplementation::DEFAULT;
    }

    std::string nodeImplToString(NodeImplementation impl) {
        switch (impl) {
            case NodeImplementation::ON_START: return "On Start";
            case NodeImplementation::ON_UPDATE: return "On Update";
            case NodeImplementation::ON_COLLISION_ENTER: return "On Collision Enter";
            case NodeImplementation::WHILE_COLLIDING: return "While Colliding";
            case NodeImplementation::ON_COLLISION_EXIT: return "On Collision Exit";
            case NodeImplementation::ON_HOVER_ENTER: return "On Hover Enter";
            case NodeImplementation::WHILE_HOVERING: return "While Hovering";
            case NodeImplementation::ON_HOVER_EXIT: return "On Hover Exit";
            case NodeImplementation::ON_CLICK_ENTER: return "On Click Enter";
            case NodeImplementation::WHILE_CLICKING: return "While Clicking";
            case NodeImplementation::ON_CLICK_EXIT: return "On Click Exit";
            case NodeImplementation::ON_BULLET_SHOOT: return "On Bullet Shoot";
			case NodeImplementation::ON_BULLET_REFILL: return "On Bullet Refill";

            case NodeImplementation::PRINT_MESSAGE: return "Print Message";
            case NodeImplementation::MOVE_OBJECT: return "Move Object";
		    case NodeImplementation::ROTATE_OBJECT: return "Rotate Object";
			case NodeImplementation::SET_ROTATION_RATE_OF_CHANGE: return "Set Rotation Rate of Change";
			case NodeImplementation::SET_ROTATION: return "Set Rotation";
			//case NodeImplementation::ROTATE_TO: return "Rotate To";
            case NodeImplementation::SET_POSITION: return "Set Position";
            case NodeImplementation::SET_SCALE: return "Set Scale";
            case NodeImplementation::SCALE_PROPORTIONAL: return "Scale Proportional";
            case NodeImplementation::SET_FLOAT: return "Set Float";
            case NodeImplementation::MODIFY_FLOAT: return "Modify Float Variable";
			case NodeImplementation::PLAY_SOUND: return "Play Sound";
			case NodeImplementation::SET_STATE: return "Set State";
			case NodeImplementation::RESET_TIMER: return "Reset Timer";
            case NodeImplementation::SET_TEXTURE: return "Set Texture";
            case NodeImplementation::SET_POSITION_ON_COLLIDED: return "Set Position On Collided";
			case NodeImplementation::SET_STATE_ON_COLLIDED: return "Set State On Collided";
			case NodeImplementation::APPLY_FORCE_ON_COLLIDED: return "Apply Force On Collided";
			case NodeImplementation::SET_GLOBAL_FLOAT: return "Set Global Float";
			case NodeImplementation::MODIFY_GLOBAL_FLOAT: return "Modify Global Float";
			case NodeImplementation::TRIGGER_BULLET_REFILL_EVENT: return "Trigger Bullet Refill Event";
			case NodeImplementation::RESTART_LEVEL: return "Restart Level";
			case NodeImplementation::LOAD_LEVEL: return "Load Level";
			case NodeImplementation::DISABLE_COLLISION: return "Disable Collision";
			case NodeImplementation::IS_SINLE_PHYSICS: return "Is Sinle Physics";
			case NodeImplementation::ENABLE_PHYSICS: return "Enable Physics";
			case NodeImplementation::DISABLE_PHYSICS: return "Disable Physics";
			case NodeImplementation::ENTER_WATER: return "Enter Water";
			case NodeImplementation::EXIT_WATER: return "Exit Water";
			case NodeImplementation::HIT_GROUND: return "Hit Ground";
            case NodeImplementation::NOT_ON_GROUND: return "Not On Ground";
            case NodeImplementation::CHANGE_ACCEL_DIRECTION: return "Change Acceleration Direction";
            case NodeImplementation::SET_SINLE_ACCEL: return "Set Sinle Acceleration";
            case NodeImplementation::PLAY_RANDOM_SOUND: return "Play Random Sound";
            case NodeImplementation::CHECK_ANIMATION_END: return "Check Animation End";

            case NodeImplementation::CHECK_KEY_HELD: return "Check Key Held";
            case NodeImplementation::COMPARE_VALUE: return "Compare Value";
            case NodeImplementation::COMPARE_VAR: return "Compare Variable";
			case NodeImplementation::CHECK_STATE: return "Check State";
			case NodeImplementation::CHECK_TIMER: return "Check Timer";
			case NodeImplementation::COMPARE_GLOBAL_FLOAT: return "Compare Global Float";
			case NodeImplementation::CHECK_COLLIDED_STATE: return "Check Collided State";

            default: return "Default";
        }
    }

    GameState stringToGamestate(const std::string& stateStr) {
		if (stateStr == "MENU") return GameState::MENU;
		if (stateStr == "PLAYING") return GameState::PLAYING;
		if (stateStr == "CONTROL") return GameState::CONTROL;
		if (stateStr == "PAUSED") return GameState::PAUSED;
        if (stateStr == "EXIT") return GameState::EXIT;
        if (stateStr == "EXITCONFIRMATION") return GameState::EXITCONFIRMATION;
        //if (stateStr == "LEVELS") return GameState::LEVELS;
		return GameState::NONE;
	}

	std::string GameStateToString(GameState gm) {
		switch (gm) {
		case GameState::MENU: return "MENU";
		case GameState::PLAYING: return "PLAYING";
		case GameState::CONTROL: return "CONTROL";
		case GameState::PAUSED: return "PAUSED";
		case GameState::EXIT: return "EXIT";
		case GameState::EXITCONFIRMATION: return "EXITCONFIRMATION";
		//case GameState::LEVELS: return "LEVELS";
		default: return "NONE";
		}
	}

    // Add this helper function in JsonIO.cpp or create a utilities file
    int stringToGLFWKey(const std::string& keyStr) {
        // Movement keys
        if (keyStr == "W") return GLFW_KEY_W;
        if (keyStr == "A") return GLFW_KEY_A;
        if (keyStr == "S") return GLFW_KEY_S;
        if (keyStr == "D") return GLFW_KEY_D;

        // Arrow keys
        if (keyStr == "UP") return GLFW_KEY_UP;
        if (keyStr == "DOWN") return GLFW_KEY_DOWN;
        if (keyStr == "LEFT") return GLFW_KEY_LEFT;
        if (keyStr == "RIGHT") return GLFW_KEY_RIGHT;

        // Special keys
        if (keyStr == "SPACE") return GLFW_KEY_SPACE;
        if (keyStr == "SHIFT") return GLFW_KEY_LEFT_SHIFT;
        if (keyStr == "CTRL") return GLFW_KEY_LEFT_CONTROL;
        if (keyStr == "ALT") return GLFW_KEY_LEFT_ALT;
        if (keyStr == "ENTER") return GLFW_KEY_ENTER;
        if (keyStr == "ESC") return GLFW_KEY_ESCAPE;

        return GLFW_KEY_UNKNOWN;
    }


    /**
     * \brief Atomically replaces a target file with a temporary file.
     * \param tmp Path to the temporary file (already fully written and flushed).
     * \param dst Final destination path to replace.
     * \param err Optional error string out.
     * \return \c true on success, \c false otherwise.
     *
     * Strategy:
     * 1) Try remove(\c dst)   (ignore errors)
     * 2) Try rename(\c tmp,\c dst)
     * 3) If rename fails (not uncommon on Windows), copy(\c tmp -> \c dst) and delete \c tmp .
     */
    bool AtomicMove(const std::string& tmp, const std::string& dst, std::string* err) {
        std::error_code ec;
        std::filesystem::remove(dst, ec);
        std::filesystem::rename(tmp, dst, ec);
        if (ec) {
            std::filesystem::copy_file(tmp, dst,
                std::filesystem::copy_options::overwrite_existing, ec);
            std::filesystem::remove(tmp, ec);
            if (ec) { if (err) *err = "Move failed: " + ec.message(); return false; }
        }
        return true;
    }


    /**
     * \brief Reads a JSON file from disk and parses it into a RapidJSON document.
     * \param path File path to load.
     * \param doc  RapidJSON document to populate.
     * \param err  Optional error string out (on failure).
     * \return \c true on success, \c false if file open, parse, or root type fails.
     *
     * \note On parse error, \c err includes offset and human-readable message.
     */
    bool ReadFileToDocument(const std::string& path,
        rapidjson::Document& doc,
        std::string* err) {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) { if (err) *err = "Could not open " + path; return false; }

        rapidjson::IStreamWrapper isw(ifs);
        doc.ParseStream(isw);

        if (doc.HasParseError()) {
            if (err) *err = std::string("JSON parse error @")
                + std::to_string(doc.GetErrorOffset()) + ": "
                + rapidjson::GetParseError_En(doc.GetParseError());
            return false;
        }
        if (!doc.IsObject()) { if (err) *err = "Root JSON must be an object"; return false; }
        return true;
    }


    /**
     * \brief Writes a RapidJSON document to disk (pretty or compact) using a temp file.
     * \param path   Destination file path.
     * \param doc    RapidJSON document to write.
     * \param pretty If \c true , use \c PrettyWriter with 2-space indent.
     * \param err    Optional error string out (on failure).
     * \return \c true on success, \c false if writing or atomic move fails.
     *
     * \details The function creates the parent directory if needed, writes to
     *          \c path + ".tmp" first, then atomically replaces the target.
     */
    bool WriteDocumentToFile(const std::string& path,
        rapidjson::Document& doc,
        bool pretty,
        std::string* err) {
        std::error_code ec;
        const auto parent = std::filesystem::path(path).parent_path();
        if (!parent.empty())
            std::filesystem::create_directories(parent, ec);

        const std::string tmp = path + ".tmp";
        std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
        if (!ofs) { if (err) *err = "Cannot open temp file for writing: " + tmp; return false; }

        rapidjson::OStreamWrapper osw(ofs);
        if (pretty) {
            rapidjson::PrettyWriter<rapidjson::OStreamWrapper> w(osw);
            w.SetIndent(' ', 2);
            doc.Accept(w);
        }
        else {
            rapidjson::Writer<rapidjson::OStreamWrapper> w(osw);
            doc.Accept(w);
        }

        ofs.flush();
        ofs.close();
        return AtomicMove(tmp, path, err);
    }


    /// \name JSON small-value helpers
    /// @{

    /**
     * \brief Fetches a sub-value by key if the parent is an object.
     * \param parent Object to read from.
     * \param key    Null-terminated key string.
     * \return Pointer to value if present and parent is object; otherwise \c nullptr .
     */
    const rapidjson::Value* GetObj(const rapidjson::Value& parent, const char* key) {
        if (!parent.IsObject() || !parent.HasMember(key)) return nullptr;
        return &parent[key];
    }

    /**
     * \brief Reads a string field from a JSON object.
     * \param obj  JSON object.
     * \param key  Field name.
     * \param out  Output string on success.
     * \return \c true if present and string; otherwise \c false .
     */
    bool GetString(const rapidjson::Value& obj, const char* key, std::string& out) {
        if (auto* v = GetObj(obj, key); v && v->IsString()) { out = v->GetString(); return true; }
        return false;
    }

    /**
     * \brief Reads a boolean-like field (bool, int, or number) from a JSON object.
     * \param obj JSON object.
     * \param key Field name.
     * \param out Output boolean (non-zero numeric treated as true).
     * \return \c true on success, \c false otherwise.
     */
    bool GetBool(const rapidjson::Value& obj, const char* key, bool& out) {
        if (auto* v = GetObj(obj, key)) {
            if (v->IsBool()) { out = v->GetBool();   return true; }
            if (v->IsInt()) { out = (v->GetInt() != 0);  return true; }
            if (v->IsNumber()) { out = (v->GetDouble() != 0.0); return true; }
        }
        return false;
    }

    /**
     * \brief Reads an integer field from a JSON object.
     */
    bool GetInt(const rapidjson::Value& obj, const char* key, int& out) {
        if (auto* v = GetObj(obj, key); v && v->IsInt()) { out = v->GetInt(); return true; }
        return false;
    }

    /**
     * \brief Reads a float field from a JSON object.
     * \details Accepts either float or generic number; performs safe cast.
     */
    bool GetFloat(const rapidjson::Value& obj, const char* key, float& out) {
        if (auto* v = GetObj(obj, key)) {
            if (v->IsFloat()) { out = v->GetFloat();  return true; }
            if (v->IsNumber()) { out = static_cast<float>(v->GetDouble()); return true; }
        }
        return false;
    }

    /// @}


    /// \name Vec3 helpers
    /// @{

    bool GetArray2f(const rapidjson::Value& obj, const char* key, std::array<float, 2>& out) {
        auto* v = GetObj(obj, key);
        if (!v || !v->IsArray() || v->Size() != 2) return false;
        for (rapidjson::SizeType i = 0; i < 2; ++i) {
            if (!(*v)[i].IsNumber()) return false;
            out[i] = static_cast<float>((*v)[i].GetDouble());
        }
        return true;
    }

    /**
     * \brief Reads an array of exactly 3 numeric values into \c out .
     * \param obj JSON object.
     * \param key Field name.
     * \param out Output array \c {x,y,z} .
     * \return \c true on success; \c false if shape/type mismatch.
     */
    bool GetArray3f(const rapidjson::Value& obj, const char* key, std::array<float, 3>& out) {
        auto* v = GetObj(obj, key);
        if (!v || !v->IsArray() || v->Size() != 3) return false;
        for (rapidjson::SizeType i = 0; i < 3; ++i) {
            if (!(*v)[i].IsNumber()) return false;
            out[i] = static_cast<float>((*v)[i].GetDouble());
        }
        return true;
    }

    /**
     * \brief Reads an array of exactly 4 numeric values into \c out .
     * \param obj JSON object.
     * \param key Field name.
     * \param out Output array \c {x,y,z,w} .
     * \return \c true on success; \c false if shape/type mismatch.
	 */
    bool GetArray4f(const rapidjson::Value& obj, const char* key, std::array<float, 4>& out) {
        auto* v = GetObj(obj, key);
        if (!v || !v->IsArray() || v->Size() != 4) return false;
        for (rapidjson::SizeType i = 0; i < 4; ++i) {
            if (!(*v)[i].IsNumber()) return false;
            out[i] = static_cast<float>((*v)[i].GetDouble());
        }
        return true;
    }

    /**
     * \brief Writes a 2-float array field under \c key into \c obj .
	 */
    void PutArray2f(rapidjson::Value& obj, const char* key, const std::array<float, 2>& v, rapidjson::Document::AllocatorType& a) {
        rapidjson::Value arr(rapidjson::kArrayType);
        arr.PushBack(rapidjson::Value().SetFloat(v[0]), a);
        arr.PushBack(rapidjson::Value().SetFloat(v[1]), a);
        obj.AddMember(rapidjson::Value(key, a), arr, a);
    }

    /**
     * \brief Writes a 3-float array field under \c key into \c obj .
     */
    void PutArray3f(rapidjson::Value& obj, const char* key, const std::array<float, 3>& v, rapidjson::Document::AllocatorType& a) {
        rapidjson::Value arr(rapidjson::kArrayType);
        arr.PushBack(rapidjson::Value().SetFloat(v[0]), a);
        arr.PushBack(rapidjson::Value().SetFloat(v[1]), a);
        arr.PushBack(rapidjson::Value().SetFloat(v[2]), a);
        obj.AddMember(rapidjson::Value(key, a), arr, a);
    }
    /**
     * \brief Writes a 4-float array field under \c key into \c obj .
	 */
    void PutArray4f(rapidjson::Value& obj, const char* key, const std::array<float, 4>& v, rapidjson::Document::AllocatorType& a) {
        rapidjson::Value arr(rapidjson::kArrayType);
        arr.PushBack(rapidjson::Value().SetFloat(v[0]), a);
        arr.PushBack(rapidjson::Value().SetFloat(v[1]), a);
        arr.PushBack(rapidjson::Value().SetFloat(v[2]), a);
        arr.PushBack(rapidjson::Value().SetFloat(v[3]), a);
        obj.AddMember(rapidjson::Value(key, a), arr, a);
    }

    /**
     * \brief Reads a 2-float array field into a \c glm::vec2 .
	 */
    bool ReadVec2(const rapidjson::Value& obj, const char* key, glm::vec2& out) {
        std::array<float, 2> tmp{};
        if (!GetArray2f(obj, key, tmp)) return false;
        out = glm::vec2(tmp[0], tmp[1]);
        return true;
    }

    /**
     * \brief Reads a 3-float array field into a \c glm::vec3 .
     */
    bool ReadVec3(const rapidjson::Value& obj, const char* key, glm::vec3& out) {
        std::array<float, 3> tmp{};
        if (!GetArray3f(obj, key, tmp)) return false;
        out = glm::vec3(tmp[0], tmp[1], tmp[2]);
        return true;
    }

    /**
     * \brief Reads a 4-float array field into a \c glm::vec4 .
	 */
    bool ReadVec4(const rapidjson::Value& obj, const char* key, glm::vec4& out) {
        std::array<float, 4> tmp{};
        if (!GetArray4f(obj, key, tmp)) return false;
        out = glm::vec4(tmp[0], tmp[1], tmp[2], tmp[3]);
        return true;
    }

    /**
     * \brief Writes a \c glm::vec2 to JSON as a 2-float array.
	 */
    void WriteVec2(rapidjson::Value& obj, const char* key, const glm::vec2& v, rapidjson::Document::AllocatorType& a) {
        PutArray2f(obj, key, { v.x, v.y }, a);
    }

    /**
     * \brief Writes a \c glm::vec3 to JSON as a 3-float array.
     */
    void WriteVec3(rapidjson::Value& obj, const char* key, const glm::vec3& v, rapidjson::Document::AllocatorType& a) {
        PutArray3f(obj, key, { v.x, v.y, v.z }, a);
    }

    /**
     * \brief Writes a \c glm::vec4 to JSON as a 4-float array.
	 */
    void WriteVec4(rapidjson::Value& obj, const char* key, const glm::vec4& v, rapidjson::Document::AllocatorType& a) {
        PutArray4f(obj, key, { v.x, v.y, v.z, v.w }, a);
    }

    /**
     * \brief Reads a 2-float array field into two separate floats.
	 */
    bool ReadVec2(const rapidjson::Value& obj, const char* key, float& x, float& y) {
        std::array<float, 2> tmp{};
        if (!GetArray2f(obj, key, tmp)) return false;
        x = tmp[0]; y = tmp[1];
        return true;
    }

    /**
     * \brief Reads a 3-float array field into three separate floats.
     */
    bool ReadVec3(const rapidjson::Value& obj, const char* key, float& x, float& y, float& z) {
        std::array<float, 3> tmp{};
        if (!GetArray3f(obj, key, tmp)) return false;
        x = tmp[0]; y = tmp[1]; z = tmp[2];
        return true;
    }

    /**
     * \brief Reads a 4-float array field into four separate floats.
	 */
    bool ReadVec4(const rapidjson::Value& obj, const char* key, float& x, float& y, float& z, float& w) {
        std::array<float, 4> tmp{};
        if (!GetArray4f(obj, key, tmp)) return false;
		x = tmp[0]; y = tmp[1]; z = tmp[2]; w = tmp[3];
        return true;
    }

    /**
	 * \brief Writes two floats as a 2-float array field.
     * */
    void WriteVec2(rapidjson::Value& obj, const char* key, float x, float y, rapidjson::Document::AllocatorType& a) {
        PutArray2f(obj, key, { x, y }, a);
    }

    /**
     * \brief Writes three floats as a 3-float array field.
     */
    void WriteVec3(rapidjson::Value& obj, const char* key, float x, float y, float z, rapidjson::Document::AllocatorType& a) {
        PutArray3f(obj, key, { x, y, z }, a);
    }

    /**
	 * \brief Writes four floats as a 4-float array field.
     * */
    void WriteVec4(rapidjson::Value& obj, const char* key, float x, float y, float z, float w, rapidjson::Document::AllocatorType& a) {
        PutArray4f(obj, key, { x, y, z, w }, a);
    }

    /// @}


    /// \name Color helpers (RGBA)
    /// @{

    /**
     * \brief Reads an RGBA array field into separate components.
     * \return \c true on success; requires exactly four numeric elements.
     */
    bool ReadColor(const rapidjson::Value& obj, const char* key, float& r, float& g, float& b, float& a) {
        const auto* v = GetObj(obj, key);
        if (!v || !v->IsArray() || v->Size() != 4) return false;

        for (rapidjson::SizeType i = 0; i < 4; ++i) {
            if (!(*v)[i].IsNumber()) return false;
        }

        r = static_cast<float>((*v)[0].GetDouble());
        g = static_cast<float>((*v)[1].GetDouble());
        b = static_cast<float>((*v)[2].GetDouble());
        a = static_cast<float>((*v)[3].GetDouble());
        return true;
    }

    /**
     * \brief Writes RGBA components as a 4-float array field.
     */
    void WriteColor(rapidjson::Value& obj, const char* key, float r, float g, float b, float a, rapidjson::Document::AllocatorType& alloc) {
        rapidjson::Value arr(rapidjson::kArrayType);
        arr.PushBack(rapidjson::Value().SetFloat(r), alloc);
        arr.PushBack(rapidjson::Value().SetFloat(g), alloc);
        arr.PushBack(rapidjson::Value().SetFloat(b), alloc);
        arr.PushBack(rapidjson::Value().SetFloat(a), alloc);
        obj.AddMember(rapidjson::Value(key, alloc), arr, alloc);
    }

    /**
     * \brief Reads RGBA into a \c glm::vec4 .
     */
    bool ReadColor(const rapidjson::Value& obj, const char* key, glm::vec4& out) {
        float r, g, b, a;
        if (!ReadColor(obj, key, r, g, b, a)) return false;
        out = glm::vec4(r, g, b, a);
        return true;
    }

    /**
     * \brief Writes a \c glm::vec4 as a 4-float RGBA array.
     */
    void WriteColor(rapidjson::Value& obj, const char* key, const glm::vec4& v, rapidjson::Document::AllocatorType& alloc) {
        WriteColor(obj, key, v.r, v.g, v.b, v.a, alloc);
    }

    /// @}

    /* ---------- Load/Save collision tags ---------- */
    bool LoadCollisionTags(const std::string& filepath, std::vector<std::string>& outTags, std::string* err) {
        rapidjson::Document doc;
        if (!ReadFileToDocument(RUNTIME_DIR_R + filepath, doc, err)) return false;

        if (!doc.IsObject()) {
            if (err) *err = "Collision tag file is not a JSON object";
            return false;
        }

        if (!doc.HasMember("collisionTags") || !doc["collisionTags"].IsArray()) {
            if (err) *err = "Missing or invalid 'tags' array";
            return false;
        }

        outTags.clear();
        for (const auto& v : doc["collisionTags"].GetArray()) {
            if (v.IsString()) {
                outTags.emplace_back(v.GetString());
            }
        }

        return true;
    }

    bool SaveCollisionTags(const std::string& filepath, const std::vector<std::string>& tags, std::string* err) {
        rapidjson::Document doc;
        doc.SetObject();
        auto& a = doc.GetAllocator();

        doc.AddMember("version", 1, a);

        rapidjson::Value tagArray(rapidjson::kArrayType);
        for (const auto& tag : tags) {
            tagArray.PushBack(rapidjson::Value(tag.c_str(), a), a);
        }

        doc.AddMember("collisionTags", tagArray, a);

        bool runtime = WriteDocumentToFile(RUNTIME_DIR_R + filepath, doc, true, err);
        bool proj = WriteDocumentToFile(PathManager::Instance().getSourcePath() + "/assets" + filepath, doc, true, err);

        return (runtime && proj);
    }

    /* ---------- Saving obj state in json for undo/redo ---------- */
    std::string serializeTransform(GameObject* obj) {
        Transform* t = obj ? obj->getComponent<Transform>() : nullptr;
        if (!t) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value jt(kObjectType);
        WriteVec3(jt, "pos", t->x, t->y, t->z, a);
        jt.AddMember("rotation", Value().SetFloat(t->rotation), a);
        WriteVec3(jt, "scale", t->scaleX, t->scaleY, t->scaleZ, a);
        doc.AddMember("Transform", jt, a);

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeTransform(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;

        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("Transform")) return;

        const Value& jt = doc["Transform"];
        Transform* t = obj->hasComponent<Transform>()
            ? obj->getComponent<Transform>()
            : obj->addComponent<Transform>();

        ReadVec3(jt, "pos", t->x, t->y, t->z);
        GetFloat(jt, "rotation", t->rotation);
        ReadVec3(jt, "scale", t->scaleX, t->scaleY, t->scaleZ);
    }

    std::string serializeRender(GameObject* obj) {
        Render* r = obj ? obj->getComponent<Render>() : nullptr;
        if (!r) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value jr(kObjectType);
        jr.AddMember("shape", Value(ShapeToStr(r->modelRef.shape), a), a);
        jr.AddMember("hasTex", Value().SetBool(r->hasTex), a);
        if (r->hasTex && !r->texFile.empty())
            jr.AddMember("texture", Value(r->texFile.c_str(), a), a);
        jr.AddMember("hasAnimation", Value(r->hasAnimation ? 1 : 0), a);
        WriteVec4(jr, "clr", r->clr.r, r->clr.g, r->clr.b, r->clr.a, a);
        doc.AddMember("Render", jr, a);

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeRender(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;

        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("Render")) return;

        const Value& jr = doc["Render"];
        Render* r = obj->hasComponent<Render>()
            ? obj->getComponent<Render>()
            : obj->addComponent<Render>();

        std::string shp = (jr.HasMember("shape") && jr["shape"].IsString())
            ? jr["shape"].GetString() : "square";
        r->modelRef = renderer::models[static_cast<int>(StrToShape(shp))];

        if (jr.HasMember("hasTex"))
            r->hasTex = jr["hasTex"].IsBool() ? jr["hasTex"].GetBool() : (jr["hasTex"].GetInt() != 0);

        if (r->hasTex && jr.HasMember("texture") && jr["texture"].IsString()) {
            r->texFile = jr["texture"].GetString();
            r->texHDL = 0;
            r->texChanged = true;
        }
        else {
            r->texFile.clear();
            r->texHDL = 0;
            r->texChanged = false;
        }

        if (jr.HasMember("hasAnimation"))
            r->hasAnimation = jr["hasAnimation"].IsBool()
            ? jr["hasAnimation"].GetBool() : (jr["hasAnimation"].GetInt() != 0);

        ReadVec4(jr, "clr", r->clr.r, r->clr.g, r->clr.b, r->clr.a);
    }

    std::string serializeParticle(GameObject* obj) {
        Particle* p = obj ? obj->getComponent<Particle>() : nullptr;
        if (!p || !p->isMaster) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value jp(kObjectType);
        jp.AddMember("isMaster", Value(p->isMaster ? 1 : 0), a);
        WriteVec4(jp, "clr", p->master.clr.r, p->master.clr.g, p->master.clr.b, p->master.clr.a, a);
        WriteVec2(jp, "velocity", p->master.velocity.x, p->master.velocity.y, a);
        jp.AddMember("timer", Value(p->master.timer), a);
        jp.AddMember("offset", Value(p->master.offset), a);
        jp.AddMember("particleAmt", Value(p->master.particleAmt), a);
        doc.AddMember("Particle", jp, a);

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeParticle(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("Particle")) return;

        const Value& jp = doc["Particle"];
        Particle* p = obj->hasComponent<Particle>()
            ? obj->getComponent<Particle>()
            : obj->addComponent<Particle>();

        if (jp.HasMember("isMaster"))
            p->isMaster = jp["isMaster"].IsBool() ? jp["isMaster"].GetBool() : (jp["isMaster"].GetInt() != 0);

        ReadVec4(jp, "clr", p->master.clr.r, p->master.clr.g, p->master.clr.b, p->master.clr.a);
        ReadVec2(jp, "velocity", p->master.velocity.x, p->master.velocity.y);
        if (jp.HasMember("timer") && jp["timer"].IsNumber())      p->master.timer = jp["timer"].GetFloat();
        if (jp.HasMember("offset") && jp["offset"].IsNumber())     p->master.offset = jp["offset"].GetFloat();
        if (jp.HasMember("particleAmt") && jp["particleAmt"].IsInt())   p->master.particleAmt = jp["particleAmt"].GetInt();
    }

    std::string serializePhysics(GameObject* obj) {
        Physics* p = obj ? obj->getComponent<Physics>() : nullptr;
        if (!p) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value jp(kObjectType);
        jp.AddMember("physicsFlag", p->physicsFlag, a);
        jp.AddMember("autoUpdate", p->autoUpdate, a);
        jp.AddMember("moveSpeed", p->moveSpeed, a);
        jp.AddMember("jumpForce", p->jumpForce, a);
        jp.AddMember("damping", p->damping, a);
        //jp.AddMember("friction", p->friction, a);
        jp.AddMember("maxFallSpeed", p->maxFallSpeed, a);
        jp.AddMember("groundAccelForce", p->groundAccelForce, a);
        jp.AddMember("airAccelForce", p->airAccelForce, a);
        jp.AddMember("apexHangMul", p->apexHangMul, a);
        jp.AddMember("jumpBufferTime", p->jumpBufferTime, a);
        jp.AddMember("coyoteTime", p->coyoteTime, a);
        jp.AddMember("ledgeAssistRange", p->ledgeAssistRange, a);
        jp.AddMember("ledgeClimbSpeed", p->ledgeClimbSpeed, a);
        jp.AddMember("mass", p->dynamics.mass, a);
        jp.AddMember("buoyancy", p->buoyancy, a);
        jp.AddMember("freezeRotation", p->freezeRotation, a);
        jp.AddMember("angularDrag", p->dynamics.angularDrag, a);
        jp.AddMember("inertiaOverride", p->inertiaOverride, a);

        doc.AddMember("Physics", jp, a);

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializePhysics(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;

        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("Physics")) return;

        const Value& jp = doc["Physics"];
        Physics* p = obj->hasComponent<Physics>()
            ? obj->getComponent<Physics>()
            : obj->addComponent<Physics>();

        p->physicsFlag = jp["physicsFlag"].GetBool();
        p->autoUpdate = jp["autoUpdate"].GetBool();
        p->moveSpeed = jp["moveSpeed"].GetFloat();
        p->jumpForce = jp["jumpForce"].GetFloat();
        p->damping = jp["damping"].GetFloat();
        //p->friction = jp.HasMember("friction") ? jp["friction"].GetFloat() : 0.80f;
        p->maxFallSpeed = jp.HasMember("maxFallSpeed") ? jp["maxFallSpeed"].GetFloat() : -30.f;
        p->groundAccelForce = jp.HasMember("groundAccelForce") ? jp["groundAccelForce"].GetFloat() : 20.f;
        p->airAccelForce = jp.HasMember("airAccelForce") ? jp["airAccelForce"].GetFloat() : 8.f;
        p->apexHangMul = jp.HasMember("apexHangMul") ? jp["apexHangMul"].GetFloat() : 0.2f;
        p->jumpBufferTime = jp.HasMember("jumpBufferTime") ? jp["jumpBufferTime"].GetFloat() : 0.15f;
        p->coyoteTime = jp.HasMember("coyoteTime") ? jp["coyoteTime"].GetFloat() : 0.15f;
        p->ledgeAssistRange = jp.HasMember("ledgeAssistRange") ? jp["ledgeAssistRange"].GetFloat() : 0.8f;
        p->ledgeClimbSpeed = jp.HasMember("ledgeClimbSpeed") ? jp["ledgeClimbSpeed"].GetFloat() : 8.f;
        p->dynamics.mass = jp["mass"].GetFloat();
        p->buoyancy = jp["buoyancy"].GetBool();
        if (jp.HasMember("freezeRotation")) p->freezeRotation = jp["freezeRotation"].GetBool();
        if (jp.HasMember("angularDrag")) p->dynamics.angularDrag = jp["angularDrag"].GetFloat();
        if (jp.HasMember("inertiaOverride")) p->inertiaOverride = jp["inertiaOverride"].GetFloat();

        if (!p->freezeRotation) {
            if (auto* c = obj->getComponent<CollisionInfo>())
                p->recalculateInertia(c->colliderSize, c->colliderType);
        }
    }

    std::string serializeCollision(GameObject* obj) {
        CollisionInfo* c = obj ? obj->getComponent<CollisionInfo>() : nullptr;
        if (!c) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value jc(kObjectType);
        jc.AddMember("collisionFlag", c->collisionFlag, a);
        jc.AddMember("autoFitScale", c->autoFitScale, a);
        jc.AddMember("width", c->colliderSize.x, a);
        jc.AddMember("height", c->colliderSize.y, a);
        jc.AddMember("x", c->colliderPos.x, a);
        jc.AddMember("y", c->colliderPos.y, a);
        jc.AddMember("rotation", c->colliderRot, a);
        jc.AddMember("collisionRes", Value(CollisionResponseModeToStr(c->collisionRes), a), a);
        jc.AddMember("collisionTag", Value(c->collisionTag.c_str(), a), a);
        jc.AddMember("colliderType", Value(ShapeToStr(c->colliderType), a), a);
        jc.AddMember("bounciness", c->material.bounciness, a);
        jc.AddMember("friction", c->material.friction, a);
        jc.AddMember("bounceCombine", static_cast<int>(c->material.bounceCombine), a);
        jc.AddMember("frictionCombine", static_cast<int>(c->material.frictionCombine), a);

        Value ignoreArray(kArrayType);
        for (const std::string& tag : c->ignoreCollisionsWith)
            ignoreArray.PushBack(Value(tag.c_str(), a), a);
        jc.AddMember("ignoreCollisionsWith", ignoreArray, a);
        doc.AddMember("Collision", jc, a);

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeCollision(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("Collision")) return;

        const Value& jc = doc["Collision"];
        CollisionInfo* c = obj->hasComponent<CollisionInfo>()
            ? obj->getComponent<CollisionInfo>()
            : obj->addComponent<CollisionInfo>();

        c->collisionFlag = jc["collisionFlag"].GetBool();
        c->autoFitScale = jc["autoFitScale"].GetBool();
        c->colliderSize.x = JsonIO::GetFloatOr(jc, "width", 0.f);
        c->colliderSize.y = JsonIO::GetFloatOr(jc, "height", 0.f);
        c->colliderPos.x = JsonIO::GetFloatOr(jc, "x", 0.f);
        c->colliderPos.y = JsonIO::GetFloatOr(jc, "y", 0.f);
        c->colliderRot = JsonIO::GetFloatOr(jc, "rotation", 0.f);

        if (jc.HasMember("collisionRes")) c->collisionRes = StrToCollisionResponseMode(jc["collisionRes"].GetString());
        if (jc.HasMember("collisionTag")) c->collisionTag = jc["collisionTag"].GetString();
        if (jc.HasMember("colliderType") && jc["colliderType"].IsString())
            c->colliderType = StrToShape(jc["colliderType"].GetString());

        if (jc.HasMember("bounciness")) c->material.bounciness = jc["bounciness"].GetFloat();
        if (jc.HasMember("friction")) c->material.friction = jc["friction"].GetFloat();
        if (jc.HasMember("bounceCombine")) c->material.bounceCombine = static_cast<PhysicsMaterial::CombineMode>(jc["bounceCombine"].GetInt());
        if (jc.HasMember("frictionCombine")) c->material.frictionCombine = static_cast<PhysicsMaterial::CombineMode>(jc["frictionCombine"].GetInt());

        if (jc.HasMember("ignoreCollisionsWith")) {
            const Value& arr = jc["ignoreCollisionsWith"];
            c->ignoreCollisionsWith.clear();
            for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
                c->ignoreCollisionsWith.insert(arr[i].GetString());
        }

        if(Physics* p = obj->getComponent<Physics>())
            if (!p->freezeRotation) {
                p->recalculateInertia(c->colliderSize, c->colliderType);
            }
    }

    std::string serializeInput(GameObject* obj) {
        if (!obj || !obj->hasComponent<Input>()) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        doc.AddMember("Input", true, doc.GetAllocator());
        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeInput(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("Input")) return;
        if (!obj->hasComponent<Input>()) obj->addComponent<Input>();
    }

    std::string serializeFont(GameObject* obj) {
        FontComponent* f = obj ? obj->getComponent<FontComponent>() : nullptr;
        if (!f) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value jf(kObjectType);
        jf.AddMember("word", Value(f->word.c_str(), a), a);
        jf.AddMember("scale", f->scale, a);
        jf.AddMember("fontType", f->fontType, a);
        WriteVec3(jf, "clr", f->clr.x, f->clr.y, f->clr.z, a);
        doc.AddMember("Font", jf, a);

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeFont(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;

        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("Font")) return;

        const Value& jf = doc["Font"];
        FontComponent* f = obj->hasComponent<FontComponent>()
            ? obj->getComponent<FontComponent>()
            : obj->addComponent<FontComponent>();

        f->word = jf["word"].GetString();
        f->scale = jf["scale"].GetFloat();
        f->fontType = jf["fontType"].GetInt();
        ReadVec3(jf, "clr", f->clr.x, f->clr.y, f->clr.z);
    }

    std::string serializeAudio(GameObject* obj) {
        AudioComponent* audio = obj ? obj->getComponent<AudioComponent>() : nullptr;
        if (!audio) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value ja(kObjectType);
        Value channels(kObjectType);
        for (const auto& [channelName, ch] : audio->audioChannels) {
            Value channelObj(kObjectType);
            channelObj.AddMember("audioFile", Value(ch.audioFile.c_str(), a), a);
            channelObj.AddMember("volume", ch.volume, a);
            channelObj.AddMember("pitch", ch.pitch, a);
            channelObj.AddMember("loop", ch.loop, a);
            channelObj.AddMember("muted", ch.muted, a);
            channelObj.AddMember("playOnStart", ch.playOnStart, a);
            channelObj.AddMember("fadeInOnStart", ch.fadeInOnStart, a);
            channelObj.AddMember("fadeInDuration", ch.fadeInDuration, a);
            channelObj.AddMember("fadeOutOnStop", ch.fadeOutOnStop, a);
            channelObj.AddMember("fadeOutDuration", ch.fadeOutDuration, a);
            channelObj.AddMember("isSpatial", ch.isSpatial, a);
            channelObj.AddMember("minDistance", ch.minDistance, a);
            channelObj.AddMember("maxDistance", ch.maxDistance, a);
            channelObj.AddMember("spread", ch.spread, a);
            channelObj.AddMember("persistOnSceneChange", ch.persistOnSceneChange, a);
            channels.AddMember(Value(channelName.c_str(), a), channelObj, a);
        }
        ja.AddMember("channels", channels, a);
        doc.AddMember("AudioComponent", ja, a);

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeAudio(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("AudioComponent")) return;

        const Value& ja = doc["AudioComponent"];
        AudioComponent* ac = obj->hasComponent<AudioComponent>()
            ? obj->getComponent<AudioComponent>()
            : obj->addComponent<AudioComponent>();

        // stop anything currently playing before overwriting
        for (auto& [name, ch] : ac->audioChannels)
            if (ch.channel) { ch.channel->stop(); ch.channel = nullptr; }
        ac->audioChannels.clear();

        if (ja.HasMember("channels") && ja["channels"].IsObject()) {
            const Value& channels = ja["channels"];
            for (auto it = channels.MemberBegin(); it != channels.MemberEnd(); ++it) {
                std::string channelName = it->name.GetString();
                const Value& co = it->value;
                AudioChannel* ch = ac->getOrCreateChannel(channelName);

                if (co.HasMember("audioFile"))       ch->audioFile = co["audioFile"].GetString();
                if (co.HasMember("volume"))          ch->volume = co["volume"].GetFloat();
                if (co.HasMember("pitch"))           ch->pitch = co["pitch"].GetFloat();
                if (co.HasMember("loop"))            ch->loop = co["loop"].GetBool();
                if (co.HasMember("muted"))           ch->muted = co["muted"].GetBool();
                if (co.HasMember("playOnStart"))     ch->playOnStart = co["playOnStart"].GetBool();
                if (co.HasMember("fadeInOnStart"))   ch->fadeInOnStart = co["fadeInOnStart"].GetBool();
                if (co.HasMember("fadeInDuration"))  ch->fadeInDuration = co["fadeInDuration"].GetFloat();
                if (co.HasMember("fadeOutOnStop"))   ch->fadeOutOnStop = co["fadeOutOnStop"].GetBool();
                if (co.HasMember("fadeOutDuration")) ch->fadeOutDuration = co["fadeOutDuration"].GetFloat();
                if (co.HasMember("isSpatial"))       ch->isSpatial = co["isSpatial"].GetBool();
                if (co.HasMember("minDistance"))     ch->minDistance = co["minDistance"].GetFloat();
                if (co.HasMember("maxDistance"))     ch->maxDistance = co["maxDistance"].GetFloat();
                if (co.HasMember("spread"))     ch->spread = co["spread"].GetFloat();
                if (co.HasMember("persistOnSceneChange"))     ch->persistOnSceneChange = co["persistOnSceneChange"].GetBool();
            }
        }
    }

    // Animation serializer - base implementation
    std::string serializeAnimation(GameObject* obj) {
        Animation* anim = obj ? obj->getComponent<Animation>() : nullptr;
        if (!anim) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value js(kObjectType);
        for (size_t i = 0; i < stateNames.size() && i < anim->animState.size(); ++i) {
            const AnimateState& st = anim->animState[i];
            Value stateObj(kObjectType);
            Value animStateObj(kObjectType);
            animStateObj.AddMember("texture", Value(st.texFile.c_str(), a), a);
            animStateObj.AddMember("loop", Value(st.loop ? 1 : 0), a);
            animStateObj.AddMember("play", Value(st.play ? 1 : 0), a);
            animStateObj.AddMember("totalColumn", Value(st.totalColumn), a);
            animStateObj.AddMember("totalRow", Value(st.totalRow), a);
            animStateObj.AddMember("frameTime", Value(st.frameTime), a);
            animStateObj.AddMember("initialFramCol", Value(st.initialFrame.x), a);
            animStateObj.AddMember("initialFramRow", Value(st.initialFrame.y), a);
            animStateObj.AddMember("lastFramCol", Value(st.lastFrame.x), a);
            animStateObj.AddMember("lastFramRow", Value(st.lastFrame.y), a);
            stateObj.AddMember("animState", animStateObj, a);
            js.AddMember(Value(stateNames[i].c_str(), a), stateObj, a);
        }
        doc.AddMember("Animation", js, a);

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeAnimation(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("Animation")) return;

        const Value& smObj = doc["Animation"];
        Animation* anim = obj->hasComponent<Animation>()
            ? obj->getComponent<Animation>()
            : obj->addComponent<Animation>();
        anim->animState.resize(stateNames.size());

        for (size_t i = 0; i < stateNames.size() && i < anim->animState.size(); ++i) {
            if (!smObj.HasMember(stateNames[i].c_str())) continue;
            const Value& stateObj = smObj[stateNames[i].c_str()];
            if (!stateObj.IsObject() || !stateObj.HasMember("animState")) continue;
            const Value& ao = stateObj["animState"];
            if (!ao.IsObject()) continue;
            AnimateState& st = anim->animState[i];

            if (ao.HasMember("texture") && ao["texture"].IsString()) st.texFile = ao["texture"].GetString();
            else { st.texFile.clear(); st.texChanged = false; }

            st.texChanged = true;

            auto readBoolOrInt = [&](const char* key, bool& out) {
                if (!ao.HasMember(key)) return;
                out = ao[key].IsBool() ? ao[key].GetBool() : (ao[key].GetInt() != 0);
                };
            readBoolOrInt("loop", st.loop);
            readBoolOrInt("play", st.play);

            if (ao.HasMember("totalColumn") && ao["totalColumn"].IsInt())      st.totalColumn = ao["totalColumn"].GetInt();
            if (ao.HasMember("totalRow") && ao["totalRow"].IsInt())         st.totalRow = ao["totalRow"].GetInt();
            if (ao.HasMember("frameTime") && ao["frameTime"].IsNumber())     st.frameTime = ao["frameTime"].GetFloat();
            if (ao.HasMember("initialFramCol") && ao["initialFramCol"].IsNumber()) st.initialFrame.x = ao["initialFramCol"].GetFloat();
            if (ao.HasMember("initialFramRow") && ao["initialFramRow"].IsNumber()) st.initialFrame.y = ao["initialFramRow"].GetFloat();
            if (ao.HasMember("lastFramCol") && ao["lastFramCol"].IsNumber())    st.lastFrame.x = ao["lastFramCol"].GetFloat();
            if (ao.HasMember("lastFramRow") && ao["lastFramRow"].IsNumber())    st.lastFrame.y = ao["lastFramRow"].GetFloat();
        }
    }

    std::string serializeStateMachine(GameObject* obj) {
        if (!obj || !obj->hasComponent<StateMachine>()) return "";

        // if no animation component, just save empty statemachine shell
        if (!obj->hasComponent<Animation>()) return "{\"StateMachine\":{}}";

        // reuse animation serializer, then swap the key
        std::string animJson = serializeAnimation(obj);
        if (animJson.empty()) return "{\"StateMachine\":{}}";

        // swap "Animation" key to "StateMachine"
        using namespace rapidjson;
        Document doc; doc.Parse(animJson.c_str());
        if (doc.HasParseError() || !doc.HasMember("Animation"))
            return "{\"StateMachine\":{}}";

        // move value from "Animation" to "StateMachine"
        Value smValue(std::move(doc["Animation"]));  // not a copy, move it
        doc.RemoveMember("Animation");
        doc.AddMember("StateMachine", smValue, doc.GetAllocator());

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeStateMachine(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("StateMachine")) return;

        // ensure StateMachine component exists
        if (!obj->hasComponent<StateMachine>()) obj->addComponent<StateMachine>();

        // swap "StateMachine" key to "Animation" then reuse animation deserializer
        Value animValue(std::move(doc["StateMachine"]));
        doc.RemoveMember("StateMachine");
        doc.AddMember("Animation", animValue, doc.GetAllocator());

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        std::string animJson = buf.GetString();

        deserializeAnimation(obj, animJson);
    }


    std::string serializeTileMap(GameObject* obj) {
        TileMap* tm = obj ? obj->getComponent<TileMap>() : nullptr;
        if (!tm) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value jt(kObjectType);
        jt.AddMember("tileW", tm->tileW, a);
        jt.AddMember("tileH", tm->tileH, a);
        jt.AddMember("columns", tm->columns, a);
        jt.AddMember("rows", tm->rows, a);
        jt.AddMember("enableCol", Value(tm->enableCol ? 1 : 0), a);

        Value tileArray(kArrayType);
        for (const auto& [key, id] : tm->tiles) {
            Value t(kObjectType);
            t.AddMember("x", key.x, a);
            t.AddMember("y", key.y, a);
            t.AddMember("id", Value(id.c_str(), a), a);
            tileArray.PushBack(t, a);
        }
        jt.AddMember("tiles", tileArray, a);
        doc.AddMember("TileMap", jt, a);

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeTileMap(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("TileMap")) return;

        const Value& tmj = doc["TileMap"];
        TileMap* tm = obj->hasComponent<TileMap>()
            ? obj->getComponent<TileMap>()
            : obj->addComponent<TileMap>();

        if (tmj.HasMember("tileW"))    tm->tileW = tmj["tileW"].GetFloat();
        if (tmj.HasMember("tileH"))    tm->tileH = tmj["tileH"].GetFloat();
        if (tmj.HasMember("columns"))  tm->columns = tmj["columns"].GetInt();
        if (tmj.HasMember("rows"))     tm->rows = tmj["rows"].GetInt();
        if (tmj.HasMember("enableCol"))
            tm->enableCol = tmj["enableCol"].IsBool()
            ? tmj["enableCol"].GetBool() : (tmj["enableCol"].GetInt() != 0);

        tm->tiles.clear();
        if (tmj.HasMember("tiles") && tmj["tiles"].IsArray()) {
            for (const auto& t : tmj["tiles"].GetArray())
                tm->setTile(t["x"].GetInt(), t["y"].GetInt(), t["id"].GetString());
        }
    }

    std::string serializeLuaScript(GameObject* obj) {
        LuaScript* ls = obj ? obj->getComponent<LuaScript>() : nullptr;
        if (!ls) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value jl(kObjectType);
        Value scriptsArray(kArrayType);
        for (const auto& scriptEntry : ls->scripts) {
            //if (scriptEntry.filename.empty() && scriptEntry.scriptName.empty()) continue;
            Value entry(kObjectType);
            entry.AddMember("filename", Value(scriptEntry.filename.c_str(), a), a);
            entry.AddMember("scriptName", Value(scriptEntry.scriptName.c_str(), a), a);
            scriptsArray.PushBack(entry, a);
        }

        if (!scriptsArray.Empty()) {
            jl.AddMember("scripts", scriptsArray, a);
            doc.AddMember("LuaScript", jl, a);
        }

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeLuaScript(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("LuaScript")) return;

        const Value& jl = doc["LuaScript"];
        LuaScript* ls = obj->hasComponent<LuaScript>()
            ? obj->getComponent<LuaScript>()
            : obj->addComponent<LuaScript>();
        ls->scripts.clear();

        if (jl.HasMember("scripts") && jl["scripts"].IsArray()) {
            for (const auto& se : jl["scripts"].GetArray()) {
                if (!se.IsObject()) continue;
                LuaScript::ScriptEntry entry;
                if (se.HasMember("filename") && se["filename"].IsString())   entry.filename = se["filename"].GetString();
                if (se.HasMember("scriptName") && se["scriptName"].IsString()) entry.scriptName = se["scriptName"].GetString();
                entry.changed = true;
                ls->scripts.push_back(entry);
            }
        }
        ls->scriptsChanged = true;
    }

    std::string serializeVisualLC(GameObject* obj) {
        VisualLC* vl = obj ? obj->getComponent<VisualLC>() : nullptr;
        if (!vl) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value jvl(kObjectType);
        if (!vl->filename.empty())
            jvl.AddMember("filename", Value(vl->filename.c_str(), a), a);
        doc.AddMember("VisualLC", jvl, a);

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeVisualLC(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("VisualLC")) return;

        const Value& jvl = doc["VisualLC"];
        VisualLC* vl = obj->hasComponent<VisualLC>()
            ? obj->getComponent<VisualLC>()
            : obj->addComponent<VisualLC>();

        if (jvl.HasMember("filename") && jvl["filename"].IsString()) {
            vl->filename = jvl["filename"].GetString();
            if (!vl->filename.empty()) {
                std::string filepath = "/Logic/" + vl->filename;
                std::string err;
                if (!JsonIO::loadVisualLogic(filepath, vl->nodes, vl->connections, &err))
                    DebugLog::addMessage("Failed to load Visual Logic file: " + err);
            }
        }
    }

    std::string serializeCamera(GameObject* obj) 
    {
        camera* cam = obj ? obj->getComponent<camera>() : nullptr;
        if (!cam) return "";
        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();
        Value jcam(kObjectType);
        jcam.AddMember("useScreenSize", cam->useScreenSize, a);
        jcam.AddMember("height", cam->height, a);
        jcam.AddMember("width", cam->width, a);
        jcam.AddMember("zoom", cam->zoom, a);
        jcam.AddMember("minZoom", cam->minZoom, a);
        jcam.AddMember("maxZoom", cam->maxZoom, a);
        jcam.AddMember("mapWidth", cam->mapWidth, a);
        jcam.AddMember("mapHeight", cam->mapHeight, a);
        doc.AddMember("Camera", jcam, a);
        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
	}

    void deserializeCamera(GameObject* obj, const std::string& json) 
    {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("Camera")) return;
        const Value& jcam = doc["Camera"];
        camera* cam = obj->hasComponent<camera>()
            ? obj->getComponent<camera>()
            : obj->addComponent<camera>();
        if (jcam.HasMember("useScreenSize") && jcam["useScreenSize"].IsBool())
            cam->useScreenSize = jcam["useScreenSize"].GetBool();
        if (jcam.HasMember("height") && jcam["height"].IsFloat())
            cam->height = jcam["height"].GetFloat();
        if (jcam.HasMember("width") && jcam["width"].IsInt())
            cam->width = jcam["width"].GetInt();
        if (jcam.HasMember("zoom") && jcam["zoom"].IsFloat())
            cam->zoom = jcam["zoom"].GetFloat();
        if (jcam.HasMember("minZoom") && jcam["minZoom"].IsFloat())
            cam->minZoom = jcam["minZoom"].GetFloat();
        if (jcam.HasMember("maxZoom") && jcam["maxZoom"].IsFloat())
            cam->maxZoom = jcam["maxZoom"].GetFloat();
        if (jcam.HasMember("mapWidth") && jcam["mapWidth"].IsFloat())
            cam->mapWidth = jcam["mapWidth"].GetFloat();
        if (jcam.HasMember("mapHeight") && jcam["mapHeight"].IsFloat())
            cam->mapHeight = jcam["mapHeight"].GetFloat();
	}

    std::string serializeWeapon(GameObject* obj) {
        WeaponComponent* wc = obj ? obj->getComponent<WeaponComponent>() : nullptr;
        if (!wc) return "";
        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();
        Value jweap(kObjectType);
        Value barrelArr(kArrayType);
        barrelArr.PushBack(wc->barrelOffset.x, a);
        barrelArr.PushBack(wc->barrelOffset.y, a);
        jweap.AddMember("barrelOffset", barrelArr, a);
        jweap.AddMember("aimAtMouse", wc->aimAtMouse ? 1 : 0, a);
        doc.AddMember("WeaponComponent", jweap, a);
        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }
    
    void deserializeWeapon(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("WeaponComponent")) return;
        const Value& jweap = doc["WeaponComponent"];
        WeaponComponent* wc = obj->hasComponent<WeaponComponent>()
            ? obj->getComponent<WeaponComponent>()
            : obj->addComponent<WeaponComponent>();
        if (jweap.HasMember("barrelOffset") && jweap["barrelOffset"].IsArray()
            && jweap["barrelOffset"].Size() >= 2) {
            wc->barrelOffset.x = jweap["barrelOffset"][0].GetFloat();
            wc->barrelOffset.y = jweap["barrelOffset"][1].GetFloat();
        }
        if (jweap.HasMember("aimAtMouse")) {
            wc->aimAtMouse = jweap["aimAtMouse"].IsBool()
                ? jweap["aimAtMouse"].GetBool()
                : (jweap["aimAtMouse"].GetInt() != 0);
        }
    }

    std::string serializeWaterBody(GameObject* obj) {
        WaterBody* w = obj ? obj->getComponent<WaterBody>() : nullptr;
        if (!w) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();

        Value jw(kObjectType);
        jw.AddMember("waveAmplitude", w->waveAmplitude, a);
        jw.AddMember("waveFrequency", w->waveFrequency, a);
        jw.AddMember("waveSpeed", w->waveSpeed, a);
        jw.AddMember("waterAlpha", w->waterAlpha, a);
        jw.AddMember("subdivisions", w->subdivisions, a);
        jw.AddMember("width", w->width, a);
        jw.AddMember("height", w->height, a);
        jw.AddMember("isTrigger", w->isTrigger, a);
        jw.AddMember("buoyancyForce", w->buoyancyForce, a);
        jw.AddMember("waterDrag", w->waterDrag, a);
        jw.AddMember("renderWaves", w->renderWaves, a);
        jw.AddMember("useTexture", w->useTexture, a);
        WriteVec4(jw, "waterColor", w->waterColor.r, w->waterColor.g, w->waterColor.b, w->waterColor.a, a);
        doc.AddMember("WaterBody", jw, a);

        StringBuffer buf; Writer<StringBuffer> w2(buf); doc.Accept(w2);
        return buf.GetString();
    }

    void deserializeWaterBody(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;
        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("WaterBody")) return;

        const Value& jw = doc["WaterBody"];
        WaterBody* w = obj->hasComponent<WaterBody>()
            ? obj->getComponent<WaterBody>()
            : obj->addComponent<WaterBody>();

        if (jw.HasMember("waveAmplitude"))  w->waveAmplitude = jw["waveAmplitude"].GetFloat();
        if (jw.HasMember("waveFrequency"))  w->waveFrequency = jw["waveFrequency"].GetFloat();
        if (jw.HasMember("waveSpeed"))      w->waveSpeed = jw["waveSpeed"].GetFloat();
        if (jw.HasMember("waterAlpha"))     w->waterAlpha = jw["waterAlpha"].GetFloat();
        if (jw.HasMember("subdivisions"))   w->subdivisions = jw["subdivisions"].GetInt();
        if (jw.HasMember("width"))          w->width = jw["width"].GetFloat();
        if (jw.HasMember("height"))         w->height = jw["height"].GetFloat();
        if (jw.HasMember("isTrigger"))      w->isTrigger = jw["isTrigger"].GetBool();
        if (jw.HasMember("buoyancyForce"))  w->buoyancyForce = jw["buoyancyForce"].GetFloat();
        if (jw.HasMember("waterDrag"))      w->waterDrag = jw["waterDrag"].GetFloat();
        if (jw.HasMember("renderWaves"))    w->renderWaves = jw["renderWaves"].GetBool();
        if (jw.HasMember("useTexture"))     w->useTexture = jw["useTexture"].GetBool();
        ReadVec4(jw, "waterColor", w->waterColor.r, w->waterColor.g, w->waterColor.b, w->waterColor.a);
    }

    std::string serializeButton(GameObject* obj) {
        Button* b = obj ? obj->getComponent<Button>() : nullptr;
        if (!b) return "";

        using namespace rapidjson;
        Document doc; doc.SetObject();
        auto& a = doc.GetAllocator();
        Value jb(kObjectType);

        jb.AddMember("action", static_cast<int>(b->actionName), a);
        jb.AddMember("isHovered", b->isHovered, a);
        jb.AddMember("isPressed", b->isPressed, a);
        jb.AddMember("onClickChannel", Value(b->onClickChannel.c_str(), a), a);
        jb.AddMember("onHoverChannel", Value(b->onHoverChannel.c_str(), a), a);
        // Only add scenePath if not empty
        if (!b->scenePath.empty()) {
            jb.AddMember("scenePath", rapidjson::Value(b->scenePath.c_str(), a), a);
        }
        doc.AddMember("Button", jb, a);
        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    void deserializeButton(GameObject* obj, const std::string& json) {
        if (!obj || json.empty()) return;
        using namespace rapidjson;

        Document doc; doc.Parse(json.c_str());
        if (doc.HasParseError() || !doc.HasMember("Button")) return;

        const Value& jb = doc["Button"];
        Button* b = obj->hasComponent<Button>()
            ? obj->getComponent<Button>()
            : obj->addComponent<Button>();

        if (jb.HasMember("action") && jb["action"].IsString()) {
            b->actionName = JsonIO::StrToButtonAction(jb["action"].GetString());
        }
        if (jb.HasMember("isHovered")) b->isHovered = jb["isHovered"].GetBool();
        if (jb.HasMember("isPressed")) b->isPressed = jb["isPressed"].GetBool();
        if (jb.HasMember("onClickChannel")) b->onClickChannel = jb["onClickChannel"].GetString();
        if (jb.HasMember("onHoverChannel")) b->onHoverChannel = jb["onHoverChannel"].GetString();
        if (jb.HasMember("scenePath")) b->scenePath = jb["scenePath"].GetString();
    }

    std::string serializeGameObj(GameObject* obj) {
        if (!obj) return "";

        using namespace rapidjson; // save my life

        Document doc;
        doc.SetObject();
        auto& a = doc.GetAllocator();

        // basic info
        doc.AddMember("name", Value(obj->getObjectName().c_str(), a), a);
        doc.AddMember("prefabID", Value(obj->getObjectPrefabID().c_str(), a), a);
        doc.AddMember("layer", obj->getLayer(), a);

        // parse component json and merge its single top-level member into main doc
        auto merge = [&](const std::string& componentJson) {
            if (componentJson.empty()) return;
            Document tmp; tmp.Parse(componentJson.c_str());
            if (tmp.HasParseError() || !tmp.IsObject()) return;
            for (auto it = tmp.MemberBegin(); it != tmp.MemberEnd(); ++it) {
                Value key(it->name.GetString(), a);
                Value val;
                val.CopyFrom(it->value, a);
                doc.AddMember(key, val, a);
            }
        };

        merge(serializeTransform(obj));
        merge(serializeRender(obj));
        merge(serializeParticle(obj));
        merge(serializePhysics(obj));
        merge(serializeCollision(obj));
        merge(serializeInput(obj));
        merge(serializeFont(obj));
        merge(serializeAudio(obj));
        merge(serializeStateMachine(obj));
        merge(serializeTileMap(obj));
        merge(serializeLuaScript(obj));
        merge(serializeVisualLC(obj));
        merge(serializeWaterBody(obj));
        merge(serializeWeapon(obj));
        merge(serializeButton(obj));

        StringBuffer buf; Writer<StringBuffer> w(buf); doc.Accept(w);
        return buf.GetString();
    }

    GameObject* deserializeGameObj(GameObjectManager& manager, const std::string& jsonStr) {
        if (jsonStr.empty()) return nullptr;

        using namespace rapidjson;

        try {
            Document doc;
            doc.Parse(jsonStr.c_str());
            if (doc.HasParseError()) return nullptr;

            std::string name = doc.HasMember("name") ? doc["name"].GetString() : "Unnamed";
            GameObject* obj = manager.createGameObject(name);
            if (!obj) return nullptr;

            if (doc.HasMember("prefabID")) obj->getObjectPrefabID() = doc["prefabID"].GetString();

            if (doc.HasMember("layer")) manager.assignObjectToLayer(obj, doc["layer"].GetInt());

            // deserialize all components
            deserializeTransform(obj, jsonStr);
            deserializeRender(obj, jsonStr);
            deserializeParticle(obj, jsonStr);
            deserializePhysics(obj, jsonStr);
            deserializeCollision(obj, jsonStr);
            deserializeInput(obj, jsonStr);
            deserializeFont(obj, jsonStr);
            deserializeAudio(obj, jsonStr);
            deserializeStateMachine(obj, jsonStr);
            deserializeTileMap(obj, jsonStr);
            deserializeLuaScript(obj, jsonStr);
            deserializeVisualLC(obj, jsonStr);
            deserializeWaterBody(obj, jsonStr);
            deserializeWeapon(obj, jsonStr);
            deserializeButton(obj, jsonStr);

            return obj;
        }
        catch (...) {
            DebugLog::addMessage("Failed to deserialize GameObject with RapidJSON.");
            return nullptr;
        }
    }
    /* ---------- END ---------- */


    /* ---------- Visual Logic for editor ---------- */
    bool saveVisualLogic(const std::string& filepath, const std::vector<LogicNode>& nodes, const std::vector<LogicConnection>& connections, std::string* err) {

        rapidjson::Document doc;
        doc.SetObject();
        auto& a = doc.GetAllocator();

        doc.AddMember("version", 1, a);

        // serialize nodes
        rapidjson::Value nodesArray(rapidjson::kArrayType);
        for (const auto& node : nodes) {
            rapidjson::Value nodeObj(rapidjson::kObjectType);

            nodeObj.AddMember("id", node.id, a);
            nodeObj.AddMember("implementation", rapidjson::Value(JsonIO::nodeImplToString(node.imp).c_str(), a), a);
            nodeObj.AddMember("type", static_cast<int>(node.type), a);

            rapidjson::Value posArray(rapidjson::kArrayType);
            posArray.PushBack(node.position.x, a);
            posArray.PushBack(node.position.y, a);
            nodeObj.AddMember("position", posArray, a);

            nodeObj.AddMember("hasInput", node.hasInput, a);
            nodeObj.AddMember("hasOutput", node.hasOutput, a);

            nodeObj.AddMember("hasTrueOutput", node.hasTrueOutput, a);
            nodeObj.AddMember("hasFalseOutput", node.hasFalseOutput, a);

            if (!node.parameters.empty()) {
                rapidjson::Value paramsObj(rapidjson::kObjectType);
                for (const auto& [key, value] : node.parameters) {
                    paramsObj.AddMember(
                        rapidjson::Value(key.c_str(), a),
                        rapidjson::Value(value.c_str(), a),
                        a
                    );
                }
                nodeObj.AddMember("parameters", paramsObj, a);
            }

            nodesArray.PushBack(nodeObj, a);
        }
        doc.AddMember("nodes", nodesArray, a);

        // serialize connections
        rapidjson::Value connectionsArray(rapidjson::kArrayType);
        for (const auto& conn : connections) {
            rapidjson::Value connObj(rapidjson::kObjectType);

            connObj.AddMember("fromNodeId", conn.fromNodeId, a);
            connObj.AddMember("outputPin", rapidjson::Value(conn.outputPin.c_str(), a), a);
            connObj.AddMember("toNodeId", conn.toNodeId, a);
            connObj.AddMember("inputPin", rapidjson::Value(conn.inputPin.c_str(), a), a);

            connectionsArray.PushBack(connObj, a);
        }
        doc.AddMember("connections", connectionsArray, a);
        
        // write to file
        bool runtime = WriteDocumentToFile(RUNTIME_DIR_R+filepath, doc, true, err);
        bool proj = WriteDocumentToFile(PathManager::Instance().getSourcePath() + "/assets" + filepath, doc, true, err);

		return (runtime && proj);
    }

    bool loadVisualLogic(const std::string& filepath, std::vector<LogicNode>& nodes, std::vector<LogicConnection>& connections, std::string* err) {
        rapidjson::Document doc;
        if (!ReadFileToDocument(RUNTIME_DIR_R+filepath, doc, err)) {
			std::cout << "Failed to load visual logic from " << filepath << ": " << (err ? *err : "unknown error") << std::endl;
            return false;
        }

        // clear existing data
        nodes.clear();
        connections.clear();

        // load nodes
        if (doc.HasMember("nodes") && doc["nodes"].IsArray()) {
            for (const auto& nodeVal : doc["nodes"].GetArray()) {
                if (!nodeVal.IsObject()) continue;

                LogicNode node;
                node.id = GetIntOr(nodeVal, "id", 0);

                std::string name;
                if (GetString(nodeVal, "implementation", name)) {
                    node.imp = stringToNodeImpl(name);
                }

                node.type = static_cast<NodeType>(GetIntOr(nodeVal, "type", 0));

                if (nodeVal.HasMember("position") && nodeVal["position"].IsArray()) {
                    const auto& posArray = nodeVal["position"].GetArray();
                    if (posArray.Size() >= 2) {
                        node.position.x = posArray[0].GetFloat();
                        node.position.y = posArray[1].GetFloat();
                    }
                }

                GetBool(nodeVal, "hasInput", node.hasInput);
                GetBool(nodeVal, "hasOutput", node.hasOutput);

                GetBool(nodeVal, "hasTrueOutput", node.hasTrueOutput);
                GetBool(nodeVal, "hasFalseOutput", node.hasFalseOutput);

                if (nodeVal.HasMember("parameters") && nodeVal["parameters"].IsObject()) {
                    const auto& paramsObj = nodeVal["parameters"];
                    for (auto it = paramsObj.MemberBegin(); it != paramsObj.MemberEnd(); ++it) {
                        if (it->value.IsString()) {
                            node.parameters[it->name.GetString()] = it->value.GetString();
                        }
                    }
                }

                nodes.push_back(node);
            }
        }

        // load connections
        if (doc.HasMember("connections") && doc["connections"].IsArray()) {
            for (const auto& connVal : doc["connections"].GetArray()) {
                if (!connVal.IsObject()) continue;

                LogicConnection conn;
                conn.fromNodeId = GetIntOr(connVal, "fromNodeId", 0);
                conn.toNodeId = GetIntOr(connVal, "toNodeId", 0);

                std::string outputPin, inputPin;
                if (GetString(connVal, "outputPin", outputPin)) {
                    conn.outputPin = outputPin;
                }
                if (GetString(connVal, "inputPin", inputPin)) {
                    conn.inputPin = inputPin;
                }

                connections.push_back(conn);
            }
        }

        return true;
    }
	/* ---------- END ---------- */

} // namespace JsonIO



//==============================================================================
// GameObjectManager JSON serialization
//==============================================================================

/**
 * \brief Serializes all current game objects (and components) to a JSON file.
 * \param filename Destination JSON path.
 * \param isNew    If \c true , writes only a version header (empty scene shell).
 *
 * \details
 * Layout (version 1):
 * \code{.json}
 * {
 *   "version": 1,
 *   "objects": [
 *     {
 *       "name": "player",
 *       "components": {
 *         "Transform": { "pos":[x,y,z], "rotation":r, "scale":[sx,sy,sz] },
 *         "Render":    { "shape":"square|circle|triangle", "hasTex":true, "texture":"..." , "hasAnimation":0|1 },
 *         "Input":     {},
 *         "Physics":   { "inWater":0|1, "buoyancy":0|1 },
 *         "CollisionInfo": {},
 *         "Animation": { "loop":0|1, "currentFrameColumn":i, "currentFrameRow":i, "totalColumn":i, "totalRow":i, "frameTime":f }
 *       }
 *     }
 *   ]
 * }
 * \endcode
 */
void GameObjectManager::saveToJson(const std::string& filename, bool isNew) const {
    rapidjson::Document doc;
    doc.SetObject();
    auto& a = doc.GetAllocator();

    // Version header
    doc.AddMember("version", rapidjson::Value(1), a);

    if (!isNew) {
        rapidjson::Value objects(rapidjson::kArrayType);

        for (auto const& kv : m_gameObjects) {
            const GameObject* obj = kv.second.get();

            rapidjson::Value jObj(rapidjson::kObjectType);

            // Object name
            jObj.AddMember("name",
                rapidjson::Value(obj->getObjectName().c_str(), a),
                a);

            // Components container
            rapidjson::Value comps(rapidjson::kObjectType);

            // ---- Transform ----
            if (auto t = obj->getComponent<Transform>()) {
                rapidjson::Value jt(rapidjson::kObjectType);

                // pos / rotation / scale
                JsonIO::WriteVec3(jt, "pos", t->x, t->y, t->z, a);
                jt.AddMember("rotation", rapidjson::Value().SetFloat(t->rotation), a);
                JsonIO::WriteVec3(jt, "scale", t->scaleX, t->scaleY, t->scaleZ, a);

                comps.AddMember("Transform", jt, a);
            }

            // ---- Render ----
            if (auto r = obj->getComponent<Render>()) {
                rapidjson::Value jr(rapidjson::kObjectType);

                // shape
                jr.AddMember("shape",
                    rapidjson::Value(JsonIO::ShapeToStr(r->modelRef.shape), a),
                    a);

                // hasTex / texture
                jr.AddMember("hasTex", rapidjson::Value().SetBool(r->hasTex), a);
                if (r->hasTex && !r->texFile.empty()) {
                    jr.AddMember("texture",
                        rapidjson::Value(r->texFile.c_str(), a),
                        a);
                }

                // 0/1 to mirror prior text formats
                jr.AddMember("hasAnimation", rapidjson::Value(r->hasAnimation ? 1 : 0), a);

                // Optional future flags:
                // jr.AddMember("visible",     rapidjson::Value().SetBool(r->visible), a);
                // jr.AddMember("transparent", rapidjson::Value().SetBool(r->isTransparent), a);
                // jr.AddMember("wireframe",   rapidjson::Value().SetBool(r->wireframe), a);

                comps.AddMember("Render", jr, a);
            }

            // ---- Input (presence only) ----
            if (obj->hasComponent<Input>()) {
                comps.AddMember("Input", rapidjson::Value(rapidjson::kObjectType), a);
            }

            // ---- Physics ----
            if (auto p = obj->getComponent<Physics>()) {
                rapidjson::Value jp(rapidjson::kObjectType);
                // store as 0/1 to mirror .txt
                jp.AddMember("inWater", rapidjson::Value(p->inWater ? 1 : 0), a);
                jp.AddMember("buoyancy", rapidjson::Value(p->buoyancy ? 1 : 0), a); // member is 'buoyancy' in struct
                comps.AddMember("Physics", jp, a);
            }

            // ---- CollisionInfo (presence only) ----
            if (obj->hasComponent<CollisionInfo>()) {
                comps.AddMember("CollisionInfo", rapidjson::Value(rapidjson::kObjectType), a);
            }

            // ---- Animation ----
            /*if (auto an = obj->getComponent<Animation>()) {
                rapidjson::Value ja(rapidjson::kObjectType);
                ja.AddMember("loop", rapidjson::Value(an->loop ? 1 : 0), a);
                ja.AddMember("currentFrameColumn", rapidjson::Value(an->currentFrameColumn), a);
                ja.AddMember("currentFrameRow", rapidjson::Value(an->currentFrameRow), a);
                ja.AddMember("totalColumn", rapidjson::Value(an->totalColumn), a);
                ja.AddMember("totalRow", rapidjson::Value(an->totalRow), a);
                ja.AddMember("frameTime", rapidjson::Value().SetFloat(an->frameTime), a);
                comps.AddMember("Animation", ja, a);
            }*/

            // ---- Button ----
            if (auto b = obj->getComponent<Button>()) {
                rapidjson::Value jb(rapidjson::kObjectType);

                jb.AddMember("action", rapidjson::Value(static_cast<int>(b->actionName)), a);

                //add boolean states
                jb.AddMember("isHovered", b->isHovered ? 1 : 0, a);
                jb.AddMember("isPressed", b->isPressed ? 1 : 0, a);

                //add the Button object to your main JSON
                comps.AddMember("Button", jb, a);
			}

            jObj.AddMember("components", comps, a);
            objects.PushBack(jObj, a);
        }

        // Attach the objects array
        doc.AddMember("objects", objects, a);
    }

    std::string err;
    if (!JsonIO::WriteDocumentToFile(filename, doc, /*pretty=*/true, &err)) {
        std::cerr << "Scene save failed: " << err << "\n";
    }
    else {
        std::cout << "Scene saved (JSON): " << filename << "\n";
    }
}


/**
 * \brief Loads a scene from a JSON file and rebuilds all game objects.
 * \param filename Source JSON path.
 *
 * \details
 * Behavior:
 * - Validates version (warns on mismatch, still attempts to load).
 * - Clears existing scene before creation.
 * - Accepts booleans and numeric 0/1 for legacy compatibility.
 * - Defers texture upload by setting \c texHDL=0 and \c texChanged=true when needed.
 */
void GameObjectManager::loadFromJson(const std::string& filename) {
    rapidjson::Document doc;
    std::string err;
    if (!JsonIO::ReadFileToDocument(filename, doc, &err)) {
        std::cerr << "Scene load failed: " << err << "\n";
        return;
    }

    // Version check (non-fatal warning)
    int version = 1;
    if (doc.HasMember("version") && doc["version"].IsInt())
        version = doc["version"].GetInt();
    if (version != 1)
        std::cerr << "Warning: unsupported scene version " << version << "\n";

    // Start fresh
    m_gameObjects.clear();

    // Validate presence of objects array
    if (!doc.HasMember("objects") || !doc["objects"].IsArray()) {
        std::cerr << "Scene JSON missing 'objects' array.\n";
        return;
    }

    // Rebuild objects
    for (auto& jObj : doc["objects"].GetArray()) {
        if (!jObj.IsObject()) continue;

        std::string name = (jObj.HasMember("name") && jObj["name"].IsString())
            ? jObj["name"].GetString()
            : ("Object_" + std::to_string(m_gameObjects.size()));

        GameObject* go = createGameObject(name);

        if (!jObj.HasMember("components") || !jObj["components"].IsObject())
            continue;

        const auto& comps = jObj["components"];

        // ---- Transform ----
        if (comps.HasMember("Transform") && comps["Transform"].IsObject()) {
            const auto& jt = comps["Transform"];
            auto* t = go->addComponent<Transform>();

            (void)JsonIO::ReadVec3(jt, "pos", t->x, t->y, t->z);
            (void)JsonIO::GetFloat(jt, "rotation", t->rotation);
            (void)JsonIO::ReadVec3(jt, "scale", t->scaleX, t->scaleY, t->scaleZ);
        }

        // ---- Render ----
        if (comps.HasMember("Render") && comps["Render"].IsObject()) {
            const auto& jr = comps["Render"];
            auto* r = go->addComponent<Render>();

            // shape
            std::string shp = (jr.HasMember("shape") && jr["shape"].IsString())
                ? jr["shape"].GetString()
                : "square";
            r->modelRef = renderer::models[static_cast<int>(JsonIO::StrToShape(shp))];

            // hasTex / texture (accept bool or 0/1)
            if (jr.HasMember("hasTex")) {
                r->hasTex = jr["hasTex"].IsBool()
                    ? jr["hasTex"].GetBool()
                    : (jr["hasTex"].GetInt() != 0);
            }
            else {
                r->hasTex = false;
            }

            if (r->hasTex && jr.HasMember("texture") && jr["texture"].IsString()) {
                r->texFile = jr["texture"].GetString();
                r->texHDL = 0;         // will be uploaded later by renderer
                r->texChanged = true;     // flag for upload
            }
            else {
                r->texFile.clear();
                r->texHDL = 0;
                r->texChanged = false;
            }

            if (jr.HasMember("hasAnimation")) {
                r->hasAnimation = jr["hasAnimation"].IsBool()
                    ? jr["hasAnimation"].GetBool()
                    : (jr["hasAnimation"].GetInt() != 0);
            }
            else {
                r->hasAnimation = false;
            }

            // Optional future flags:
            // if (jr.HasMember("visible")     && jr["visible"].IsBool())     r->visible       = jr["visible"].GetBool();
            // if (jr.HasMember("transparent") && jr["transparent"].IsBool()) r->isTransparent = jr["transparent"].GetBool();
            // if (jr.HasMember("wireframe")   && jr["wireframe"].IsBool())   r->wireframe     = jr["wireframe"].GetBool();
        }

        // ---- Input (presence only) ----
        if (comps.HasMember("Input") && comps["Input"].IsObject()) {
            go->addComponent<Input>();
        }

        // ---- Physics ----
        if (comps.HasMember("Physics") && comps["Physics"].IsObject()) {
            const auto& jp = comps["Physics"];
            auto* p = go->addComponent<Physics>();
            if (jp.HasMember("inWater")) {
                p->inWater = jp["inWater"].IsBool()
                    ? jp["inWater"].GetBool()
                    : (jp["inWater"].GetInt() != 0);
            }
            if (jp.HasMember("buoyancy")) {
                p->buoyancy = jp["buoyancy"].IsBool()
                    ? jp["buoyancy"].GetBool()
                    : (jp["buoyancy"].GetInt() != 0);
            }
        }

        // ---- CollisionInfo (presence only) ----
        if (comps.HasMember("CollisionInfo") && comps["CollisionInfo"].IsObject()) {
            go->addComponent<CollisionInfo>();
        }

		// ---- Button ----
        if (comps.HasMember("Button") && comps["Button"].IsObject()) {
			const auto& jb = comps["Button"];
			auto* b = go->addComponent<Button>();

            if (jb.HasMember("action") && jb["action"].IsInt()) {
                b->actionName = static_cast<ButtonAction>(jb["action"].GetInt());
            }

            if (jb.HasMember("scenePath") && jb["scenePath"].IsString()) {
                b->scenePath = jb["scenePath"].GetString();
            }
        }
    }

    std::cout << "Scene loaded (JSON): " << filename << "\n";
}

/**
 * \brief Serializes a layer reference with both ID and name.
 * \param parent The JSON object to add the layer data to.
 * \param layerID The layer ID to serialize.
 * \param lm Reference to LayerManager for name lookup.
 * \param a RapidJSON allocator.
 */
void JsonIO::SerializeLayerRef(rapidjson::Value& parent, int layerID,
    const LayerManager& lm,
    rapidjson::Document::AllocatorType& a) {
    // Skip default game layer (ID 1)
    if (layerID == -1 || layerID == 1) return;

    // Always save ID (reliable)
    parent.AddMember("layer", layerID, a);

    // Also save name (readable, fallback)
    Layer* layer = const_cast<LayerManager&>(lm).getLayer(layerID);
    if (layer) {
        parent.AddMember("layerName",
            rapidjson::Value(layer->getLayerName().c_str(), a), a);
    }
}

/**
 * \brief Deserializes a layer reference, supporting both ID and name.
 * \param parent The JSON object containing layer data.
 * \param lm Reference to LayerManager for name lookup.
 * \param defaultLayer Default layer ID if not found (default: 1 = Game layer).
 * \return The resolved layer ID.
 */
int JsonIO::DeserializeLayerRef(const rapidjson::Value& parent,
    LayerManager& lm,
    int defaultLayer) {
    // Priority 1: Try layer ID
    if (parent.HasMember("layer") && parent["layer"].IsInt()) {
        int layerID = parent["layer"].GetInt();
        if (lm.getLayer(layerID)) {
            return layerID;
        }
    }

    // Priority 2: Try layer name (fallback)
    if (parent.HasMember("layerName") && parent["layerName"].IsString()) {
        std::string layerName = parent["layerName"].GetString();
        Layer* layer = lm.getLayerByName(layerName);
        if (layer) {
            return layer->getLayerID();
        }
    }

    return defaultLayer;
}