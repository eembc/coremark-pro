/*
(C) 2014 EEMBC(R).  All rights reserved.                            

All EEMBC Benchmark Software are products of EEMBC 
and are provided under the terms of the EEMBC Benchmark License Agreements.  
The EEMBC Benchmark Software are proprietary intellectual properties of EEMBC and its Members 
and is protected under all applicable laws, including all applicable copyright laws.  
If you received this EEMBC Benchmark Software without having 
a currently effective EEMBC Benchmark License Agreement, you must discontinue use. 
Please refer to LICENSE.md for the specific license agreement that pertains to this Benchmark Software.
*/

#ifndef _TH_FILE_H
#define _TH_FILE_H
#if _MSC_VER>=1500
/* visual studio 9 and above needs sal.h before th_types.h to avoid conflicts */
#include <sal.h>
#include <sys/stat.h>
#endif
#include "th_cfg.h"
#include "th_types.h"
#include "th_al.h"

#if	HAVE_STAT_H
#include <stat.h>
#elif HAVE_SYS_STAT_H
#include <sys/stat.h>
#elif PRIVATE_STAT_H
typedef struct stat stat;
#elif STUB_STAT
	struct stat {
		int st_size;
	};
#else
#error "struct stat not defined!"
#endif

/* File I/O - Streams */
#if (!USE_TH_FILEIO) && (!defined(ee_FILE))
	#define ee_FILE FILE
	#ifndef FILE_TYPE_DEFINED
	typedef struct FILE FILE;
	#define FILE_TYPE_DEFINED 1
	#endif
	#define ee_DIR DIR
#endif
#ifndef ee_stat 
	#define ee_stat stat
#endif

#if NEED_SEEK_PARAMS
	#define SEEK_SET	0	/* Seek from beginning of file.  */
	#define SEEK_CUR	1	/* Seek from current position.  */
	#define SEEK_END	2	/* Seek from end of file.  */
#endif

#if NEED_STD_FILES
	extern void *th_stdin;
	extern void *th_stdout;
	extern void *th_stderr;
#endif

int      th_fclose  (ee_FILE *fp);
int      th_ferror  (ee_FILE *fp);
int      th_feof    (ee_FILE *fp);
void     th_clearerr(ee_FILE *fp);
int      th_fileno  (ee_FILE *fp);
int	 th_fflush  (ee_FILE *fp);
int	 th_fprintf (ee_FILE *fp, const char* format, ...);
int      th_vfprintf(ee_FILE *fp, const char *format, va_list ap);
int	 th_fseek   (ee_FILE *fp, long int offset, int whence);
long     th_ftell   (ee_FILE *fp);
int		th_fscanf(ee_FILE *stream, const char *format, ...);

/* DIRENT routines, and helpers */

char      *th_getcwd   (char *buffer, size_t size);
char      *th_getwd    (char *buffer);
int        th_chdir    (const char *filename);
/* TODO: Initial version of MITH, no support for DIRENT!
ee_DIR    *th_opendir  (const char *dirname);
struct ee_dirent *th_readdir  (ee_DIR *dirstream);
int        th_closedir (ee_DIR *dirstream);
void       th_rewinddir(ee_DIR *dirstream);
*/
int        th_stat     (const char *path, void * buf);
int        th_lstat    (const char *path, void * buf);
int        th_fstat    (int fildes, void * buf);
int        th_unlink   (const char *filename);
int th_rename(const char *oldpath, const char *newpath);


/* Basic file I/O routines */

int      th_putc    (int c, ee_FILE *fp);
int      th_getc    (ee_FILE *fp);
int      th_fputs   (const char *s, ee_FILE *fp);
int      th_ungetc  (int size, ee_FILE *fp);
char    *th_fgets   (char *string,int count,ee_FILE *fp);
size_t   th_fread   (void *buf, size_t size, size_t count, ee_FILE *fp);
size_t   th_fwrite  (const void *buf, size_t size, size_t count, ee_FILE *fp);
ee_FILE *th_fopen   (const char *filename, const char *mode);
ee_FILE *th_fdopen	(int fildes, const char *mode);
ee_FILE *th_freopen (const char *filename, const char *mode, ee_FILE *fp);
ee_FILE *th_tmpfile (void);
char	*th_mktemp  (char *templat);

int		th_fscanf(ee_FILE *stream, const char *format, ...);
int		th_vfscanf(ee_FILE *stream, const char *format, va_list ap);
int		th_sscanf(const char *str, const char *format, ...);
/* NON Standard File I/O Routines */

int         th_filecmp (const char *file1, const char *file2);
ee_FILE	   *th_fcreate (const char *filename, const char *mode, char *data, size_t size);
size_t  th_fsize   (const char *filename);
#ifndef FILENAME_MAX
#define FILENAME_MAX 256
#endif

/* Common  kernel functions */
void get_auto_data_int(char *fname,e_s32 **out,int *numread);
void get_auto_data_dbl(char *fname,e_f64 **out,int *numread);
void get_auto_data_byte(char *fname,e_u8 **out,int *numread);

#endif /* _TH_FILE_H */
