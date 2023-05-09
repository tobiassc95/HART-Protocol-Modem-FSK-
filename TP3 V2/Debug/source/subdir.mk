################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/App.c \
../source/CMP.c \
../source/DAC.c \
../source/FSK_MOD.c \
../source/PIT_driver.c \
../source/demod.c \
../source/ftm.c \
../source/measure.c \
../source/portpin.c \
../source/systick.c \
../source/timer.c \
../source/uart.c 

OBJS += \
./source/App.o \
./source/CMP.o \
./source/DAC.o \
./source/FSK_MOD.o \
./source/PIT_driver.o \
./source/demod.o \
./source/ftm.o \
./source/measure.o \
./source/portpin.o \
./source/systick.o \
./source/timer.o \
./source/uart.o 

C_DEPS += \
./source/App.d \
./source/CMP.d \
./source/DAC.d \
./source/FSK_MOD.d \
./source/PIT_driver.d \
./source/demod.d \
./source/ftm.d \
./source/measure.d \
./source/portpin.d \
./source/systick.d \
./source/timer.d \
./source/uart.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -D__USE_CMSIS -DDEBUG -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


