librsocket.a: rsocket.o
	ar -rcs librsocket.a rsocket.o
	@echo Run make test_help for instructions on how to run the testfile

rsocket.o: rsocket.c
	gcc -c rsocket.c -o rsocket.o

clean:
	rm librsocket.a rsocket.o

test_help:
	@echo Run the test as gcc test.c librsocket.a -o test
