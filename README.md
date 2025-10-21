# Image-Manipulation
Image manipulation program that can perform grayscale, blend, rotate, saturate, gaussian blur, and pointilism on PPM images. You will need a PPM image viewer extension to see results.

To use, compile the project with make, which will generate the executable ./project. From there, usage follows this command line template: ./project <input.ppm> <output.ppm> <operation> [args]. For blend you must include 2 input images

Examples with dog.ppm and cat.ppm:

./project dog.ppm dog_grayscale.ppm grayscale

./project dog.ppm dog_blurred.ppm blur <sigma>

./project dog.ppm dog_rotated.ppm rotate-ccw

./project dog.ppm cat.ppm blend dog_cat_blend.ppm <alpha>

./project dog.ppm dog_pointilism.ppm pointilism

./project dog.ppm dog_saturated.ppm saturate <scale factor>


