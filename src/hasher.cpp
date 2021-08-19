#include <atlbase.h>
#include <atlconv.h>

#include <Windows.h>
#include <stdio.h>

#include "futils.h"

static std::string ToStr(wchar_t const* input) { return std::string(CW2A(input, CP_UTF8)); }
static std::wstring ToWstr(char const* input) { return std::wstring(CA2W(input, CP_UTF8)); }

void hasher_t::addDirectory(std::wstring dir)
{
    WIN32_FIND_DATAW* FindFileData = new WIN32_FIND_DATAW;
    directoryInfo_t retval;

    dir += L"\\*";
    HANDLE hFind = FindFirstFileW(dir.c_str(), FindFileData);
    dir = dir.substr(0, dir.length() - (mask.length()));

    if (hFind == INVALID_HANDLE_VALUE)
        return;

    size_t precision = total_work.file_count / 1024;
    if (!precision) precision = 1;
    do
    {
        if (wcscmp(FindFileData->cFileName, L"..") && wcscmp(FindFileData->cFileName, L"."))
            if ((FindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                addDirectory(dir + FindFileData->cFileName);
            else
            {
                files_vector.push_back(
                    {
                        (((uint64_t)FindFileData->ftLastWriteTime.dwHighDateTime) << 32 | (uint64_t)FindFileData->ftLastWriteTime.dwLowDateTime),//last Write
                        dir + L"\\" + FindFileData->cFileName,//name
                        L"",//hash leave blank
                        (((uint64_t)FindFileData->nFileSizeHigh) << 32 | (uint64_t)FindFileData->nFileSizeLow)//size 
                    });

                if (files_vector.size() % precision == 0)
                    basic_progress_callbask(HA_CALR_FILE_ADDING, total_work, { files_vector.size(),0 }, 0);
            }
    } while (FindNextFileW(hFind, FindFileData));

    FindClose(hFind);
    delete FindFileData;

    return;
}

//main compute thread function
void hasher_t::computeHash(hasher_t* cins)
{
    unsigned char* hash_buff = new unsigned char[64];
    char* hex_hash_buff = new char[129];


    FILE* fp = nullptr;
    file_deskriptor_t fds = { 0, L"",L"",0 };

    while (1)
    {
        memset(hash_buff, 0, 64);
        memset(hex_hash_buff, 0, 129);

        size_t index_tmp = cins->index;

        cins->files_vector_mutex.lock();
        {
            if (cins->index >= cins->files_vector.size())
            {
                cins->files_vector_mutex.unlock();
                break;
            }

            fds = cins->files_vector[cins->index++];
        }
        cins->files_vector_mutex.unlock();

        _wfopen_s(&fp, fds.FPath.c_str(), L"rb");

        if (fp == nullptr)
        {
            cins->files_vector_mutex.lock();
            cins->files_vector[index_tmp].FHash = L"--------------------------------------------------------------------------------------------------------------------------------";
            cins->files_vector_mutex.unlock();

            cins->total_work_mutex.lock();
            cins->total_work.file_count--;
            cins->total_work.total_size -= fds.Size;
            cins->total_work_mutex.unlock();

            continue;
        }

        fsha512(fp, hash_buff, cins->sha512_progress_callback);

        cins->work_progres_mutex.lock();
        cins->work_progres.file_count++;
        cins->work_progres_mutex.unlock();

        for (size_t i = 0; i < 64; i++)
            snprintf(hex_hash_buff + strlen(hex_hash_buff), 129 - strlen(hex_hash_buff), "%02X", hash_buff[i]);

        cins->files_vector_mutex.lock();
        cins->files_vector[index_tmp].FHash = ToWstr(hex_hash_buff);
        cins->files_vector_mutex.unlock();

        fclose(fp);
        fp = nullptr;
    }

    return;
}


void hasher_t::initializer_thread(hasher_t* cins)
{
    cins->basic_progress_callbask(HA_CALR_DIRECTORY_LISTING, { 0 }, { 0 }, 0);
    cins->total_work = cins->DirStat(cins->directory, cins->mask);

    cins->addDirectory(cins->directory);

    for (int i = 0; i < cins->thread_count; i++)
        cins->thread_vector.push_back(new std::thread(&cins->computeHash, cins));

    cins->threads_started = true;
}


hasher_t::hasher_t(int thread_count_, std::wstring directory_, std::wstring mask_)
{

    sha512_progress_callback = [&](size_t total, size_t computed, size_t computed_diff, int err_code)//internal sha512 callback
    {
        directoryInfo_t total_work_targ;
        directoryInfo_t total_progress_targ;

        work_progres_mutex.lock();
        {
            work_progres.total_size += computed_diff;
            total_progress_targ = work_progres;
        }
        work_progres_mutex.unlock();

        total_work_mutex.lock();
        total_work_targ = total_work;
        total_work_mutex.unlock();

        basic_progress_callbask(HA_CALR_COMPUTING_HASH, total_work_targ, total_progress_targ, 0);
    };

    thread_count = thread_count_;
    directory = directory_;
    mask = mask_;

    flist_thread = new std::thread(&initializer_thread, this);
}

hasher_t::~hasher_t()
{
    if (flist_thread != nullptr)
    {
        flist_thread->join();
        delete flist_thread;
        flist_thread = nullptr;
    }

    thread_vector.foreach([&](std::thread*& th) { if (th == nullptr) return;  th->join(); delete th; });
    thread_vector.clear();
}

VectorEx<hasher_t::file_deskriptor_t> hasher_t::getResult()
{
    return files_vector;
}

hasher_t* hasher_t::wait()
{

    while (!threads_started) Sleep(25);

    thread_vector.foreach([&](std::thread*& th) { if (th == nullptr) return;  th->join(); delete th; });
    thread_vector.clear();

    return this;
}

//generate directory info file cont & size
hasher_t::directoryInfo_t hasher_t::DirStat(std::wstring dir, std::wstring& mask)
{
    WIN32_FIND_DATAW* FindFileData = new WIN32_FIND_DATAW;//to prevent stack overflow
    hasher_t::directoryInfo_t retval;

    dir += L"\\*";
    HANDLE hFind = FindFirstFileW(dir.c_str(), FindFileData);
    dir = dir.substr(0, dir.length() - (mask.length()));

    if (hFind == INVALID_HANDLE_VALUE)
        return { 0,0 };

    do
    {
        if (wcscmp(FindFileData->cFileName, L"..") && wcscmp(FindFileData->cFileName, L"."))
            if ((FindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                retval += DirStat(dir + FindFileData->cFileName, mask);
            else
            {
                retval.file_count++;
                retval.total_size += (((uint64_t)FindFileData->nFileSizeHigh) << 32 | (uint64_t)FindFileData->nFileSizeLow);
            }
    } while (FindNextFileW(hFind, FindFileData));

    FindClose(hFind);
    delete FindFileData;

    return retval;
}


void hasher_t::basic_progress_callbask(int call_reason, hasher_t::directoryInfo_t total_work, hasher_t::directoryInfo_t done, int error_code)
{
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
        printf("\rcomputing SHA512 %lf%%  files:%llu/%llu  %s", (long double)((long double)done.total_size * (long double)100LL / (long double)total_work.total_size), done.file_count, total_work.file_count, (done.total_size == total_work.total_size) ? "\n" : "");
        break;

    default:
        printf("unknown call reason\n");
        break;
    }
    stdout_mutex.unlock();
}