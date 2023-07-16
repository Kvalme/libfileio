/*
 *  Copyright (c) 2015, <copyright holder> <email>
 */

#include "posixfile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <filesystem>

#ifndef WIN32
	#include <sys/mman.h>
#include <dirent.h>
#else
	#include <windows.h>
	#include <sys/stat.h>
	void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
	int munmap(void *start, size_t length);
	#define MAP_PRIVATE 1
	#define MAP_FAILED 0
	#define MAP_SHARED 1
	#define PROT_READ 0
	#define getpagesize() 65536
#endif

using namespace FileIO;

#define THROW(reason, code) throw(FileIOError(reason, code))

namespace fs = std::filesystem;

class PosixFile : public File
{
	public:
		int read(void *buf, std::size_t size) override;
		int write(const void *buf, std::size_t size) override;
		void seek(int64_t position, FILE_POSITION whence = POS_CUR) override;
		void reset() override;
		void *mmap() override;
		~PosixFile();
		PosixFile(const fs::path &filename, READ_MODE mode, int fdesc);
	protected:
		int _fdesc_;
		void *_mapped_address_;
};

FileIO::PosixFileFactory::PosixFileFactory(const std::vector<std::filesystem::path> &base_paths) :
	base_paths_(base_paths)
{

}

File *PosixFileFactory::operator()(const std::string &fname, READ_MODE mode)
{
	// Choose correct path
	fs::path selected_path;
	for (auto &path : base_paths_)
	{
		if (fs::exists(path / fname))
		{
			selected_path = path / fname;
			break;
		}
	}

	int fdesc;
	switch (mode)
	{
		case READ_ONLY:
			fdesc = ::open(selected_path.c_str(), O_RDONLY);
			if (fdesc < 0) return 0;
			else return new PosixFile(fname, mode, fdesc);
			break;
		case WRITE_ONLY:
			fdesc = ::creat(selected_path.c_str(), 0666);
			if (fdesc < 0) return 0;
			else return new PosixFile(fname, mode, fdesc);
			break;
		case READ_WRITE:
			fdesc = ::open(selected_path.c_str(), O_RDWR);
			if (fdesc < 0) return 0;
			else return new PosixFile(selected_path, mode, fdesc);
			break;
	}

	return 0;
}

int PosixFile::read(void *buf, std::size_t size)
{
	if (_fdesc_ < 0) THROW("Invalid file descriptor!", INVALID_FILE);
	if (_file_mode_ == WRITE_ONLY) THROW("Read attemption for write-only file", READ_ON_WRITE);
	ssize_t readed = ::read(_fdesc_ , buf, size);
	if (readed == 0) THROW("Read from " + _filename_ + ": " + strerror(errno), READ_ERROR);
	return readed;
}
int PosixFile::write(const void *buf, std::size_t size)
{
	if (_fdesc_ < 0) THROW("Invalid file descriptor!", INVALID_FILE);
	if (_file_mode_ == READ_ONLY) THROW("Write attemption for read-only file", WRITE_ON_READ);
	ssize_t written = ::write(_fdesc_ , buf, size);
	if (written == 0) THROW("Write to " + _filename_ + ": " + strerror(errno), WRITE_ERROR);
	return written;
}
void PosixFile::seek(int64_t position, FILE_POSITION whence)
{
	if (_fdesc_ < 0) THROW("Invalid file descriptor!", INVALID_FILE);
	int iwhence;
	switch (whence)
	{
		case POS_CUR:
			iwhence = SEEK_CUR;
			break;
		case POS_END:
			iwhence = SEEK_END;
			break;
		case POS_SET:
		default:
			iwhence = SEEK_SET;
			break;
	}
	if (lseek(_fdesc_, position, iwhence) == -1) THROW("Seek failed for file:" + _filename_ + ": " + strerror(errno), SEEK_ERROR);
}
void PosixFile::reset()
{
	seek(0, POS_SET);
}
void *PosixFile::mmap()
{
	if (_file_mode_ != READ_ONLY) THROW("File mapping supported only for read-only files", MAP_FOR_WRITE);

	//Отмапим файл
	_mapped_address_ = (char *) ::mmap(0, _file_size_, PROT_READ, MAP_SHARED, _fdesc_, 0);
	if (_mapped_address_ == MAP_FAILED)
	{
		THROW(strerror(errno), MAP_ERROR);
	}
	return _mapped_address_;
}
PosixFile::~PosixFile()
{
	if (_mapped_address_)munmap(_mapped_address_, _file_size_);
	if (_fdesc_ >= 0) ::close(_fdesc_);
}
PosixFile::PosixFile(const fs::path &filename, READ_MODE mode, int fdesc) : File(filename, mode), _fdesc_(fdesc), _mapped_address_(0)
{
	if (mode != WRITE_ONLY)
	{
		struct stat file_stat;
		if (fstat(_fdesc_, &file_stat) < 0) THROW(strerror(errno), INVALID_FILE);
		_file_size_ = file_stat.st_size;
	}
}

bool PosixFileFactory::ListDir(const std::string &path, std::vector< std::string > *subdirs, std::vector< std::string > *files)
{
	DIR *dir = opendir(path.c_str());
	if (!dir) return false;

	for(dirent *dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir))
	{
		struct stat statBuf;
		lstat((path + "/" + dirEntry->d_name).c_str(), &statBuf);
		if (S_ISDIR(statBuf.st_mode)) subdirs->push_back(dirEntry->d_name);
		else if(S_ISREG(statBuf.st_mode) || S_ISLNK(statBuf.st_mode)) files->push_back(dirEntry->d_name);
	}
	closedir(dir);

	return true;
}

#ifdef WIN32
void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	HANDLE handle;

	if (start != NULL || !(flags & MAP_PRIVATE))  return MAP_FAILED;

	if (offset % getpagesize() != 0) return MAP_FAILED;

	handle = CreateFileMapping((HANDLE) _get_osfhandle(fd), NULL, PAGE_WRITECOPY, 0, 0, NULL);

	if (handle != NULL)
	{
		start = MapViewOfFile(handle, FILE_MAP_COPY, 0, offset, length);
		CloseHandle(handle);
	}
	return start;
}

int munmap(void *start, size_t length)
{
	UnmapViewOfFile(start);
	return 0;
}
#endif
