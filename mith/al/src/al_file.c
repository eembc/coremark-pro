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

#include "th_cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FILE_TYPE_DEFINED
#include "th_lib.h"
#include "al_file.h"

#if FAKE_FILEIO

int      al_fclose  (ee_FILE *fp) { return 0; }
int      al_ferror  (ee_FILE *fp) { return 0; }
int      al_feof    (ee_FILE *fp) { return 1; }
void     al_clearerr(ee_FILE *fp) {}
int      al_fileno  (ee_FILE *fp) { return 0; }
int	 al_fflush  (ee_FILE *fp) { return 0; }
int      al_vfprintf(ee_FILE *fp, const char *format, va_list ap) { return 0; }
int	 al_fseek   (ee_FILE *fp, long int offset, int whence) { return 0; }
long     al_ftell   (ee_FILE *fp) { return 0; }

/* DIRENT routines, and helpers */

char      *al_getcwd   (char *buffer, size_t size) { return NULL; }
char      *al_getwd    (char *buffer) { return NULL; }
int        al_chdir    (const char *filename) { return 0; }
int        al_stat     (const char *path, void * buf) { return 0; }
int        al_lstat    (const char *path, void * buf) { return 0; }
int        al_fstat    (int fildes, void * buf) { return 0; }
int 	al_rename(const char *oldpath, const char *newpath) { return 0; }


/* Basic file I/O routines */

int      al_putc    (int c, ee_FILE *fp) { return 0; }
int      al_getc    (ee_FILE *fp) { return 0; }
int      al_fputs   (const char *s, ee_FILE *fp) { return 0; }
int      al_ungetc  (int size, ee_FILE *fp) { return 0; }
char    *al_fgets   (char *string,int count,ee_FILE *fp) { return NULL; }
size_t   al_fread   (void *buf, size_t size, size_t count, ee_FILE *fp) { return 0; }
size_t   al_fwrite  (const void *buf, size_t size, size_t count, ee_FILE *fp) { return 0; }
ee_FILE *al_fopen   (const char *filename, const char *mode) { return NULL; }
ee_FILE *al_fdopen	(int fildes, const char *mode) { return NULL; }
ee_FILE *al_freopen (const char *filename, const char *mode, ee_FILE *fp) { return NULL; }
ee_FILE *al_tmpfile (void) { return NULL; }
char	*al_mktemp  (char *templat) { return NULL; }
int al_unlink   (const char *filename){return 0;}


#else /* use stdio or board specific implementation */

#if !HOST_EXAMPLE_CODE
#error "Please implement fileio or define FAKE_FILEIO if file io is not needed by the workloads being used (e.g. fp suite)"
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif 

int      al_fclose  (ee_FILE *fp) {return fclose (fp);}
int      al_ferror  (ee_FILE *fp) {return ferror (fp);}
int      al_feof    (ee_FILE *fp) {return feof   (fp);}
void     al_clearerr(ee_FILE *fp) {clearerr (fp);}
int      al_fileno  (ee_FILE *fp) {return fileno(fp);}
int      al_fflush  (ee_FILE *fp) {return fflush (fp);}
int      al_vfprintf(ee_FILE *fp, const char *format, va_list ap){return vfprintf( fp, format, ap );}
int      al_fseek  (ee_FILE *fp, long int offset, int whence) {return fseek   (fp,offset,whence);}
long     al_ftell  (ee_FILE *fp) {return ftell(fp);}
int        al_stat     (const char *path, void * buf){return stat(path,buf);}
int        al_lstat    (const char *path, void * buf){return stat(path,buf);}
int        al_fstat    (int fildes, void * buf){return fstat(fildes,buf);}
int al_rename(const char *oldpath, const char *newpath) {
  return rename(oldpath,newpath);
}
int      al_putc   (int c, ee_FILE *fp) {return putc(c,fp);}
int	     al_getc   (ee_FILE *fp) {return getc(fp);}
int      al_ungetc (int ch, ee_FILE *file){return ungetc(ch,file);}
int      al_fputs  (const char *s, ee_FILE *fp){return fputs(s,fp);}
char    *al_fgets  (char *string,int count,ee_FILE *file){return fgets(string,count,file);}
size_t      al_fread  (void *buf, size_t size, size_t count, ee_FILE *fp) {return fread   (buf,size,count,fp);}
size_t      al_fwrite (const void *buf, size_t size, size_t count, ee_FILE *fp) {return fwrite  (buf,size,count,fp);}
#ifndef EE_DEMO
ee_FILE *al_fopen  (const char *filename, const char *mode) {return fopen   (filename,mode);}
#else
ee_FILE *al_fopen  (const char *filename, const char *mode) {
	if (al_strstr(filename,"../data")!=NULL)
		filename+=3;
	return fopen   (filename,mode);
}
#endif
ee_FILE *al_fdopen (int fildes, const char *mode) {return fdopen(fildes,mode);}
ee_FILE *al_freopen(const char *filename, const char *mode, ee_FILE *fp) {return freopen(filename,mode,fp);}
ee_FILE *al_tmpfile(void) {return tmpfile();}
#if NEED_MKSTEMP
char    *al_mktemp (char *templat) {return tmpnam(templat);}
#else
char    *al_mktemp (char *templat) {
	char *tmp=strdup(templat); 
	int ret=mkstemp(tmp); 
	if (ret==-1) 
		return NULL; 
	return tmp;
}
#endif

#if ISO99
#define _unlink_file _unlink
#else
#define _unlink_file unlink
int unlink(const char *path);
#endif
int al_unlink   (const char *filename){return _unlink_file(filename);}


#endif
