SRCS=display_driver.c main.c res\\roboto_regular_font.c res\\weather_glyphs.c
CC=gcc
OUT=smartframe

$(OUT):$(SRCS)
	$(CC) -Wall -g $(SRCS) -o $(OUT) -lbcm2835
	
clean:
	rm -f $(OUT)
