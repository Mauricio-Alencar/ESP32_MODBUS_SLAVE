set(COMPONENT_SRCS "esp_timer_example_main.c"
		   "mb.c"
		   "mb_crc.c"
		   "mb_event.c"
		   "mb_serial.c"
		   "mb_timer.c")

set(COMPONENT_ADD_INCLUDEDIRS ". include")

register_component()
