sort:
	@echo " Compile sort_main ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sort_main.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lbf -lhp_file -o ./build/sort_main -O2

run: sort
	@rm -f *.db
	./build/sort_main

clean:
	@rm -f *.db
	@rm -f ./build/*