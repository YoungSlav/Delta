#pragma once

#include "stdafx.h"
#include "Object.h"
#include "Asset.h"
#include <filesystem>

namespace Delta
{


class AssetManager final : public Object
{
public:
	template <typename... Args>
	AssetManager(Args&&... args) :
		Object(std::forward<Args>(args)...),
		rootFolder(getRootFolder()),
		projectName(getExecutableName())
	{
		resourcesFolders.reserve(2);
		{
			namespace fs = std::filesystem;
			fs::path folderPath = fs::path(rootFolder) / engineResourcesRelativePath;
			if ( fs::exists(folderPath) && fs::is_directory(folderPath) )
				resourcesFolders.push_back(folderPath.string());
		}
		{
			namespace fs = std::filesystem;
			std::string projRel = std::format(projectResourcesRelativePath, projectName);
			fs::path folderPath = fs::path(rootFolder) / projRel;
			if ( fs::exists(folderPath) && fs::is_directory(folderPath) )
				resourcesFolders.push_back(folderPath.string());
		}
	}

	template<typename ClassName, typename... Args>
	std::shared_ptr<ClassName> findOrLoad(const std::string& Name, Args&&... args)
	{
		static_assert(std::is_base_of_v<Asset, ClassName>);

		LOG(Log, "Loading asset: '{}'", Name);
		
		auto it = loadedAssets.find(Name);
		if ( it != loadedAssets.end() )
		{
			LOG(Log, "    Found existing asset for '{}'", Name);
			return std::dynamic_pointer_cast<ClassName>(it->second);
		}

		if ( std::shared_ptr<ClassName> asset = spawn<ClassName>(Name, std::forward<Args>(args)...) )
		{
			loadedAssets.insert({Name, asset});
			bool AssetLoaded = asset->load();
			if ( AssetLoaded )
			{
				return asset;
			}
			else
			{
				LOG(Log, "    Failed to load asset '{}'", Name);
				asset->destroy();
			}
		}

		return nullptr;
	}

	std::string findAsset(const std::string& AssetName);

protected:
	virtual bool initialize_Internal() override;

private:
	
	static bool fileExist(const std::string& FileName);
	static std::string getRootFolder();
	static std::string getExecutableName();

private:

	std::vector<std::shared_ptr<class Asset>> pendingLoadingAssets;
	std::map<std::string, std::shared_ptr<class Asset>> loadedAssets;


	const std::string rootFolder;
	const std::string projectName;
	const std::string engineResourcesRelativePath = "Resources/";
	static constexpr std::string_view projectResourcesRelativePath = "Games/{}/Resources/";

	std::vector<std::string> resourcesFolders;
	std::vector<std::string> shaderFolders;
};

}
