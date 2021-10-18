SRCS=display_driver.o main.o roboto_regular_font.o weather_glyphs.o
CC=gcc
OUT=smartframe.out

$(OUT):$(SRCS)
	$(CC) -Wall -g $(SRCS) -o $(OUT) -lbcm2835

main.o: main.c
	$(CC) -Wall -g -c main.c

display_driver.o: display_driver.c
	$(CC) -Wall -g -c display_driver.c

roboto_regular_font.o: res/roboto_regular_font.c
	$(CC) -Wall -g -c res/roboto_regular_font.c

weather_glyphs.o: res/weather_glyphs.c
	$(CC) -Wall -g -c res/weather_glyphs.c

clean:
	rm -f $(OUT)

