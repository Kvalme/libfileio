/*
 *  Copyright (c) 2015, Denis Biryukov <denis(dot)birukov(at)gmail(dot)com>
 */

#pragma once

#include "fileio.h"

namespace FileIO
{

class PosixFileFabric : public FileFabric
{
	public:
		virtual File *operator()(const std::string &fname, READ_MODE mode);
		virtual bool ListDir(const std::string &path, std::vector< std::string > *subdirs, std::vector< std::string > *files);
};

}
