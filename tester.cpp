#include "json.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace cee;

int main () {
  state::data * st = state::mk(10);
  json::data * js = json::mk_object (st);
  
  json::object_set_bool(st, js, "b", true);
  json::object_set_bool(st, js, "b1", false);
  
  json::object_set_string(st, js, "s1", "xxx\n");
  json::data * js1 = json::mk_object (st);
  json::object_set_string(st, js1, "s2", "yyy");
  json::object_set(st, js, "y1", js1);
  
  json::data * js2 = json::mk_array (st, 10);
  json::array_append_string(st, js2, "false");
  json::array_append_string(st, js2, "true");
  json::object_set(st, js, "a1", js2);
  
  size_t jlen = json::snprint(st, NULL, 0, js, json::readable);
  printf (" %u\n", jlen);
  jlen = json::snprint(st, NULL, 0, js, json::compact);
  printf (" %u\n", jlen);
  
  char buf[1000];
  json::snprint(st, buf, 109, js, json::readable);
  printf ("%s\n", buf);
  
  json::snprint(st, buf, 109, js, json::compact);
  printf ("%s\n", buf);
  json::data * result = NULL;
  int line;
  printf ("pasing\n");
  json::parse(st, buf, jlen, &result, true, &line);
  printf ("end of parsing\n");
  
  json::snprint(st, buf, 109, result, json::compact);
  printf ("parsed -> printed\n");
  printf ("%s\n", buf);
  
  del(result);
  del(js);
  del(st);
  return 0;
}
