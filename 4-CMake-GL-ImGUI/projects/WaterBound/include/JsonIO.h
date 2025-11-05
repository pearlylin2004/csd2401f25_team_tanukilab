//JsonIO.h
#pragma once
#include <cstdio>
#include <string>
#include <cstring>
#include <fstream>
#include <algorithm>     
#include <filesystem>
#include <iostream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>

#include "GameObject.h"
#include "Component.h"
#include "renderer.h"


namespace JsonIO {

	// ---- Path helpers ----
	std::string runtimeScenePath(const std::string& name);
	std::string sourceScenePath(const std::string& name);
	std::string projectSourceRoot();

	// ---- Shape string helpers ----
	const char* ShapeToStr(shape s);
	shape StrToShape(const std::string& s);

	// ---- JSON file IO helpers ----
	bool syncSceneToRuntime(const std::string& name);
	bool AtomicMove(const std::string& tmp, const std::string& dst, std::string* err = nullptr);
	bool ReadFileToDocument(const std::string& path, rapidjson::Document& doc, std::string* err = nullptr);
	bool WriteDocumentToFile(const std::string& path, rapidjson::Document& doc, bool pretty = true, std::string* err = nullptr);

}
