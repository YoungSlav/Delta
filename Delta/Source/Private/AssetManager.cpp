#include "stdafx.h"
#include <fstream>
#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#endif

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
#if defined(_WIN32)
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    fs::path p(buffer);
    // Go up 4 levels from the executable path to repo root
    for (int i = 0; i < 4 && p.has_parent_path(); ++i) p = p.parent_path();
    p += fs::path::preferred_separator;
    return p.string();
#else
    // On non-Windows, use the current working directory as project root
    fs::path p = fs::current_path();
    p += fs::path::preferred_separator;
    return p.string();
#endif
}
std::string AssetManager::getExecutableName()
{
#if defined(_WIN32)
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string fullPath(buffer);
    // Find the position of the last path separator
    std::string::size_type lastSlashPos = fullPath.find_last_of("\\/");
    // Extract the file name (from the last path separator to the end)
    std::string fileName = fullPath.substr(lastSlashPos + 1);
    // Find the position of the last dot (file extension)
    std::string::size_type lastDotPos = fileName.find_last_of('.');
    if (lastDotPos != std::string::npos)
    {
        fileName = fileName.substr(0, lastDotPos);
    }
    return fileName;
#else
    // Fallback on non-Windows: use a stable project name
    return std::string("Delta");
#endif
}
