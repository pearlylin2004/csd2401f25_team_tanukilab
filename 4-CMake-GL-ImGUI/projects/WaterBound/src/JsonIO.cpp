//JsonIO.cpp

#include "JsonIO.h"
#include "GameObjectManager.h"
#include "renderer.h"

#ifndef RUNTIME_SCENE_DIR_R
#define RUNTIME_SCENE_DIR_R "./shaders/Scene"
#endif
#ifndef SOURCE_SCENE_DIR_R
#define SOURCE_SCENE_DIR_R "./projects/WaterBound/shaders/Scene"
#endif
#ifndef SOURCE_DIR_R
#define SOURCE_DIR_R "./projects/WaterBound/shaders"
#endif

namespace Paths {
	std::filesystem::path exe_dir() {
#ifdef _WIN32
		wchar_t buf[MAX_PATH];
		GetModuleFileNameW(nullptr, buf, MAX_PATH);
		return std::filesystem::path(buf).parent_path();
#else
		return std::filesystem::canonical("/proc/self/exe").parent_path();
#endif
	}

	std::filesystem::path shaders() { return exe_dir() / "shaders"; }
	std::filesystem::path scenes() { return shaders() / "Scene"; }
	std::filesystem::path config() {
		auto direct = exe_dir() / "config.json";
		if (std::filesystem::exists(direct))
			return direct;
		return exe_dir() / "resources" / "config.json";
	}

	std::filesystem::path scripts() { return exe_dir() / "scripting"; }

	void ensure_dir(const std::filesystem::path& p) {
		std::error_code ec;
		std::filesystem::create_directories(p, ec);
	}

}

namespace JsonIO {

	// -------- path helpers --------
	std::string runtimeScenePath(const std::string& name) {
		return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
	}
	std::string sourceScenePath(const std::string& name) {
		return std::string(SOURCE_SCENE_DIR_R) + "/" + name;
	}
	std::string projectSourceRoot() {
		return std::string(SOURCE_DIR_R);
	}

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

	// -------- shape helpers --------
	const char* ShapeToStr(shape s) {
		switch (s) {
		case shape::square:   return "square";
		case shape::circle:   return "circle";
		case shape::triangle: return "triangle";
		default:              return "square";
		}
	}
	shape StrToShape(const std::string& s) {
		if (s == "circle")   return shape::circle;
		if (s == "triangle") return shape::triangle;
		return shape::square;
	}

	// centralized player state helpers
	PlayerState parsePlayerState(const std::string& s) {
		if (s == "Idle")     return PlayerState::Idle;
		if (s == "Walking")  return PlayerState::Walking;
		if (s == "Jumping")  return PlayerState::Jumping;
		if (s == "Falling")  return PlayerState::Falling;
		if (s == "Shooting") return PlayerState::Shooting;
		return PlayerState::Idle;
	}

	const char* playerStateToStr(PlayerState state) {
		switch (state) {
		case PlayerState::Idle:     return "Idle";
		case PlayerState::Walking:  return "Walking";
		case PlayerState::Jumping:  return "Jumping";
		case PlayerState::Falling:  return "Falling";
		case PlayerState::Shooting: return "Shooting";
		default:                    return "Idle";
		}
	}

	// -------- JSON file IO  --------
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
		if (pretty) { rapidjson::PrettyWriter<rapidjson::OStreamWrapper> w(osw); w.SetIndent(' ', 2); doc.Accept(w); }
		else { rapidjson::Writer<rapidjson::OStreamWrapper>       w(osw);                     doc.Accept(w); }

		ofs.flush();
		ofs.close();
		return AtomicMove(tmp, path, err);
	}


	const rapidjson::Value* GetObj(const rapidjson::Value& parent, const char* key) {
		if (!parent.IsObject() || !parent.HasMember(key)) return nullptr;
		return &parent[key];
	}

	bool GetString(const rapidjson::Value& obj, const char* key, std::string& out) {
		if (auto* v = GetObj(obj, key); v && v->IsString()) { out = v->GetString(); return true; }
		return false;
	}

	bool GetBool(const rapidjson::Value& obj, const char* key, bool& out) {
		if (auto* v = GetObj(obj, key)) {
			if (v->IsBool()) { out = v->GetBool(); return true; }
			if (v->IsInt()) { out = (v->GetInt() != 0); return true; }
			if (v->IsNumber()) { out = (v->GetDouble() != 0.0); return true; }
		}
		return false;
	}

	bool GetInt(const rapidjson::Value& obj, const char* key, int& out) {
		if (auto* v = GetObj(obj, key); v && v->IsInt()) { out = v->GetInt(); return true; }
		return false;
	}

	bool GetFloat(const rapidjson::Value& obj, const char* key, float& out) {
		if (auto* v = GetObj(obj, key)) {
			if (v->IsFloat()) { out = v->GetFloat(); return true; }
			if (v->IsNumber()) { out = static_cast<float>(v->GetDouble()); return true; }
		}
		return false;
	}

	bool GetArray3f(const rapidjson::Value& obj, const char* key, std::array<float, 3>& out) {
		auto* v = GetObj(obj, key);
		if (!v || !v->IsArray() || v->Size() != 3) return false;
		for (rapidjson::SizeType i = 0; i < 3; ++i) {
			if (!(*v)[i].IsNumber()) return false;
			out[i] = static_cast<float>((*v)[i].GetDouble());
		}
		return true;
	}

	void PutArray3f(rapidjson::Value& obj, const char* key, const std::array<float, 3>& v, rapidjson::Document::AllocatorType& a) {
		rapidjson::Value arr(rapidjson::kArrayType);
		arr.PushBack(rapidjson::Value().SetFloat(v[0]), a);
		arr.PushBack(rapidjson::Value().SetFloat(v[1]), a);
		arr.PushBack(rapidjson::Value().SetFloat(v[2]), a);
		obj.AddMember(rapidjson::Value(key, a), arr, a);
	}

	bool ReadVec3(const rapidjson::Value& obj, const char* key, glm::vec3& out) {
		std::array<float, 3> tmp{};
		if (!GetArray3f(obj, key, tmp)) return false;
		out = glm::vec3(tmp[0], tmp[1], tmp[2]);
		return true;
	}

	void WriteVec3(rapidjson::Value& obj, const char* key, const glm::vec3& v, rapidjson::Document::AllocatorType& a) {
		PutArray3f(obj, key, { v.x, v.y, v.z }, a);
	}

	bool ReadVec3(const rapidjson::Value& obj, const char* key, float& x, float& y, float& z) {
		std::array<float, 3> tmp{};
		if (!GetArray3f(obj, key, tmp)) return false;
		x = tmp[0]; y = tmp[1]; z = tmp[2];
		return true;
	}

	void WriteVec3(rapidjson::Value& obj, const char* key, float x, float y, float z, rapidjson::Document::AllocatorType& a) {
		PutArray3f(obj, key, { x, y, z }, a);
	}

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

	void WriteColor(rapidjson::Value& obj, const char* key, float r, float g, float b, float a, rapidjson::Document::AllocatorType& alloc) {
		rapidjson::Value arr(rapidjson::kArrayType);
		arr.PushBack(rapidjson::Value().SetFloat(r), alloc);
		arr.PushBack(rapidjson::Value().SetFloat(g), alloc);
		arr.PushBack(rapidjson::Value().SetFloat(b), alloc);
		arr.PushBack(rapidjson::Value().SetFloat(a), alloc);
		obj.AddMember(rapidjson::Value(key, alloc), arr, alloc);
	}

	bool ReadColor(const rapidjson::Value& obj, const char* key, glm::vec4& out) {
		float r, g, b, a;
		if (!ReadColor(obj, key, r, g, b, a)) return false;
		out = glm::vec4(r, g, b, a);
		return true;
	}

	void WriteColor(rapidjson::Value& obj, const char* key, const glm::vec4& v, rapidjson::Document::AllocatorType& alloc) {
		WriteColor(obj, key, v.r, v.g, v.b, v.a, alloc);
	}

} // namespace JsonIO

void GameObjectManager::saveToJson(const std::string& filename, bool isNew) const {
	rapidjson::Document doc;
	doc.SetObject();
	auto& a = doc.GetAllocator();

	// version header
	doc.AddMember("version", rapidjson::Value(1), a);

	if (!isNew) {

		// objects array
		rapidjson::Value objects(rapidjson::kArrayType);

		for (auto const& kv : m_gameObjects) {
			const GameObject* obj = kv.second.get();

			rapidjson::Value jObj(rapidjson::kObjectType);

			// name
			jObj.AddMember("name",
				rapidjson::Value(obj->getObjectName().c_str(), a),
				a);

			// components container
			rapidjson::Value comps(rapidjson::kObjectType);

			// ---- Transform ----
			if (auto t = obj->getComponent<Transform>()) {
				rapidjson::Value jt(rapidjson::kObjectType);

				// pos / rotation / scale (compact helpers)
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

				// keep your text semantics (0/1)
				jr.AddMember("hasAnimation", rapidjson::Value(r->hasAnimation ? 1 : 0), a);

				
	//          jr.AddMember("visible",     rapidjson::Value().SetBool(r->visible), a);
	//          jr.AddMember("transparent", rapidjson::Value().SetBool(r->isTransparent), a);
	//          jr.AddMember("wireframe",   rapidjson::Value().SetBool(r->wireframe), a);

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
				jp.AddMember("buoyancy", rapidjson::Value(p->buoancy ? 1 : 0), a); // note: member is 'buoancy' in your struct
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

		// attach array
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



void GameObjectManager::loadFromJson(const std::string& filename) {
	rapidjson::Document doc;
	std::string err;
	if (!JsonIO::ReadFileToDocument(filename, doc, &err)) {
		std::cerr << "Scene load failed: " << err << "\n";
		return;
	}

	int version = 1;
	if (doc.HasMember("version") && doc["version"].IsInt())
		version = doc["version"].GetInt();
	if (version != 1)
		std::cerr << "Warning: unsupported scene version " << version << "\n";

	m_gameObjects.clear();

	if (!doc.HasMember("objects") || !doc["objects"].IsArray()) {
		std::cerr << "Scene JSON missing 'objects' array.\n";
		return;
	}

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
				r->texHDL = 0;
				r->texChanged = true;   
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

			
//          if (jr.HasMember("visible")     && jr["visible"].IsBool())     r->visible       = jr["visible"].GetBool();
//          if (jr.HasMember("transparent") && jr["transparent"].IsBool()) r->isTransparent = jr["transparent"].GetBool();
//          if (jr.HasMember("wireframe")   && jr["wireframe"].IsBool())   r->wireframe     = jr["wireframe"].GetBool();
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
