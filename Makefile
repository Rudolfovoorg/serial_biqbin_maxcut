#### BiqBin makefile ####

# container image name
IMAGE ?= serial-biqbin-maxcut
# container image tag
TAG ?= 1.0.0
DOCKER_BUILD_PARAMS ?=
 
# Directories
OBJ = obj

# Compiler: other options (linux users) --> CC=icc make
CC ?= gcc

# NOTE: -framework Accelerate is for MAC, Linux users set to -lopenblas -lm (or use intel mkl)
LINALG 	 = -lopenblas -lm 
OPTI     = -O3 -fPIC -g

# binary
BINS =  biqbin

# Test input and expected output
PARAMS = test/params
TEST_INSTANCE = test/Instances/rudy/g05_60.0
TEST_EXPECTED = test/Instances/rudy/g05_60.0-expected_output
TEST_INSTANCES = $(foreach i, $(shell seq 0 9), test/Instances/rudy/g05_60.$$i)
TEST_EXPECTED_OUTS = $(foreach i, $(shell seq 0 9), test/Instances/rudy/g05_60.$$i-expected_output)




# Define file paths as variables
PARAMS = test/params
TEST_INSTANCE = test/Instances/rudy/g05_60.0
TEST_EXPECTED = test/Instances/rudy/g05_60.0-expected_output
TEST_INSTANCES = $(foreach i, $(shell seq 0 9), test/Instances/rudy/g05_60.$$i)
TEST_EXPECTED_OUTS = $(foreach i, $(shell seq 0 9), test/Instances/rudy/g05_60.$$i-expected_output)

# Test command
TEST = ./test.sh \
	./$(BINS) \
	$(TEST_INSTANCE) \
	$(TEST_EXPECTED) \
	$(PARAMS)

# Test command (for a list of files from g05_60.0 to g05_60.9)
TEST_ALL = for i in $(shell seq 0 9); do \
		./test.sh \
			./$(BINS) \
			test/Instances/rudy/g05_60.$$i \
			test/Instances/rudy/g05_60.$$i-expected_output \
			$(PARAMS); \
	done

# Python test command
TEST_PYTHON = ./test.sh \
	"python3 test.py" \
	$(TEST_INSTANCE) \
	$(TEST_EXPECTED) \
	$(PARAMS)

# Python test command (for a list of files from g05_60.0 to g05_60.9)
TEST_ALL_PYTHON = for i in $(shell seq 0 9); do \
		./test.sh \
			"python3 test.py" \
			test/Instances/rudy/g05_60.$$i \
			test/Instances/rudy/g05_60.$$i-expected_output \
			$(PARAMS); \
	done


# BiqBin objects
BBOBJS = $(OBJ)/bundle.o $(OBJ)/allocate_free.o $(OBJ)/bab_functions.o \
	 	 $(OBJ)/bounding.o $(OBJ)/cutting_planes.o \
         $(OBJ)/evaluate.o $(OBJ)/heap.o $(OBJ)/ipm_mc_pk.o \
         $(OBJ)/heuristic.o $(OBJ)/main.o $(OBJ)/operators.o \
         $(OBJ)/process_input.o $(OBJ)/qap_simulated_annealing.o \
		 $(OBJ)/biqbin.o

# All objects
OBJS = $(BBOBJS)
CFLAGS = $(OPTI) -Wall -W -pedantic


#### Rules ####

.PHONY : all clean test


# Default rule is to create all binaries #
all: $(BINS) biqbin.so

test: all
	$(TEST)
	$(TEST_PYTHON)

test-all: all
	$(TEST_ALL)
	$(TEST_ALL_PYTHON)

# Test command for all files (g05_60.0 to g05_60.9)

docker: 
	docker build $(DOCKER_BUILD_PARAMS) --progress=plain -t $(IMAGE):$(TAG)  . 

docker-clean: 
	docker rmi -f $(IMAGE):$(TAG) 

docker-test:
	docker run --rm $(IMAGE):$(TAG) $(TEST)

# Rules for binaries #
$(BINS) : $(OBJS)
	$(CC) -o $@ $^ $(INCLUDES) $(LIB) $(OPTI) $(LINALG)  


# BiqBin code rules 
$(OBJ)/%.o : %.c | $(OBJ)/
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ)/:
	 mkdir -p $@

biqbin.so: $(OBJS)
	$(CC) -shared -o biqbin.so $(OBJS) $(LINALG)

# Clean rule #
clean :
	rm -rf $(BINS) $(OBJ)
	rm -rf test/test.dat.output*
	rm -rf *.output*
	rm -f biqbin.so
	rm -rf __pycache__
