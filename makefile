
build:
# debug
	gcc -Wall -Wextra -Wpedantic -g -O0 -o main main.c include/ray.c include/tracer.c include/ppm.c && main.exe

# optimized
#	 gcc -Wall -Wextra -Wpedantic -O3 -march=native -o main main.c include/ray.c include/tracer.c include/ppm.c && main.exe

