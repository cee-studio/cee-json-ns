#ifndef CEE_JSON_ONE
#define CEE_JSON_ONE
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "cee.hpp"
 
#ifndef CEE_JSON_H
#define CEE_JSON_H
#ifndef CEE_JSON_AMALGAMATION
#include "cee.hpp"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#endif

#define MAX_JSON_DEPTH 500

namespace cee {
  namespace json {
    
struct null {};
struct undefined {};

enum type {
  type_is_undefined,	///< Undefined value
  type_is_null,	      ///< null value
  type_is_boolean,	  ///< boolean value
  type_is_number,	    ///< numeric value
  type_is_string,	    ///< string value
  type_is_object,	    ///< object value 
  type_is_array	      ///< array value
};

struct data {
	enum type t;
  union {
    singleton::data * null;
    singleton::data * undefined;
    singleton::data * boolean;
    boxed::data     * number;
    str::data       * string;
    vect::data      * array;
    map::data       * object;
  } value;
};

enum format {
  compact = 0,
  readable = 1
};

extern enum type type  (json::data *);
extern bool is_undefined (json::data *);
extern bool is_null (json::data *);
extern bool to_bool (json::data *);

extern json::data * find (json::data *, char *);
extern json::data * get(json::data *, char *, json::data * def);

extern bool save (json::data *, FILE *, int how);
extern json::data * load_from_file (FILE *, bool force_eof, int * error_at_line);
extern json::data * load_from_buffer (int size, char *, int line);
extern int cmp (json::data *, json::data *);

extern vect::data  * to_array (json::data *);
extern map::data   * to_object (json::data *);
extern boxed::data * to_number (json::data *);
extern str::data   * to_string (json::data *);

extern json::data * mk_true();
extern json::data * mk_false();
extern json::data * mk_undefined ();
extern json::data * mk_null ();
extern json::data * mk_object();
extern json::data * mk_number (double d);
extern json::data * mk_string(str::data * s);
extern json::data * mk_array(int s);

extern void object_set (json::data *, char *, json::data *);
extern void object_set_bool (json::data *, char *, bool);
extern void object_set_string (json::data *, char *, char *);
extern void object_set_number (json::data *, char *, double);

extern void array_append (json::data *, json::data *);
extern void array_append_bool (json::data *, bool);
extern void array_append_string (json::data *, char *);
extern void array_append_number (json::data *, double);

extern size_t snprint (char * buf, size_t size, json::data *, enum format);

extern bool parse(char * buf, uintptr_t len, json::data **out, bool force_eof,
                  int *error_at_line);

  }
}

#endif // ORCA_JSON_H 
#ifndef CEE_JSON_TOKENIZER_H
#define CEE_JSON_TOKENIZER_H
#include "cee.hpp"

namespace cee {
  namespace json {

enum token {
  tock_eof = 255,
  tock_err,
  tock_str,
  tock_number,
  tock_true,
  tock_false,
  tock_null
};

struct tokenizer {
  int line;
  char * buf;
  char * buf_end;
  str::data * str;
  double real;
};

extern enum token next_token(struct tokenizer * t);
    
  }
}
#endif // ORCA_JSON_TOK_H
 
/* convert to C */
///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CEE_JSON_UTF8_H
#define CEE_JSON_UTF8_H
#ifndef CEE_JSON_AMALGAMATION
#include <stdint.h>
#include <stdbool.h>
#endif

static const uint32_t utf_illegal = 0xFFFFFFFFu;
static bool utf_valid(uint32_t v)
{
  if(v>0x10FFFF)
    return false;
  if(0xD800 <=v && v<= 0xDFFF) // surragates
    return false;
  return true;
}

//namespace utf8 {
static bool utf8_is_trail(char ci)
{
  unsigned char c=ci;
  return (c & 0xC0)==0x80;
}


static int utf8_trail_length(unsigned char c) 
{
  if(c < 128)
    return 0;
  if(c < 194)
    return -1;
  if(c < 224)
    return 1;
  if(c < 240)
    return 2;
  if(c <=244)
    return 3;
  return -1;
}

static int utf8_width(uint32_t value)
{
  if(value <=0x7F) {
    return 1;
  }
  else if(value <=0x7FF) {
    return 2;
  }
  else if(value <=0xFFFF) {
    return 3;
  }
  else {
    return 4;
  }
}

// See RFC 3629
// Based on: http://www.w3.org/International/questions/qa-forms-utf-8
static uint32_t next(char ** p, char * e, bool html)
{
  if(*p==e)
    return utf_illegal;

  unsigned char lead = **p;
  (*p)++;

  // First byte is fully validated here
  int trail_size = utf8_trail_length(lead);

  if(trail_size < 0)
    return utf_illegal;

  //
  // Ok as only ASCII may be of size = 0
  // also optimize for ASCII text
  //
  if(trail_size == 0) {
    if(!html || (lead >= 0x20 && lead!=0x7F) || lead==0x9 || lead==0x0A || lead==0x0D)
      return lead;
    return utf_illegal;
  }

  uint32_t c = lead & ((1<<(6-trail_size))-1);

  // Read the rest
  unsigned char tmp;
  switch(trail_size) {
    case 3:
      if(*p==e)
        return utf_illegal;
      tmp = **p;
      (*p)++;
      if (!utf8_is_trail(tmp))
        return utf_illegal;
      c = (c << 6) | ( tmp & 0x3F);
    case 2:
      if(*p==e)
        return utf_illegal;
      tmp = **p;
      (*p)++;
      if (!utf8_is_trail(tmp))
        return utf_illegal;
      c = (c << 6) | ( tmp & 0x3F);
    case 1:
      if(*p==e)
        return utf_illegal;
      tmp = **p;
      (*p)++;
      if (!utf8_is_trail(tmp))
        return utf_illegal;
      c = (c << 6) | ( tmp & 0x3F);
  }

  // Check code point validity: no surrogates and
  // valid range
  if(!utf_valid(c))
    return utf_illegal;

  // make sure it is the most compact representation
  if(utf8_width(c)!=trail_size + 1)
    return utf_illegal;

  if(html && c<0xA0)
    return utf_illegal;
  return c;
} // valid


/*
bool validate_with_count(char * p, char * e, size_t *count,bool html)
{
  while(p!=e) {
    if(next(p,e,html)==utf_illegal)
      return false;
    (*count)++;
  }
  return true;
}
*/

static bool utf8_validate(char * p, char * e)
{
  while(p!=e) 
    if(next(&p, e, false)==utf_illegal)
      return false;
  return true;
}


struct utf8_seq {
  char c[4];
  unsigned len;
};

static void utf8_encode(uint32_t value, struct utf8_seq *out) {
  //struct utf8_seq out={0};
  if(value <=0x7F) {
    out->c[0]=value;
    out->len=1;
  }
  else if(value <=0x7FF) {
    out->c[0]=(value >> 6) | 0xC0;
    out->c[1]=(value & 0x3F) | 0x80;
    out->len=2;
  }
  else if(value <=0xFFFF) {
    out->c[0]=(value >> 12) | 0xE0;
    out->c[1]=((value >> 6) & 0x3F) | 0x80;
    out->c[2]=(value & 0x3F) | 0x80;
    out->len=3;
  }
  else {
    out->c[0]=(value >> 18) | 0xF0;
    out->c[1]=((value >> 12) & 0x3F) | 0x80;
    out->c[2]=((value >> 6) & 0x3F) | 0x80;
    out->c[3]=(value & 0x3F) | 0x80;
    out->len=4;
  }
}
#endif 
namespace cee {
  namespace json {
json::data * mk_true () {
  static char b[CEE_SINGLETON_SIZE];
  return (data *) singleton::init ((uintptr_t)type_is_boolean, b);
}
json::data * mk_false () {
  static char b[CEE_SINGLETON_SIZE];
  return (data *) singleton::init ((uintptr_t)type_is_boolean, b);
}
json::data * mk_bool(bool b) {
  if (b)
    return mk_true();
  else
    return mk_false();
}
json::data * mk_undefined () {
  static char b[CEE_SINGLETON_SIZE];
  return (data *) singleton::init ((uintptr_t)type_is_undefined, b);
}
json::data * mk_null () {
  static char b[CEE_SINGLETON_SIZE];
  return (data *) singleton::init ((uintptr_t)type_is_null, b);
}
map::data * to_object (json::data * p) {
  if (p->t == type_is_object) {
    return p->value.object;
  }
  return NULL;
}
vect::data * to_array (json::data * p) {
  if (p->t == type_is_array) {
    return p->value.array;
  }
  return NULL;
}
str::data * to_string (json::data * p) {
  if (p->t == type_is_string) {
    return p->value.string;
  }
  return NULL;
}
boxed::data * to_number (json::data * p) {
  if (p->t == type_is_number) {
    return p->value.number;
  }
  return NULL;
}
bool to_bool (json::data * p) {
  if (p == mk_true())
    return true;
  else if (p == mk_false())
    return false;
  segfault();
  return false;
}
json::data * mk_number (double d) {
  boxed::data *p = boxed::from_double (d);
  tagged::data * t = tagged::mk (type_is_number, p);
  return (data *)t;
}
json::data * mk_string(str::data *s) {
  tagged::data * t = tagged::mk(type_is_string, s);
  return (data *)t;
}
json::data * mk_array(int s) {
  vect::data * v = vect::mk(s);
  tagged::data * t = tagged::mk(type_is_array, v);
  return (data *)t;
}
json::data * mk_object() {
  map::data * m = map::mk ((cmp_fun)strcmp);
  tagged::data * t = tagged::mk(type_is_object, m);
  return (data *)t;
}
void object_set(json::data * j, char * key, json::data * v) {
  map::data * o = to_object(j);
  if (!o)
    segfault();
  map::add(o, str::mk("%s", key), v);
}
void object_set_bool(json::data * j, char * key, bool b) {
  map::data * o = to_object(j);
  if (!o)
    segfault();
  map::add(o, str::mk("%s", key), mk_bool(b));
}
void object_set_string (json::data * j, char * key, char * str) {
  map::data * o = to_object(j);
  if (!o)
    segfault();
  map::add(o, str::mk("%s", key), mk_string(str::mk("%s", str)));
}
void object_set_number (json::data * j, char * key, double real) {
  map::data * o = to_object(j);
  if (!o)
    segfault();
  map::add(o, str::mk("%s", key), mk_number(real));
}
void array_append (json::data * j, json::data *v) {
  vect::data * o = to_array(j);
  if (!o)
    segfault();
  vect::append(o, v);
}
void array_append_bool (json::data * j, bool b) {
  vect::data * o = to_array(j);
  if (!o)
    segfault();
  vect::append(o, mk_bool(b));
}
void array_append_string (json::data * j, char * x) {
  vect::data * o = to_array(j);
  if (!o)
    segfault();
  vect::append(o, mk_string(str::mk("%s", x)));
}
/*
 * this function assume the file pointer points to the begin of a file
 */
json::data * load_from_file (FILE * f, bool force_eof, int * error_at_line) {
  int fd = fileno(f);
  struct stat buf;
  fstat(fd, &buf);
  off_t size = buf.st_size;
  char * b = (char *)malloc(size);
  if (!b)
    segfault();
  int line = 0;
  json::data * j;
  if (!parse(b, size, &j, true, &line)) {
    // report error
  }
  return j;
}
bool save(json::data * j, FILE *f, enum format how) {
  size_t s = json::snprint(NULL, 0, j, how);
  char * p = (char *)malloc(s+1);
  snprint(p, s+1, j, how);
  if (fwrite(p, s+1, 1, f) != 1) {
    fprintf(stderr, "%s", strerror(errno));
    return false;
  }
  return true;
}
  }
}
/* JSON parser
   C reimplementation of 
     Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>'s orca_json.cpp
*/
namespace cee {
  namespace json {
enum state_type {
  st_init = 0,
  st_object_or_array_or_value_expected = 0 ,
  st_object_key_or_close_expected,
  st_object_colon_expected,
  st_object_value_expected,
  st_object_close_or_comma_expected,
  st_array_value_or_close_expected,
  st_array_close_or_comma_expected,
  st_error,
  st_done
} state_type;
static const uintptr_t json_max_depth = 512;
bool parse(char * buf, uintptr_t len, json::data **out, bool force_eof,
           int *error_at_line)
{
  struct tokenizer tock = {0};
  tock.buf = buf;
  tock.buf_end = buf + len;
  *out = NULL;
  enum state_type state = st_init;
  str::data * key = NULL;
  stack::data * sp = stack::mk_e (dp_noop, json_max_depth);
  tuple::data * top = NULL;
  tuple::data * result = NULL;
  static enum del_policy del_noops[2] = { dp_noop, dp_noop };
  stack::push(sp, tuple::mk_e(del_noops, (void *)st_done, NULL));
  while(!stack::empty(sp) && !stack::full(sp) &&
        state != st_error && state != st_done) {
    if (result) {
      del(result);
      result = NULL;
    }
    int c = next_token(&tock);
    top = (tuple::data *)stack::top(sp, 0);
    switch(state) {
    case st_object_or_array_or_value_expected:
      if(c=='[') {
        top->_[1]= mk_array(10);
        state=st_array_value_or_close_expected;
      }
      else if(c=='{') {
        top->_[1]= mk_object();
        state=st_object_key_or_close_expected;
      }
      else if(c==tock_str) {
        top->_[1]= mk_string(tock.str);
        tock.str = NULL;
        state=(static_cast<enum state_type>(reinterpret_cast<intptr_t>(top->_[0])));
        { result = (struct tuple::data *)stack::pop(sp); };
      }
      else if(c==tock_true) {
        top->_[1]= mk_true();
        state=(static_cast<enum state_type>(reinterpret_cast<intptr_t>(top->_[0])));
        { result = (struct tuple::data *)stack::pop(sp); };
      }
      else if(c==tock_false) {
        top->_[1] = mk_false();
        state=(static_cast<enum state_type>(reinterpret_cast<intptr_t>(top->_[0])));
        { result = (struct tuple::data *)stack::pop(sp); };
      }
      else if(c==tock_null) {
        top->_[1] = mk_null();
        state=(static_cast<enum state_type>(reinterpret_cast<intptr_t>(top->_[0])));
        { result = (struct tuple::data *)stack::pop(sp); };
      }
      else if(c==tock_number) {
        top->_[1] = mk_number (tock.real);
        state=(static_cast<enum state_type>(reinterpret_cast<intptr_t>(top->_[0])));
        { result = (struct tuple::data *)stack::pop(sp); };
      }
      else
        state = st_error;
      break;
    case st_object_key_or_close_expected:
      if(c=='}') {
        state=(static_cast<enum state_type>(reinterpret_cast<intptr_t>(top->_[0])));
        { result = (struct tuple::data *)stack::pop(sp); };
      }
      else if (c==tock_str) {
        key = tock.str;
        tock.str = NULL;
        state = st_object_colon_expected;
      }
      else
        state = st_error;
      break;
    case st_object_colon_expected:
      if(c!=':')
        state=st_error;
      else
        state=st_object_value_expected;
      break;
    case st_object_value_expected:
      {
        map::data * obj = json::to_object((json::data *)top->_[1]);
        if(c==tock_str) {
          map::add(obj, key, mk_string(tock.str));
          tock.str = NULL;
          state=st_object_close_or_comma_expected;
        }
        else if(c==tock_true) {
          map::add(obj, key, mk_true());
          state=st_object_close_or_comma_expected;
        }
        else if(c==tock_false) {
          map::add(obj, key, mk_false());
          state=st_object_close_or_comma_expected;
        }
        else if(c==tock_null) {
          map::add(obj, key, mk_null());
          state=st_object_close_or_comma_expected;
        }
        else if(c==tock_number) {
          map::add(obj, key, mk_number(tock.real));
          state=st_object_close_or_comma_expected;
        }
        else if(c=='[') {
          json::data * a = mk_array(10);
          map::add(obj, key, a);
          state=st_array_value_or_close_expected;
          stack::push(sp, tuple::mk_e(del_noops, (void *)st_object_close_or_comma_expected, a));
        }
        else if(c=='{') {
          json::data * o = mk_object();
          map::add(obj, key, o);
          state=st_object_key_or_close_expected;
          stack::push(sp, tuple::mk_e(del_noops, (void *)st_object_close_or_comma_expected, o));
        }
        else
          state=st_error;
      }
      break;
    case st_object_close_or_comma_expected:
      if(c==',')
        state=st_object_key_or_close_expected;
      else if(c=='}') {
        state=(static_cast<enum state_type>(reinterpret_cast<intptr_t>(top->_[0])));
        { result = (struct tuple::data *)stack::pop(sp); };
      }
      else
        state=st_error;
      break;
    case st_array_value_or_close_expected:
      {
        if(c==']') {
          state=(static_cast<enum state_type>(reinterpret_cast<intptr_t>(top->_[0])));
          { result = (struct tuple::data *)stack::pop(sp); };
          break;
        }
        struct vect::data * ar = json::to_array((json::data *)top->_[1]);
        if(c==tock_str) {
          vect::append(ar, mk_string(tock.str));
          state=st_array_close_or_comma_expected;
        }
        else if(c==tock_true) {
          vect::append(ar, mk_true());
          state=st_array_close_or_comma_expected;
        }
        else if(c==tock_false) {
          vect::append(ar, mk_false());
          state=st_array_close_or_comma_expected;
        }
        else if(c==tock_null) {
          vect::append(ar, mk_null());
          state=st_array_close_or_comma_expected;
        }
        else if(c==tock_number) {
          vect::append(ar, mk_number(tock.real));
          state=st_array_close_or_comma_expected;
        }
        else if(c=='[') {
          json::data * a = mk_array(10);
          state=st_array_value_or_close_expected;
          stack::push(sp, tuple::mk_e(del_noops, (void *)st_array_close_or_comma_expected, a));
        }
        else if(c=='{') {
          json::data * o = mk_object();
          state=st_object_key_or_close_expected;
          stack::push(sp, tuple::mk_e(del_noops, (void *)st_array_close_or_comma_expected, o));
        }
        else
          state=st_error;
        break;
      }
    case st_array_close_or_comma_expected:
      if(c==']') {
        state=(static_cast<enum state_type>(reinterpret_cast<intptr_t>(top->_[0])));
        { result = (struct tuple::data *)stack::pop(sp); };
      }
      else if(c==',')
        state=st_array_value_or_close_expected;
      else
        state=st_error;
      break;
    case st_done:
    case st_error:
      break;
    };
  }
  del(sp);
  if(state==st_done) {
    if(force_eof) {
      if(next_token(&tock)!=tock_eof) {
        *error_at_line=tock.line;
        return false;
      }
    }
    *out = (json::data *)(result->_[1]);
    del(result);
    return true;
  }
  *error_at_line=tock.line;
  return false;
}
  }
}
/* JSON snprint
   C reimplementation of
     Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>'s orca_json.cpp
*/
namespace cee {
  namespace json {
struct counter {
  uintptr_t next;
  vect::data * array;
  map::data * object;
  char tabs;
  char more_siblings;
};
static struct counter * push(uintptr_t tabs, bool more_siblings,
                             stack::data * sp, json::data * j) {
  struct counter * p = NULL;
  if (j == NULL) {
    p = (struct counter *)block::mk(sizeof(struct counter));
    p->tabs = 0;
  }
  else {
    switch(j->t) {
      case type_is_object:
        {
          p = (struct counter *) block::mk(sizeof(struct counter));
          map::data * mp = to_object(j);
          p->array = map::keys(mp);
          p->object = to_object(j);
          p->tabs = tabs;
          p->next = 0;
          p->more_siblings = 0;
        }
        break;
      case type_is_array:
        {
          p = (struct counter *)block::mk(sizeof(struct counter));
          p->array = to_array(j);
          p->tabs = tabs;
          p->next = 0;
          p->more_siblings = 0;
        }
        break;
      default:
        {
          p = (struct counter *)block::mk(sizeof(struct counter));
          p->array = NULL;
          p->tabs = tabs;
          p->next = 0;
          p->more_siblings = 0;
        }
        break;
    }
    p->more_siblings = more_siblings;
  }
  enum del_policy o[2] = { dp_del, dp_noop };
  stack::push(sp, tuple::mk_e(o, p, j));
  return p;
}
static void pad (uintptr_t * offp, char * buf, struct counter * cnt, enum format f)
{
  if (!f) return;
  uintptr_t offset = *offp;
  if (buf) {
    int i;
    for (i = 0; i < cnt->tabs; i++)
      buf[offset + i] = '\t';
  }
  offset += cnt->tabs;
  *offp = offset;
  return;
}
static void delimiter (uintptr_t * offp, char * buf, enum format f,
                       struct counter * cnt, char c)
{
  uintptr_t offset = *offp;
 if (!f) {
    if (buf) buf[offset] = c;
    offset ++; // only count one
    *offp = offset;
    return;
  }
  switch (c) {
    case '[':
    case '{':
      pad(offp, buf, cnt, f);
      if (buf) {
       buf[offset] = c;
        buf[offset+1] = '\n';
      }
      offset +=2;
      break;
    case ']':
    case '}':
      if (buf) buf[offset] = '\n';
      offset ++;
      pad(&offset, buf, cnt, f);
      if (buf) buf[offset] = c;
      offset ++;
      if (buf) buf[offset] = '\n';
      offset ++;
      break;
    case ':':
      if (buf) {
        buf[offset] = ' ';
        buf[offset+1] = ':';
        buf[offset+2] = '\t';
      }
      offset +=3;
      break;
    case ',':
      if (buf) {
        buf[offset] = ',';
        buf[offset+1] = '\n';
      }
      offset +=2;
      break;
  }
  *offp = offset;
}
static void str_append(char * out, uintptr_t *offp, char *begin, unsigned len) {
  uintptr_t offset = *offp;
  if (out) out[offset] = '"';
  offset ++;
  char *i,*last;
  char buf[8] = "\\u00";
  for(i=begin,last = begin;i < begin + len;) {
    char *addon = 0;
    unsigned char c=*i;
    switch(c) {
    case 0x22: addon = "\\\""; break;
    case 0x5C: addon = "\\\\"; break;
    case '\b': addon = "\\b"; break;
    case '\f': addon = "\\f"; break;
    case '\n': addon = "\\n"; break;
    case '\r': addon = "\\r"; break;
    case '\t': addon = "\\t"; break;
    default:
      if(c<=0x1F) {
        static char const tohex[]="0123456789abcdef";
        buf[4]=tohex[c >> 4];
        buf[5]=tohex[c & 0xF];
        buf[6]=0;
        addon = buf;
      }
    };
    if(addon) {
      //a.append(last,i-last);
      if (out) memcpy(out+offset, last, i-last);
      offset += i-last;
      if (out) memcpy(out+offset, addon, strlen(addon));
      offset += strlen(addon);
      i++;
      last = i;
    }
    else {
      i++;
    }
  }
  if (out) memcpy(out+offset, last, i-last);
  offset += i-last;
  if (out) out[offset] = '"';
  offset++;
  *offp = offset;
}
/*
 * compute how many bytes are needed to serialize orca_json as a string
 */
size_t snprint (char * buf, size_t size, json::data * j, enum format f) {
  tuple::data * cur;
  json::data * cur_orca_json;
  struct counter * ccnt;
  uintptr_t incr = 0;
  stack::data * sp = stack::mk_e(dp_noop, 500);
  push (0, false, sp, j);
  uintptr_t offset = 0;
  while (!stack::empty(sp) && !stack::full(sp)) {
    cur = (tuple::data *) stack::top(sp, 0);
    cur_orca_json = (json::data *)(cur->_[1]);
    ccnt = (struct counter *)(cur->_[0]);
    switch(cur_orca_json->t) {
      case type_is_null:
        {
          pad(&offset, buf, ccnt, f);
          if (buf)
            memcpy(buf + offset, "null", 4);
          offset += 4;
          if (ccnt->more_siblings)
            delimiter(&offset, buf, f, ccnt, ',');
          del(stack::pop(sp));
        }
        break;
      case type_is_boolean:
        {
          pad(&offset, buf, ccnt, f);
          char * s = "false";
          if (to_bool(cur_orca_json))
            s = "true";
          if (buf)
            memcpy(buf + offset, s, strlen(s));
          offset += strlen(s);
          if (ccnt->more_siblings)
            delimiter(&offset, buf, f, ccnt, ',');
          del(stack::pop(sp));
        }
        break;
      case type_is_undefined:
        {
          pad(&offset, buf, ccnt, f);
          if (buf)
            memcpy(buf + offset, "undefined", 9);
          offset += 9;
          if (ccnt->more_siblings)
            delimiter(&offset, buf, f, ccnt, ',');
          del(stack::pop(sp));
        }
        break;
      case type_is_string:
        {
          char * str = (char *)to_string(cur_orca_json);
          pad(&offset, buf, ccnt, f);
          str_append(buf, &offset, str, strlen(str));
          if (ccnt->more_siblings)
            delimiter(&offset, buf, f, ccnt, ',');
          del(stack::pop(sp));
        }
        break;
      case type_is_number:
        {
          pad(&offset, buf, ccnt, f);
          incr = boxed::snprint(NULL, 0, to_number(cur_orca_json));
          if (buf) {
            boxed::snprint(buf+offset, incr, to_number(cur_orca_json));
          }
          offset+=incr;
          if (ccnt->more_siblings)
            delimiter(&offset, buf, f, ccnt, ',');
          del(stack::pop(sp));
        }
        break;
      case type_is_array:
        {
          uintptr_t i = ccnt->next;
          if (i == 0)
            delimiter(&offset, buf, f, ccnt, '[');
          uintptr_t n = vect::size(ccnt->array);
          if (i < n) {
            bool more_siblings = false;
            if (1 < n && i+1 < n)
              more_siblings = true;
            ccnt->next++;
            push (ccnt->tabs + 1, more_siblings, sp, (json::data *)(ccnt->array->_[i]));
          }
          else {
            delimiter(&offset, buf, f, ccnt, ']');
            if (ccnt->more_siblings)
            delimiter(&offset, buf, f, ccnt, ',');
            del(stack::pop(sp));
          }
        }
        break;
      case type_is_object:
        {
          uintptr_t i = ccnt->next;
          if (i == 0)
            delimiter(&offset, buf, f, ccnt, '{');
          uintptr_t n = vect::size(ccnt->array);
          if (i < n) {
            bool more_siblings = false;
            if (1 < n && i+1 < n)
              more_siblings = true;
            ccnt->next++;
            char * key = (char *)ccnt->array->_[i];
            json::data * j1 = (json::data *)map::find(ccnt->object, ccnt->array->_[i]);
            unsigned klen = strlen(key);
            pad(&offset, buf, ccnt, f);
            str_append(buf, &offset, key, klen);
            delimiter(&offset, buf, f, ccnt, ':');
            push (ccnt->tabs + 1, more_siblings, sp, j1);
          }
          else {
            delimiter(&offset, buf, f, ccnt, '}');
            if (ccnt->more_siblings)
            delimiter(&offset, buf, f, ccnt, ',');
            del(ccnt->array);
            del(stack::pop(sp));
          }
        }
        break;
    }
  }
  del (sp);
  if (buf)
    buf[offset] = '\0';
  return offset;
}
  }
}
namespace cee {
  namespace json {
static bool check(char * buf, char * s, char **ret)
{
  char * next = buf;
  for (next = buf; *s && *next == *s; next++, s++);
  if (*s==0) {
    *ret = next;
    return true;
  }
  else {
    *ret = buf;
   return false;
  }
  return false;
}
static bool read_4_digits(struct tokenizer * t, uint16_t *x)
{
  char *buf;
  if (t->buf_end - t->buf >= 5) {
    buf = t->buf;
  }
  else
    return false;
  int i;
  for(i=0; i<4; i++) {
    char c=buf[i];
    if( ('0'<= c && c<='9') || ('A'<= c && c<='F') || ('a'<= c && c<='f') ) {
      continue;
    }
    return false;
  }
  unsigned v;
  sscanf(buf,"%x",&v);
  *x=v;
  return true;
}
static bool parse_string(struct tokenizer * t) {
  char c;
  // we should use a more efficient stretchy buffer here
  t->str = str::mk_e(128, "");
  if (t->buf == t->buf_end)
    return false;
  c=t->buf[0];
  t->buf++;
  if (c != '"') return false;
  bool second_surragate_expected=false;
  uint16_t first_surragate = 0;
  for(;;) {
    if(t->buf == t->buf_end)
      return false;
    c = t->buf[0];
    t->buf ++;
    if(second_surragate_expected && c!='\\')
      return false;
    if(0<= c && c <= 0x1F)
      return false;
    if(c=='"')
      break;
    if(c=='\\') {
      if(t->buf == t->buf_end)
        return false;
      if(second_surragate_expected && c!='u')
        return false;
      switch(c) {
      case '"':
      case '\\':
      case '/':
        t->str = str::add(t->str, c);
        break;
      case 'b': t->str = str::add(t->str, '\b'); break;
      case 'f': t->str = str::add(t->str, '\f'); break;
      case 'n': t->str = str::add(t->str, '\n'); break;
      case 'r': t->str = str::add(t->str, '\r'); break;
      case 't': t->str = str::add(t->str, '\t'); break;
      case 'u':
        {
          // don't support utf16
          uint16_t x;
          if (!read_4_digits(t, &x))
            return false;
         struct utf8_seq s = { 0 };
          utf8_encode(x, &s);
          t->str = str::ncat(t->str, s.c, s.len);
        }
        break;
      default:
        return false;
      }
    }
    else {
      t->str = str::add(t->str, c);
    }
  }
  if(!utf8_validate(t->str->_, str::end(t->str)))
    return false;
  return true;
}
static bool parse_number(struct tokenizer *t) {
  int x = sscanf(t->buf, "%lf", &t->real);
  return x == 1;
}
enum token next_token(struct tokenizer * t) {
  for (;;t->buf++) {
    if (t->buf == t->buf_end)
      return tock_eof;
    char c = t->buf[0];
    t->buf ++;
    switch (c) {
      case '[':
      case '{':
      case ':':
      case ',':
      case '}':
      case ']':
        return (enum token)c;
      case ' ':
      case '\t':
      case '\r':
        break;
      case '\n':
        t->line++;
        break;
      case '"':
        t->buf --;
        if(parse_string(t))
          return tock_str;
        return tock_err;
      case 't':
        if(check(t->buf, "rue", &t->buf))
          return tock_true;
        return tock_err;
      case 'n':
        if(check(t->buf, "ull", &t->buf))
          return tock_null;
        return tock_err;
      case 'f':
        if(check(t->buf, "alse", &t->buf))
          return tock_false;
        return tock_err;
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        t->buf --;
        if(parse_number(t))
          return tock_number;
        return tock_err;
      case '/':
        if(check(t->buf + 1, "/", &t->buf)) {
          for (;t->buf < t->buf_end && (c = t->buf[0]) && c != '\n'; t->buf++);
          if(c=='\n')
            break;
          return tock_eof;
        }
        return tock_err;
      default:
        return tock_err;
    }
  }
}
  }
}
#endif
