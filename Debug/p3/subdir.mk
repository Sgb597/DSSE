################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../p3/practica3_cinta.c 

OBJS += \
./p3/practica3_cinta.o 

C_DEPS += \
./p3/practica3_cinta.d 


# Each subdirectory must supply rules for building sources it contributes
p3/%.o: ../p3/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-linux-gnueabi-gcc -I"/home/manager/eclipse-workspace/practicas/ev3c/include" -I/home/manager/Documentos/ev3c_addons/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


