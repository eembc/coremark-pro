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

#ifndef AL_FILE_H
#define AL_FILE_H

int      al_fclose  (ee_FILE *fp);
int      al_ferror  (ee_FILE *fp);
int      al_feof    (ee_FILE *fp);
void     al_clearerr(ee_FILE *fp);
int      al_fileno  (ee_FILE *fp);
int	 al_fflush  (ee_FILE *fp);
int	 al_fprintf (ee_FILE *fp, const char* format, ...);
int      al_vfprintf(ee_FILE *fp, const char *format, va_list ap);
int	 al_fseek   (ee_FILE *fp, long int offset, int whence);
long     al_ftell   (ee_FILE *fp);

/* DIRENT routines, and helpers */

int        al_stat     (const char *path, void * buf);
int        al_lstat    (const char *path, void * buf);
int        al_fstat    (int fildes, void * buf);
int        al_unlink   (const char *filename);
int al_rename(const char *oldpath, const char *newpath);

/* Basic file I/O routines */

int      al_putc    (int c, ee_FILE *fp);
int      al_getc    (ee_FILE *fp);
int      al_fputs   (const char *s, ee_FILE *fp);
int      al_ungetc  (int size, ee_FILE *fp);
char    *al_fgets   (char *string,int count,ee_FILE *fp);
size_t   al_fread   (void *buf, size_t size, size_t count, ee_FILE *fp);
size_t   al_fwrite  (const void *buf, size_t size, size_t count, ee_FILE *fp);
ee_FILE *al_fopen   (const char *filename, const char *mode);
ee_FILE *al_fdopen	(int fildes, const char *mode);
ee_FILE *al_freopen (const char *filename, const char *mode, ee_FILE *fp);
ee_FILE *al_tmpfile (void);
char	*al_mktemp  (char *templat);

#endif /* AL_FILE_H */
