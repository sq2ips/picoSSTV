from PIL import Image
import numpy as np
import struct
import sys
import os

output_file = "image.bin"

w = 320
h = 256

print(f"Expected resolution: {w}x{h}")

files = os.listdir("images/")
files = [f for f in files if f[0] != '.']
files = sorted(files)
print(f"Found {len(files)} total files.")
images = []
for file in files:
        try:
            img = Image.open(f"images/{file}")
            img = img.convert("RGB")
            width, height = img.size
            if width != w or height != h:
                #raise Exception(f"Incorrect resolution of file {file}, expected {w}x{h}, got {width}x{height}.")
                print(f"Warning: expected {w}x{h} resolution, got: {width}x{height}, scaling...")
                img = img.resize((w, h))
            images.append(np.array(img))
        except Exception as e:
            print(f"Couldn't load image {file}: {e}")
        else:
             print(f"Loaded image {file}.")

if images == []:
     raise Exception("No images loaded, exiting...")
else:
     print(f"Loaded {len(images)} images.")


print("Combining images..")

with open(output_file, "wb") as f:
    for img in images:
        r = (img[:, :, 0] >> 3).astype(np.uint16)
        g = (img[:, :, 1] >> 2).astype(np.uint16)
        b = (img[:, :, 2] >> 3).astype(np.uint16)
        rgb565 = (r << 11) | (g << 5) | b
        for pixel in rgb565.flatten():
            f.write(struct.pack("<H", pixel))


print(f"Wrote {os.stat(output_file).st_size} bytes")

print(f"Conversion complete!")
print(f"Output file: {output_file}")