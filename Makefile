PATH_INCLUDES = Includes
PATH_OBJECTS = Objects
PATH_SOURCES = Sources

CC = gcc
CCFLAGS = -W -Wall -I$(PATH_INCLUDES) -O2 -pthread -lrt

BINARY = Simulator
OBJECTS = $(PATH_OBJECTS)/Core.o $(PATH_OBJECTS)/Hex_Parser.o $(PATH_OBJECTS)/Log.o $(PATH_OBJECTS)/Main.o $(PATH_OBJECTS)/Peripheral_ADC.o $(PATH_OBJECTS)/Peripheral_I2C_EEPROM.o $(PATH_OBJECTS)/Peripheral_Timer.o $(PATH_OBJECTS)/Peripheral_UART.o $(PATH_OBJECTS)/Program_Memory.o $(PATH_OBJECTS)/Register_File.o

all: $(OBJECTS)
	$(CC) $(CCFLAGS) $(OBJECTS) -o $(BINARY)

clean:
	rm -f $(BINARY) $(OBJECTS)

# TODO generic rules or dependencies
$(PATH_OBJECTS)/Core.o: $(PATH_SOURCES)/Core.c $(PATH_INCLUDES)/Core.h $(PATH_INCLUDES)/Log.h $(PATH_INCLUDES)/Program_Memory.h $(PATH_INCLUDES)/Register_File.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(PATH_OBJECTS)/Hex_Parser.o: $(PATH_SOURCES)/Hex_Parser.c $(PATH_INCLUDES)/Hex_Parser.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(PATH_OBJECTS)/Log.o: $(PATH_SOURCES)/Log.c $(PATH_INCLUDES)/Log.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(PATH_OBJECTS)/Main.o: $(PATH_SOURCES)/Main.c $(PATH_INCLUDES)/Log.h $(PATH_INCLUDES)/Peripheral_ADC.h $(PATH_INCLUDES)/Peripheral_I2C_EEPROM.h $(PATH_INCLUDES)/Peripheral_Timer.h $(PATH_INCLUDES)/Peripheral_UART.h $(PATH_INCLUDES)/Register_File.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(PATH_OBJECTS)/Peripheral_ADC.o: $(PATH_SOURCES)/Peripherals/Peripheral_ADC.c $(PATH_INCLUDES)/Peripheral_ADC.h $(PATH_INCLUDES)/Register_File.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(PATH_OBJECTS)/Peripheral_I2C_EEPROM.o: $(PATH_SOURCES)/Peripherals/Peripheral_I2C_EEPROM.c $(PATH_INCLUDES)/Peripheral_I2C_EEPROM.h $(PATH_INCLUDES)/Register_File.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(PATH_OBJECTS)/Peripheral_Timer.o: $(PATH_SOURCES)/Peripherals/Peripheral_Timer.c $(PATH_INCLUDES)/Peripheral_Timer.h $(PATH_INCLUDES)/Register_File.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(PATH_OBJECTS)/Peripheral_UART.o: $(PATH_SOURCES)/Peripherals/Peripheral_UART.c $(PATH_INCLUDES)/Peripheral_UART.h $(PATH_INCLUDES)/Register_File.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(PATH_OBJECTS)/Program_Memory.o: $(PATH_SOURCES)/Program_Memory.c $(PATH_INCLUDES)/Hex_Parser.h $(PATH_INCLUDES)/Program_Memory.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(PATH_OBJECTS)/Register_File.o: $(PATH_SOURCES)/Register_File.c $(PATH_INCLUDES)/Register_File.h
	$(CC) $(CCFLAGS) -c $< -o $@
