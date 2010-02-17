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
class FileManager;
typedef boost::shared_ptr<File> FilePtr;
typedef boost::function < File* ( const std::string &filename, READ_MODE mode ) > FileFabric;

class FileManager
{
	public:
		FileManager ( unsigned int cache_size );
		~FileManager();

		/**
		 * Производит открытие файла. Для открытия файла производится поиск по всем зарегистрированным модулям взаимодействия с файлами, в порядке их регистрации.
		 * @param filename имя файла, который требуется открыть
		 * @param mode режим открытия файла
		 * @param cache режим кеширования файла
		 * @return объект для работы с открываемым файлом
		 */
		FilePtr open ( const std::string &filename, READ_MODE mode = READ_ONLY, CACHE_MODE cache = CACHE );
		/**
		 * Производит удаление из кеша указанного кол-ва файлов
		 * @param count количество файлов, которое требуется удалить. По умолчанию производится полная очистка кеша
		 * @return true в случае успешного выполнения операции.
		 */
		bool clean ( int count = -1 );
		/**
		 * Производит регистрацию модуля взаимодействия с файлами.
		 * @param file_manager фабрика
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
		 * Производит чтение данных из файла
		 * @param buf адрес по которому будут помещены считанные данные
		 * @param size размер считываемых данных в байтах
		 */
		virtual void read ( void *buf, int size ) = 0;
		/**
		 * Производит запись данных в файл
		 * @param buf адрес по которому находятся данные для записи
		 * @param size размер записываемых данных в байтах
		 */
		virtual void write ( void *buf, int size ) = 0;
		/**
		 * Производит смещение в файле на указанное кол-во байт
		 * @param position величина смещения в байтах
		 * @param whence точка от которой задано смещение, по умолчанию - от текущей точки
		 */
		virtual void seek ( int64_t position, FILE_POSITION whence = POS_CUR ) = 0;
		/**
		 * Производит отображение файла в память
		 * @return указатель на начало отображенной памяти
		 */
		virtual void *mmap() = 0;
		/**
		 * Производит сброс указателей чтения/записи
		 */
		virtual void reset() = 0;
		/**
		 * Получение размера файла
		 * @return размер файлами
		 */
		virtual uint64_t get_file_size() const { return _file_size_;}

		virtual ~File() {};
	protected:
		File ( const std::string &filename, READ_MODE mode ) : _filename_ ( filename ), _file_mode_ ( mode ), _file_size_(0) {};
		std::string _filename_;
		READ_MODE _file_mode_;
		uint64_t _file_size_;
};

File* CreatePosixFile ( const std::string& filename, READ_MODE mode );

} //namespace FileIO
