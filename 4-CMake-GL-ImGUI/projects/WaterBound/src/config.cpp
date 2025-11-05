/* Start Header ************************************************************************
\file       config.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       October, 23rd, 2025
\brief      JSON-based configuration loading using RapidJSON.
* End Header **************************************************************************/

#include "config.h"

bool  gShowFpsInTitle = true;
bool  gShowInputDebug = false;
float gClearColor[4] = { 0.2f, 0.2f, 0.25f, 1.0f };

/* ========================== JsonIO helpers ========================== */
namespace JsonIO {

    static bool AtomicMove(const std::string& tmp, const std::string& dst, std::string* err) {
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

    bool ReadFileToDocument(const std::string& path,
        rapidjson::Document& doc,
        std::string* err)
    {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) { if (err) *err = "Could not open " + path; return false; }
        rapidjson::IStreamWrapper isw(ifs);   // uses stream�s own buffering
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

    bool WriteDocumentToFile(const std::string& path,
        rapidjson::Document& doc,
        bool pretty,
        std::string* err)
    {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        const std::string tmp = path + ".tmp";

        std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
        if (!ofs) { if (err) *err = "Cannot open temp file for writing: " + tmp; return false; }

        rapidjson::OStreamWrapper osw(ofs);
        if (pretty) { rapidjson::PrettyWriter<rapidjson::OStreamWrapper> w(osw); w.SetIndent(' ', 2); doc.Accept(w); }
        else { rapidjson::Writer<rapidjson::OStreamWrapper>       w(osw);                     doc.Accept(w); }

        ofs.flush();
        ofs.close();
        return AtomicMove(tmp, path, err);
    }
} // namespace JsonIO

/* ===================== AppConfig field mappers ====================== */
namespace {
    inline void applyInt(const rapidjson::Value& o, const char* k, int& v) { if (o.HasMember(k) && o[k].IsInt())    v = o[k].GetInt(); }
    inline void applyBool(const rapidjson::Value& o, const char* k, bool& v) { if (o.HasMember(k) && o[k].IsBool())   v = o[k].GetBool(); }
    inline void applyStr(const rapidjson::Value& o, const char* k, std::string& v) { if (o.HasMember(k) && o[k].IsString()) v = o[k].GetString(); }
    inline void applyFloat4(const rapidjson::Value& o, const char* k, float(&dst)[4]) {
        if (!o.HasMember(k) || !o[k].IsArray() || o[k].Size() != 4) return;
        const auto& a = o[k]; for (rapidjson::SizeType i = 0; i < 4; ++i) if (!a[i].IsNumber()) return;
        for (rapidjson::SizeType i = 0; i < 4; ++i) dst[i] = static_cast<float>(a[i].GetDouble());
    }

    inline void toDocument(const AppConfig& src, rapidjson::Document& doc) {
        doc.SetObject();
        auto& a = doc.GetAllocator();

        rapidjson::Value window(rapidjson::kObjectType);
        window.AddMember("width", src.width, a);
        window.AddMember("height", src.height, a);
        window.AddMember("title", rapidjson::Value(src.title.c_str(), a), a);
        window.AddMember("vsync", src.vsync, a);
        window.AddMember("fullscreen", src.fullscreen, a);

        rapidjson::Value render(rapidjson::kObjectType);
        rapidjson::Value cc(rapidjson::kArrayType);
        cc.PushBack(src.clear_color[0], a)
            .PushBack(src.clear_color[1], a)
            .PushBack(src.clear_color[2], a)
            .PushBack(src.clear_color[3], a);
        render.AddMember("clear_color", cc, a);

        rapidjson::Value debug(rapidjson::kObjectType);
        debug.AddMember("show_fps_in_title", src.show_fps_in_title, a);
        debug.AddMember("show_input_debug", src.show_input_debug, a);

        doc.AddMember("window", window, a);
        doc.AddMember("render", render, a);
        doc.AddMember("debug", debug, a);
    }
}

/* ============================ Public API ============================ */
bool LoadConfig(const std::string& path, AppConfig& out, std::string* err)
{
    rapidjson::Document doc;
    if (!JsonIO::ReadFileToDocument(path, doc, err)) return false;   // <�� change here

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

    gShowFpsInTitle = out.show_fps_in_title;
    gShowInputDebug = out.show_input_debug;
    for (int i = 0; i < 4; ++i) gClearColor[i] = out.clear_color[i];
    return true;
}

bool SaveConfig(const std::string& path, const AppConfig& cfg, std::string* err, bool pretty)
{
    rapidjson::Document doc;
    toDocument(cfg, doc);
    return JsonIO::WriteDocumentToFile(path, doc, pretty, err);      // uses JsonIO
}