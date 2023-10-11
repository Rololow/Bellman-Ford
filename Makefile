CC=gcc
CFLAGS=-Wall -Werror -g
LIBS=-lcunit -lpthread
INCLUDE_HEADERS_DIRECTORY=-Iinclude
SHELL := /bin/bash


# compile the program with gcc
sp: src/graph.c src/mtgraph.c sp.c 
	$(CC) $(INCLUDE_HEADERS_DIRECTORY) $(CFLAGS) -o main $^ $(LIBS)
	@echo "Created"

%.o: %.c                  # if for example you want to compute example.c this will create an object file called example.o in the same directory as example.c. Don't forget to clean it in your "make clean"
	$(CC) $(INCLUDE_HEADERS_DIRECTORY) $(CFLAGS) -o $@ -c $<
	
help:
	@echo "+------------------------------------------------------------------------------------+"
	@echo "|  make sp      : compile the program                                                |"
	@echo "|  make run     : compile and run the program with default behaviour                 |"
	@echo "|  make time    : compile and run the program with default behaviour and time it     |"
	@echo "|  make myfile  : compile the program and run it with your own graph as a bin file   |"
	@echo "|  make mygraph : compile the program and run it with your own graph as a ntf file   |"
	@echo "|  make tests   : compile and run the tests + valgrind                               |"
	@echo "|  make clean   : remove all the object files and the executable that were created   |"
	@echo "|  make help    : show this help message                                             |"
	@echo "+------------------------------------------------------------------------------------+"

run: 
	@make -s sp 
	@./main tests/graph_bin/default.bin -f outputs/defaultOutput.bin
	@echo "Done"

time:
	@make -s sp
	@echo "Running the program with this graph : 100 nodes and 10 000 edges"
	@echo "1 thread"
	@time ./main tests/graph_bin/graph_100.bin -f time.bin -n 1 
	@echo "-----------------------------------------------------------------------------------" 
	@echo "2 threads"
	@time ./main tests/graph_bin/graph_100.bin -f time.bin -n 2 
	@echo "-----------------------------------------------------------------------------------" 
	@echo "4 threads"
	@time ./main tests/graph_bin/graph_100.bin -f time.bin -n 4 
	@echo "-----------------------------------------------------------------------------------" 
	@echo "10 threads"
	@time ./main tests/graph_bin/graph_100.bin -f time.bin -n 10 
	@echo "-----------------------------------------------------------------------------------" 
	@echo "100 threads"
	@time ./main tests/graph_bin/graph_100.bin -f time.bin -n 100 
	@echo "-----------------------------------------------------------------------------------" 
	@echo "200 threads"
	@time ./main tests/graph_bin/graph_100.bin -f time.bin -n 200


clean:
ifeq ($(OS),Windows_NT)
	@cmd /c del src/*.o
	@cmd /c del main
	@cmd /c del tests/*.o
	@cmd /c del src/tests
	@cmd /c del tests/graph_bin/p*
	@cmd /c del tests/output_tests/*_c.bin
	@cmd /c del tests/output_tests/*out_py.bin
	@cmd /c del my*.bin
	@cmd /c del *.bin
	@echo "Cleaned"
else
	@rm -f src/*.o
	@rm -f main
	@rm -f tests/*.o
	@rm -f src/tests
	@rm -f my*.bin
	@rm -f tests/graph_bin/p_*
	@rm -f tests/output_tests/*_c.bin
	@rm -f tests/output_tests/*out_py.bin
	@rm -f *.bin
	@echo "Cleaned"
endif
	

tests: src/graph.c src/cu_tests.c
	   $(CC) $(INCLUDE_HEADERS_DIRECTORY) $(CFLAGS) -o src/tests $^ $(LIBS)
	@make 
	@echo " "
	@echo "========================== VALGRIND CHECKING MEMORY =========================="
	@echo " "
# run them with valgrind to check for memory leaks and see output
	valgrind --leak-check=full ./main tests/graph_bin/default.bin -n 4 -f outputs/defaultOutput.bin
	@echo " "
	@echo "========================== VALGRIND CHECKING THREADS ========================="
	@echo " "
	valgrind --tool=helgrind ./main tests/graph_bin/default.bin -n 4 -f outputs/defaultOutput.bin

# Compare the output file of python with the with output file C code
	@echo " "
	@echo "========================== RUNNING CUNIT TESTS ==============================="	
	@echo " "
	@./main tests/graph_bin/default.bin -f tests/output_tests/default_c.bin -n 1
	@./main tests/graph_bin/neg_cost.bin -f tests/output_tests/neg_c.bin -n 1
	@./main tests/graph_bin/graph_100.bin -f tests/output_tests/graph_100_out_c.bin
	@python3 ./tests/necessary_python/sp.py tests/graph_bin/graph_100.bin -f tests/output_tests/graph_100_out_py.bin
	@./src/tests

myfile:
	@make -s sp
	@echo "Enter the name of your bin file (or path if not in this directory): "
	@read FileName;\
	echo "Running the program with the graph : $$FileName.bin and saving it to myFileOut.bin"; \
	./main $$FileName -f outputs/myFileOut.bin; \
	echo "Done output saved to outputs as myGraphOut.bin";

mygraph:
	@make -s sp
	@echo "Enter the name of your ntf file (or path if not in this directory): "
	@read FileName; echo "Creating the bin file"; \
	python3 tests/necessary_python/create_graph.py -t $$FileName -o tests/graph_bin/p_$$FileName.bin; \
	echo "Done"; \
	echo "Running the program with your graph"; \
	./main tests/graph_bin/p_$$FileName.bin -f outputs/myGraphOut.bin; \
	echo "Done, output saved to outputs as myGraphOut.bin"; 

# a .PHONY target forces make to execute the command even if the target already exists
.PHONY: clean tests time

