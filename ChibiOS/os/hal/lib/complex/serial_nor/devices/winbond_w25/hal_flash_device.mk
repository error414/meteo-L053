# List of all the Winbond W25 device files.
SNORSRC := $(CHIBIOS)/os/hal/lib/complex/serial_nor/hal_serial_nor.c \
           $(CHIBIOS)/os/hal/lib/complex/serial_nor/devices/winbond_w25/hal_flash_device.c

# Required include directories
SNORINC := $(CHIBIOS)/os/hal/lib/complex/serial_nor \
           $(CHIBIOS)/os/hal/lib/complex/serial_nor/devices/winbond_w25

# Shared variables
ALLCSRC += $(SNORSRC)
ALLINC  += $(SNORINC)
