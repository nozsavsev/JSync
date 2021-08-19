#define _CRT_SECURE_NO_WARNINGS
#include <openssl/sha.h>
#include <filesystem>
#include <iostream>

#include <chrono>

using namespace std::chrono;

class timer
{
protected:
    high_resolution_clock::time_point begin;
    high_resolution_clock::time_point end;
public:
    timer() { begin = end = high_resolution_clock::now(); }
    inline void Start() { begin = high_resolution_clock::now(); }
    inline void Stop() { end = high_resolution_clock::now(); }

    template <typename T>
    long long GetResult() { return duration_cast<T>(end - begin).count(); }
};

void main()
{



}










/*
using namespace std;
using namespace std::filesystem;

unsigned char* hash_buff = new unsigned char[64];
char* hex_hash_buff = new char[129];

void fsha512(FILE* src, size_t src_file_size)
{
    static char* buffer = (char*)malloc(1024LL * 1024LL * 1024LL);

    static size_t calculated = 0, rd_retval = 0;
    static SHA512_CTX context;

    SHA512_Init(&context);

    while (calculated < src_file_size)
    {
        if ((rd_retval = fread(buffer, 1024LL * 1024LL * 1024LL, 1, src)) == 0) break;
        SHA512_Update(&context, buffer, rd_retval);
        calculated += rd_retval;
    }

    SHA512_Final(hash_buff, &context);
}

void main()
{
    FILE* fp = NULL;

    for (auto& p : recursive_directory_iterator("E:\\Games\\", directory_options::skip_permission_denied))
        if (p.is_regular_file())
        {
            _wfopen_s(&fp, p.path().wstring().c_str(), L"rb");
            if (fp)
            {
                fsha512(fp, p.file_size());
                fclose(fp);
                fp = nullptr;
            }
        }
}




void replicateDirectoryStructure(uint64_t originLen, std::wstring src, std::wstring mask, std::wstring dst)
{
    WIN32_FIND_DATAW* FindFileData = new WIN32_FIND_DATAW;//to prevent stack overflow
    hasher_t::directoryInfo_t retval;

    src += mask;
    HANDLE hFind = FindFirstFileW(src.c_str(), FindFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        wprintf(L"SUS ===> %s\n", src.c_str());
        return;
    }

    src = src.substr(0, src.length() - (mask.length()));

    do
    {
        if (wcscmp(FindFileData->cFileName, L"..") && wcscmp(FindFileData->cFileName, L"."))
        {
            if ((FindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
            {

                src + L"\\" + FindFileData->cFileName;
                wprintf(L"mkdir %s\\%s\n", dst.c_str(), src.substr(originLen).c_str());
                replicateDirectoryStructure(originLen, src, mask, dst);
                src = src.substr(0, originLen);
            }
        }
    } while (FindNextFileW(hFind, FindFileData));

    FindClose(hFind);
    delete FindFileData;

    return;
}


void WatchDirectory(LPTSTR lpDir)
{
    DWORD dwWaitStatus;
    HANDLE dwChangeHandles[3];
    WCHAR lpDrive[4];
    WCHAR lpFile[_MAX_FNAME];
    WCHAR lpExt[_MAX_EXT];

    _wsplitpath_s(lpDir, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);

    lpDrive[2] = '\\';
    lpDrive[3] = '\0';

    dwChangeHandles[0] = FindFirstChangeNotification(lpDir, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME);
    dwChangeHandles[1] = FindFirstChangeNotification(lpDrive, TRUE, FILE_NOTIFY_CHANGE_DIR_NAME);
    dwChangeHandles[2] = FindFirstChangeNotification(lpDrive, TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE);

    while (TRUE)
    {
        dwWaitStatus = WaitForMultipleObjects(3, dwChangeHandles, FALSE, INFINITE);

        switch (dwWaitStatus)
        {

        case WAIT_OBJECT_0:
            FindNextChangeNotification(dwChangeHandles[0]);
            printf("FILE: create | rename | delete\n");
            break;

        case WAIT_OBJECT_0 + 1:
            FindNextChangeNotification(dwChangeHandles[1]);
            printf("DIR : create | rename | delete\n");
            break;

        case WAIT_OBJECT_0 + 2:
            FindNextChangeNotification(dwChangeHandles[2]);
            printf("FILE: last write\n");
            break;

        default:
            wprintf(L"\n ERROR : Unhandled dwWaitStatus.\n");
            break;
        }
    }
}
*/