JSON_SRC=value.cc parser.cc snprint.cc tokenizer.cc
JSON_HDR=json.h tokenizer.h utf8.h

HEADERS=stdlib.h string.h errno.h sys/types.h sys/stat.h unistd.h stdio.h

define json_amalgamation
	@echo "#ifndef CEE_JSON_ONE" > $(1)
	@echo "#define CEE_JSON_ONE" >> $(1)
	@echo "#define _GNU_SOURCE" >> $(1)
	@for ii in $(HEADERS); do echo '#include <'$$ii'>' >> $(1); done
	@echo "#include \"cee.h\"" >> $(1)
	@echo " " >> $(1)
	@for ii in $(JSON_HDR); do cat $$ii >> $(1); echo " " >> $(1); done
	@echo "#define CEE_JSON_AMALGAMATION" > tmp.c
	@for ii in $(JSON_SRC); do echo '#include "'$$ii'"' >> tmp.cc; done
	$(CXX) -E $(2) -nostdinc tmp.cc >> $(1)
	@echo "#endif" >> $(1)
endef

.PHONY: release clean distclean

all: tester

json-one.c: $(JSON_SRC) cee.h
	$(call json_amalgamation, json-one.cc)

json-one.o: json-one.cc cee.h
	$(CXX) -c json-one.cc

cee.o: cee.cc cee.h
	$(CXX) -c -g cee.cc

release: $(JSON_SRC)
	$(call json_amalgamation, json.cc, -P)
	@mkdir -p release
	@mv json.cc release
	@cp json.h release

tester: json-one.o cee.o
	$(CXX)  -static -g tester.cc json-one.o cee.o

clean:
	rm -f a.cc cee.o json-one.c json-one.o tmp.cc

distclean: clean
	rm -f cee.cc cee.h