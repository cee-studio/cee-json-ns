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
    list::data      * array;
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

extern list::data  * to_array (json::data *);
extern map::data   * to_object (json::data *);
extern boxed::data * to_number (json::data *);
extern str::data   * to_string (json::data *);

extern json::data * mk_true(state::data *);
extern json::data * mk_false(state::data *);
extern json::data * mk_undefined (state::data *);
extern json::data * mk_null (state::data *);
extern json::data * mk_object(state::data *);
extern json::data * mk_number (state::data *, double d);
extern json::data * mk_string(state::data *, str::data * s);
extern json::data * mk_array(state::data *, int s);

extern void object_set (state::data *, json::data *, char *, json::data *);
extern void object_set_bool (state::data *, json::data *, char *, bool);
extern void object_set_string (state::data *, json::data *, char *, char *);
extern void object_set_number (state::data *, json::data *, char *, double);

extern void array_append (state::data *, json::data *, json::data *);
extern void array_append_bool (state::data *, json::data *, bool);
extern void array_append_string (state::data *, json::data *, char *);
extern void array_append_number (state::data *, json::data *, double);

extern size_t snprint (state::data *, char * buf, size_t size, json::data *, 
                       enum format);

extern bool parse(state::data *, char * buf, uintptr_t len, json::data **out, 
                  bool force_eof, int *error_at_line);

  }
}

#endif // ORCA_JSON_H