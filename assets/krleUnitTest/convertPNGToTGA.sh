#!/bin/bash
for file in ./png/*.png; do
    filename=$(basename "$file" .png)
    magick "$file" -define tga:bits=32 -depth 8 -alpha on -compress none "./tga/${filename}.tga"
done