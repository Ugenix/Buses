obj-m += accel.o
accel-objs := ./src/accel.o ./src/ADXL345.o

accel-module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

