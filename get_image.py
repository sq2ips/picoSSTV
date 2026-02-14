from PIL import Image
import sys
import os

output_file = "image.bin"

w = 320
h = 240

if len(sys.argv) != 2:
    print("Usage: python3 input.png")
    sys.exit(1)

input_file = sys.argv[1]

if not os.path.exists(input_file):
    print("Error: Input file does not exist.")
    sys.exit(1)

# Open image
print(f"Loading {input_file}...")
img = Image.open(input_file)
# Convert to RGB (ensures no alpha channel)
img = img.convert("RGB")
width, height = img.size
if width != w or height != h:
    print(f"Incorrect resolution, use {w}x{h}.")
    sys.exit(1)

pixels = img.load()
print("Statting conversion...")
# Write raw RGB888 binary
with open(output_file, "wb") as f:
    for y in range(height):
        for x in range(width):
            r, g, b = pixels[x, y]
            f.write(bytes([r, g, b]))

print(f"Conversion complete!")
print(f"Resolution: {width}x{height}")
print(f"Output file: {output_file}")
print(f"Total bytes written: {width * height * 3}")