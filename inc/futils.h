#pragma once

#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include "VectorEx.h"

#define FCPY_MAX_BUFFER_SIZE   (1024LL*1024LL*1024LL)   //512 MB
#define FCPY_MIN_BUFFER_SIZE   (1024LL)                //1KB

#define FCPY_BUFFER_PRESCALER  (256LL)

#define FCPY_NOERROR                    (0)
#define FCPY_ERROR_FIRST_FILE           (1)
#define FCPY_ERROR_SECOND_FILE          (2)
#define FCPY_ERROR_SIZE_MISMATCH        (3)
#define FCPY_ERROR_OUT_OF_MEMORY        (4)

void fcpy_default_Progress_Callback(size_t total, size_t now, size_t diff, int err_code);
int fcpy(FILE* src, FILE* dst, std::function <void(size_t, size_t, size_t, int)> progress_callback = fcpy_default_Progress_Callback, int progress_call_count = 200);





#define FSHA_MAX_BUFFER_SIZE   (1024LL*1024LL*1024LL)   //512 MB
#define FSHA_MIN_BUFFER_SIZE   (1024LL*1024LL*10LL)    //10MB

#define FSHA_BUFFER_PRESCALER  (128LL)

#define FSHA_NOERROR                    (0)
#define FSHA_ERROR_FILE                 (1)
#define FSHA_ERROR_BUFFER               (2)
#define FSHA_ERROR_OPENSSL              (3)
#define FSHA_ERROR_OUT_OF_MEMORY        (4)

void fsha512_default_Progress_Callback(size_t total, size_t now, size_t diff, int err_code);
int fsha512(FILE* src, unsigned char* out_buf, std::function <void(size_t, size_t, size_t, int)> progress_callback = fsha512_default_Progress_Callback);





//hasher callback reason codes
#define HA_CALR_ERROR ((int)0)
#define HA_CALR_DIRECTORY_LISTING ((int)1)
#define HA_CALR_FILE_ADDING ((int)2)
#define HA_CALR_COMPUTING_HASH ((int)3)

//multi thread file hasher
//usefull when you have a lot of small files and want to compute hash for them
class hasher_t
{
public:
    class file_deskriptor_t
    {
    public:

        size_t LastWrite;

        std::wstring FPath;
        std::wstring FHash;
        uint64_t Size;
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
    std::atomic<bool> threads_started = false;
    int thread_count;
    VectorEx<std::thread*> thread_vector;

    //initial thread will list files and start enother dirs
    std::thread* flist_thread;

    std::wstring directory;//directory to scan
    std::wstring mask;     //filetypes

    //size of all files included
    directoryInfo_t total_work;
    std::mutex total_work_mutex;

    //progress
    directoryInfo_t work_progres;
    std::mutex work_progres_mutex;

    //sha512 name and size of each file
    VectorEx<file_deskriptor_t> files_vector;
    std::mutex files_vector_mutex;

    //internal iterators
    size_t last_computed = 0;
    uint64_t index = 0;

    //add fileanames and sizes to result vector
    void addDirectory(std::wstring dir);//add directory and all files to the list


    static void computeHash(hasher_t* cins);//main compute thread function

    std::function<void(size_t, size_t, size_t, int)> sha512_progress_callback;

    static void initializer_thread(hasher_t* cins);//this threal starts all computing threads


public:
    hasher_t(int thread_count_, std::wstring directory_, std::wstring mask_ = L"*");

    ~hasher_t();

    VectorEx<file_deskriptor_t> getResult();

    hasher_t* wait();//wait untill operation is end retruns pointer to this instance

    //generate directory info file cont & size
    static directoryInfo_t DirStat(std::wstring dir, std::wstring& mask);

    static void basic_progress_callbask(int call_reason, directoryInfo_t total_work, directoryInfo_t done, int error_code);
    //also an example
    //DO NOT FORGET TO SYNC STDOUT WITH MUTEX :) 
    /*{
        static std::mutex stdout_mutex;

        stdout_mutex.lock();
        switch (call_reason)
        {
        case HA_CALR_ERROR:
            printf("error with code %d\n", error_code);
            break;

        case HA_CALR_DIRECTORY_LISTING:
            printf("inital directory scan, please wait it may take a while\n");
            break;

        case HA_CALR_FILE_ADDING:
            printf("\radding files %d%%  %s", (int)(done.file_count * 100LL / total_work.file_count), (done.file_count == total_work.file_count) ? "\n" : "");
            break;

        case HA_CALR_COMPUTING_HASH:
            printf("\rcomputing SHA512 %d%%  files:%llu/%llu  %s", (int)(done.total_size * 100LL / total_work.total_size), done.file_count, total_work.file_count, (done.total_size == total_work.total_size) ? "\n" : "");
            break;

        default:
            printf("unknown call reason\n");
            break;
        }
        stdout_mutex.unlock();
    }
    */
};