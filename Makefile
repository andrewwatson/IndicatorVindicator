DEVICE=2c001c000d51353432393339

all: firmware

firmware:
	particle compile electron
	mv *.bin firmwares/

clean:
	rm -f firmwares/*.bin

flash: clean firmware
	particle flash ${DEVICE} firmwares/*.bin

start:
	particle call ${DEVICE} start test

stop:
	particle call ${DEVICE} stop test