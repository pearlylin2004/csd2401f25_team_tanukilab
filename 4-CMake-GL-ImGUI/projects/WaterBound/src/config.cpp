/* Start Header ************************************************************************
\file       config.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       October, 23rd, 2025
\brief      JSON-based configuration loading using RapidJSON.
* End Header **************************************************************************/

#include "config.h"

/* Optional global mirrors */
bool  gShowFpsInTitle = true;
bool  gShowInputDebug = false;
float gClearColor[4] = { 0.2f, 0.2f, 0.25f, 1.0f };

namespace {
    inline void applyInt(const rapidjson::Value& o, const char* k, int& v) { if (o.HasMember(k) && o[k].IsInt())    v = o[k].GetInt(); }
    inline void applyBool(const rapidjson::Value& o, const char* k, bool& v) { if (o.HasMember(k) && o[k].IsBool())   v = o[k].GetBool(); }
    inline void applyStr(const rapidjson::Value& o, const char* k, std::string& v) { if (o.HasMember(k) && o[k].IsString()) v = o[k].GetString(); }
    inline void applyFloat4(const rapidjson::Value& o, const char* k, float(&dst)[4]) {
        if (!o.HasMember(k) || !o[k].IsArray() || o[k].Size() != 4) return;
        const auto& a = o[k]; for (rapidjson::SizeType i = 0; i < 4; ++i) if (!a[i].IsNumber()) return;
        for (rapidjson::SizeType i = 0; i < 4; ++i) dst[i] = static_cast<float>(a[i].GetDouble());
    }
}

bool LoadConfig(const std::string& path, AppConfig& out, std::string* err)
{
    FILE* fp = std::fopen(path.c_str(), "rb");
    if (!fp) { if (err)*err = "Could not open " + path + " (using defaults)"; return false; }

    char buf[1 << 12];
    rapidjson::FileReadStream is(fp, buf, sizeof(buf));
    rapidjson::Document doc; doc.ParseStream(is);
    std::fclose(fp);

    if (doc.HasParseError()) {
        if (err)*err = std::string("JSON parse error @")
            + std::to_string(doc.GetErrorOffset()) + ": " + rapidjson::GetParseError_En(doc.GetParseError()); return false;
    }
    if (!doc.IsObject()) { if (err)*err = "Root JSON must be an object"; return false; }

    if (doc.HasMember("window") && doc["window"].IsObject()) {
        const auto& w = doc["window"];
        applyInt(w, "width", out.width);
        applyInt(w, "height", out.height);
        applyStr(w, "title", out.title);
        applyBool(w, "vsync", out.vsync);
        applyBool(w, "fullscreen", out.fullscreen);
    }
    if (doc.HasMember("render") && doc["render"].IsObject()) {
        const auto& r = doc["render"];
        applyFloat4(r, "clear_color", out.clear_color);
    }
    if (doc.HasMember("debug") && doc["debug"].IsObject()) {
        const auto& d = doc["debug"];
        applyBool(d, "show_fps_in_title", out.show_fps_in_title);
        applyBool(d, "show_input_debug", out.show_input_debug);
    }

    // sync globals if used elsewhere
    gShowFpsInTitle = out.show_fps_in_title;
    gShowInputDebug = out.show_input_debug;
    for (int i = 0; i < 4; ++i) gClearColor[i] = out.clear_color[i];
    return true;
}

namespace {
    inline void toDocument(const AppConfig& src, rapidjson::Document& doc) {
        doc.SetObject();
        auto& a = doc.GetAllocator();

        // window
        rapidjson::Value window(rapidjson::kObjectType);
        window.AddMember("width", src.width, a);
        window.AddMember("height", src.height, a);
        window.AddMember("title", rapidjson::Value(src.title.c_str(), a), a);
        window.AddMember("vsync", src.vsync, a);
        window.AddMember("fullscreen", src.fullscreen, a);

        // render
        rapidjson::Value render(rapidjson::kObjectType);
        rapidjson::Value cc(rapidjson::kArrayType);
        cc.PushBack(src.clear_color[0], a)
            .PushBack(src.clear_color[1], a)
            .PushBack(src.clear_color[2], a)
            .PushBack(src.clear_color[3], a);
        render.AddMember("clear_color", cc, a);

        // debug
        rapidjson::Value debug(rapidjson::kObjectType);
        debug.AddMember("show_fps_in_title", src.show_fps_in_title, a);
        debug.AddMember("show_input_debug", src.show_input_debug, a);

        doc.AddMember("window", window, a);
        doc.AddMember("render", render, a);
        doc.AddMember("debug", debug, a);
    }

    // Robust atomic write: write to *.tmp then replace destination
    inline bool atomic_write_json(const std::string& path,
        rapidjson::Document& doc,
        bool pretty,
        std::string* err)
    {
        std::filesystem::path p{ path };
        std::error_code ec;
        std::filesystem::create_directories(p.parent_path(), ec);

        const std::string tmp = path + ".tmp";

        if (FILE* fp = std::fopen(tmp.c_str(), "wb")) {
            char buffer[1 << 12];
            rapidjson::FileWriteStream os(fp, buffer, sizeof(buffer));

            if (pretty) {
                rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
                writer.SetIndent(' ', 2);
                if (!doc.Accept(writer)) {
                    std::fclose(fp);
                    if (err) *err = "Failed to pretty-write JSON.";
                    std::filesystem::remove(tmp, ec);
                    return false;
                }
            }
            else {
                rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
                if (!doc.Accept(writer)) {
                    std::fclose(fp);
                    if (err) *err = "Failed to write JSON.";
                    std::filesystem::remove(tmp, ec);
                    return false;
                }
            }

            std::fflush(fp);
            std::fclose(fp);

            std::filesystem::remove(p, ec);
            std::filesystem::rename(tmp, p, ec);
            if (ec) {
                std::filesystem::copy_file(tmp, p,
                    std::filesystem::copy_options::overwrite_existing, ec);
                std::filesystem::remove(tmp, ec);
                if (ec) {
                    if (err) *err = "Failed to move JSON to destination: " + ec.message();
                    return false;
                }
            }
            return true;
        }
        else {
            if (err) *err = "Could not open temp file for writing: " + tmp;
            return false;
        }
    }
}


bool SaveConfig(const std::string& path, const AppConfig& cfg, std::string* err, bool pretty)
{
    rapidjson::Document doc;
    toDocument(cfg, doc);
    return atomic_write_json(path, doc, pretty, err);
}