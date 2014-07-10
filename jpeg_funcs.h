#pragma once

class JPEGHandler
{
public:
	JPEGHandler(unsigned char *pSource, int len);
	void DecompressToRGB(unsigned char *pDest, int len);
	int CompressFromRGB(unsigned char *pDest, int len, int width, int height);

private:
	unsigned char	*mpData;
	int		mDataLen;
};
