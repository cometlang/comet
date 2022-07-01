[up](index.md)

## Image
inherits [Object](object.md)

The coordinate system for the images starts with 0,0 at the bottom-left and ends with (width-1),(height-1) at the top-right.  It is always in 24-bit colour, 8 bits per channel and does not currently support an alpha channel.  Jpegs are written at a constant 90 quality.

## Enum - IMAGE_FORMAT
 - `PNG`
 - `JPEG`
 - `BMP`

### constructor
 - `Image(width, height)` creates an image of width x height pixels

### methods
 - `set_pixel(x, y, r, g, b)` sets the pixel colour at coordinates x, y to the colour values (0-255) of r,g,b
 - `write_to_file(filename, format)` writes the image to the filename, in the given format (a value of the `IMAGE_FORMAT` enum)

