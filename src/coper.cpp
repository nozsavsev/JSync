#pragma once

#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include "VectorEx.h"

//hasher callback reason codes
#define CP_CALR_ERROR ((int)0)
#define CP_CALR_DIRECTORY_LISTING ((int)1)
#define CP_CALR_FILE_ADDING ((int)2)
#define CP_CALR_FCPY_PROGRESS ((int)3)
#define CP_CALR_DIRECTORY_STRUCTURE_REPRODUCING ((int)4)

//multi thread file hasher
//useful when you have a lot of small files and want to compute hash for them
class coper_t
{
public:
	class file_deskriptor_t
	{
	public:
		std::wstring FPath = L"";
		bool done = false;
	};

	class directoryInfo_t
	{
	public:

		uint64_t file_count = 0;
		uint64_t dir_count = 0;
		uint64_t total_size = 0;

		directoryInfo_t& operator+=(const directoryInfo_t& lhs)
		{
			total_size += lhs.total_size;
			dir_count += lhs.dir_count;
			file_count += lhs.file_count;
			return (*this);
		}
	};

private:
	directoryInfo_t everything;
	VectorEx <file_deskriptor_t> src_files;
	VectorEx <file_deskriptor_t> err_files;


public:
	coper_t(int thread_count_, std::wstring source, std::wstring dst, std::wstring mask_ = L"*")
	{



	}

	void basic_progress_callbask(int call_reason, coper_t::directoryInfo_t total_work, coper_t::directoryInfo_t done, int error_code)
	{
		static std::mutex stdout_mutex;

		stdout_mutex.lock();
		switch (call_reason)
		{
		case CP_CALR_ERROR:
			printf("error with code %d\n", error_code);
			break;

		case CP_CALR_DIRECTORY_LISTING:
			printf("initial directory scan, please wait it may take a while\n");
			break;

		case CP_CALR_FILE_ADDING:
			printf("\radding files %d%%  %s", (int)(done.file_count * 100LL / total_work.file_count), (done.file_count == total_work.file_count) ? "\n" : "");
			break;

		case CP_CALR_FCPY_PROGRESS:
			printf("\rcomputing SHA512 %lf%%  files:%llu/%llu  %s", (long double)(done.total_size * 100LL / (long double)total_work.total_size), done.file_count, total_work.file_count, (done.total_size == total_work.total_size) ? "\n" : "");
			break;

		case CP_CALR_DIRECTORY_STRUCTURE_REPRODUCING:
			printf("\rreproducing directory structure %lf%%    %s", total_work.dir_count * 100 / (long double)done.dir_count, (done.total_size == total_work.total_size) ? "\n" : "");
			break;

		default:
			printf("unknown call reason\n");
			break;
		}
		stdout_mutex.unlock();
	}

};