build:
# debug
	gcc -Wall -Wextra -Wpedantic -g -O0 -o main main.c main.exe

# optimized
#	 gcc -Wall -Wextra -Wpedantic -O3 -march=native -o main main.c && main.exe

msvc:
	cl /Zi /DEBUG /EHsc main.c  && main.exe
