CC = mpicc
CFLAGS =
FILE = master-slave

MPIRUN = mpirun
MPIRUN_FLAGS = -np 10 --hostfile hostfile

build: $(FILE)
	mpicc $(FILE).c -o $(FILE)

run: $(FILE)
	$(MPIRUN) $(MPIRUN_FLAGS) ./$(FILE)

mpi_hello: $(FILE).c
	$(CC) $(CFLAGS) $(FILE).c -o $(FILE)

clean:
	rm -f $(FILE)
