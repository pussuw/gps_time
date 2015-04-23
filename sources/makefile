# System headers / init
include $(SOURCEPREFIX)vendor/makefile
include $(SOURCEPREFIX)common/makefile
include $(SOURCEPREFIX)hal/makefile

# Targets
ifeq ($(target),gps_reader)
include $(SOURCEPREFIX)gps_reader_node/makefile
else ifeq ($(target),clock_module)
include $(SOURCEPREFIX)clock_module/makefile
endif
