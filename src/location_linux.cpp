#include "fileio.h"

#include <filesystem>

#include <limits.h>
#include <unistd.h>

using namespace FileIO;

std::filesystem::path Location::GetExecutablePath()
{
	std::filesystem::path path;

	char result[PATH_MAX] = {0};
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

	return count > 0 ? std::filesystem::weakly_canonical(result).parent_path() : "./";
}


std::filesystem::path Location::GetSaveGameFolder()
{
	std::filesystem::path path;

	path = getenv("HOME") ? getenv("HOME") : ".";
	path /= ".config";

	return path;
}
