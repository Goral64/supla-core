################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/database.cpp \
../src/db.cpp \
../src/ipcclient.cpp \
../src/queue.cpp \
../src/supla-scheduler.cpp \
../src/worker.cpp 

C_SRCS += \
../src/cfg.c \
../src/eh.c \
../src/ini.c \
../src/jsmn.c \
../src/lck.c \
../src/log.c \
../src/safearray.c \
../src/schedulercfg.c \
../src/sthread.c \
../src/tools.c 

OBJS += \
./src/cfg.o \
./src/database.o \
./src/db.o \
./src/eh.o \
./src/ini.o \
./src/ipcclient.o \
./src/jsmn.o \
./src/lck.o \
./src/log.o \
./src/queue.o \
./src/safearray.o \
./src/schedulercfg.o \
./src/sthread.o \
./src/supla-scheduler.o \
./src/tools.o \
./src/worker.o 

C_DEPS += \
./src/cfg.d \
./src/eh.d \
./src/ini.d \
./src/jsmn.d \
./src/lck.d \
./src/log.d \
./src/safearray.d \
./src/schedulercfg.d \
./src/sthread.d \
./src/tools.d 

CPP_DEPS += \
./src/database.d \
./src/db.d \
./src/ipcclient.d \
./src/queue.d \
./src/supla-scheduler.d \
./src/worker.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__DEBUG=1 -O0 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -I/usr/include/mysql -O0 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


