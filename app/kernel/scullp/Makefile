
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DSCULLP_DEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

LDDINC=$(PWD)
EXTRA_CFLAGS += $(DEBFLAGS) -I$(LDDINC)

TARGET = scullp

ifneq ($(KERNELRELEASE),)

scullp-objs := scullp_main.o mmap.o

obj-m	:= scullp.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif


install:
	install -d $(INSTALLDIR)
	install -c $(TARGET).o $(INSTALLDIR)

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions


depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
