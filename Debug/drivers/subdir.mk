################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/button.c \
../drivers/fsl_clock.c \
../drivers/fsl_common.c \
../drivers/fsl_dmamux.c \
../drivers/fsl_dspi.c \
../drivers/fsl_dspi_cmsis.c \
../drivers/fsl_dspi_edma.c \
../drivers/fsl_edma.c \
../drivers/fsl_ftm.c \
../drivers/fsl_gpio.c \
../drivers/fsl_lptmr.c \
../drivers/fsl_lpuart.c \
../drivers/fsl_rtc.c \
../drivers/fsl_sdhc.c \
../drivers/fsl_tpm.c \
../drivers/fsl_uart.c \
../drivers/screen.c \
../drivers/ss_display_driver.c \
../drivers/time.c 

OBJS += \
./drivers/button.o \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_dmamux.o \
./drivers/fsl_dspi.o \
./drivers/fsl_dspi_cmsis.o \
./drivers/fsl_dspi_edma.o \
./drivers/fsl_edma.o \
./drivers/fsl_ftm.o \
./drivers/fsl_gpio.o \
./drivers/fsl_lptmr.o \
./drivers/fsl_lpuart.o \
./drivers/fsl_rtc.o \
./drivers/fsl_sdhc.o \
./drivers/fsl_tpm.o \
./drivers/fsl_uart.o \
./drivers/screen.o \
./drivers/ss_display_driver.o \
./drivers/time.o 

C_DEPS += \
./drivers/button.d \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_dmamux.d \
./drivers/fsl_dspi.d \
./drivers/fsl_dspi_cmsis.d \
./drivers/fsl_dspi_edma.d \
./drivers/fsl_edma.d \
./drivers/fsl_ftm.d \
./drivers/fsl_gpio.d \
./drivers/fsl_lptmr.d \
./drivers/fsl_lpuart.d \
./drivers/fsl_rtc.d \
./drivers/fsl_sdhc.d \
./drivers/fsl_tpm.d \
./drivers/fsl_uart.d \
./drivers/screen.d \
./drivers/ss_display_driver.d \
./drivers/time.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DPRINTF_FLOAT_ENABLE=0 -D__USE_CMSIS -DCR_INTEGER_PRINTF -DSDK_DEBUGCONSOLE=0 -D__MCUXPRESSO -DDEBUG -DSDK_OS_BAREMETAL -DFSL_RTOS_BM -DCPU_MK66FX1M0VMD18_cm4 -DCPU_MK66FX1M0VMD18 -I../board -I../CMSIS -I../drivers -I../CMSIS_driver -I../device -I../fatfs/fatfs_include -I../usb/host/class -I../sdmmc/inc -I../fatfs -I../usb/host -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\drivers" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\CMSIS" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\board" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\source" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\utilities" -I"C:\Users\Beau\Documents\MCUXpressoIDE_10.2.1_795\workspace\MK66FX1M0__PWM_GPIO\startup" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


