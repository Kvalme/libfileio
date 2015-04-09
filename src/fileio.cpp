/*************************************************************************
 * libfileio - filesystem abstraction interface
 *
 * Copyright (c) 2010 Denis Biryukov <denis.birukov@gmail.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 ************************************************************************/

#include "fileio.h"

namespace FileIO
{
#define THROW(reason, code) throw(FileIOError(reason, code))

FileManager::FileManager ( unsigned int cache_size ) : _cache_size_ ( cache_size )
{
}
FileManager::~FileManager()
{
	_file_fabricks_.clear();
	_file_cache_.clear();
}

FilePtr FileManager::open ( const std::string &filename, READ_MODE mode , CACHE_MODE cache )
{
	File *file;
	//Check cache
	std::map<std::string, FilePtr>::iterator cached_file = _file_cache_.find ( filename );
	if ( cached_file != _file_cache_.end() )
	{
		cached_file->second->reset();
		return cached_file->second;
	}
	if ( _file_fabricks_.empty() ) THROW ( "No file fabricks installed", NO_FABRICKS );
	for ( std::vector<FileFabric*>::iterator it = _file_fabricks_.begin(); it != _file_fabricks_.end(); ++it )
	{
		file = (*it)->operator()( filename, mode );
		if ( file )
		{
			FilePtr fileptr ( file );
			if ( cache == CACHE && _file_cache_.find ( filename ) == _file_cache_.end() )
			{
				if(_file_cache_.size() < _cache_size_)_file_cache_.insert ( std::make_pair ( filename, fileptr ) );
				else if(clean(1))_file_cache_.insert ( std::make_pair ( filename, fileptr ) );
			}
			return fileptr;
		}
	}
	THROW ( "Unable ot open file " + filename, NO_SUCH_FILE );
	return FilePtr ( ( File* ) 0 );
}
bool FileManager::clean ( int count )
{
	count = ( count == -1 ) ? _file_cache_.size() : count;
	for ( auto it = _file_cache_.begin(); it != _file_cache_.end(); )
	{
		_file_cache_.erase ( it++ );
		count--;
	}
	return count == 0;
}
void FileManager::register_fileio ( FileFabric *file_manager )
{
	_file_fabricks_.push_back ( file_manager );
}

void FileManager::ListDir(const std::string &path, std::vector< std::string > *subdirs, std::vector< std::string > *files)
{
	if ( _file_fabricks_.empty() ) THROW ( "No file fabricks installed", NO_FABRICKS );
	for ( std::vector<FileFabric*>::iterator it = _file_fabricks_.begin(); it != _file_fabricks_.end(); ++it )
	{
		if ((*it)->ListDir(path, subdirs, files)) return;
	}
	THROW ( "Unable to list dir " + path, PATH_NOT_FOUND);
}

} //namespace FileIO
