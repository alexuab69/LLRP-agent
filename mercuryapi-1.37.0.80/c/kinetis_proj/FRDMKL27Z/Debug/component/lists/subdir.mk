################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../component/lists/generic_list.c 

OBJS += \
./component/lists/generic_list.o 

C_DEPS += \
./component/lists/generic_list.d 


# Each subdirectory must supply rules for building sources it contributes
component/lists/%.o: ../component/lists/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL27Z64VLH4 -DCPU_MKL27Z64VLH4_cm0plus -DFRDM_KL27Z -DFREEDOM -DSERIAL_PORT_TYPE_UART=1 -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\ANJALI_PANCHAL\Repos\Swtree\Branches\oem\JDK-10189_Radiometer\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL27Z\board" -I../../include -I../../../src/api -I"C:\ANJALI_PANCHAL\Repos\Swtree\Branches\oem\JDK-10189_Radiometer\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL27Z\source" -I"C:\ANJALI_PANCHAL\Repos\Swtree\Branches\oem\JDK-10189_Radiometer\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL27Z" -I"C:\ANJALI_PANCHAL\Repos\Swtree\Branches\oem\JDK-10189_Radiometer\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL27Z\drivers" -I"C:\ANJALI_PANCHAL\Repos\Swtree\Branches\oem\JDK-10189_Radiometer\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL27Z\device" -I"C:\ANJALI_PANCHAL\Repos\Swtree\Branches\oem\JDK-10189_Radiometer\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL27Z\CMSIS" -I"C:\ANJALI_PANCHAL\Repos\Swtree\Branches\oem\JDK-10189_Radiometer\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL27Z\utilities" -I"C:\ANJALI_PANCHAL\Repos\Swtree\Branches\oem\JDK-10189_Radiometer\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL27Z\component\serial_manager" -I"C:\ANJALI_PANCHAL\Repos\Swtree\Branches\oem\JDK-10189_Radiometer\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL27Z\component\lists" -I"C:\ANJALI_PANCHAL\Repos\Swtree\Branches\oem\JDK-10189_Radiometer\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL27Z\component\uart" -Os -fno-common -g3 -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -flto -ffat-lto-objects -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


