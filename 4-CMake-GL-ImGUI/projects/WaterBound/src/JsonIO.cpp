/* Start Header ************************************************************************/
/*!
\file		JsonIO.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
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

#ifndef RUNTIME_SCENE_DIR_R
#define RUNTIME_SCENE_DIR_R "./assets/Scene"
#endif
#ifndef SOURCE_ASSETS_DIR_R
#define SOURCE_ASSETS_DIR_R "./projects/WaterBound/assets"
#endif
#ifndef RUNTIME_DIR_R
#define RUNTIME_DIR_R "./assets"
#endif
#ifndef SOURCE_DIR_R
#define SOURCE_DIR_R "./projects/WaterBound/assets"
#endif
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
        return std::string(SOURCE_ASSETS_DIR_R) + "/Scene/" + name;
    }

    /**
     * \brief Returns the project source root for shaders-related assets.
     * \return Value of \c SOURCE_DIR_R .
     */
    std::string projectSourceRoot() {
        return std::string(SOURCE_DIR_R);
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
     * \brief Reads a 3-float array field into a \c glm::vec3 .
     */
    bool ReadVec3(const rapidjson::Value& obj, const char* key, glm::vec3& out) {
        std::array<float, 3> tmp{};
        if (!GetArray3f(obj, key, tmp)) return false;
        out = glm::vec3(tmp[0], tmp[1], tmp[2]);
        return true;
    }

    /**
     * \brief Writes a \c glm::vec3 to JSON as a 3-float array.
     */
    void WriteVec3(rapidjson::Value& obj, const char* key, const glm::vec3& v, rapidjson::Document::AllocatorType& a) {
        PutArray3f(obj, key, { v.x, v.y, v.z }, a);
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
     * \brief Writes three floats as a 3-float array field.
     */
    void WriteVec3(rapidjson::Value& obj, const char* key, float x, float y, float z, rapidjson::Document::AllocatorType& a) {
        PutArray3f(obj, key, { x, y, z }, a);
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
                jp.AddMember("buoyancy", rapidjson::Value(p->buoancy ? 1 : 0), a); // member is 'buoancy' in struct
                comps.AddMember("Physics", jp, a);
            }

            // ---- CollisionInfo (presence only) ----
            if (obj->hasComponent<CollisionInfo>()) {
                comps.AddMember("CollisionInfo", rapidjson::Value(rapidjson::kObjectType), a);
            }

            // ---- Animation ----
            if (auto an = obj->getComponent<Animation>()) {
                rapidjson::Value ja(rapidjson::kObjectType);
                ja.AddMember("loop", rapidjson::Value(an->loop ? 1 : 0), a);
                ja.AddMember("currentFrameColumn", rapidjson::Value(an->currentFrameColumn), a);
                ja.AddMember("currentFrameRow", rapidjson::Value(an->currentFrameRow), a);
                ja.AddMember("totalColumn", rapidjson::Value(an->totalColumn), a);
                ja.AddMember("totalRow", rapidjson::Value(an->totalRow), a);
                ja.AddMember("frameTime", rapidjson::Value().SetFloat(an->frameTime), a);
                comps.AddMember("Animation", ja, a);
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
            go->addComponent<Input>(1.f);
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
                p->buoancy = jp["buoyancy"].IsBool()
                    ? jp["buoyancy"].GetBool()
                    : (jp["buoyancy"].GetInt() != 0);
            }
        }

        // ---- CollisionInfo (presence only) ----
        if (comps.HasMember("CollisionInfo") && comps["CollisionInfo"].IsObject()) {
            go->addComponent<CollisionInfo>();
        }

        // ---- Animation ----
        if (comps.HasMember("Animation") && comps["Animation"].IsObject()) {
            const auto& ja = comps["Animation"];
            auto* a = go->addComponent<Animation>();
            if (ja.HasMember("loop")) {
                a->loop = ja["loop"].IsBool()
                    ? ja["loop"].GetBool()
                    : (ja["loop"].GetInt() != 0);
            }
            if (ja.HasMember("currentFrameColumn") && ja["currentFrameColumn"].IsInt())
                a->currentFrameColumn = ja["currentFrameColumn"].GetInt();
            if (ja.HasMember("currentFrameRow") && ja["currentFrameRow"].IsInt())
                a->currentFrameRow = ja["currentFrameRow"].GetInt();
            if (ja.HasMember("totalColumn") && ja["totalColumn"].IsInt())
                a->totalColumn = ja["totalColumn"].GetInt();
            if (ja.HasMember("totalRow") && ja["totalRow"].IsInt())
                a->totalRow = ja["totalRow"].GetInt();
            if (ja.HasMember("frameTime") && ja["frameTime"].IsNumber())
                a->frameTime = ja["frameTime"].GetFloat();
        }
    }

    std::cout << "Scene loaded (JSON): " << filename << "\n";
}
