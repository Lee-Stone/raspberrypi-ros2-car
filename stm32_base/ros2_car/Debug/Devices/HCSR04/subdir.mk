################################################################################
# 自动生成的文件。不要编辑！
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
../Devices/HCSR04/HCSR04.c 

OBJS += \
./Devices/HCSR04/HCSR04.o 

C_DEPS += \
./Devices/HCSR04/HCSR04.d 


# 每个子目录必须为构建它所贡献的源提供规则
Devices/HCSR04/%.o Devices/HCSR04/%.su Devices/HCSR04/%.cyclo: ../Devices/HCSR04/%.c Devices/HCSR04/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/21035/Desktop/Workspace/github/CaddonThaw/raspberrypi-ros2-car/stm32_base/ros2_car/Devices" -I"C:/Users/21035/Desktop/Workspace/github/CaddonThaw/raspberrypi-ros2-car/stm32_base/ros2_car/Devices/OLED" -I"C:/Users/21035/Desktop/Workspace/github/CaddonThaw/raspberrypi-ros2-car/stm32_base/ros2_car/Devices/MPU6050" -I"C:/Users/21035/Desktop/Workspace/github/CaddonThaw/raspberrypi-ros2-car/stm32_base/ros2_car/Devices/TB6612" -I"C:/Users/21035/Desktop/Workspace/github/CaddonThaw/raspberrypi-ros2-car/stm32_base/ros2_car/Devices/Encoder" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Devices-2f-HCSR04

clean-Devices-2f-HCSR04:
	-$(RM) ./Devices/HCSR04/HCSR04.cyclo ./Devices/HCSR04/HCSR04.d ./Devices/HCSR04/HCSR04.o ./Devices/HCSR04/HCSR04.su

.PHONY: clean-Devices-2f-HCSR04

