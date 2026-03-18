################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/board.c \
../board/clock_config.c \
../board/pin_mux.c 

OBJS += \
./board/board.o \
./board/clock_config.o \
./board/pin_mux.o 

C_DEPS += \
./board/board.d \
./board/clock_config.d \
./board/pin_mux.d 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


