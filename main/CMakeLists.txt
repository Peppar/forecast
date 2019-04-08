set(COMPONENT_SRCS
  e-ink.c
  forecast.c
  forecast_graphics.c
  icons.c
  main.c
  text.c)

set(COMPONENT_ADD_INCLUDEDIRS ".")

set(COMPONENT_EMBED_FILES
  images/sun.raw
  images/moon.raw
  images/cloudy.raw
  images/mist.raw
  images/light_rain.raw
  images/medium_hail.raw
  images/medium_rain.raw
  images/medium_sleet.raw
  images/medium_snow.raw
  images/heavy_hail.raw
  images/heavy_rain.raw
  images/heavy_snow.raw
  images/storm.raw
  images/glyphs.raw)

register_component()
