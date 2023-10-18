#!/usr/bin/python3

import os
import argparse
import cairosvg
from PIL import Image
from io import BytesIO

def list_svg_files(directory):
    """
    List all SVG files in the given directory.
    """
    return [os.path.join(directory, f) for f in os.listdir(directory) if f.endswith('.svg')]

def get_filename_without_extension(filepath):
    """
    Given a filepath, return the filename without its extension.
    """
    return os.path.splitext(os.path.basename(filepath))[0]

def scale_image_to_max_size(image, max_width, max_height):
    """
    Scale the image to fit within max_width and max_height, while preserving the aspect ratio.
    """
    img_width, img_height = image.size
    img_ratio = img_width / img_height
    max_ratio = max_width / max_height

    # Compare the aspect ratios
    if img_ratio > max_ratio:
        # Image is wider than the desired ratio, so set width to max_width and scale height
        new_width = max_width
        new_height = int(new_width / img_ratio)
    else:
        # Image is taller or equal to the desired ratio, so set height to max_height and scale width
        new_height = max_height
        new_width = int(new_height * img_ratio)

    return image.resize((new_width, new_height), Image.NEAREST)


def svg_to_raster(input_svg):
    # Convert SVG to raster in memory (using BytesIO)
    output_buffer = BytesIO()
    cairosvg.svg2png(url=input_svg, write_to=output_buffer)
    output_buffer.seek(0)

    # Use PIL to open the raster image
    return Image.open(output_buffer)

def handle_transparency(img):
    """
    Replace transparent pixels with {255, 0, 255}
    """
    img = img.convert("RGBA")

    datas = img.getdata()
    newData = []

    for item in datas:
        # Change all white (also shades of whites)
        # pixels to yellow
        if item[3] == 0:  # Alpha is 0, which means transparent
            newData.append((255, 0, 255, 255))
        else:
            newData.append(item)

    img.putdata(newData)
    return img.convert("RGB")

def generate_palette_and_bitmap(image):
    image = handle_transparency(image)
    
    # Getting all colors
    colors = list(image.getdata())
    unique_colors = list(set(colors))
    
    # Create a palette
    palette = {color: idx for idx, color in enumerate(unique_colors)}
    if len(palette) >= 30:
        print(f"TOO MANY COLORS {len(palette)}")
        sys.exit()
    
    # Create a bitmap
    bitmap = [palette[color] for color in colors]
    
    return unique_colors, bitmap

def generate_c_code(palette, bitmap, width, height, filename, var_index):
    palette_str = ",\n".join([f"{{ {color[0]}, {color[1]}, {color[2]} }}" for color in palette])
    bitmap_str = ", ".join(map(str, bitmap))
    
    return f"""
bitmap_data bmp_db_{var_index} = {{
    .palette = {{
        {palette_str}
    }},
    .bitmap = {{
        {bitmap_str}
    }},
    .width = {width},
    .height = {height},
    .name = "{filename}"
}};
"""

def main():
    # Setup command-line argument parsing
    parser = argparse.ArgumentParser(description="Convert SVG images to C code with a color palette.")
    parser.add_argument("directory", type=str, help="Path to the directory containing SVG files to convert.")
    parser.add_argument("--max-width", type=int, default=160, help="Maximum width of the output raster image.")
    parser.add_argument("--max-height", type=int, default=80, help="Maximum height of the output raster image.")

    args = parser.parse_args()

    svg_files = list_svg_files(args.directory)
    
    print(f"""
#ifndef BITMAPS_DEFINITIONS
#define BITMAPS_DEFINITIONS

typedef struct {{
    color_t palette[30];
    int width;
    int height;
    const char name[10];
    uint8_t bitmap[];
}} bitmap_data;     

""")
    i = 0
    variablesDeclaration = "bitmap_data *bitmap_db[] = { "
    for svg_file in svg_files:
        image = svg_to_raster(svg_file)
        image = scale_image_to_max_size(image, args.max_width, args.max_height)
        palette, bitmap = generate_palette_and_bitmap(image)
        filename_no_ext = get_filename_without_extension(svg_file)
        c_code = generate_c_code(palette, bitmap, image.width, image.height, filename_no_ext, i)
        variablesDeclaration += f"&bmp_db_{i}, "
        print(c_code)
        i += 1

    variablesDeclaration += "};"
    print(variablesDeclaration)
    print("""

#endif
""")
if __name__ == "__main__":
    main()
