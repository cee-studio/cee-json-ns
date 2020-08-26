#ifndef CEE_JSON_AMALGAMATION
#include "json.hpp"
#include <stdlib.h>
#include "cee.hpp"
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace cee {
  namespace json {

json::data * mk_true (state::data *st) {
  static char b[CEE_SINGLETON_SIZE];
  return (data *) singleton::init (b, (uintptr_t)type_is_boolean, 1);
}

json::data * mk_false (state::data *st) {
  static char b[CEE_SINGLETON_SIZE];
  return (data *) singleton::init (b, (uintptr_t)type_is_boolean, 0);
}

json::data * mk_bool(state::data * st, bool b) {
  if (b)
    return mk_true(st);
  else
    return mk_false(st);
}

json::data * mk_undefined (state::data * st) {
  static char b[CEE_SINGLETON_SIZE];
  return (data *) singleton::init (b, (uintptr_t)type_is_undefined, 0);
}

json::data * mk_null (state::data *st) {
  static char b[CEE_SINGLETON_SIZE];
  return (data *) singleton::init (b, (uintptr_t)type_is_null, 0);
}

map::data * to_object (json::data * p) {
  if (p->t == type_is_object)
    return p->value.object;
  else
    return NULL;
}

list::data * to_array (json::data * p) {
  if (p->t == type_is_array)
    return p->value.array;
  else
    return NULL;
}

str::data * to_string (json::data * p) {
  if (p->t == type_is_string)
    return p->value.string;
  else
    return NULL;
}

boxed::data * to_number (json::data * p) {
  if (p->t  == type_is_number)
    return p->value.number;
  else
    return NULL;
}

bool to_bool (json::data * p) {
  switch(p->t) {
    case type_is_null:
    case type_is_undefined:
      return false;
    case type_is_boolean:
      {
        singleton::data * d = (singleton::data *)p;
        if (d->val)
          return true;
        else
          return false;
      }
    default:
      segfault();
      break;
  }
  segfault();
  return false;
}

json::data * mk_number (state::data * st, double d) {
  boxed::data *p = boxed::from_double (st, d);
  tagged::data * t = tagged::mk (st, type_is_number, p);
  return (data *)t;
}

json::data * mk_string(state::data *st, str::data *s) {
  tagged::data * t = tagged::mk(st, type_is_string, s);
  return (data *)t;
}

json::data * mk_array(state::data * st, int s) {
  list::data * v = list::mk(st, s);
  tagged::data * t = tagged::mk(st, type_is_array, v);
  return (data *)t;
}
    
json::data * mk_object(state::data * st) {
  map::data * m = map::mk (st, (cmp_fun)strcmp);
  tagged::data * t = tagged::mk(st, type_is_object, m);
  return (data *)t;
}

void object_set(state::data * st, json::data * j, char * key, json::data * v) {
  map::data * o = to_object(j);
  if (!o) 
    segfault();
  map::add(o, str::mk(st, "%s", key), v);
}

void object_set_bool(state::data * st, json::data * j, char * key, bool b) {
  map::data * o = to_object(j);
  if (!o) 
    segfault();
  map::add(o, str::mk(st, "%s", key), mk_bool(st, b));
}

void object_set_string (state::data * st, json::data * j, char * key, char * str) {
  map::data * o = to_object(j);
  if (!o) 
    segfault();
  map::add(o, str::mk(st, "%s", key), mk_string(st, str::mk(st, "%s", str)));
}

void object_set_number (state::data * st, json::data * j, char * key, double real) {
  map::data * o = to_object(j);
  if (!o) 
    segfault();
  map::add(o, str::mk(st, "%s", key), mk_number(st, real));
}

void array_append (json::data * j, json::data *v) {
  list::data * o = to_array(j);
  if (!o) 
    segfault();
  list::append(&o, v);
}

void array_append_bool (state::data * st, json::data * j, bool b) {
  list::data * o = to_array(j);
  if (!o) 
    segfault();
  list::append(&o, mk_bool(st, b));
}

void array_append_string (state::data * st, json::data * j, char * x) {
  list::data * o = to_array(j);
  if (!o) 
    segfault();
  list::append(&o, mk_string(st, str::mk(st, "%s", x)));
}

/*
 * this function assume the file pointer points to the begin of a file
 */
json::data * load_from_file (state::data * st, FILE * f, bool force_eof, 
                             int * error_at_line) {
  int fd = fileno(f);
  struct stat buf;
  fstat(fd, &buf);
  off_t size = buf.st_size;
  char * b = (char *)malloc(size);
  if (!b) 
    segfault();
  
  int line = 0;
  json::data * j;
  if (!parse(st, b, size, &j, true, &line)) {
    // report error
  }
  return j;
}

bool save(state::data * st, json::data * j, FILE *f, enum format how) {
  size_t s = json::snprint(st, NULL, 0, j, how);
  char * p = (char *)malloc(s+1);
  snprint(st, p, s+1, j, how);
  if (fwrite(p, s+1, 1, f) != 1) {
    fprintf(stderr, "%s", strerror(errno));
    return false;
  }
  return true;
}
    
  }
}