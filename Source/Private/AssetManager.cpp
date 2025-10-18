#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <system_error>
#if defined(_WIN32)
#  include <windows.h>
#elif defined(__APPLE__)
#  include <mach-o/dyld.h>
#elif defined(__linux__)
#  include <unistd.h>
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
    auto hasResources = [](const fs::path& base) -> bool
    {
        fs::path r = base / "Resources";
        std::error_code ec;
        return fs::exists(r, ec) && fs::is_directory(r, ec);
    };

    // Try to find the executable directory
    fs::path exeDir;
#if defined(_WIN32)
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH)
        exeDir = fs::path(buf).parent_path();
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size); // query size
    std::vector<char> pathBuf(size + 1, 0);
    if (_NSGetExecutablePath(pathBuf.data(), &size) == 0)
        exeDir = fs::path(pathBuf.data()).parent_path();
#elif defined(__linux__)
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0)
    {
        buf[n] = '\0';
        exeDir = fs::path(buf).parent_path();
    }
#endif

    // Candidate roots to check (search upwards for a folder that contains Resources)
    std::vector<fs::path> roots;
    roots.push_back(fs::current_path());
    if (!exeDir.empty()) roots.push_back(exeDir);

    for (const fs::path& start : roots)
    {
        fs::path p = start;
        for (int i = 0; i < 8 && !p.empty(); ++i)
        {
            if (hasResources(p))
            {
                fs::path withSep = p;
                withSep += fs::path::preferred_separator;
                return withSep.string();
            }
            p = p.parent_path();
        }
    }

    // Fallback to current directory with separator
    fs::path fallback = fs::current_path();
    fallback += fs::path::preferred_separator;
    return fallback.string();
}
std::string AssetManager::getExecutableName()
{
    // Fallback on non-Windows: use a stable project name
    return std::string("Delta");
}
