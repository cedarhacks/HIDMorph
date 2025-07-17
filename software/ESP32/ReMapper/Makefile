.PHONY: build flash monitor clean

flash_uart: build
	idf.py -p /dev/ttyUSB0 flash

flash: build
	idf.py -p /dev/ttyACM0 flash

build:
	idf.py build

monitor:
	idf.py -p /dev/ttyACM0 monitor

clean:
	idf.py clean