/*
	Software License Agreement (BSD License)
	
	Copyright (c) 1997-2008, David Lindauer, (LADSoft).
	All rights reserved.
	
	Redistribution and use of this software in source and binary forms, with or without modification, are
	permitted provided that the following conditions are met:
	
	* Redistributions of source code must retain the above
	  copyright notice, this list of conditions and the
	  following disclaimer.
	
	* Redistributions in binary form must reproduce the above
	  copyright notice, this list of conditions and the
	  following disclaimer in the documentation and/or other
	  materials provided with the distribution.
	
	* Neither the name of LADSoft nor the names of its
	  contributors may be used to endorse or promote products
	  derived from this software without specific prior
	  written permission of LADSoft.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
	PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
	TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include "libp.h"
#include <limits.h>
#include <float.h>

#ifdef _i386_
#define USE_FLOAT
#endif

#define STP 1

static char *__fil2strd (FILE *restrict fil, int width, int *restrict ch, int *restrict chars)
{
  char buf[256], *p = buf;
  int radix = 10;

  if (*ch != EOF && width && (*ch == '+' || *ch == '-')) {
    *p++ = *ch;
    *ch = fgetc(fil);
    (*chars)++;
    width--;
  }
  if (*ch != EOF && width &&
      (*ch == 'i' || *ch == 'I' || *ch == 'n' || *ch == 'N')) {
    if (*ch == 'i' || *ch == 'I') {
      int i;
      for (i = 0; i < 3 && *ch != EOF && width; i++) {
        if ("inf"[i] != (*ch | 0x20))
          return NULL;
        *p++ = *ch;
        *ch = fgetc(fil);
        (*chars)++;
        width--;
      }
      for (i = 0; i < 5 && *ch != EOF && width; i++) {
        if ("inity"[i] == (*ch | 0x20)) {
          *p++ = *ch;
          *ch = fgetc(fil);
          (*chars)++;
          width--;
        }
        else {
          fil->level += i + 1;
          *(--fil->curp) = *ch;
          *chars -= i;
          width += i;
          while (i--)
            *(--fil->curp) = *(--p);
          *ch = fgetc(fil);
          break;
        }
      }
    }
    else if (*ch == 'n' || *ch == 'N') {
      int i;
      for (i = 0; i < 3 && *ch != EOF && width; i++) {
        if ("nan"[i] != (*ch | 0x20))
          return NULL;
        *p++ = *ch;
        *ch = fgetc(fil);
        (*chars)++;
        width--;
      }
      if (*ch == '(') {
        for (i = 0; i < 7 && *ch != EOF && width && *ch != ')'; i++) {
          *p++ = *ch;
          *ch = fgetc(fil);
          (*chars)++;
          width--;
        }
        if (*ch == ')') {
          *p++ = *ch;
          *ch = fgetc(fil);
          (*chars)++;
          width--;
        }
        else {
          fil->level += i + 1;
          *(--fil->curp) = *ch;
          *chars -= i;
          width += i;
          while (i--)
            *(--fil->curp) = *(--p);
          *ch = fgetc(fil);
        }
      }
    }
    *p = 0;
    return buf;
  }
  if (*ch != EOF && width && *ch == '0') {
    *p++ = *ch;
    *ch = fgetc(fil);
    (*chars)++;
    width--;
    if (*ch != EOF && width && (*ch == 'x' || *ch == 'X')) {
      *p++ = *ch;
      *ch = fgetc(fil);
      (*chars)++;
      width--;
      radix = 2;
    }
  }
  if (radix == 10)
    while (*ch != EOF && width && isdigit(*ch)) {
      *p++ = *ch;
      *ch = fgetc(fil);
      (*chars)++;
      width--;
    }
  else
    while (*ch != EOF && width && isxdigit(*ch)) {
      *p++ = *ch;
      *ch = fgetc(fil);
      (*chars)++;
      width--;
    }
  if (*ch != EOF && width && *ch == '.') {
    *p++ = *ch;
    *ch = fgetc (fil);
    (*chars)++;
    width--;
  }
  if (radix == 10)
    while (*ch != EOF && width && isdigit(*ch)) {
      *p++ = *ch;
      *ch = fgetc(fil);
      (*chars)++;
      width--;
    }
  else
    while (*ch != EOF && width && isxdigit(*ch)) {
      *p++ = *ch;
      *ch = fgetc(fil);
      (*chars)++;
      width--;
    }
  if (*ch != EOF && width && 
      (*ch == 'e' || *ch == 'E' || *ch == 'p' || *ch == 'P')) {
    *p++ = *ch;
    *ch = fgetc(fil);
    (*chars)++;
    width--;
    if (*ch != EOF && width && (*ch == '+' || *ch == '-')) {
      *p++ = *ch;
      *ch = fgetc(fil);
      (*chars)++;
      width--;
    }
    if (*ch != EOF && width && isdigit(*ch)) {
      *p++ = *ch;
      *ch = fgetc(fil);
      (*chars)++;
      width--;
    }
    else return NULL;
    while (*ch != EOF && width && isdigit(*ch)) {
      *p++ = *ch;
      *ch = fgetc(fil);
      (*chars)++;
      width--;
    }
  }
  *p = 0;
  return buf;
}

char *__strtoone(FILE *restrict fil, const char *restrict format, void *restrict arg, int *restrict count,int *restrict chars,int *restrict argn, int *restrict ch)
{
    LLONG_TYPE max=INT_MAX,min= INT_MIN;
	int ignore = 0;
	int width = INT_MAX;
	int size = 0;
	int sgn = 0;
   int skip=0,didit = 0;
   int type ;
	unsigned cu;
   char *s;
   int i;
   int radix = 0 ;
   int lc = 0x20 ;
	long double fval;
   LLONG_TYPE c ;
   wchar_t fbuf[40];
   char _sctab[256] ;
   mbstate_t st;
	int found = 0;
	
	if (*format == '*') {
		ignore = 1;
		format++;
	}
	if (isdigit(*format))  {
      width = 0;
      while (isdigit(*format))
			width = width *10 + *format++ -'0';
    }
   if (*format == 'h' || *format == 'l') {
		size = *format++;
      if (size == 'h') {
        max = SHRT_MAX;
        min = SHRT_MIN;
        if (*format == 'h') {
            max = CHAR_MAX;
            min = CHAR_MIN;
            size = 'c';
            format++;
        }
      } else if (*format == 'l') {
         max = LLONG_MAX;
         min = LLONG_MIN;
         size = '1' ;
         format++ ;
      }
   }
   else if (*format == 'I' && *(format + 1) == '6' && *(format + 2) == '4') {
     max = LLONG_MAX;
     min = LLONG_MIN;
     size = '1';
     format += 3;
   }
   else if (*format == 'L')
    size = *format++;
   else if (*format == 'z' || *format == 't')
    format++;
   else if (*format == 'j') {
    size = '1';
    format++;
   }

   type = *format++ ;
   if (isalpha(type) && !(type & 0x20)) {
     lc = 0;
     type |= 0x20;
   }
   switch((type)) {
		case 'c':
            if (size == 'l') {
                memset(&st,0,sizeof(st));
                while (width >= 0 && *ch) {
                    int l ;
                    l = mbrtowc(fbuf,ch,1,&st);
                    width--;
                    if (l == (size_t) -1) {
                        break;
                    } else if (l > 0 && !ignore) {
                        *((wchar_t *)arg)++ = fbuf[0];
                    }
                    (*chars)++ ;
                    *ch = fgetc(fil);
                }
            }
			else {
              if (width == INT_MAX) 
                width = 1;
              while (width-- && *ch != EOF) {
				if (!ignore) {
                   *((char *)arg)++ = (char)*ch;
				}		
				*ch = fgetc(fil) ;
				(*chars)++;
              }
			}
         if (!ignore) {
            (*count)++;
            (*argn)++;
         }     
         break ;
        case 'x':
        case 'p':
        case 'o':
		case 'd':
        case 'u':
        case 'i':
        case 'b':
         while (*ch != EOF && isspace(*ch)) {
            *ch = fgetc(fil) ;
            (*chars)++ ;
         }
		 switch(type)
		 {
		 	case 'x':
			case 'p':
				if (!isxdigit(*ch) && *ch != '-' && *ch != '+')
					return NULL;
				break;
			case 'o':
				if ((!isdigit(*ch) || (*ch) > '7') && *ch != '-' && *ch != '+')
					return NULL;
				break;
			case 'b':
				if ((!isdigit(*ch) || (*ch) > '1') && *ch != '-' && *ch != '+')
					return NULL;
				break;
			default:
				if (!isdigit(*ch) && *ch != '-' && *ch != '+')
					return NULL;
				break;
		 }
         switch(type) {
            case 'd':
                 c = __xstrtoimax(fil,width,ch,chars,10,max,0, STRT_ERR_RET_SIGNED);
                 break;
            case 'i':
                 c = __xstrtoimax(fil,width,ch,chars,0,max,0, STRT_ERR_RET_SIGNED);
                 break;
            case 'u':
                 c = __xstrtoimax(fil,width,ch,chars,10,max-min,0, STRT_ERR_RET_UNSIGNED);
                 break;
            case 'x':
            case 'p':
                 c = __xstrtoimax(fil,width,ch,chars,16,max-min,0, STRT_ERR_RET_UNSIGNED);
                 break;
            case 'o':
                 c = __xstrtoimax(fil,width,ch,chars,8,max-min,0, STRT_ERR_RET_UNSIGNED);
                 break;
            case 'b':
                 c = __xstrtoimax(fil,width,ch,chars,2,max-min,0, STRT_ERR_RET_UNSIGNED);
                 break;
         }
            if (!ignore) {
                (*count)++;
                (*argn)++;
                if (size == '1' || size == 'L')
                   *(LLONG_TYPE *)arg = c ;
                else if (size == 'l')
                   *(long *)arg = c ;
                else if (size == 'h') 
    					*(short *)arg = (short)c;
                    else if (size == 'c')
                        *(char *)arg = (char)c;
    				else
                   *(int *)arg = (int)c;
    	    }
         break ;
		case 'e':
		case 'f':
		case 'g': 
        case 'a':
#ifndef USE_FLOAT
				fprintf(stderr,"FP not linked");
#else
				struct __file2 fil2;
                FILE fil1;
                char buf[256], *p;
                int ch1, chars1;
                memset(&fil1, 0, sizeof (fil1));
                fil1.flags = _F_IN | _F_READ | _F_BUFFEREDSTRING;
                fil1.buffer = fil1.curp = buf;
                fil1.token = FILTOK;
				fil1.extended = &fil2;
		         while (*ch != EOF && isspace(*ch)) {
		            *ch = fgetc(fil) ;
		            (*chars)++ ;
		         }
			 if (!isdigit(*ch) && *ch != '-' && *ch != '+' && *ch != '.' &&
			     *ch != 'i' && *ch != 'I' && *ch != 'n' && *ch != 'N')
				 return NULL;
                if (!(p = __fil2strd(fil, width, ch, chars)))
                  return NULL;
                strcpy (buf, p);
                fil1.level = strlen(buf) + 1;
                fil1.bsize = strlen(buf);
                ch1 = fgetc(&fil1);
                fval = __xstrtod(&fil1,256,&ch1,&chars1, LDBL_MAX, LDBL_MAX_EXP, LDBL_MAX_10_EXP,1);
				if (!ignore) {
					(*count)++;
					(*argn)++;
               if (size == 'L') 
                  *(long double *)arg = fval;
               else if (size == 'l') 
                  *(double *)arg = fval;
               else 
                    *(float *)arg = fval;
				}
#endif
			break;
		case 'n':
         if (!ignore) {
		   (*argn)++;
             if (size == '1') {
                *(LLONG_TYPE *)arg = (*chars) ;
             } else if (size == 'l' || size == 'L')
                *(long *)arg = (*chars) ;
             else if (size == 'h')
                *(short *)arg = (*chars) ;
             else if (size == 'c')
                *(char *)arg = (*chars) ;
             else
                *(int *)arg = (*chars);
         }
	     break;
      case '[': {
           int t = 0,c ;
           if (*format == '^') {
              t++ ;
              format++ ;
           }
           for (i=0; i < sizeof(_sctab); i++) {
              _sctab[i] =  t ? 0 : STP;
           }
           while (*format == ']' || *format == '-')
              _sctab[*format++] ^= STP ;
           while ((c = *format++) != ']' && c)  {
              if (c == '-' && *format != ']') {
                 for (i= *(format-2) ; i <= (*format); i++)
                    if (t)
                      _sctab[i] |= STP ;
                    else
                      _sctab[i] &= ~STP ;
                 format++ ;
              } else
                 if (t)
                   _sctab[(unsigned char)c] |= STP ;
                 else
                   _sctab[(unsigned char)c] &= ~STP ;

           }
           if ( c== 0)
             format-- ;
         }
         /* fall through */
		case 's':
         if (type == 's') {
            for (i=0; i < sizeof(_sctab); i++) {
                _sctab[i] = isspace(i) ? STP : 0;
            }
         } 
         while (type == 's' && *ch != EOF && (_sctab[*ch] & STP)) {
            *ch = fgetc(fil) ;
            (*chars)++ ;
//			found=1;
         }
			s = (char *)arg;
         skip = width == INT_MAX ? 0 : 1;
         memset(&st,0,sizeof(st));
         while (*ch != EOF && !(_sctab[*ch]& STP) && (width || !skip)) {
            if (size == 'l') {
                    int l ;
                    l = mbrtowc(fbuf,ch,1,&st);
                    if (l == (size_t) -1) {
                        break;
                    } else if (l > 0 && !ignore) {
                        *((wchar_t *)arg)++ = fbuf[0];
                    }
                    width--;
                    (*chars)++;
					found=1;
                    *ch = fgetc(fil);
            } else {
               if (!ignore)
                 *s++ = *ch;
               *ch = fgetc(fil) ;
               width--;
               (*chars)++;
				found=1;
            }
         }

         if (!ignore) {
            if (size == 'l')
                *(wchar_t *)arg = L'\0';
            else
                *s = 0;
			if (found)
	           (*count) ++;
            (*argn)++;
         }
         break;
		case '%':
            if (*ch != '%')
					return 0;
            *ch = fgetc(fil) ;
            (*chars)++;
				break;
		default:
			format++;
	}
	return format;
}

int __scanf(FILE *restrict fil, const char *restrict format,void *restrict arglist)
{
   int i = 0, j = 0, k = 0;
   int ch = fgetc(fil) ;
   if (ch == EOF) return EOF;
   while (format && *format /* && ch != EOF */) {
      while (format && *format != '%' && *format) {
        if (isspace(*format)) {
            while (ch != EOF && isspace(ch)) {
               ch = fgetc(fil) ;
               j++ ;
            }
            while (*format && isspace(*format)) format++;
		}
		else  {
            if (*format++ != ch) {
                goto __scanf_end;
//                if (ch != EOF)
//                    ungetc(ch,fil);
//                return i;
            }
            ch = fgetc(fil) ;
            j++;
		}
	  }
      if (*format /* && ch != EOF */) {
	      format++;
          format = __strtoone(fil,format,((char **)arglist)[k],&i,&j,&k,&ch);
	  }
	}
__scanf_end:
    if (ch != EOF)
        ungetc(ch,fil);
    else if (!i) return EOF;
    return(i);
}
