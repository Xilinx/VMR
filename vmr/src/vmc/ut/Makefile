
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

#Add '-I' as prefix for include directories
INCLUDE = $(foreach dir, $(INCLUDE_DIR), $(addprefix -I, $(dir)))

ifdef TEST_DIR
include $(TEST_DIR)/unittest.mk
endif

#Linker flags for linking wrap definition inplace of real definition
LDFLAGS	= -L$(CMOCKA_DIR)/build/src/ -Wl,-rpath=$(CMOCKA_DIR)/build/src/ 
LDFLAGS += $(foreach MOCK,$(MOCKS),-Wl,--wrap=$(MOCK))
LDFLAGS += -lcmocka

OBJS = $(patsubst %.c, %.o, $(SRC))

CC = gcc

all : $(TARGET)

ifdef TEST_DIR
$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o : %.c
	$(CC) -o $@ -c $^ $(INCLUDE)
else
        $(info ERROR : TEST_DIR not Defined)
        $(info usage on cl: "make TEST_DIR=test/vmc/<test_directory>")
        $(error Make file exit at compilation)
endif

.PHONY : clean
clean : 
	rm $(OBJS) $(TARGET)

