/**************************************************************************
*   Copyright (C) 2010 by Denis A. Birukov aka Messenger of death         *
*   denis.birukov@gmail.com                                               *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#pragma once
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <vector>
#include <map>
#include <stdint.h>

namespace FileIO
{

enum READ_MODE
{
	READ_ONLY,
	WRITE_ONLY,
	READ_WRITE
};

enum CACHE_MODE
{
	CACHE,
	NO_CACHE
};

enum FILE_POSITION
{
	POS_SET,
	POS_CUR,
	POS_END
};
class File;
typedef boost::shared_ptr<File> FilePtr;
typedef boost::function < File* ( const std::string &filename, READ_MODE mode ) > FileFabric;
enum FILEIO_ERROR_CODE
{
	NO_FABRICKS,
	NO_SUCH_FILE,
	INVALID_FILE,
	READ_ERROR,
	READ_ON_WRITE,
	WRITE_ERROR,
	WRITE_ON_READ,
	SEEK_ERROR,
	MAP_FOR_WRITE,
	MAP_ERROR
};

class FileIOError
{
	public:
		FileIOError ( std::string reason, FILEIO_ERROR_CODE code ) : _reason_ ( reason ), _error_code_(code) {}
		~FileIOError() {}
		FILEIO_ERROR_CODE get_error_code() const
		{
			return _error_code_;
		}
		const char* what() const
		{
			return _reason_.c_str();
		}
	private:
		std::string _reason_;
		FILEIO_ERROR_CODE _error_code_;
};

class FileManager
{
	public:
		FileManager ( unsigned int cache_size );
		~FileManager();

		/**
		 * Opens a file. Searching for file name in all fileio modules in the order of module registration
		 * @param filename file name to open
		 * @param mode open mode
		 * @param cache caching mode
		 * @return object that provides access to the opened file
		 * @author Denis A. Biryukov
		 */
		FilePtr open ( const std::string &filename, READ_MODE mode = READ_ONLY, CACHE_MODE cache = CACHE );
		/**
		 * Removing 'count' files from cache
		 * @param count count of files that needs to be removed from cache. By default this function purge cache.
		 * @return true on success operation
		 */
		bool clean ( int count = -1 );
		/**
		 * Register a fileio module
		 * @param file_manager fileio module fabrick
		 */
		void register_fileio ( const FileFabric &file_manager );
	private:
		std::vector<FileFabric> _file_fabricks_;
		std::map<std::string, FilePtr> _file_cache_;
		unsigned int _cache_size_;
};

class File
{
	public:
		/**
		 * Reads data from file
		 * @param buf buffer for data
		 * @param size amout of data to read. In bytes
		 * @return amount of bytes actually read
		 */
		virtual int read ( void *buf, int size ) = 0;
		/**
		 * Writes data to file
		 * @param buf buffer with data that needs to be written
		 * @param size amount of data to write.In bytes
		 * @return amount of bites actually written
		 */
		virtual int write ( const void *buf, int size ) = 0;
		/**
		 * Seeks in file for 'position' bites starting from 'whence'
		 * @param position amount of bites to move
		 * @param whence point in file from where we should move. By default - current position
		 */
		virtual void seek ( int64_t position, FILE_POSITION whence = POS_CUR ) = 0;
		/**
		 * Maps file onto memory
		 * @return pointer to the mapped area
		 */
		virtual void *mmap() = 0;
		/**
		 * Resets read/writes pointers. After this functions any read will performs from the start of file
		 */
		virtual void reset() = 0;
		/**
		 * Returns file size
		 * @return file size
		 */
		virtual uint64_t get_file_size() const
		{
			return _file_size_;
		}

		virtual ~File() {};
	protected:
		File ( const std::string &filename, READ_MODE mode ) : _filename_ ( filename ), _file_mode_ ( mode ), _file_size_ ( 0 ) {};
		std::string _filename_;
		READ_MODE _file_mode_;
		uint64_t _file_size_;
};

File* CreatePosixFile ( const std::string& filename, READ_MODE mode );

} //namespace FileIO
