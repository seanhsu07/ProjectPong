################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Utilities/stm32f0_discovery.c 

OBJS += \
./Utilities/stm32f0_discovery.o 

C_DEPS += \
./Utilities/stm32f0_discovery.d 


# Each subdirectory must supply rules for building sources it contributes
Utilities/%.o: ../Utilities/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F0 -DSTM32F051R8Tx -DSTM32F0DISCOVERY -DDEBUG -DSTM32F051 -DUSE_STDPERIPH_DRIVER -I"D:/Downloads/EtchASketch-ad6c48ab964b30ddecd45d6918bb127ab5c8d4e2/EtchASketch-ad6c48ab964b30ddecd45d6918bb127ab5c8d4e2/Utilities" -I"D:/Downloads/EtchASketch-ad6c48ab964b30ddecd45d6918bb127ab5c8d4e2/EtchASketch-ad6c48ab964b30ddecd45d6918bb127ab5c8d4e2/StdPeriph_Driver/inc" -I"D:/Downloads/EtchASketch-ad6c48ab964b30ddecd45d6918bb127ab5c8d4e2/EtchASketch-ad6c48ab964b30ddecd45d6918bb127ab5c8d4e2/inc" -I"D:/Downloads/EtchASketch-ad6c48ab964b30ddecd45d6918bb127ab5c8d4e2/EtchASketch-ad6c48ab964b30ddecd45d6918bb127ab5c8d4e2/CMSIS/device" -I"D:/Downloads/EtchASketch-ad6c48ab964b30ddecd45d6918bb127ab5c8d4e2/EtchASketch-ad6c48ab964b30ddecd45d6918bb127ab5c8d4e2/CMSIS/core" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


