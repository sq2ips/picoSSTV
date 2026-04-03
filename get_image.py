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


print("Combining images..")

with open(output_file, "wb") as f:
    print(f"Saving number of images {len(images)}, bytes: {len(images).to_bytes(1)}")
    f.write(len(images).to_bytes(1))
    for img in images:
        with open('tmp', 'wb') as tmp:
            img.save(tmp, format="JPEG", qualty=100, subsampling=0)
        size = os.stat('tmp').st_size
        print(f"Size of image: {size} (bytes: {size.to_bytes(4,byteorder='big')})")
        f.write(size.to_bytes(4,byteorder='big'))
        img.save(f, format="JPEG", qualty=100, subsampling=0)

os.remove('tmp')

print(f"Wrote {os.stat(output_file).st_size} bytes")

print(f"Conversion complete!")
print(f"Output file: {output_file}")