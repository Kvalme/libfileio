#include "fileio.h"

#include <filesystem>

#include <windows.h>
#include <knownfolders.h>
#include <shlobj.h>

using namespace FileIO;

std::filesystem::path Location::GetExecutablePath()
{
	char path[MAX_PATH];
	GetModuleFileNameA(0, path, (sizeof(path)));

	return std::filesystem::weakly_canonical(path).parent_path();
}


std::filesystem::path Location::GetSaveGameFolder()
{
	LPWSTR wszPath = NULL;
	SHGetKnownFolderPath(FOLDERID_SavedGames, KF_FLAG_CREATE, NULL, &wszPath);

	std::filesystem::path path(wszPath);
	CoTaskMemFree(wszPath);

	return path;
}
