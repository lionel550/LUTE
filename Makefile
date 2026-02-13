TARGET = lute
BUILD_DIR = build

$(BUILD_DIR)/$(TARGET).hex: main.c
	mkdir -p $(BUILD_DIR)
	avr-gcc -mmcu=atmega328p -Wall -Os -DF_CPU=16000000UL -c -o $(BUILD_DIR)/$(TARGET).o main.c
	avr-gcc -mmcu=atmega328p $(BUILD_DIR)/$(TARGET).o -o $(BUILD_DIR)/$(TARGET)
	avr-objcopy -O ihex -R .eeprom $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(TARGET).hex

upload: $(BUILD_DIR)/$(TARGET).hex
	avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyACM0 -b 115200 -U flash:w:$(BUILD_DIR)/$(TARGET).hex

clean:
	rm -rf $(BUILD_DIR)
