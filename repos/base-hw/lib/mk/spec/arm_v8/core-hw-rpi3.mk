INC_DIR += $(REP_DIR)/src/core/spec/rpi3

# add C++ sources
SRC_CC += kernel/vm_thread_off.cc
SRC_CC += platform_services.cc
SRC_CC += spec/arm/bcm2837_pic.cc

NR_OF_CPUS = 4

# include less specific configuration
include $(REP_DIR)/lib/mk/spec/arm_v8/core-hw.inc
