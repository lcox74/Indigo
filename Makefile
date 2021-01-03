SRCS=display_driver.c main.c
CC=gcc
OUT=smartframe

$(OUT):$(SRCS)
	$(CC) -Wall -g $(SRCS) -o $(OUT) -lbcm2835
	
clean:
	rm -f $(OUT)
