################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../startup/startup_mk66f18.c 

OBJS += \
./startup/startup_mk66f18.o 

C_DEPS += \
./startup/startup_mk66f18.d 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DPRINTF_FLOAT_ENABLE=0 -D__USE_CMSIS -DCR_INTEGER_PRINTF -DSDK_DEBUGCONSOLE=0 -D__MCUXPRESSO -DDEBUG -DSDK_OS_BAREMETAL -DFSL_RTOS_BM -DCPU_MK66FX1M0VMD18_cm4 -DCPU_MK66FX1M0VMD18 -I../board -I../CMSIS -I../drivers -I../CMSIS_driver -I../device -I../fatfs/fatfs_include -I../usb/host/class -I../sdmmc/inc -I../fatfs -I../usb/host -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\drivers" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\CMSIS" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\board" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\source" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\utilities" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\startup" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


