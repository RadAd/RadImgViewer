#include "FreeImageHelper.h"

#include <stdlib.h>
#include <string.h>

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
