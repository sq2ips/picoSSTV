from PIL import Image
import sys
import os

output_file = "image.bin"

w = 320
h = 240

print(f"Expected resolution: {w}x{h}")

files = os.listdir("images/")
print(f"Found {len(files)} total files.")
images = []
for file in files:
        try:
            img = Image.open(file)
            img = img.convert("RGB")
            width, height = img.size
            if width != w or height != h:
                raise Exception(f"Incorrect resolution of file {file}, expected {w}x{h}, got {width}x{height}.")
            images.append(img)
        except Exception as e:
            print(f"Couldn't load image {file}: {e}")
        finally:
             print(f"Loaded image {file}.")

if images == []:
     raise Exception("No images loaded, exiting...")
else:
     print(f"Loaded {len(images)} images.")


print("Statting conversion...")
# Write raw RGB888 binary
with open(output_file, "wb") as f:
    for img in images:
        pixels = img.load()
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[x, y]
                f.write(bytes([b, g, r]))

print(f"Conversion complete!")
print(f"Output file: {output_file}")
print(f"Total bytes written: {width * height * 3 * len(images)}")