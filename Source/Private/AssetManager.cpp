#include "stdafx.h"
#include <fstream>
#include <iostream>
// Fully portable: no platform-specific includes

#include "AssetManager.h"

using namespace Delta;

bool AssetManager::initialize_Internal()
{
	Object::initialize_Internal();

	LOG(Log, "Loading asset manager");
	LOG(Log, "Found resource folders: " );
	LOG_INDENT
	for ( auto folder : resourcesFolders )
		LOG(Log, "'{}'", folder );

	return true;
}

bool AssetManager::fileExist(const std::string& FileName)
{
	namespace fs = std::filesystem;
	fs::path p(FileName);
	return fs::exists(p) && fs::is_regular_file(p);
}

std::string AssetManager::findAsset(const std::string& AssetName)
{
	namespace fs = std::filesystem;
	for ( const std::string& folder : resourcesFolders )
	{
		fs::path Candidate = fs::path(folder) / AssetName;
		if ( fileExist(Candidate.string()) )
			return Candidate.string();
	}
	return "";
}

std::string AssetManager::getRootFolder()
{
	namespace fs = std::filesystem;
	// Use the current working directory as project root on all platforms
	fs::path p = fs::current_path();
	p += fs::path::preferred_separator;
	return p.string();
}
std::string AssetManager::getExecutableName()
{
	// Fallback on non-Windows: use a stable project name
	return std::string("Delta");
}
