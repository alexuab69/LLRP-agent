################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
O:/full/tm/modules/mercuryapi/c/src/api/hex_bytes.c \
O:/full/tm/modules/mercuryapi/c/src/api/osdep_KL2xZ.c \
O:/full/tm/modules/mercuryapi/c/src/samples/readasync_baremetal.c \
O:/full/tm/modules/mercuryapi/c/src/api/serial_reader.c \
O:/full/tm/modules/mercuryapi/c/src/api/serial_reader_l3.c \
O:/full/tm/modules/mercuryapi/c/src/api/serial_transport_KL2xZ.c \
O:/full/tm/modules/mercuryapi/c/src/api/tm_reader.c \
O:/full/tm/modules/mercuryapi/c/src/api/tm_reader_async.c \
O:/full/tm/modules/mercuryapi/c/src/api/tmr_param.c \
O:/full/tm/modules/mercuryapi/c/src/api/tmr_strerror.c \
O:/full/tm/modules/mercuryapi/c/src/api/tmr_utils.c 

OBJS += \
./source/hex_bytes.o \
./source/osdep_KL2xZ.o \
./source/readasync_baremetal.o \
./source/serial_reader.o \
./source/serial_reader_l3.o \
./source/serial_transport_KL2xZ.o \
./source/tm_reader.o \
./source/tm_reader_async.o \
./source/tmr_param.o \
./source/tmr_strerror.o \
./source/tmr_utils.o 

C_DEPS += \
./source/hex_bytes.d \
./source/osdep_KL2xZ.d \
./source/readasync_baremetal.d \
./source/serial_reader.d \
./source/serial_reader_l3.d \
./source/serial_transport_KL2xZ.d \
./source/tm_reader.d \
./source/tm_reader_async.d \
./source/tmr_param.d \
./source/tmr_strerror.d \
./source/tmr_utils.d 


# Each subdirectory must supply rules for building sources it contributes
source/hex_bytes.o: O:/full/tm/modules/mercuryapi/c/src/api/hex_bytes.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/osdep_KL2xZ.o: O:/full/tm/modules/mercuryapi/c/src/api/osdep_KL2xZ.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/readasync_baremetal.o: O:/full/tm/modules/mercuryapi/c/src/samples/readasync_baremetal.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/serial_reader.o: O:/full/tm/modules/mercuryapi/c/src/api/serial_reader.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/serial_reader_l3.o: O:/full/tm/modules/mercuryapi/c/src/api/serial_reader_l3.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/serial_transport_KL2xZ.o: O:/full/tm/modules/mercuryapi/c/src/api/serial_transport_KL2xZ.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/tm_reader.o: O:/full/tm/modules/mercuryapi/c/src/api/tm_reader.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/tm_reader_async.o: O:/full/tm/modules/mercuryapi/c/src/api/tm_reader_async.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/tmr_param.o: O:/full/tm/modules/mercuryapi/c/src/api/tmr_param.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/tmr_strerror.o: O:/full/tm/modules/mercuryapi/c/src/api/tmr_strerror.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

source/tmr_utils.o: O:/full/tm/modules/mercuryapi/c/src/api/tmr_utils.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DSINGLE_THREAD_ASYNC_READ -DBARE_METAL -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DDEBUG -DFRDM_KL25Z -DFREEDOM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\board" -I../../include -I../../../src/api -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\source" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\drivers" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\CMSIS" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\utilities" -I"O:\full\tm\modules\mercuryapi\c\kinetis_proj\FRDMKL25Z\startup" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -flto -ffat-lto-objects -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


