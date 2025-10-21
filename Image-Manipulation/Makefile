CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -g

# Links together files needed to create executable
project: project.o image_manip.o ppm_io.o
	$(CC) -o project project.o image_manip.o ppm_io.o -lm

# Compiles the object code for the project
project.o: project.c image_manip.c ppm_io.c image_manip.h ppm_io.h
	$(CC) $(CFLAGS) -c project.c image_manip.c ppm_io.c

image_manip.o: image_manip.c image_manip.h ppm_io.c ppm_io.h
	$(CC) $(CFLAGS) -c image_manip.c ppm_io.c

ppm_io.o: ppm_io.c ppm_io.h
	$(CC) $(CFLAGS) -c ppm_io.c

checkerboard: checkerboard.o ppm_io.o
	$(CC) -o checkerboard checkerboard.o ppm_io.o -lm

checkerboard.o: checkerboard.c ppm_io.h
	$(CC) $(CFLAGS) -c checkerboard.c

# Removes all object files and the executable named main, so we can start fresh                                                                                                                                                              
clean:
	rm -f *.o project checkerboard
