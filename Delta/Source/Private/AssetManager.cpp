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
	std::ifstream file(FileName);
	if ( file )
		return true;
	else
		return false;
}

std::string AssetManager::findAsset(const std::string& AssetName)
{
	for ( const std::string& folder : resourcesFolders )
	{
		std::string Candidate = folder + AssetName;
		if ( fileExist(Candidate) )
			return Candidate;
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
