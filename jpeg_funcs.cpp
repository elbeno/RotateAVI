#include <stdio.h>
#include <setjmp.h>
#include <memory.h>
#include <cassert>

extern "C"
{
#include <jpeglib.h>
}

#include "jpeg_funcs.h"

// ----------------------------------------------------------------------------

struct my_error_mgr
{
	struct jpeg_error_mgr pub;	/* "public" fields */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);
}

// ----------------------------------------------------------------------------

struct my_source_mgr
{
	struct jpeg_source_mgr pub;	/* public fields */
	const char *data;
	int len;
};

typedef my_source_mgr * my_src_ptr;

METHODDEF(void)
my_init_source (j_decompress_ptr cinfo)
{
	/* no work necessary here */
}

METHODDEF(boolean)
my_fill_input_buffer (j_decompress_ptr cinfo)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;
	// we have the whole buffer in memory
	src->pub.bytes_in_buffer = src->len;
	src->pub.next_input_byte = (const JOCTET *)src->data;
	return TRUE;
}

METHODDEF(void)
my_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	my_src_ptr src = (my_src_ptr) cinfo->src;
	src->pub.next_input_byte += (size_t) num_bytes;
	src->pub.bytes_in_buffer -= (size_t) num_bytes;
}

METHODDEF(void)
my_term_source (j_decompress_ptr cinfo)
{
	/* no work necessary here */
}

// ----------------------------------------------------------------------------

JPEGHandler::JPEGHandler(unsigned char *pSource, int len)
	: mpData(pSource)
	, mDataLen(len)
{
}

/*
  static const unsigned int std_luminance_quant_tbl[64] = {
	0x06, 0x06, 0x06, 0x08, 0x0a, 0x08, 0x0a, 0x0a, 
	0x0a, 0x0a, 0x0a, 0x0e, 0x0e, 0x0e, 0x0a, 0x10, 
	0x12, 0x12, 0x12, 0x12, 0x10, 0x15, 0x1f, 0x21, 
	0x13, 0x21, 0x1f, 0x15, 0x19, 0x29, 0x29, 0x23,
	0x23, 0x29, 0x29, 0x19, 0x29, 0x29, 0x29, 0x27,
	0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 
	0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 
	0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29
  };
  static const unsigned int std_chrominance_quant_tbl[64] = {
	0x0a, 0x0e, 0x0e, 0x10, 0x12, 0x10, 0x13, 0x13,
	0x13, 0x13, 0x15, 0x1d, 0x19, 0x1d, 0x15, 0x1d,
	0x25, 0x25, 0x25, 0x25, 0x1d, 0x2b, 0x3e, 0x40,
	0x27, 0x40, 0x3e, 0x2b, 0x35, 0x54, 0x54, 0x48,
	0x48, 0x54, 0x54, 0x35, 0x54, 0x54, 0x54, 0x4e,
	0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54,   
	0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54,
	0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54
  };
*/
static unsigned int std_luminance_quant_tbl[64];
static unsigned int std_chrominance_quant_tbl[64];

void JPEGHandler::DecompressToRGB(unsigned char *pDest, int len)
{
	/* Step 1: allocate and initialize JPEG decompression object */
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source */
	struct my_source_mgr src_mgr;
	cinfo.src = (jpeg_source_mgr *)&src_mgr;
	src_mgr.pub.init_source = my_init_source;
	src_mgr.pub.fill_input_buffer = my_fill_input_buffer;
	src_mgr.pub.skip_input_data = my_skip_input_data;
	src_mgr.pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src_mgr.pub.term_source = my_term_source;
	src_mgr.len = mDataLen;
	src_mgr.data = (const char*)mpData;
	src_mgr.pub.bytes_in_buffer = 0;
	src_mgr.pub.next_input_byte = 0;

	/* Step 3: read file parameters with jpeg_read_header() */
	jpeg_read_header(&cinfo, TRUE);

	/* Step 4: set parameters for decompression */
	// defaults were set by jpeg_read_header
	cinfo.out_color_space = JCS_YCbCr; 	/* colorspace of output image */


	/* Step 5: Start decompressor */
	jpeg_start_decompress(&cinfo);

	/* We may need to do some setup of our own at this point before reading
	* the data.  After jpeg_start_decompress() we have the correct scaled
	* output image dimensions available, as well as the output colormap
	* if we asked for color quantization.
	* In this example, we need to make an output work buffer of the right size.
	*/ 
	/* JSAMPLEs per row in output buffer */
	int row_stride = cinfo.output_width * cinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	// check the output size
	assert(len == row_stride * cinfo.output_height);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */
	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	while (cinfo.output_scanline < cinfo.output_height)
	{
		// we want to reverse the scanline output
		//char *pD = &pDest[(cinfo.output_height - cinfo.output_scanline - 1) * row_stride];
		unsigned char *pD = &pDest[cinfo.output_scanline * row_stride];

		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could ask for
		* more than one scanline at a time if that's more convenient.
		*/
		jpeg_read_scanlines(&cinfo, buffer, 1);

		// copy to output
		char *pSrc = (char *)*buffer;
		memcpy(pD, *buffer, row_stride);

		/*
		// copy to output, reverse RGB
		for (int i = 0; i < cinfo.output_width; ++i)
		{
			*pD++ = pSrc[2];
			*pD++ = pSrc[1];
			*pD++ = pSrc[0];
			pSrc += 3;
		}
		*/
	}

	/* Step 7: Finish decompression */
	(void) jpeg_finish_decompress(&cinfo);

	// step 7.5 : copy quant tables
	for (int i = 0; i < 64; ++i)
	{
		std_luminance_quant_tbl[i] = cinfo.quant_tbl_ptrs[0]->quantval[i];
		std_chrominance_quant_tbl[i] = cinfo.quant_tbl_ptrs[1]->quantval[i];
	}
	
	/* Step 8: Release JPEG decompression object */
	jpeg_destroy_decompress(&cinfo);
}

// ----------------------------------------------------------------------------

struct my_dest_mgr
{
	struct jpeg_destination_mgr pub;	/* public fields */
	const char *data;
	int len;
	int complen;
};

typedef my_dest_mgr * my_dest_ptr;

METHODDEF(void)
my_init_destination (j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

	// we have the whole buffer in memory
	dest->pub.next_output_byte = (JOCTET *)dest->data;
	dest->pub.free_in_buffer = dest->len;
}

METHODDEF(boolean)
my_empty_output_buffer (j_compress_ptr cinfo)
{
	return TRUE;
}

METHODDEF(void)
my_term_destination (j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
	dest->complen = (int)(dest->len - dest->pub.free_in_buffer);
}

// ----------------------------------------------------------------------------

extern "C"
{
extern void emit_dri (j_compress_ptr cinfo);
extern void emit_sos (j_compress_ptr cinfo);
extern void emit_dqts_MPJG (j_compress_ptr cinfo);
extern void emit_sof (j_compress_ptr cinfo, int code);

struct jpeg_comp_master {
  JMETHOD(void, prepare_for_pass, (j_compress_ptr cinfo));
  JMETHOD(void, pass_startup, (j_compress_ptr cinfo));
  JMETHOD(void, finish_pass, (j_compress_ptr cinfo));

  /* State variables made visible to other modules */
  boolean call_pass_startup;	/* True if pass_startup must be called */
  boolean is_last_pass;		/* True during last pass */
};

extern void add_huff_table (j_compress_ptr cinfo, JHUFF_TBL **htblptr, const UINT8 *bits, const UINT8 *val);
}


void mjpeg_huff_tables (j_compress_ptr cinfo)
/* Set up the standard MJPEG Huffman tables  */
{
  static const UINT8 bits_dc_luminance[17] =
    { 0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  static const UINT8 val_dc_luminance[] =
    { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B };
  
  static const UINT8 bits_dc_chrominance[17] =
    { 0x01,0x00,0x03,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00 };
  static const UINT8 val_dc_chrominance[] =
    { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B };
  
  static const UINT8 bits_ac_luminance[17] =
    { 0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,0x00,0x01,0x7D };
  static const UINT8 val_ac_luminance[] =
    {	0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,
		0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,
		0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,
		0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,
		0x24,0x33,0x62,0x72,0x82,0x09,0x0A,0x16,
		0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,
		0x29,0x2A,0x34,0x35,0x36,0x37,0x38,0x39,
		0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
		0x4A,0x53,0x54,0x55,0x56,0x57,0x58,0x59,
		0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
		0x6A,0x73,0x74,0x75,0x76,0x77,0x78,0x79,
		0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
		0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
		0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
		0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,
		0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,
		0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,
		0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xE1,0xE2,
		0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,
		0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
		0xF9,0xFA };
  
  static const UINT8 bits_ac_chrominance[17] =
    { 0x11,0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,0x02,0x77 };
  static const UINT8 val_ac_chrominance[] =
    {	0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,
		0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,
		0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,
		0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,
		0x15,0x62,0x72,0xD1,0x0A,0x16,0x24,0x34,
		0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,
		0x27,0x28,0x29,0x2A,0x35,0x36,0x37,0x38,
		0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,
		0x49,0x4A,0x53,0x54,0x55,0x56,0x57,0x58,
		0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,
		0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
		0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,
		0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,
		0x97,0x98,0x99,0x9A,0xA2,0xA3,0xA4,0xA5,
		0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,
		0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xC2,0xC3,
		0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,
		0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,
		0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,
		0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
		0xF9,0xFA };
  
  add_huff_table(cinfo, &cinfo->dc_huff_tbl_ptrs[0],
		 bits_dc_luminance, val_dc_luminance);
  add_huff_table(cinfo, &cinfo->ac_huff_tbl_ptrs[0],
		 bits_ac_luminance, val_ac_luminance);
  add_huff_table(cinfo, &cinfo->dc_huff_tbl_ptrs[1],
		 bits_dc_chrominance, val_dc_chrominance);
  add_huff_table(cinfo, &cinfo->ac_huff_tbl_ptrs[1],
		 bits_ac_chrominance, val_ac_chrominance);
}

int JPEGHandler::CompressFromRGB(unsigned char *pDest, int len, int width, int height)
{
	/* Step 1: allocate and initialize JPEG compression object */
	struct jpeg_compress_struct cinfo;
	struct my_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination */
	struct my_dest_mgr dest_mgr;
	cinfo.dest = (jpeg_destination_mgr *)&dest_mgr;
	dest_mgr.pub.init_destination = my_init_destination;
	dest_mgr.pub.empty_output_buffer = my_empty_output_buffer;
	dest_mgr.pub.term_destination = my_term_destination;
	dest_mgr.len = len;
	dest_mgr.data = (const char *)pDest;
	dest_mgr.pub.next_output_byte = 0;
	dest_mgr.pub.free_in_buffer = 0;

	/* Step 3: set parameters for compression */
	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width = width; 	/* image width and height, in pixels */
	cinfo.image_height = height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_YCbCr; 	/* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	mjpeg_huff_tables(&cinfo);

	// quant tables
	jpeg_add_quant_table(&cinfo, 0, std_luminance_quant_tbl, 100, 1);
	jpeg_add_quant_table(&cinfo, 1, std_chrominance_quant_tbl, 100, 1);

	// 4:2:2
	jpeg_component_info *compptr = &cinfo.comp_info[0];
	compptr->h_samp_factor = 2;
	compptr->v_samp_factor = 1;
	compptr = &cinfo.comp_info[1];
	compptr->h_samp_factor = 1;
	compptr->v_samp_factor = 1;
	compptr = &cinfo.comp_info[2];
	compptr->h_samp_factor = 1;
	compptr->v_samp_factor = 1;

	/* Step 4: Start compressor */
	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);

	// step 4.5: omit the normal jpeg writing for MJPG, do it myself
	// at this point the header has been written
	emit_dri(&cinfo);
	emit_dqts_MPJG(&cinfo);
    emit_sof(&cinfo, 0xc0);
	emit_sos(&cinfo);
	cinfo.master->call_pass_startup = FALSE;

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */
	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride = cinfo.image_width * 3;	/* JSAMPLEs per row in image_buffer */
	while (cinfo.next_scanline < cinfo.image_height)
	{
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could pass
		* more than one scanline at a time if that's more convenient.
		*/
		row_pointer[0] = (JSAMPROW)&mpData[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/* Step 6: Finish compression */
	jpeg_finish_compress(&cinfo);

	/* Step 7: release JPEG compression object */
	jpeg_destroy_compress(&cinfo);

	return dest_mgr.complen;
}

