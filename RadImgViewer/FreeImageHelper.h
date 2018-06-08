#pragma once

#include <Windows.h>
#include <FreeImage.h>
#include <tchar.h>
#include <string>
#include <Rad\tstring.h>

#ifdef UNICODE
#define FreeImage_GetFIFFromFilename FreeImage_GetFIFFromFilenameU
#define FreeImage_Load FreeImage_LoadU
#define FreeImage_Save FreeImage_SaveU

#define FreeImage_GetFormatFromFIF FreeImage_GetFormatFromFIFU
#define FreeImage_OpenMultiBitmap FreeImage_OpenMultiBitmapU
#endif

typedef DLL_API BOOL DLL_CALLCONV FilterSupportsType(FREE_IMAGE_FORMAT fif);

const wchar_t *FreeImage_GetFormatFromFIFU(FREE_IMAGE_FORMAT fif);
FIMULTIBITMAP* FreeImage_OpenMultiBitmapU(FREE_IMAGE_FORMAT fif, const wchar_t *filename, BOOL create_new, BOOL read_only, BOOL keep_cache_in_memory FI_DEFAULT(FALSE), int flags FI_DEFAULT(0));

int GetFilter(std::tstring& buf, const TCHAR SEP, FilterSupportsType* FilterSupports, FREE_IMAGE_FORMAT searchformat = FIF_UNKNOWN);
FREE_IMAGE_FORMAT GetFilterFromIndex(FilterSupportsType* FilterSupports, int searchindex);
