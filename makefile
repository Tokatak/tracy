build:
# debug
	gcc -Wall -Wextra -Wpedantic -g -O0 -o main main.c main.exe

# optimized
#	 gcc -Wall -Wextra -Wpedantic -O3 -march=native -o main main.c && main.exe

msvc:
	cl /Zi /DEBUG /EHsc main.c  && main.exe


profile-clean:
	rm gmon.out && rm executable.exe && rm callgraph.txt
profile-build:
	gcc -g -pg -O2 -no-pie main.c -o executable.exe && ./executable.exe
profile-run:
	gprof -q executable.exe gmon.out > callgraph.txt
profile: profile-clean profile-build profile-run
	