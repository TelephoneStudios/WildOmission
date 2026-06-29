// Copyright Telephone Studios. All Rights Reserved.

#include "Interfaces/MenuInterface.h"
#include "HAL/FileManagerGeneric.h"

TArray<FString> IMenuInterface::GetAllWorldNamesV1()
{
	////////////////////////////////////////////////////////////////////////////////////
	// Special thanks to Ixiguis on the Unreal Engine forums for this useful function //
	////////////////////////////////////////////////////////////////////////////////////
	class FFindSavesVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FFindSavesVisitor() {}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (!bIsDirectory)
			{
				FString FullFilePath(FilenameOrDirectory);
				if (FPaths::GetExtension(FullFilePath) == TEXT("sav"))
				{
					FString CleanFilename = FPaths::GetBaseFilename(FullFilePath);
					CleanFilename = CleanFilename.Replace(TEXT(".sav"), TEXT(""));
					SavesFound.Add(CleanFilename);
				}
			}
			return true;
		}
		TArray<FString> SavesFound;
	};

	TArray<FString> Saves;
	const FString SavesFolder = FPaths::ProjectSavedDir() + TEXT("SaveGames");

	if (!SavesFolder.IsEmpty())
	{
		FFindSavesVisitor Visitor;
		FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*SavesFolder, Visitor);
		Saves = Visitor.SavesFound;
	}

	return Saves;
}

TArray<FString> IMenuInterface::GetAllWorldFolderNames()
{
	TArray<FString> Saves;
	const FString SavesFolder = FPaths::ProjectSavedDir() + TEXT("SaveGames");

	TArray<FString> FoundFolders;
	if (FPaths::DirectoryExists(SavesFolder))
	{
		FFileManagerGeneric::Get().FindFiles(FoundFolders, *SavesFolder, false, true);
		for (int i = 0; i < FoundFolders.Num(); i++)
		{
			FoundFolders[i] = SavesFolder + FoundFolders[i];
			UE_LOG(LogTemp, Warning, TEXT("Found Folder: %s"), *FoundFolders[i]);
		}
	}

	return FoundFolders;
}

bool IMenuInterface::WorldAlreadyExists(const FString& WorldNameToTest)
{
	TArray<FString> WorldNames = GetAllWorldFolderNames();
	for (const FString& WorldName : WorldNames)
	{
		if (WorldNameToTest.ToLower() == WorldName.ToLower())
		{
			return true;
		}
	}

	return false;
}
