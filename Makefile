SRCS=src/display_driver.o src/main.o
CC=gcc
OUT=smartframe.out

$(OUT):$(SRCS)
	$(CC) -Wall -g $(SRCS) -o $(OUT) -lbcm2835 -ljpeg

src/main.o: src/main.c
	$(CC) -Wall -g -c src/main.c -o src/main.o

src/display_driver.o: src/display_driver.c
	$(CC) -Wall -g -c src/display_driver.c -o src/display_driver.o


clean:
	rm -f $(OUT)

