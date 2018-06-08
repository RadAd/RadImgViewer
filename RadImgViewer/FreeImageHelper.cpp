#include "FreeImageHelper.h"

#include <stdlib.h>
#include <string.h>

// =====================================================================
// File IO functions
// =====================================================================

unsigned DLL_CALLCONV
_ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
    return (unsigned) fread(buffer, size, count, (FILE *) handle);
}

unsigned DLL_CALLCONV
_WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle) {
    return (unsigned) fwrite(buffer, size, count, (FILE *) handle);
}

int DLL_CALLCONV
_SeekProc(fi_handle handle, long offset, int origin) {
    return fseek((FILE *) handle, offset, origin);
}

long DLL_CALLCONV
_TellProc(fi_handle handle) {
    return ftell((FILE *) handle);
}

// ----------------------------------------------------------

void
SetDefaultIO(FreeImageIO *io) {
    io->read_proc = _ReadProc;
    io->seek_proc = _SeekProc;
    io->tell_proc = _TellProc;
    io->write_proc = _WriteProc;
}


#undef FreeImage_GetFormatFromFIF

const wchar_t *FreeImage_GetFormatFromFIFU(FREE_IMAGE_FORMAT fif)
{
    static wchar_t s_fmt[10];
    const char *fmt = FreeImage_GetFormatFromFIF(fif);
    int i = 0;
    do
    {
        s_fmt[i] = fmt[i];
    } while (fmt[i] != '\0');
    return s_fmt;
}

FIMULTIBITMAP* FreeImage_OpenMultiBitmapU(FREE_IMAGE_FORMAT fif, const wchar_t *filename, BOOL create_new, BOOL read_only, BOOL keep_cache_in_memory, int flags)
{
    FreeImageIO io;
    SetDefaultIO(&io);
    FILE *handle = NULL;
    if (_wfopen_s(&handle, filename, L"rb") == 0)
        return FreeImage_OpenMultiBitmapFromHandle(fif, &io, handle, flags);
    else
        return NULL;
}

int GetFilter(std::tstring& buf, const TCHAR SEP, FilterSupportsType* FilterSupports, FREE_IMAGE_FORMAT searchformat)
{
    int FoundIndex = 1;
    int CountedIndex = 0;
    int Count = FreeImage_GetFIFCount();
    for (int Index = 0; Index < Count; ++Index)
    {
        //FREE_IMAGE_FORMAT format = FreeImage_GetFIFByIndex(Index);
        FREE_IMAGE_FORMAT format = static_cast<FREE_IMAGE_FORMAT>(Index);
        if (FilterSupports(format))
        {
            ++CountedIndex;
            if (format == searchformat)
                FoundIndex = CountedIndex;

            const char* desc = FreeImage_GetFIFDescription(format);
            buf.append(desc, desc + strlen(desc));
            buf += SEP;

            const char* FIFExtensionList = FreeImage_GetFIFExtensionList(format);
            buf += _T('*');
            buf += _T('.');
            while (*FIFExtensionList != _T('\0'))
            {
                if (*FIFExtensionList == _T(','))
                {
                    buf += _T(';');
                    buf += _T('*');
                    buf += _T('.');
                }
                else
                    buf += *FIFExtensionList;
                ++FIFExtensionList;
            }
            buf += SEP;
        }
    }
    return FoundIndex;
}

FREE_IMAGE_FORMAT GetFilterFromIndex(FilterSupportsType* FilterSupports, int searchindex)
{
    int CountedIndex = 0;
    int Count = FreeImage_GetFIFCount();
    for (int Index = 0; Index < Count; ++Index)
    {
        //FREE_IMAGE_FORMAT format = FreeImage_GetFIFByIndex(Index);
        FREE_IMAGE_FORMAT format = static_cast<FREE_IMAGE_FORMAT>(Index);
        if (FilterSupports(format))
        {
            ++CountedIndex;
            if (searchindex == CountedIndex)
                return format;
        }
    }
    return FIF_UNKNOWN;
}
