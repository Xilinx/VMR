
#Test Directory
TEST_DIR = test/vmc/sensor/se98a

ROOT_DIR = $(shell pwd)

VMR_MAIN = $(ROOT_DIR)/../../../..

#Update the name w.r.t to the package installed on the system.
DEV = xilinx_v70_gen5x8_qdma_2_202220_1

#Cmocka directory for Cmocka library
CMOCKA_DIR = $(VMR_MAIN)/../cmocka-1.1.0

INCLUDE_DIR = $(VMR_MAIN)/vmr/src/vmc/sensors/inc \
	      $(VMR_MAIN)/vmr/src/include \
	      $(VMR_MAIN)/vmr/src/vmc \
	      $(VMR_MAIN)/vmr/src/vmc/platforms \
	      $(CMOCKA_DIR)/include \
	      $(VMR_MAIN)/build/build_dir/$(DEV)/export/$(DEV)/sw/config0_0/freertos10_xilinx_domain/bspinclude/include
    
TARGET = run_test

#Code Coverage reports
COV_OUT_DIR  = Cov_Report
COV_INFO     = Cov.info
COV_INFO_ALL = Cov_all.info

#Add '-I' as prefix for include directories
INCLUDE = $(foreach dir, $(INCLUDE_DIR), $(addprefix -I, $(dir)))

ifdef TEST_DIR
include $(TEST_DIR)/unittest.mk
endif

#Linker flags for linking wrap definition inplace of real definition
LDFLAGS	= -L$(CMOCKA_DIR)/build/src/ -Wl,-rpath=$(CMOCKA_DIR)/build/src/ 
LDFLAGS += $(foreach MOCK,$(MOCKS),-Wl,--wrap=$(MOCK))
LDFLAGS += -lcmocka

#Code coverage flags for the compiler
COVFLAGS = -fprofile-arcs -ftest-coverage 

#Code Coverage Flags
COV_DATA = $(patsubst %.c, %.gcda, $(COV_SRC)) \
          $(patsubst %.c, %.gcno, $(COV_SRC))

OBJS = $(patsubst %.c, %.o, $(SRC))

CC = gcc

all : $(TARGET)

ifdef TEST_DIR
$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS) $(COVFLAGS)

%.o : %.c
	$(CC) -o $@ -c $^ $(INCLUDE) $(COVFLAGS)
else
        $(info ERROR : TEST_DIR not Defined)
        $(info usage on cl: "make TEST_DIR=test/vmc/<test_directory>")
        $(error Make file exit at compilation)
endif

#Below target generates code coverage report
.PHONY : lcov 
lcov : $(COV_INFO)
	genhtml $(COV_INFO) --output-directory $(COV_OUT_DIR)
	
$(COV_INFO) : $(COV_DATA)
	lcov --capture --directory $(VMR_MAIN) --output-file $(COV_INFO_ALL)
	lcov --remove $(COV_INFO_ALL) -output-file $(COV_INFO) '$(ROOT_DIR)/*'

.PHONY : clean
clean : 
	rm $(OBJS) $(TARGET) $(COV_DATA) $(COV_INFO_ALL) $(COV_INFO) $(COV_OUT_DIR) -r
	find ./ -type f -name '*.gc*' -delete

