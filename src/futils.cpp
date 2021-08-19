#include "futils.h"
#include <stdio.h>
#include <openssl/sha.h>










// ************************************************************************************************************** //
// **                                                                                                          ** //
// ************************************************************************************************************** //

int fcpy(FILE* src, FILE* dst, std::function <void(size_t, size_t, size_t, int)> progress_callback, int progress_call_count)
{
    if (src == nullptr || dst == nullptr)
        return (src == nullptr) ? FCPY_ERROR_FIRST_FILE : FCPY_ERROR_SECOND_FILE;

    int retval = 0;         //success code

    size_t fseek_original_location = 0;
    size_t src_file_size = 0;
    size_t buffer_size = 0;

    char* buffer = nullptr;

    //----- Save initial location -----//

    fseek_original_location = _ftelli64(src);

    //----- Get file size -----//

    _fseeki64(src, 0, SEEK_END);
    //{
    src_file_size = _ftelli64(src);
    //}
    _fseeki64(src, 0, SEEK_SET);

    //----- Determine & allocate buffer -----//

    // for small files < 1 kB buffer_size = FCPY_MAX_BUFFER_SIZE, for middle size files (15 kB) buffer_size = 15 bytes
    buffer_size = ((src_file_size / FCPY_BUFFER_PRESCALER) < FCPY_MAX_BUFFER_SIZE && (src_file_size / FCPY_BUFFER_PRESCALER) > 0) ? (src_file_size / FCPY_BUFFER_PRESCALER) : FCPY_MAX_BUFFER_SIZE;

    if (src_file_size <= FCPY_MIN_BUFFER_SIZE)
        buffer_size = src_file_size;


    buffer = new char[buffer_size];

    if (!buffer)
    {
        progress_callback(0, 0, 0, FCPY_ERROR_OUT_OF_MEMORY);
        return FCPY_ERROR_OUT_OF_MEMORY;
    }

    //----- Copy file -----//

    size_t copied = 0;

    size_t rd_retval = 0;
    size_t wr_retval = 0;

    while (copied < src_file_size)
    {
        rd_retval = fread(buffer, 1, buffer_size, src);
        if (rd_retval == 0)
        {
            progress_callback(0, 0, 0, FCPY_ERROR_FIRST_FILE);
            retval = FCPY_ERROR_FIRST_FILE;
            break;
        }

        wr_retval = fwrite(buffer, 1, rd_retval, dst);

        if (wr_retval == 0)
        {
            progress_callback(0, 0, 0, FCPY_ERROR_SECOND_FILE);
            retval = FCPY_ERROR_SECOND_FILE;
            break;
        }

        if (wr_retval != rd_retval)
        {
            progress_callback(0, 0, 0, FCPY_ERROR_SIZE_MISMATCH);
            retval = FCPY_ERROR_SIZE_MISMATCH;
            break;
        }

        copied += rd_retval;

        progress_callback(src_file_size, copied, rd_retval, FCPY_NOERROR);
    }

    //----- Cleanup -----//

    _fseeki64(src, fseek_original_location, SEEK_SET);

    if (buffer)
    {
        delete[] buffer;
        buffer = nullptr;
    }

    return retval;
}

void fcpy_default_Progress_Callback(size_t total, size_t now, size_t diff, int err_code)
{
    constexpr const char* err_strings[] = { "No error", "first file descriptor error", "second file descriptor error", "read write size mismatch","out of memory" };

    constexpr int last_code = (sizeof(err_strings) / sizeof(err_strings[0])) - 1;

    printf("\rcopying file %lf%%    ", (long double)(now * 100 / (long double)total));

    if (err_code)
        printf("\n%s", (err_code <= last_code) ? err_strings[err_code] : "unknown error code");
}


// ************************************************************************************************************** //
// **                                                                                                          ** //
// ************************************************************************************************************** //

int fsha512(FILE* src, unsigned char* out_buf, std::function <void(size_t, size_t, size_t, int)> progress_callback)
{
    if (src == nullptr || out_buf == nullptr)
        return (src == nullptr) ? FSHA_ERROR_FILE : FSHA_ERROR_BUFFER;

    int retval = 0;         //success code

    size_t fseek_original_location = 0;
    size_t src_file_size = 0;
    size_t buffer_size = 0;

    char* buffer = nullptr;

    //----- Save initial location -----//

    fseek_original_location = _ftelli64(src);

    //----- Get file size -----//

    _fseeki64(src, 0, SEEK_END);
    //{
    src_file_size = _ftelli64(src);
    //}
    _fseeki64(src, 0, SEEK_SET);

    //----- Determine & allocate buffer -----//

    // for small files < 1 kB buffer_size = FSHA_MAX_BUFFER_SIZE, for middle size files (15 kB) buffer_size = 15 bytes
    buffer_size = ((src_file_size / FSHA_BUFFER_PRESCALER) < FSHA_MAX_BUFFER_SIZE && (src_file_size / FSHA_BUFFER_PRESCALER) > 0) ? (src_file_size / FSHA_BUFFER_PRESCALER) : FSHA_MAX_BUFFER_SIZE;

    if (buffer_size > src_file_size)
        buffer_size = src_file_size;


    if (src_file_size <= FSHA_MIN_BUFFER_SIZE)
        buffer_size = src_file_size;

    buffer = new char[buffer_size];

    if (!buffer)
    {
        progress_callback(0, 0, 0, FSHA_ERROR_OUT_OF_MEMORY);
        return FSHA_ERROR_OUT_OF_MEMORY;
    }

    //----- Compute SHA512 -----//

    size_t calculated = 0;
    size_t rd_retval = 0;
    SHA512_CTX context;

    if (SHA512_Init(&context) == 0)
        progress_callback(0, 0, 0, FSHA_ERROR_OPENSSL);

    size_t last_comp = 0;

    while (calculated < src_file_size)
    {
        rd_retval = fread(buffer, buffer_size, 1, src);
        if (rd_retval == 0)
        {
            progress_callback(src_file_size, calculated, 0, FSHA_ERROR_FILE);
            retval = FSHA_ERROR_FILE;
            break;
        }

        if (SHA512_Update(&context, buffer, rd_retval) == 0)
            progress_callback(src_file_size, calculated, 0, FSHA_ERROR_OPENSSL);

        last_comp = rd_retval;
        calculated += rd_retval;
        progress_callback(src_file_size, calculated, rd_retval, FSHA_NOERROR);

    }



    if (SHA512_Final(out_buf, &context) == 0)
        progress_callback(src_file_size, calculated, 0, FSHA_ERROR_OPENSSL);

    progress_callback(src_file_size, calculated, 0, FSHA_NOERROR);

    //----- Cleanup -----//

    _fseeki64(src, fseek_original_location, SEEK_SET);

    if (buffer)
    {
        delete[] buffer;
        buffer = nullptr;
    }

    return retval;
}

void fsha512_default_Progress_Callback(size_t total, size_t now, size_t diff, int err_code)
{
    constexpr const char* err_strings[] = { "No error", "file descriptor error", "out buffer error", "openssl error","out of memory" };

    constexpr int last_code = (sizeof(err_strings) / sizeof(err_strings[0])) - 1;

    printf("\rcomputing sha512 %d%%    ", (int)(now * 100LL / total));

    if (err_code)
        printf("\n%s", err_code <= last_code ? err_strings[err_code] : "unknown error code");
}