// dshow_rotate.cpp : implementation file
//

#include "stdafx.h"
#include "RotateAVI.h"
#include "RotateAVIDlg.h"
#include "rotatefilter.h"

#pragma comment(lib, "strmiids")
#pragma comment(lib, "strmbasd")
#pragma comment(lib, "winmm")

void CRotateAVIDlg::strcpyTCHARtoWCHAR(const TCHAR *src, wchar_t *dest) const
{
	while (*dest++ = *src++);
}

void CRotateAVIDlg::strcpyWCHARtoTCHAR(const wchar_t *src, TCHAR *dest) const
{
	while (*dest++ = (TCHAR) *src++);
}

void CRotateAVIDlg::RunGraph(IGraphBuilder *pGraph, IBaseFilter *pMux)
{
	REFERENCE_TIME rtTotal;

	// Query for interfaces. Remember to release them later.
	HRESULT hr = pMux->QueryInterface(IID_IMediaSeeking, (void**)&mpSeek);
	if (SUCCEEDED(hr)) 
	{
		hr = pGraph->QueryInterface(IID_IMediaEventEx, (void**)&mpEvent);
		if (SUCCEEDED(hr)) 
		{
			hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&mpControl);
			if (SUCCEEDED(hr)) 
			{
				// Set the DirectShow event notification window.
				hr = mpEvent->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, 0);

				// Set the range of the progress bar to the file length (in seconds).
				hr = mpSeek->GetDuration(&rtTotal);
				SendDlgItemMessage(IDC_PROGRESS1, PBM_SETRANGE, 0, MAKELPARAM(0, rtTotal / 10000000));
				// Start the timer.
				UINT_PTR res = SetTimer(WM_TIMER, 100, 0);
				// Run the graph.
				mpControl->Run();
			}
		}
	}
}

void CRotateAVIDlg::ProcessAVI(const TCHAR *source_filename, const TCHAR *dest_filename, IBaseFilter *pVComp, eRotation rot)
{
	wchar_t src_filename_buf[MAX_PATH];
	strcpyTCHARtoWCHAR(source_filename, src_filename_buf);

	wchar_t dest_filename_buf[MAX_PATH];
	strcpyTCHARtoWCHAR(dest_filename, dest_filename_buf);

	// create the capture graph
	ICaptureGraphBuilder2 *pBuild = NULL;
	HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
									IID_ICaptureGraphBuilder2, (void **)&pBuild);

	if (SUCCEEDED(hr))
	{

		// build the rendering section
		IBaseFilter *pMux = NULL;
		hr = pBuild->SetOutputFileName(&MEDIASUBTYPE_Avi, // File type. 
										dest_filename_buf,     // File name, as a wide-character string.
										&pMux,             // Receives a pointer to the multiplexer.
										NULL);             // Receives a pointer to the file writer. 
		if (SUCCEEDED(hr))
		{
			// get the filter graph
			IGraphBuilder *pGraph = NULL;
			hr = pBuild->GetFiltergraph(&pGraph);
			if (SUCCEEDED(hr))
			{
				// add the source filter
				IBaseFilter *pSrc = NULL;
				hr = pGraph->AddSourceFilter(src_filename_buf, L"Source Filter", &pSrc);
				if (SUCCEEDED(hr))
				{
					// add the transform filter
					IBaseFilter *pTrans = NULL;
					pTrans = AddTransformFilter(pGraph, rot);

					if (pTrans)
					{
						// add the compressor
						hr = pGraph->AddFilter(pVComp, L"Compressor");
						if (SUCCEEDED(hr))
						{
							// connect the graph up - video
							hr = pBuild->RenderStream(
								NULL,       // Output pin category
								NULL,       // Media type
								pSrc,
								0,
								pTrans);
							hr = pBuild->RenderStream(
								NULL,       // Output pin category
								NULL,       // Media type
								pTrans,
								pVComp,
								pMux);

							if (SUCCEEDED(hr))
							{
								// connect the graph up - audio
								// if this fails, it means there is no audio to deal with
								hr = pBuild->RenderStream(NULL, NULL, pSrc, NULL, pMux);

								RunGraph(pGraph, pMux);
							}
							else
							{
								MessageBox(TEXT("Couldn't connect the graph"));
							}
						}
						pTrans->Release();
					}
					pSrc->Release();
				}
				pGraph->Release();
			}
			pMux->Release();
		}
		pBuild->Release();
	}
}

void CRotateAVIDlg::ReleaseResources()
{
	IMoniker *pMoniker;
	CComboBox *pCombo = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO1));
	int combo_num = pCombo->GetCount();
	for(int i = 0; i < combo_num; ++i)
	{
		pMoniker = reinterpret_cast<IMoniker *>(pCombo->GetItemData(i));
		if (pMoniker)
			pMoniker->Release();
	}

	if (mpTransFilter)
	{
		mpTransFilter->Release();
		mpTransFilter = NULL;
    }
}

void CRotateAVIDlg::FillCompressorComboBox(void)
{
	TCHAR buf[1024];
    ICreateDevEnum *pSysDevEnum = NULL;
    IEnumMoniker *pEnum = NULL;
    IMoniker *pMoniker = NULL;

    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
        CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, 
        (void**)&pSysDevEnum);
    if (FAILED(hr)) return;

	CComboBox *pCombo = static_cast<CComboBox *>(GetDlgItem(IDC_COMBO1));
	int combo_index = 0;
    hr = pSysDevEnum->CreateClassEnumerator(
             CLSID_VideoCompressorCategory, &pEnum, 0);
    if (SUCCEEDED(hr))  // failed means nothing in this category
    {
        while (S_OK == pEnum->Next(1, &pMoniker, NULL))
        {
            IPropertyBag *pPropBag = NULL;
            pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
                (void **)&pPropBag);
            VARIANT var;
            VariantInit(&var);
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
            if (SUCCEEDED(hr))
            {
				strcpyWCHARtoTCHAR(var.bstrVal, buf);
				pCombo->AddString(buf);
				pCombo->SetItemDataPtr(combo_index++, pMoniker);
            }   
            VariantClear(&var); 
            pPropBag->Release();
        }
    }
    pSysDevEnum->Release();
    pEnum->Release();

	pCombo->SetCurSel(0);
}


IBaseFilter *CRotateAVIDlg::AddTransformFilter(IGraphBuilder *pGraph, eRotation rot)
{
    if (mpTransFilter)
	{
		mpTransFilter->Release();
		mpTransFilter = NULL;
    }

    HRESULT hr = S_OK;
    mpTransFilter = new CRotateTransform(NULL, &hr, rot);
    // make the initial refcount 1 to match COM creation
    mpTransFilter->AddRef();

    // add to graph -- nb need to Query properly for the
    // right interface before giving that to the graph object
    IBaseFilter* pFilter;
    hr = mpTransFilter->QueryInterface(IID_IBaseFilter, (void**)&pFilter);
    if (SUCCEEDED(hr))
	{
		hr = pGraph->AddFilter(pFilter, L"Rotate Transform");
		return pFilter;
    }

	pFilter->Release();
	return 0;
}
