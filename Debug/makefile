################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utilities/subdir.mk
-include startup/subdir.mk
-include source/subdir.mk
-include sdmmc/src/subdir.mk
-include fatfs/fatfs_source/subdir.mk
-include drivers/subdir.mk
-include device/subdir.mk
-include board/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: MK66FX1M0__PWM_GPIO.axf

# Tool invocations
MK66FX1M0__PWM_GPIO.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -Xlinker -print-memory-usage -Xlinker --gc-sections -Xlinker -Map="MK66FX1M0__PWM_GPIO.map" -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -T MK66FX1M0_PWM_GPIO_Debug.ld -o "MK66FX1M0__PWM_GPIO.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) MK66FX1M0__PWM_GPIO.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "MK66FX1M0__PWM_GPIO.axf"; # arm-none-eabi-objcopy -v -O binary "MK66FX1M0__PWM_GPIO.axf" "MK66FX1M0__PWM_GPIO.bin" ; # checksum -p MK66FX1M0xxx18 -d "MK66FX1M0__PWM_GPIO.bin";
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
