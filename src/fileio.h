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

#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <functional>
#include <memory>
#include <filesystem>

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
enum FILEIO_ERROR_CODE
{
	NO_FACTORIES,
	NO_SUCH_FILE,
	INVALID_FILE,
	READ_ERROR,
	READ_ON_WRITE,
	WRITE_ERROR,
	WRITE_ON_READ,
	SEEK_ERROR,
	MAP_FOR_WRITE,
	PATH_NOT_FOUND,
	MAP_ERROR
};
class File;
typedef std::shared_ptr<File> FilePtr;
class FileFactory
{
	public:
		virtual File *operator()(const std::string &fname, READ_MODE mode) = 0;
		virtual bool ListDir(const std::string &path, std::vector<std::string> *subdirs, std::vector<std::string> *files) = 0;
		virtual ~FileFactory() = default;
};
class FileIOError : public std::exception
{
	public:
		FileIOError(std::string reason, FILEIO_ERROR_CODE code) : _reason_(reason), _error_code_(code)
		{
			_error_text_ = std::string("[FileIO] error_code: ") + std::to_string(_error_code_) + " reason: \"" + _reason_.c_str() + "\"";
		}
		~FileIOError() {}

		FILEIO_ERROR_CODE get_error_code() const
		{
			return _error_code_;
		}

		const char *what() const noexcept override
		{
			return _error_text_.c_str();
		}
	private:
		std::string _reason_;
		FILEIO_ERROR_CODE _error_code_;
		std::string _error_text_;
};

class FileManager
{
	public:
		FileManager(unsigned int cache_size);
		~FileManager();

		/**
		 * Opens a file. Searching for file name in all fileio modules in the order of module registration
		 * @param filename file name to open
		 * @param mode open mode
		 * @param cache caching mode
		 * @return pointer to the opened file. Object should be deleted on caller side
		 * @author Denis A. Biryukov
		 */
		FilePtr open(const std::string &filename, READ_MODE mode = READ_ONLY, CACHE_MODE cache = CACHE);
		/**
		 * Removing 'count' files from cache
		 * @param count count of files that needs to be removed from cache. By default this function purge cache.
		 * @return true on success operation
		 */
		bool clean(int count = -1);
		/**
		 * Register a fileio module
		 * @param file_manager fileio module fabrick
		 */
		void register_fileio(FileFactory *file_manager);
		/**
		 * Lists files and subdirectoryes in specified path
		 * If path not exist or can not be opened exception will be rised with ERROR_CODE set to PATH_NOT_FOUND
		 * @param path pathi to list
		 * @param subdirs pointer to place where found subdirs should be stored
		 * @param files poiner to place where found files should be stored
		 */
		void ListDir(const std::string &path, std::vector<std::string> *subdirs, std::vector<std::string> *files);
		bool Exists(const std::string &filename);
	private:
		std::vector<FileFactory *> _file_factories_;
		std::map<std::string, FilePtr> _file_cache_;
		unsigned int _cache_size_;
};

class File
{
	public:
		/**
		 * Reads data from file
		 * @param buf buffer for datav
		 * @param size amout of data to read. In bytes
		 * @return amount of bytes actually read
		 */
		virtual int read(void *buf, std::size_t size) = 0;
		/**
		 * Writes data to file
		 * @param buf buffer with data that needs to be written
		 * @param size amount of data to write.In bytes
		 * @return amount of bites actually written
		 */
		virtual int write(const void *buf, std::size_t size) = 0;
		/**
		 * Seeks in file for 'position' bites starting from 'whence'
		 * @param position amount of bites to move
		 * @param whence point in file from where we should move. By default - current position
		 */
		virtual void seek(int64_t position, FILE_POSITION whence = POS_CUR) = 0;
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
		virtual std::size_t get_file_size() const
		{
			return _file_size_;
		}

		virtual ~File() {};
	protected:
		File(const std::string &filename, READ_MODE mode) : _filename_(filename), _file_mode_(mode), _file_size_(0) {};
		std::string _filename_;
		READ_MODE _file_mode_;
		std::size_t _file_size_;
};

namespace Location
{
	std::filesystem::path GetExecutablePath();
	std::filesystem::path GetSaveGameFolder();
}

} //namespace FileIO
