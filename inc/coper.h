#pragma once

#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include "VectorEx.h"

//hasher callback reason codes
#define HA_CALR_ERROR ((int)0)
#define HA_CALR_DIRECTORY_LISTING ((int)1)
#define HA_CALR_FILE_ADDING ((int)2)
#define HA_CALR_COMPUTING_HASH ((int)3)

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
		uint64_t total_size = 0;

		directoryInfo_t& operator+=(const directoryInfo_t& lhs)
		{
			total_size += lhs.total_size;
			file_count += lhs.file_count;
			return (*this);
		}
	};

private:


public:
	coper_t(int thread_count_, std::wstring source, std::wstring mask_ = L"*", std::wstring dst);


};