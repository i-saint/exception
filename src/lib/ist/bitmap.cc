#include "bstream.h"
#include "bitmap.h"

extern "C" {
#ifdef IST_WITH_PNG
  #include <png.h>
#endif
#ifdef IST_WITH_JPG
  #include <jpeglib.h>
#endif
}

typedef unsigned char u_char;
typedef unsigned int u_int;


namespace ist {

Bitmap::Bitmap(const std::string& filename) { load(filename); }

bool Bitmap::load(const std::string& filename)
{
  clear();

  size_t len;
  if((len = filename.size())<5) {
    return false;
  }

  if((strncmp(&filename[len-3], "tga", 3)==0 && loadTGA(filename))
  || (strncmp(&filename[len-3], "bmp", 3)==0 && loadBMP(filename))
  || (strncmp(&filename[len-3], "png", 3)==0 && loadPNG(filename))
  || (strncmp(&filename[len-3], "jpg", 3)==0 && loadJPG(filename))) {
    return true;
  }
  else {
    return false;
  }
}

bool Bitmap::save(const std::string& filename) const
{
  size_t len;
  if((len = filename.size())<5) {
    return false;
  }

  if((strncmp(&filename[len-3], "tga", 3)==0 && saveTGA(filename))
  || (strncmp(&filename[len-3], "bmp", 3)==0 && saveBMP(filename))
  || (strncmp(&filename[len-3], "png", 3)==0 && savePNG(filename))
  || (strncmp(&filename[len-3], "jpg", 3)==0 && saveJPG(filename))) {
    return true;
  }
  else {
    return false;
  }
}




struct BMPHEAD {
  char B;
  char M;
  int file_size;
  short reserve1;
  short reserve2;
  int offset;

  BMPHEAD() {
    memset(this, 0, sizeof(*this));
    B='B';
    M='M';
    offset=54;
  }
};

struct BMPINFOHEAD {
  int header_size;
  int width;
  int height;
  short plane;
  short bits;
  int compression;
  int comp_image_size;
  int x_resolution;
  int y_resolution;
  int pallete_num;
  int important_pallete_num;

  BMPINFOHEAD() {
    memset(this, 0, sizeof(*this));
    header_size=sizeof(*this);
    plane=1;
    bits=24;
  }
};

struct TGAHEAD {
  u_char No_ID;
  u_char CMap_Type;
  u_char image_type;
  u_char CMap_Spec[5];
  short Ox;
  short Oy;
  short width;
  short height;
  u_char pixel;
  u_char IDesc_Type;

  TGAHEAD() {
    memset(this, 0, sizeof(*this));
    pixel=32;
    IDesc_Type=8;
  }
};


static bRGBA Read1Pixel(ist::bstream& bf)
{
  bRGBA t;
  bf >> t.b >> t.g >> t.r >> t.a;
  return t;
}


namespace RLECompress {
  std::vector<u_char> comp_pixel;


  static void WriteSameData(std::vector<bRGBA> &temp_pixel)
  {
    comp_pixel.push_back( temp_pixel.size()+0x80 );

    comp_pixel.push_back( temp_pixel[0].b );
    comp_pixel.push_back( temp_pixel[0].g );
    comp_pixel.push_back( temp_pixel[0].r );
    comp_pixel.push_back( temp_pixel[0].a );

    temp_pixel.clear();
  }

  static void WriteDifferentData(std::vector<bRGBA> &temp_pixel)
  {
    comp_pixel.push_back( temp_pixel.size()-1 );

    for(u_int d=0; d<temp_pixel.size(); d++) {
      comp_pixel.push_back( temp_pixel[d].b );
      comp_pixel.push_back( temp_pixel[d].g );
      comp_pixel.push_back( temp_pixel[d].r );
      comp_pixel.push_back( temp_pixel[d].a );
    }

    temp_pixel.clear();
  }

  static void Compress(const bRGBA *start, int width)
  {
    std::vector<bRGBA> same, diff;

    for(int i=0; i!=width; ++i, ++start) {
      const bRGBA *ip=start; ++ip;
      bRGBA dist=*start;

      if( i+1!=width && dist==*ip && same.size()<0x79 ) {
        same.push_back(dist);

        if(diff.size()!=0) {
          WriteDifferentData(diff);
        }
      }
      else  {
        if(same.size()>0x00) {
          WriteSameData(same);
        }
        else {
          diff.push_back(dist);

          if(diff.size()==0x79 ) {
            WriteDifferentData(diff);
          }
        }
      }
    }

    if(same.size()!=0x00) {
      WriteSameData(same);
    }
    else if(diff.size()!=0) {
      WriteDifferentData(diff);
    }
  }
}



// BMP

bool Bitmap::loadBMP(const std::string& filename) {
  BMPHEAD head;
  BMPINFOHEAD infohead;

  std::fstream f(filename.c_str(), std::ios::in | std::ios::binary);
  ist::biostream bf(f);
  if(!f) {
    return false;
  }

  bf >> head.B >> head.M;
  if(head.B!='B' || head.M!='M') {
    return false;
  }

  bf>> head.file_size
    >> head.reserve1
    >> head.reserve2
    >> head.offset;
  bf>> infohead.header_size
    >> infohead.width
    >> infohead.height
    >> infohead.plane
    >> infohead.bits
    >> infohead.compression
    >> infohead.comp_image_size
    >> infohead.x_resolution
    >> infohead.y_resolution
    >> infohead.pallete_num
    >> infohead.important_pallete_num;

  if(infohead.bits!=24) {
    return false;
  }

  resize(infohead.width, infohead.height);

  for(int i=getHeight()-1; i>=0; --i) {
    for(int j=0; j<getWidth(); ++j) {
      bRGBA& c = (*this)[i][j];
      bf >> c.b >> c.g >> c.r;
      c.a=255;
    }
  }

  return true;
}


bool Bitmap::saveBMP(const std::string& filename) const {
  if(empty()) {
    return false;
  }

  std::fstream f(filename.c_str(), std::ios::in | std::ios::binary);
  ist::biostream bf(f);
  if(!f) {
    return false;
  }

  BMPHEAD head;
  BMPINFOHEAD infohead;

  head.file_size = sizeof(BMPHEAD)+sizeof(BMPINFOHEAD)+getWidth()*getHeight()*3;
  infohead.width = getWidth();
  infohead.height = getHeight();

  bf<< head.B
    << head.M
    << head.file_size
    << head.reserve1
    << head.reserve2
    << head.offset;
  bf<< infohead.header_size
    << infohead.width
    << infohead.height
    << infohead.plane
    << infohead.bits
    << infohead.compression
    << infohead.comp_image_size
    << infohead.x_resolution
    << infohead.y_resolution
    << infohead.pallete_num
    << infohead.important_pallete_num;

  for(int i=getHeight()-1; i>=0; --i) {
    for(int j=0; j<getWidth(); ++j) {
      const bRGBA& c = (*this)[i][j];
      bf << c.b << c.g << c.r;
    }
  }

  return true;
}





// TGA

bool Bitmap::loadTGA(const std::string& filename) {
  TGAHEAD head;

  std::fstream f(filename.c_str(), std::ios::in | std::ios::binary);
  ist::biostream bf(f);
  if(!f) {
    return false;
  }

  bf>> head.No_ID
    >> head.CMap_Type
    >> head.image_type;
  bf.read(head.CMap_Spec, 5);
  bf>> head.Ox
    >> head.Oy
    >> head.width
    >> head.height
    >> head.pixel
    >> head.IDesc_Type;

  if(head.pixel!=32) {
    return false;
  }

  resize(head.width, head.height);

  for(int i=getHeight()-1; i>=0; --i) {
    if(head.image_type==2) {
      for(int j=0; j<getWidth(); j++) {
        (*this)[i][j] = Read1Pixel(bf);
      }
    }
    else if(head.image_type==10) {
      int loaded = 0;

      while(loaded<getWidth()) {
        u_char dist;

        bf >> dist;
        if( dist<0x80) {
          for(int j=0; j<dist+1; ++j, ++loaded) {
            (*this)[i][loaded ] = Read1Pixel(bf);
          }
        }
        else {
          bRGBA t = Read1Pixel(bf);
          for(int j=0x80; j<dist+1; ++j, ++loaded) {
            (*this)[i][loaded] = t;
          }
        }
      }
    }
  }

  return true;
}


bool Bitmap::saveTGA(const std::string& filename) const {
  if(empty()) {
    return false;
  }

  TGAHEAD head;
  head.width = getWidth();
  head.height = getHeight();
  head.image_type = 10;

  std::fstream f(filename.c_str(), std::ios::in | std::ios::binary);
  ist::biostream bf(f);
  if(!f) {
    return false;
  }

  bf<< head.No_ID
    << head.CMap_Type
    << head.image_type;
  bf.write(head.CMap_Spec, 5);
  bf<< head.Ox
    << head.Oy
    << head.width
    << head.height
    << head.pixel
    << head.IDesc_Type;


  for(int i=getHeight()-1; i>=0; --i) {
    RLECompress::Compress((*this)[i], getWidth());
  }
  for(std::vector<u_char>::iterator p=RLECompress::comp_pixel.begin(); p!=RLECompress::comp_pixel.end(); ++p) {
    bf << (*p);
  }


  std::vector<u_char>().swap(RLECompress::comp_pixel);

  return true;
}



// PNG

bool Bitmap::loadPNG(const std::string& filename) {
#ifdef IST_WITH_PNG
  FILE *fp = fopen(filename.c_str(), "rb");
  if(fp==0) {
    return false;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if(png_ptr==0) {
    fclose(fp);
    return false;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if(info_ptr==0) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
    return false;
  }

  png_init_io(png_ptr, fp);

  png_uint_32 w, h;
  int bit_depth, color_type, interlace_type;

  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);

  resize(w, h);

  png_set_strip_16(png_ptr);
  png_set_packing(png_ptr);
  if(color_type==PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_ptr);
  }
  if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth<8) {
    png_set_gray_1_2_4_to_8(png_ptr);
  }
  if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png_ptr);
  }
  png_read_update_info(png_ptr, info_ptr);


  std::vector<png_bytep> row_pointers(getHeight());
  for(int row=0; row<getHeight(); ++row) {
    row_pointers[row] = (png_bytep)png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));
  }
  png_read_image(png_ptr, &row_pointers[0]);

  if(color_type==PNG_COLOR_TYPE_RGB_ALPHA) {
    for(int i=0; i<getHeight(); ++i) {
      for(int j=0; j<getWidth(); ++j) {
        bRGBA& c = (*this)[i][j];
        c.r = row_pointers[i][j*4+0];
        c.g = row_pointers[i][j*4+1];
        c.b = row_pointers[i][j*4+2];
        c.a = row_pointers[i][j*4+3];
      }
    }
  }
  else if(color_type==PNG_COLOR_TYPE_RGB) {
    for(int i=0; i<getHeight(); ++i) {
      for(int j=0; j<getWidth(); ++j) {
        bRGBA& c = (*this)[i][j];
        c.r = row_pointers[i][j*3+0];
        c.g = row_pointers[i][j*3+1];
        c.b = row_pointers[i][j*3+2];
        c.a = 255;
      }
    }
  }

  for(int row=0; row<getHeight(); ++row) {
    png_free(png_ptr, row_pointers[row]);
  }

  png_read_end(png_ptr, info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
  fclose(fp);
  return true;

#else
  return false;
#endif // IST_WITH_PNG
}

bool Bitmap::savePNG(const std::string& filename) const
{
#ifdef IST_WITH_PNG
  FILE *fp = fopen(filename.c_str(), "wb");
  if(fp==0) {
    return false;
  }

  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

  if(png_ptr==0) {
    fclose(fp);
    return false;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if(info_ptr==0) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
    return false;
  }

  png_init_io(png_ptr, fp);
  png_set_IHDR(png_ptr, info_ptr, getWidth(), getHeight(), 8,
    PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info(png_ptr, info_ptr);

  Bitmap tmp(*this);
  std::vector<png_bytep> row_pointers(getHeight());
  for(int i=0; i<getHeight(); ++i) {
    row_pointers[i] = tmp[i][0].v;
  }

  png_write_image(png_ptr, &row_pointers[0]);

  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(fp);
  return true;

#else
  return false;
#endif // IST_WITH_PNG
}


// JPG

#ifdef IST_WITH_JPG
struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};
typedef struct my_error_mgr * my_error_ptr;

void my_error_exit(j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}
#endif // IST_WITH_JPG

bool Bitmap::loadJPG(const std::string& filename) {
#ifdef IST_WITH_JPG
  jpeg_decompress_struct cinfo;
  my_error_mgr jerr;
  FILE *infile;
  JSAMPARRAY buffer;
  int row_stride;

  clear();
  if((infile=fopen(filename.c_str(), "rb"))==0) {
    return false;
  }

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if(setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return false;
  }

  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);
  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  resize(cinfo.image_width, cinfo.image_height);
  size_t pix_count = 0;
  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, buffer, 1);
    for(size_t i=0; i<row_stride/3; ++i) {
      bRGBA col(buffer[0][i*3+0], buffer[0][i*3+1], buffer[0][i*3+2], 255);
      at(pix_count) = col;
      ++pix_count;
    }
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  return true;

#else
  return false;
#endif // IST_WITH_JPG
}

bool Bitmap::saveJPG(const std::string& filename, int quality) const {
#ifdef IST_WITH_JPG
  jpeg_compress_struct cinfo;
  jpeg_error_mgr jerr;
  FILE *outfile;
  JSAMPROW row_pointer[1];
  int row_stride;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  if((outfile = fopen(filename.c_str(), "wb"))==0) {
    return false;
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = getWidth();
  cinfo.image_height = getHeight();
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE);

  jpeg_start_compress(&cinfo, TRUE);

  row_stride = cinfo.image_width*3;

  u_char *buf = new u_char[getWidth()*getHeight()*3];
  for(size_t i=0; i<getWidth()*getHeight(); ++i) {
    buf[i*3+0] = at(i).r;
    buf[i*3+1] = at(i).g;
    buf[i*3+2] = at(i).b;
  }

  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = &buf[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);
  delete[] buf;
  fclose(outfile);

  jpeg_destroy_compress(&cinfo);
  return true;

#else
  return false;
#endif // IST_WITH_JPG
}




bool fBitmap1::load(const std::string& filename)
{
  std::fstream f(filename.c_str(), std::ios::in | std::ios::binary);
  ist::biostream bf(f);
  if(!f) {
    return false;
  }

  char fb1[3];
  bf.read(fb1, 3);
  if(fb1[0]!='F' || fb1[1]!='B' || fb1[2]!='1') {
    return false;
  }

  unsigned short w, h;
  bf >> w >> h;
  resize(w, h);
  for(size_t i=0; i<getHeight(); ++i) {
    for(size_t j=0; j<getWidth(); ++j) {
      bf >> (*this)[i][j];
    }
  }
  return true;
}

bool fBitmap1::save(const std::string& filename) const
{
  std::fstream f(filename.c_str(), std::ios::out | std::ios::binary);
  ist::biostream bf(f);
  if(!f) {
    return false;
  }

  const char fb1[] = "FB1";
  bf.write(fb1, 3);

  unsigned short w=getWidth(), h=getHeight();
  bf << w << h;
  for(size_t i=0; i<getHeight(); ++i) {
    for(size_t j=0; j<getWidth(); ++j) {
      bf << (*this)[i][j];
    }
  }
  return true;
}



}  // ist


