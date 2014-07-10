// rotatefilter.h : header file for the transform filter
//

#pragma once

#include "rotation.h"
#include <streams.h>

class CRotateTransform : public CTransformFilter
{
public:
    CRotateTransform(LPUNKNOWN pUnkOuter, HRESULT *phr, eRotation rot);

    HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT CheckInputType(const CMediaType *mtIn);
    HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	HRESULT Copy(IMediaSample *pSource, IMediaSample *pDest) const;

	eRotation mRot;
};
