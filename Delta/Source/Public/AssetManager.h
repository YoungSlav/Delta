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
		RootFolder(GetRootFolder()),
		ProjectName(GetExecutableName())
	{
		ResourcesFolders.reserve(2);
		{
			std::string folderStr = RootFolder + EngineResourcesRelativePath;
			std::filesystem::path folder(folderStr);
			if ( std::filesystem::exists(folder) && std::filesystem::is_directory(folder) )
				ResourcesFolders.push_back(folderStr);
		}
		{
			std::string folderStr = RootFolder + std::format(ProjectResourcesRelativePath, ProjectName);
			std::filesystem::path folder(folderStr);
			if ( std::filesystem::exists(folder) && std::filesystem::is_directory(folder) )
				ResourcesFolders.push_back(folderStr);
		}
	}

	template<typename ClassName, typename... Args>
	std::shared_ptr<ClassName> FindOrLoad(const std::string& Name, Args&&... args)
	{
		static_assert(std::is_base_of_v<Asset, ClassName>);
		
		auto it = LoadedAssets.find(Name);
		if ( it != LoadedAssets.end() )
		{
			return std::dynamic_pointer_cast<ClassName>(it->second);
		}

		if ( std::shared_ptr<ClassName> asset = NewObject<ClassName>(Name) )
		{
			LoadedAssets.insert({Name, asset});
			bool AssetLoaded = asset->Load(std::forward<Args>(args)...);
			if ( AssetLoaded )
			{
				return asset;
			}
			else
			{
				asset->Destroy();
			}
		}

		return nullptr;
	}

protected:
	virtual bool Initialize_Internal() override;

private:
	
	std::string FindAsset(const std::string& AssetName);
	static bool IfFileExist(const std::string& FileName);
	static std::string GetRootFolder();
	static std::string GetExecutableName();

private:

	std::vector<std::shared_ptr<class Asset>> PendingLoadingAssets;
	std::map<std::string, std::shared_ptr<class Asset>> LoadedAssets;


	const std::string RootFolder;
	const std::string ProjectName;
	const std::string EngineResourcesRelativePath = "Resources\\";
	static constexpr std::string_view ProjectResourcesRelativePath = "Games\\{}\\Resources\\";

	std::vector<std::string> ResourcesFolders;
	std::vector<std::string> ShaderFolders;
};

}