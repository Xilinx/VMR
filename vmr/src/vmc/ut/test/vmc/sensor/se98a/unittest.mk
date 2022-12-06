SRC += $(VMR_MAIN)/vmr/src/vmc/sensors/src/se98a.c \
       $(ROOT_DIR)/mocks/common/utm_cl_i2c.c \
       $(ROOT_DIR)/test/vmc/sensor/se98a/utt_se98a.c 

#Source code for which code coverage report will be generated
COV_SRC = $(VMR_MAIN)/vmr/src/vmc/sensors/src/se98a.c

MOCKS += i2c_send_rs_recv

