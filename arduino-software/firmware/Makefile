all: fuses isp

isp:
	avrdude -F -p m328p -c usbasp  -U flash:w:aquarium_controller.hex:i

fuses:
	avrdude -F -p m328p -c usbasp -U lfuse:w:0xf7:m -U hfuse:w:0xc1:m
