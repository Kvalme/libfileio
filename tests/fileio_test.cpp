#include "fileio.h"
#include <iostream>
int main ( int argc, char *argv[] )
{
	try
	{
		if ( argc < 3 )
		{
			std::cerr << "Usage: " << argv[0] << " <file_name_for_read> <file_name_for_write>" << std::endl;
			return 0;
		}
		FileIO::FileManager manager ( 1 );
		manager.register_fileio ( new FileIO::CreatePosixFile );

		FileIO::FilePtr file = manager.open ( argv[1] );

		std::cout << "Testing read:" << std::endl;
		char *buf = new char[file->get_file_size() +1];
		file->read ( buf, file->get_file_size() );
		buf[file->get_file_size() ] = 0;
		std::cout << buf << std::endl;
		delete[] buf;

		std::cout << "Testing mmap:" << std::endl;
		file->reset();
		buf = ( char* ) file->mmap();
		std::cout << buf << std::endl;

		std::cout << "Testing cached opening:" << std::endl;
		for ( int a = 0;a < 2;a++ )
		{
			manager.open ( argv[1] );
		}

		std::cout << "Testing file write:" << std::endl;
		FileIO::FilePtr wfile = manager.open ( argv[2], FileIO::WRITE_ONLY );
		wfile->write ( buf, file->get_file_size() );

		std::cout << "Testing cached opening:" << std::endl;
		for ( int a = 0;a < 2;a++ )
		{
			manager.open ( argv[1] );
		}
	}
	catch ( FileIO::FileIOError error )
	{
		std::cerr << "====ERROR====" << std::endl;
		std::cerr << error.what()<<std::endl;
		std::cerr << "Error code:" << error.get_error_code() << std::endl;
		std::cerr << "=============" << std::endl;
	}

}

