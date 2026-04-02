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
            img = Image.open(f"images/{file}")
            img = img.convert("RGB")
            width, height = img.size
            if width != w or height != h:
                raise Exception(f"Incorrect resolution of file {file}, expected {w}x{h}, got {width}x{height}.")
            images.append(img)
        except Exception as e:
            print(f"Couldn't load image {file}: {e}")
        else:
             print(f"Loaded image {file}.")

if images == []:
     raise Exception("No images loaded, exiting...")
else:
     print(f"Loaded {len(images)} images.")


print("Statting conversion...")

with open(output_file, "wb") as f:
    for img in images:
        img.save(f, format="JPEG", qualty=100, subsampling=0)

print(f"Conversion complete!")
print(f"Output file: {output_file}")