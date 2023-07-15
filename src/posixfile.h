/*
 *  Copyright (c) 2015, Denis Biryukov <denis(dot)birukov(at)gmail(dot)com>
 */

#pragma once

#include "fileio.h"
#include <filesystem>

namespace FileIO
{

class PosixFileFaсtory : public FileFaсtory
{
	public:
		explicit PosixFileFaсtory(const std::vector<std::filesystem::path> &base_paths);
		virtual File *operator()(const std::string &fname, READ_MODE mode);
		virtual bool ListDir(const std::string &path, std::vector< std::string > *subdirs, std::vector< std::string > *files);

	private:
		std::vector<std::filesystem::path> base_paths_;
};

}
