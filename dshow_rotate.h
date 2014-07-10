// dshow_rotate.h : header file
//

#pragma once

#pragma warning(disable : 4793)
#pragma warning(disable : 4996)

#include <dshow.h>

void ProcessAVI(const wchar_t *source_filename, const wchar_t *dest_filename, IBaseFilter *pVComp);
void GetCompressionProperties(IBaseFilter *pFilter);
void playavi(const wchar_t *source_filename, HWND hwnd);

