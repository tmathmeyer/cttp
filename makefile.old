PROJECT := httpl

SRC_DFS := -D__NO_FLAGS__
binary  := build/$(PROJECT)
library := build/lib$(PROJECT).a
CFLAGS  := -pedantic -g -rdynamic

link_libs := -lpthread

src_source_files := $(wildcard src/C/*.c)
src_object_files := $(patsubst src/C/%.c, \
	build/obj/%.o, $(src_source_files))

lib_source_files := $(wildcard src/lib/C/*.c)
lib_object_files := $(patsubst src/lib/C/%.c, \
	build/lib/%.o, $(lib_source_files))
lib_header_files := $(wildcard src/lib/H/*.h)

external_proj_names := $(wildcard lib/*)
external_link_librs := $(patsubst lib/%, \
	build/link/lib%.a, $(external_proj_names))
unlink_headers      := $(patsubst lib/%, \
	%, $(external_proj_names))

.PHONY: all clean

all: $(external_link_librs) \
	 $(binary) library

uninstall:
	@rm /usr/local/bin/$(PROJECT)
	@rm /usr/lib/lib$(PROJECT).a
	@rm -r /usr/include/$(PROJECT)

clean: $(unlink_headers)
	@rm -fr build

library: $(lib_object_files)
	@ar -cq $(library) $(lib_object_files)

build/lib/%.o: src/lib/C/%.c
	@mkdir -p $(shell dirname $@)
	gcc $(CFLAGS) -c $< -o $@ $(SRC_DFS)

$(binary): $(src_object_files)
	gcc -rdynamic -o $(binary) $(src_object_files) $(external_link_librs) $(link_libs)

build/obj/%.o: src/C/%.c
	@mkdir -p $(shell dirname $@)
	gcc $(CFLAGS) -c $< -o $@ $(SRC_DFS)

build/link/lib%.a: lib/%/
	@mkdir -p $(shell dirname $@)
	@cd $< && make clean && make && make headers
	@cp $<build/lib*.a build/link
	@ln -fs ../../$<build/headers/$(shell basename $<) src/C/$(shell basename $<)

$(unlink_headers):
	@rm -rf src/C/$@
