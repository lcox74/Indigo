from PIL import Image

def image_dump(file, output):
    im = Image.open(file)

    width, height = im.size
    mono_data = im.convert('L')

    out = open(output, "w")

    index = 0
    for y in range(height):
        for x in range(width):
            pixel = mono_data.getpixel((x, y))
            out.write("0x%02x," % pixel)
            index += 1

            if (index >= 15):
                index = 0
                out.write("\n")
    
    out.close()


image_dump("roboto_regular_65.png", "Roboto")
image_dump("WeatherGlyphs.png", "WeatherGlyphs")