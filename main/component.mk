#
# Main Makefile. This is basically the same as a component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_SRCDIRS := .

#Compile image file into the resulting firmware binary
COMPONENT_EMBED_FILES := \
  images/sun.raw \
  images/moon.raw \
  images/cloudy.raw \
  images/mist.raw \
  images/light_rain.raw \
  images/medium_hail.raw \
  images/medium_rain.raw \
  images/medium_sleet.raw \
  images/medium_snow.raw \
  images/heavy_hail.raw \
  images/heavy_rain.raw \
  images/heavy_snow.raw \
  images/storm.raw \
  images/glyphs.raw
