SRCS=IT8951.c miniGUI.c main.c AsciiLib.c bmp.c
CC=gcc
OUT=IT8951

$(OUT):$(SRCS)
	$(CC) -Wall $(SRCS) -o $(OUT) -lbcm2835
	
clean:
	rm -f $(OUT)
