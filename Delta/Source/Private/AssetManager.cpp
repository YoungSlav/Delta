#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <windows.h>

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
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string fullPath(buffer);

	// Find the base directory by removing up to 3 levels
	for (int i = 0; i < 4; ++i)
	{
		std::string::size_type pos = fullPath.find_last_of("\\/");
		if (pos == std::string::npos) break; // Exit if there are no more levels
		fullPath = fullPath.substr(0, pos);
	}

	return fullPath + "\\";
}
std::string AssetManager::getExecutableName()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string fullPath(buffer);

	// Find the position of the last path separator
	std::string::size_type lastSlashPos = fullPath.find_last_of("\\/");

	// Extract the file name (from the last path separator to the end)
	std::string fileName = fullPath.substr(lastSlashPos + 1);

	// Find the position of the last dot (file extension)
	std::string::size_type lastDotPos = fileName.find_last_of('.');

	// Remove the extension (if present)
	if (lastDotPos != std::string::npos)
	{
		fileName = fileName.substr(0, lastDotPos);
	}

	return fileName;
}