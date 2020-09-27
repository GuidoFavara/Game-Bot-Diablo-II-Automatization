#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALLEGRO_USE_CONSOLE
#include <allegro.h>

#pragma warning (disable : 4996)

#pragma pack(1)
typedef struct
{
   long version;        // 06 00 00 00
   long unknown1;       // 01 00 00 00
   long unknown2;       // 00 00 00 00
   char termination[4]; // EE EE EE EE or CD CD CD CD
   long directions;     // xx 00 00 00
   long frames_per_dir; // xx 00 00 00
} DC6_HEADER_S;

typedef struct
{
   long unknown1;
   long width;
   long height;
   long offset_x;
   long offset_y; // from bottom border, not up
   long unknown2;
   long next_block;
   long length;
} DC6_FRAME_HEADER_S;
#pragma pack()


// ==========================================================================
int load_dat_palette(char * palette_name, PALETTE pal)
{
   FILE * in;
   int  i, r, g, b;

   in = fopen(palette_name, "rb");
   if (in == NULL)
   {
      fprintf(stderr, "can't open palette file \"%s\"\n", palette_name);
      return 1;
   }

   // read d2 dat palette format (BGR instead of RGB)
   for (i=0; i<256; i++)
   {
      b = fgetc(in);
      g = fgetc(in);
      r = fgetc(in);
      pal[i].r = r >> 2;
      pal[i].g = g >> 2;
      pal[i].b = b >> 2;
   }
   fclose(in);

   // ok
   return 0;
}


// ==========================================================================
void * load_dc6_in_mem(char * dc6_name)
{
   FILE * in;
   void * ptr;
   long size;

   in = fopen(dc6_name, "rb");
   if (in == NULL)
   {
      fprintf(stderr, "can't open dc6 file \"%s\"\n", dc6_name);
      fflush(stderr);
      return NULL;
   }
   fseek(in, 0, SEEK_END);
   size = ftell(in);
   fseek(in, 0, SEEK_SET);
   ptr = (void *) malloc(size);
   if (ptr == NULL)
   {
      fprintf(stderr, "not enough mem (%li) for loading \"%s\" in mem",
         size, dc6_name);
      fflush(stderr);
      return NULL;
   }
   fread(ptr, size, 1, in);
   
   // ok
   fclose(in);
   return ptr;
}


// ==========================================================================
void decompress_dc6_frame(void * src, BITMAP * dst, long size, int x0, int y0)
{
   unsigned char * ptr = (unsigned char *) src;
   long          i;
   int           i2, x=x0, y=y0, c, c2;
   
   for (i=0; i<size; i++)
   {
      c = * (ptr ++);

      if (c == 0x80)
      {
         x = x0;
         y--;
      }
      else if (c & 0x80)
         x += c & 0x7F;
      else
      {
         for (i2=0; i2<c; i2++)
         {
            c2 = * (ptr ++);
            i++;
            putpixel(dst, x, y, c2);
            x++;
         }
      }
   }
}


// ==========================================================================
void compress_dc6_frame(FILE * out, BITMAP * src, DC6_FRAME_HEADER_S * frame,
                        int x1, int y1, int x2, int y2)
{
   BITMAP * sub;
   int    done = FALSE, n, i, c, x, y, w, h;

   w = x2 - x1 + 1;
   h = y2 - y1 + 1;
   x = 0;
   y = h-1;
   sub = create_sub_bitmap(src, x1, y1, w, h);
   if (sub == NULL)
      return;

   while (! done)
   {
      // End Of Line ?
      n = 0;
      while ((_getpixel(sub, x+n, y) == 0) && (x+n < w))
         n++;
      if (x+n >= w)
      {
         // EOL
         fputc(0x80, out);
         x = 0;
         y--;
         if (y <= 0)
            done = TRUE;
      }
      else
      {
         if (n)
         {
            // JUMPS
            while (n >= 0x7F)
            {
               fputc(0xFF, out);
               n -= 0x7F;
               x += 0x7F;
            }
            if (n)
               fputc(0x80 | n, out);
         }

         // PIXELS
         x += n;
         n = 0;
         while (_getpixel(sub, x+n, y) && (x+n < w))
            n++;
         if (n)
         {
            while (n >= 0x7F)
            {
               fputc(0x7F, out);
               for (i=0; i< 0x7F; i++)
               {
                  c = _getpixel(sub, x+i, y);
                  fputc(c, out);
               }
               n -= 0x7F;
               x += 0x7F;
            }
            if (n)
            {
               fputc(n, out);
               for (i=0; i < n; i++)
               {
                  c = _getpixel(sub, x+i, y);
                  fputc(c, out);
               }
               x += n;
               n = 0;
            }
         }
      }
   }
   
   // termination bytes
   for (i=0; i<3; i++)
      fputc(0xEE, out);
   
   destroy_bitmap(sub);
}


// ==========================================================================
int make_dc6_frame(FILE * out, BITMAP * bmp, DC6_FRAME_HEADER_S * frame)
{
   int x1, y1, x2, y2, x, y, x0, y0;

   x1 = y1 = 30000;
   x2 = y2 = -30000;

   // search the "box" of the frame, up & right borders excluded
   for (y=1; y<bmp->h; y++)
   {
      for (x=0; x<bmp->w - 1; x++)
      {
         if (_getpixel(bmp, x, y))
         {
            if (x < x1) x1 = x;
            if (x > x2) x2 = x;
            if (y < y1) y1 = y;
            if (y > y2) y2 = y;
         }
      }
   }
   x2++;
   y1--;

   // search the origin points
   x0 = y0 = -1;

   for (x=0; (x < bmp->w - 1) && (x0 == -1); x++)
      if (_getpixel(bmp, x, 0) == 255)
         x0 = x;

   for (y=1; (y < bmp->h) && (y0 == -1); y++)
      if (_getpixel(bmp, bmp->w - 1, y) == 255)
         y0 = y;

   if ((x0 == -1) || (y0 == -1))
   {
      fprintf(stderr, "this bitmap don't have the origin points\n");
      return 1;
   }

   // fill the frame datas
   frame->offset_x = x1 - x0;
   frame->offset_y = - y0 + y2;
   frame->width    = x2 - x1;
   frame->height   = y2 - y1;

   // compress it, and end
   compress_dc6_frame(out, bmp, frame, x1, y1, x2, y2);
   return 0;
}


// ==========================================================================
int many_pcx_to_dc6(char * main_pcx_name, long dir, long frm)
{
   DC6_HEADER_S       header;
   DC6_FRAME_HEADER_S frame;
   BITMAP             * bmp;
   PALETTE            dummy_pal;
   char               tmp[600];
   int                i;
   long               d, f, zero = 0, frame_ptr_base, cur_ptr;
   long               start_data_ptr, end_data_ptr;
   FILE               * out;

   // check if all pcx are there
   for (d=0; d<dir; d++)
   {
      for (f=0; f<frm; f++)
      {
         sprintf(tmp, "%s-%02i-%03i.pcx", main_pcx_name, d, f);
         if (file_exists(tmp, FA_RDONLY | FA_HIDDEN | FA_ARCH, NULL) == 0)
         {
            fprintf(stderr, "can't find \"%s\"\n", tmp);
            fflush(stderr);
            return 1;
         }
      }
   }

   // no return, so all necessary pcx are available
   sprintf(tmp, "%s.dc6", main_pcx_name);
   out = fopen(tmp, "wb+");
   if (out == NULL)
   {
      fprintf(stderr, "can't create \"%s\"\n", tmp);
      fflush(stderr);
      return 1;
   }
   
   // make the dc6
   for (d=0; d<dir; d++)
   {
      for (f=0; f<frm; f++)
      {
         sprintf(tmp, "%s-%02i-%03i.pcx", main_pcx_name, d, f);
         fprintf(stderr, "%s\n", tmp);
         if (file_exists(tmp, FA_RDONLY | FA_HIDDEN | FA_ARCH, NULL) == 0)
         {
            fprintf(stderr, "can't find \"%s\"\n", tmp);
            fflush(stderr);
            return 1;
         }
         else
         {
            bmp = load_pcx(tmp, dummy_pal);
            if (bmp == NULL)
            {
               fprintf(stderr, "error while reading \"%s\"\n", tmp);
               fflush(stderr);
               fclose(out);
               return 1;
            }
            if ((d == 0) && (f == 0))
            {
               // make & write dc6 header
               header.version        = 0x00000006L;
               header.unknown1       = 0x00000001L;
               header.unknown2       = 0x00000000L;
               header.termination[0] = 0xEE;
               header.termination[1] = 0xEE;
               header.termination[2] = 0xEE;
               header.termination[3] = 0xEE;
               header.directions     = dir;
               header.frames_per_dir = frm;
               fwrite(& header, sizeof(DC6_HEADER_S), 1, out);
               frame_ptr_base = ftell(out);
               for (i=0; i < (dir * frm); i++)
                  fwrite(& zero, 4, 1, out);
            }
            cur_ptr = ftell(out);

            // update frame pointer
            fflush(out);
            fseek(out, frame_ptr_base + 4 * (frm * d + f), SEEK_SET);
            fwrite(& cur_ptr, 4, 1, out);
            fflush(out);
            fseek(out, cur_ptr, SEEK_SET);

            // temp frame header
            frame.unknown1   = 0L;
            frame.width      = 0L;
            frame.height     = 0L;
            frame.offset_x   = 0L;
            frame.offset_y   = 0L;
            frame.unknown2   = 0L;
            frame.next_block = 0L;
            frame.length     = 0L;
            fwrite(& frame, sizeof(DC6_FRAME_HEADER_S), 1, out);

            // make & write the frame
            start_data_ptr = ftell(out);
            if (make_dc6_frame(out, bmp, & frame))
            {
               destroy_bitmap(bmp);
               fclose(out);
               return 1;
            }
            end_data_ptr = ftell(out);

            // update the frame header
            frame.next_block = end_data_ptr;
            frame.length     = end_data_ptr - start_data_ptr - 3;
            fflush(out);
            fseek(out, cur_ptr, SEEK_SET);
            fwrite(& frame, sizeof(DC6_FRAME_HEADER_S), 1, out);
            fflush(out);
            fseek(out, end_data_ptr, SEEK_SET);

            // end of this frame
            destroy_bitmap(bmp);
         }
      }
   }
   fclose(out);

   return 0;
}


// ==========================================================================
int dc6_to_many_pcx(char * dc6_name, PALETTE pal)
{
   DC6_HEADER_S       * header;
   DC6_FRAME_HEADER_S * frame_header;
   long               dir, frm, * ptr;
   char               tmp_str[256], pcx_name[256];
   int                size, x1, x2, y1, y2, w, h, x0, y0;
   BITMAP             * bmp, * bmp2;

   header = load_dc6_in_mem(dc6_name);
   if (header == NULL)
      return 1;
   strcpy(tmp_str, dc6_name);
   size = strlen(tmp_str);
   if (size >= 4)
      tmp_str[size - 4] = 0;

   // search the "box" where the frames fit within
   x1 = y1 = 30000;
   x2 = y2 = - 300000;
   for (dir=0; dir<header->directions; dir++)
   {
      for (frm=0; frm<header->frames_per_dir; frm++)
      {
         ptr = (long *) (header + 1);
         ptr += (dir * header->frames_per_dir) + frm;
         frame_header = (DC6_FRAME_HEADER_S *) ((char *) header + (* ptr));
         if (frame_header->offset_x < x1)
            x1 = frame_header->offset_x;
         if (frame_header->offset_x + frame_header->width > x2)
            x2 = frame_header->offset_x + frame_header->width;
         if (frame_header->offset_y - frame_header->height < y1)
            y1 = frame_header->offset_y - frame_header->height;
         if (frame_header->offset_y > y2)
            y2 = frame_header->offset_y;
      }
   }
   if (x1 > 0) x1 = 0;
   if (x2 < 0) x2 = 0;
   if (y1 > 0) y1 = 0;
   if (y2 < 0) y2 = 0;

   w = x2 - x1 + 2;
   h = y2 - y1 + 2;
   
   pal[0].r = pal[0].g = pal[0].b = 16;
   
   // debug pcx
   bmp2 = create_bitmap(w, h);
   clear(bmp2);
   hline(bmp2, 0, h - 1 - y2, w, 255);
   vline(bmp2, -x1, 0, h, 255);

   // frames
   for (dir=0; dir<header->directions; dir++)
   {
      for (frm=0; frm<header->frames_per_dir; frm++)
      {
         sprintf(pcx_name, "%s-%02li-%03li.pcx", tmp_str, dir, frm);
         fprintf(stderr, "%s\n", pcx_name);
         fflush(stderr);

         ptr = (long *) (header + 1);
         ptr += (dir * header->frames_per_dir) + frm;
         frame_header = (DC6_FRAME_HEADER_S *) ((char *) header + (* ptr));

         bmp = create_bitmap(w, h);
         if (bmp == NULL)
         {
            fprintf(stderr, "can't create BITMAP %li * %li, direction %li, frame %li\n",
               frame_header->width, frame_header->height, dir, frm);
            fflush(stderr);
            free(header);
            return 1;
         }
         clear(bmp);

         x0 = frame_header->offset_x - x1;
         y0 = h - 1 + frame_header->offset_y - y2;
         decompress_dc6_frame((void *) (frame_header + 1), bmp,
            frame_header->length, x0, y0);
         
         // origin points
         putpixel(bmp, w-1, h - 1 - y2, 255);
         putpixel(bmp, -x1, 0, 255);
         draw_sprite(bmp2, bmp, 0, 0);
         save_pcx(pcx_name, bmp, pal);
         destroy_bitmap(bmp);
      }
   }

   // end
   free(header);
   sprintf(pcx_name, "%s-debug.pcx", tmp_str);
   fprintf(stderr, "%s\n", pcx_name);
   save_pcx(pcx_name, bmp2, pal);
   destroy_bitmap(bmp2);
   return 0;
}


// ==========================================================================
int main(int argc, char ** argv)
{
   PALETTE pal;


   allegro_init();
   set_color_depth(8);

   if ((argc < 2) || (argc > 4))
   {
      printf("\nDC6EDIT v1.02, freeware"
             "=======================\n"
             "\n"
             "syntaxe :\n"
             "   1) dc6 to pcx : dc6edit file.dc6 [palette.dat]\n"
             "   2) pcx to dc6 : dc6edit file directions frames_per_directions\n"
             "\n"
             "ex :\n"
             "   1) ds6edit mptrlita2hth.dc6 act1.dat\n"
             "   2) ds6edit mptrlita2hth 8 16\n"
      );
      return 0;
   }

   if (argc == 4)
   {
      // pcx to dc6
      if (many_pcx_to_dc6(argv[1], atoi(argv[2]), atoi(argv[3])))
         return 1;
   }
   else
   {
      // dc6 to pcx
      if (argc == 3)
      {
         if (load_dat_palette(argv[2], pal))
            return 1;
      }
      else
      {
         if (load_dat_palette("act1.dat", pal))
            return 1;
      }
      if (dc6_to_many_pcx(argv[1], pal))
         return 1;
   }
   return 0;
}
END_OF_MAIN()
