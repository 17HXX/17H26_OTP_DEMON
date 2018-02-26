################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/VR/ADC_10_17H26.c \
../vendor/VR/Ui.c \
../vendor/VR/app_att.c \
../vendor/VR/main.c \
../vendor/VR/uart_printf.c \
../vendor/VR/ui_led_pwm.c 

OBJS += \
./vendor/VR/ADC_10_17H26.o \
./vendor/VR/Ui.o \
./vendor/VR/app_att.o \
./vendor/VR/main.o \
./vendor/VR/uart_printf.o \
./vendor/VR/ui_led_pwm.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/VR/%.o: ../vendor/VR/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -D__PROJECT_VR_17H26__=1 -DDEBUG_FROM_FLASH=1 -Wall -Os -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


