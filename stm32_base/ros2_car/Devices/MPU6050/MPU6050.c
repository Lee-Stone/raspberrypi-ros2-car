#include "MPU6050.h"

#if DEVICE_MPU6050

#include "MPU6050_Reg.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include <math.h>

#define MPU6050_ADDRESS		0xD0

/**
	* @brief  MPU6050写寄存器
	* @param  RegAddress 寄存器地址
	* @param  Data 要写入的数据
	* @retval 无
	*/
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	HAL_I2C_Mem_Write(&MPU6050_I2C, MPU6050_ADDRESS, RegAddress, I2C_MEMADD_SIZE_8BIT, &Data, 1, HAL_MAX_DELAY);
}

/**
	* @brief  MPU6050读寄存器
	* @param  RegAddress 寄存器地址
	* @retval 读取到的寄存器值
	*/
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	HAL_I2C_Mem_Read(&MPU6050_I2C, MPU6050_ADDRESS, RegAddress, I2C_MEMADD_SIZE_8BIT, &Data, 1, HAL_MAX_DELAY);
	
	return Data;
}

/**
	* @brief  MPU6050初始化（寄存器配置）
	* @param  无
	* @retval 无
	*/
void MPU6050_Init(void)
{
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);
	MPU6050_WriteReg(MPU6050_CONFIG, 0x06);
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);
}

/**
	* @brief  获取MPU6050设备ID
	* @param  无
	* @retval WHO_AM_I 寄存器值
	*/
uint8_t MPU6050_GetID(void)
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

/**
	* @brief  获取MPU6050原始六轴数据
	* @param  AccX X轴加速度原始值（LSB）
	* @param  AccY Y轴加速度原始值（LSB）
	* @param  AccZ Z轴加速度原始值（LSB）
	* @param  GyroX X轴角速度原始值（LSB）
	* @param  GyroY Y轴角速度原始值（LSB）
	* @param  GyroZ Z轴角速度原始值（LSB）
	* @retval 无
	*/
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
	uint8_t DataH, DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);
	*AccX = (DataH << 8) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);
	*AccY = (DataH << 8) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);
	*AccZ = (DataH << 8) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);
	*GyroX = (DataH << 8) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);
	*GyroY = (DataH << 8) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);
	*GyroZ = (DataH << 8) | DataL;
}

// ---------------- DMP Euler (pitch/roll/yaw) ----------------

#define MPU6050_DMP_DEFAULT_HZ  (100)
#define MPU6050_DMP_Q30_SCALE   (1073741824.0f) // 2^30
#define MPU6050_DEG2RAD         (0.017453292519943295769f)

static float s_mpu6050_gyrozRadps = 0.0f;
static const signed char s_mpu6050_gyro_orientation[9] = {
	1, 0, 0,
	0, 1, 0,
	0, 0, 1,
};

/**
  * @brief  方向矩阵一行转 InvenSense 标量编码
  * @param  row 方向矩阵的一行（3个元素：-1/0/1）
  * @retval 标量编码（0~7）
  */
static inline unsigned short mpu6050_row_2_scale(const signed char *row)
{
	unsigned short b;

	if (row[0] > 0) {
		b = 0;
	} else if (row[0] < 0) {
		b = 4;
	} else if (row[1] > 0) {
		b = 1;
	} else if (row[1] < 0) {
		b = 5;
	} else if (row[2] > 0) {
		b = 2;
	} else if (row[2] < 0) {
		b = 6;
	} else {
		b = 7;
	}

	return b;
}

/**
	* @brief  方向矩阵转 InvenSense DMP 方向标量
	* @param  mtx 3x3方向矩阵（共9个元素：-1/0/1）
	* @retval 方向标量
	*/
static inline unsigned short mpu6050_orientation_matrix_to_scalar(const signed char *mtx)
{
	unsigned short scalar;

	scalar = mpu6050_row_2_scale(mtx);
	scalar |= mpu6050_row_2_scale(mtx + 3) << 3;
	scalar |= mpu6050_row_2_scale(mtx + 6) << 6;

	return scalar;
}

/**
	* @brief  初始化MPU6050 DMP（启用四元数与原始陀螺输出）
	* @param  无
	* @retval 0 成功；非0为 motion driver 返回的错误码
	*/
int MPU6050_DMP_Init(void)
{
	int result = 0;

	result = mpu_init();
	if (result) {
		return result;
	}

	result = 0;
	result += mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
	result += mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
	result += mpu_set_sample_rate(MPU6050_DMP_DEFAULT_HZ);

	result += mpu_set_gyro_fsr(2000);

	result += dmp_load_motion_driver_firmware();
	result += dmp_set_orientation(mpu6050_orientation_matrix_to_scalar(s_mpu6050_gyro_orientation));

	result += dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_SEND_RAW_GYRO);
	result += dmp_set_fifo_rate(MPU6050_DMP_DEFAULT_HZ);

	result += mpu_set_dmp_state(1);

	return result;
}

/**
	* @brief  读取DMP欧拉角（单位：度）
	* @param  pitchDeg 俯仰角指针（度），可为NULL
	* @param  rollDeg 横滚角指针（度），可为NULL
	* @param  yawDeg 偏航角指针（度），可为NULL
	* @retval 0 成功；非0为 motion driver 返回的错误码
	*/
int MPU6050_DMP_ReadEuler(float *pitchDeg, float *rollDeg, float *yawDeg)
{
	int result;

	unsigned long sensor_timestamp = 0;
	short gyro[3] = {0};
	short accel[3] = {0};
	short sensors = 0;
	unsigned char more = 0;
	long quat[4] = {0};

	do {
		result = dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
		if (result) {
			return result;
		}
	} while (more);

	const float gyrozDps = ((float)gyro[2]) / 16.4f;
	s_mpu6050_gyrozRadps = gyrozDps * MPU6050_DEG2RAD;

	const float q0 = ((float)quat[0]) / MPU6050_DMP_Q30_SCALE;
	const float q1 = ((float)quat[1]) / MPU6050_DMP_Q30_SCALE;
	const float q2 = ((float)quat[2]) / MPU6050_DMP_Q30_SCALE;
	const float q3 = ((float)quat[3]) / MPU6050_DMP_Q30_SCALE;

	const float pitch = asinf(-2.0f * q1 * q3 + 2.0f * q0 * q2) * 57.3f;
	const float roll  = atan2f(2.0f * q2 * q3 + 2.0f * q0 * q1,
							   -2.0f * q1 * q1 - 2.0f * q2 * q2 + 1.0f) * 57.3f;
	const float yaw   = atan2f(2.0f * (q1 * q2 + q0 * q3),
							   q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.3f;

	if (pitchDeg) {
		*pitchDeg = pitch;
	}
	if (rollDeg) {
		*rollDeg = roll;
	}
	if (yawDeg) {
		*yawDeg = yaw;
	}

	return 0;
}

/**
	* @brief  获取标准角速度gyroz（与ROS Imu.angular_velocity.z一致）
	* @note   本函数内部会读取一次DMP FIFO，尽量返回最新的gyroz
	* @param  无
	* @retval Z轴角速度，单位：rad/s
	*/
float MPU6050_DMP_GetROSGyroz(void)
{
	int result;

	unsigned long sensor_timestamp = 0;
	short gyro[3] = {0};
	short accel[3] = {0};
	short sensors = 0;
	unsigned char more = 0;
	long quat[4] = {0};

	do {
		result = dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
		if (result) {
			return s_mpu6050_gyrozRadps;
		}
	} while (more);

	const float gyrozDps = ((float)gyro[2]) / 16.4f;
	s_mpu6050_gyrozRadps = gyrozDps * MPU6050_DEG2RAD;

	return s_mpu6050_gyrozRadps;
}

#endif // DEVICE_MPU6050
