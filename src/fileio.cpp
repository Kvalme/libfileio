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

#include "fileio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#ifndef WIN32
#include <sys/mman.h>
#else
#include <windows.h>
#include <sys/stat.h>
void *mmap ( void *start, size_t length, int prot, int flags, int fd, off_t offset );
int munmap ( void *start, size_t length );
#define MAP_PRIVATE 1
#define MAP_FAILED 0
#define MAP_SHARED 1
#define PROT_READ 0
#define getpagesize() 65536
#endif

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
	for ( std::vector<FileFabric>::iterator it = _file_fabricks_.begin(); it != _file_fabricks_.end(); ++it )
	{
		file = ( *it ) ( filename, mode );
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
	for ( std::map<std::string, FilePtr>::iterator it = _file_cache_.begin(); it != _file_cache_.end(); )
	{
		_file_cache_.erase ( it++ );
		count--;
	}
	return count == 0;
}
void FileManager::register_fileio ( const FileFabric &file_manager )
{
	_file_fabricks_.push_back ( file_manager );
}
class PosixFile : public File
{
	public:
		int read ( void *buf, int size );
		int write ( const void *buf, int size );
		void seek ( int64_t position, FILE_POSITION whence = POS_CUR );
		void reset();
		void *mmap();
		~PosixFile();
		PosixFile ( const std::string &filename, READ_MODE mode, int fdesc );
	protected:
		int _fdesc_;
		void *_mapped_address_;
};

File* CreatePosixFile ( const std::string &filename, READ_MODE mode )
{
	int fdesc;
	switch ( mode )
	{
		case READ_ONLY:
			fdesc = ::open ( filename.c_str(), O_RDONLY );
			if ( fdesc < 0 ) return 0;
			else return new PosixFile(filename, mode, fdesc);
			break;
		case WRITE_ONLY:
			fdesc = ::creat ( filename.c_str(), 0666 );
			if ( fdesc < 0 ) return 0;
			else return new PosixFile(filename, mode, fdesc);
			break;
		case READ_WRITE:
			fdesc = ::open ( filename.c_str(), O_RDWR );
			if ( fdesc < 0 ) return 0;
			else return new PosixFile(filename, mode, fdesc);
			break;
	}
	return 0;
}
void PosixFile::read ( void *buf, int size )
{
	if ( _fdesc_ < 0 ) THROW ( "Invalid file descriptor!", INVALID_FILE );
	if ( _file_mode_ == WRITE_ONLY ) THROW ( "Read attemption for write-only file", READ_ON_WRITE );
	ssize_t readed = ::read ( _fdesc_ , buf, size )
	if (readed == 0) THROW ( "Read from " + _filename_ + ": " + strerror ( errno ), READ_ERROR );
	return readed;
}
void PosixFile::write ( const void *buf, int size )
{
	if ( _fdesc_ < 0 ) THROW ( "Invalid file descriptor!", INVALID_FILE );
	if ( _file_mode_ == READ_ONLY ) THROW ( "Write attemption for read-only file", WRITE_ON_READ);
	ssize_t written = ::write ( _fdesc_ , buf, size )
	if (written == 0) THROW ( "Write to " + _filename_ + ": " + strerror ( errno ), WRITE_ERROR );
	return written;
}
void PosixFile::seek ( int64_t position, FILE_POSITION whence )
{
	if ( _fdesc_ < 0 ) THROW ( "Invalid file descriptor!", INVALID_FILE );
	int iwhence;
	switch ( whence )
	{
		case POS_CUR:
			iwhence = SEEK_CUR;
			break;
		case POS_END:
			iwhence = SEEK_END;
			break;
		case POS_SET:
			iwhence = SEEK_SET;
			break;
	}
	if ( lseek ( _fdesc_, position, iwhence ) == -1 ) THROW ( "Seek failed for file:" + _filename_ + ": " + strerror ( errno ), SEEK_ERROR );
}
void PosixFile::reset()
{
	seek ( 0, POS_SET );
}
void* PosixFile::mmap()
{
	if ( _file_mode_ != READ_ONLY ) THROW ( "File mapping supported only for read-only files", MAP_FOR_WRITE );

	//Отмапим файл
	_mapped_address_ = ( char* ) ::mmap ( 0, _file_size_, PROT_READ, MAP_SHARED, _fdesc_, 0 );
	if ( _mapped_address_ == MAP_FAILED )
	{
		THROW ( strerror ( errno ), MAP_ERROR );
	}
	return _mapped_address_;
}
PosixFile::~PosixFile()
{
	if ( _mapped_address_ )munmap ( _mapped_address_, _file_size_ );
	if ( _fdesc_ >= 0 ) ::close ( _fdesc_ );
}
PosixFile::PosixFile ( const std::string &filename, READ_MODE mode, int fdesc ) : File ( filename, mode ), _fdesc_(fdesc), _mapped_address_(0)
{
	if(mode!=WRITE_ONLY)
	{
		struct stat file_stat;
		if ( fstat ( _fdesc_, &file_stat ) < 0 ) THROW ( strerror ( errno ), INVALID_FILE );
		_file_size_ = file_stat.st_size;
	}
}



} //namespace FileIO

#ifdef WIN32
void *mmap ( void *start, size_t length, int prot, int flags, int fd, off_t offset )
{
	HANDLE handle;

	if ( start != NULL || ! ( flags & MAP_PRIVATE ) )  return MAP_FAILED;

	if ( offset % getpagesize() != 0 ) return MAP_FAILED;

	handle = CreateFileMapping ( ( HANDLE ) _get_osfhandle ( fd ), NULL, PAGE_WRITECOPY, 0, 0, NULL );

	if ( handle != NULL )
	{
		start = MapViewOfFile ( handle, FILE_MAP_COPY, 0, offset, length );
		CloseHandle ( handle );
	}
	return start;
}

int munmap ( void *start, size_t length )
{
	UnmapViewOfFile ( start );
	return 0;
}
#endif

