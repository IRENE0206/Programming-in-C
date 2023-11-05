CC      := clang
DEBUG   := -g3
OPTIM   := -O3
CFLAGS  := -Wall -Wextra -Wpedantic -Wfloat-equal -Wvla -std=c99 -Werror
RELEASE := $(CFLAGS) $(OPTIM)
SANI    := $(CFLAGS) -fsanitize=undefined -fsanitize=address $(DEBUG)
VALG    := $(CFLAGS)  $(DEBUG)
NCLS    := $(wildcard *.ncl)
PRES := $(NCLS:.ncl=.pres)
IRES := $(NCLS:.ncl=.ires)
LIBS    := -lm


parse: driver.c nuclei.c general.c nuclei.h general.h
	$(CC) driver.c nuclei.c general.c $(RELEASE) -o parse $(LIBS)

parse_s: driver.c nuclei.c general.c nuclei.h general.h
	$(CC) driver.c nuclei.c general.c $(SANI) -o parse_s $(LIBS)

parse_v: driver.c nuclei.c general.c nuclei.h general.h
	$(CC) driver.c nuclei.c general.c $(VALG) -o parse_v $(LIBS)

all: parse parse_s parse_v interp interp_s interp_v

interp: driver.c nuclei.c linked.c general.c nuclei.h general.h
	$(CC) driver.c nuclei.c linked.c general.c $(RELEASE) -DINTERP -o interp $(LIBS)

interp_s: driver.c nuclei.c linked.c general.c nuclei.h general.h
	$(CC) driver.c nuclei.c linked.c general.c $(SANI) -DINTERP -o interp_s $(LIBS)

interp_v: driver.c nuclei.c linked.c general.c nuclei.h general.h
	$(CC) driver.c nuclei.c linked.c general.c $(VALG) -DINTERP -o interp_v $(LIBS)

extension: driver.c nuclei.c linked.c general.c nuclei.h general.h
	$(CC) driver.c nuclei.c linked.c general.c $(RELEASE) -DINTERP -DEXTENSION -o extension $(LIBS)

extension_s: driver.c nuclei.c linked.c general.c nuclei.h general.h
	$(CC) driver.c nuclei.c linked.c general.c $(SANI) -DINTERP -DEXTENSION -o extension_s $(LIBS)

extension_v: driver.c nuclei.c linked.c general.c nuclei.h general.h
	$(CC) driver.c nuclei.c linked.c general.c $(VALG) -DINTERP -DEXTENSION -o extension_v $(LIBS)

# For all .ncl files, run them and output result to a .pres (prase result) 
# or .ires (interpretted result) file.
runall : ./parse_s ./interp_s $(PRES) $(IRES)

%.pres:
	-./parse_s  $*.ncl > $*.pres 2>&1
%.ires:
	-./interp_s $*.ncl > $*.ires 2>&1

clean:
	rm -f parse parse_s parse_v interp interp_s interp_v extension extension_s extension_v $(PRES) $(IRES)
