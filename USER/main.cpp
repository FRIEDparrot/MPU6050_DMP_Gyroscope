#include "stm32f10x.h"
#include "OLED.h"
#include "Delay.h"
#include "MPU6050.h"
#include "inv_mpu.h"
#include "Serial_func.h"

using namespace std;

float pitch, roll, yaw; // 3 main Euler angles

int16_t accx_t = 0, accy_t = 0, accz_t = 0;
int16_t grox_t = 0, groy_t = 0, groz_t = 0;

/// @brief send the data of the accelerometer and gyscope data to NiMing simulation tool using Serial for simulation
/// @param func functional index 
/// @param length not exceed 28 byte
// calculate the data of Accelerometer and gyscope
void send_format_data(unsigned char f_code, unsigned char * data, unsigned char length)
{
    unsigned char buf[32];

    if (length > 28)
        return;

    buf[length + 3] = 0;
    buf[0] = 0x88;
    buf[1] = f_code;
    buf[2] = length;
    for (unsigned char i = 0; i < length; i++)
        buf[i + 3] = data[i];

    for (unsigned char i = 0; i < length + 3; i++)
        buf[length + 3] = buf[length + 3] + buf[i];

    for (unsigned char i = 0; i < length + 4; i++)
        Serial_SendByte(buf[i]);
}

// send acc wave data
void send_mpu6050_data(short aacx, short aacy, short aacz, short gyrox, short gyroy, short gyroz)
{
    unsigned char buf[12];

    buf[0] = (aacx >> 8) & 0xff;
    buf[1] = aacx & 0xff;
    buf[2] = (aacy >> 8) & 0xff;
    buf[3] = aacy & 0xff;
    buf[4] = (aacz >> 8) & 0xff;
    buf[5] = aacz & 0xff; 
    buf[6] = (gyrox >> 8) & 0xff;
    buf[7] = gyrox & 0xff;
    buf[8] = (gyroy >> 8) & 0xff;
    buf[9] = gyroy & 0xff;
    buf[10]= (gyroz >> 8) & 0xff;
    buf[11]= gyroz & 0xff;

    send_format_data(0xa1, buf, 12);
}

// send DMP data
void send_dmp_data(short aacx, short aacy, short aacz, short gyrox, short gyroy, short gyroz, short roll, short pitch, short yaw)
{
    unsigned char buf[28]; 
    
    for(uint8_t i=0;i<28;i++)buf[i]=0;

    buf[0]  = (aacx >> 8) & 0xff;
    buf[1]  = aacx & 0xff;
    buf[2]  = (aacy >> 8) & 0xff;
    buf[3]  = aacy & 0xff;
    buf[4]  = (aacz >> 8) & 0xff;
    buf[5]  = aacz & 0xff;
    buf[6]  = (gyrox >> 8) & 0xff;
    buf[7]  = gyrox & 0xff;
    buf[8]  = (gyroy >> 8) & 0xff;
    buf[9]  = gyroy & 0xff;
    buf[10] = (gyroz >> 8) & 0xff;
    buf[11] = gyroz & 0xff;
    buf[18] = (roll >> 8) & 0xff;
    buf[19] = roll & 0xff;
    buf[20] = (pitch >> 8) & 0xff;
    buf[21] = pitch & 0xff;
    buf[22] = (yaw >> 8) & 0xff;
    buf[23] = yaw & 0xff;
    send_format_data(0xaf, buf, 28);
}

/// @note software I2C operation is much stable than hardware I2C read (use it pls!!!)
int main(){
    OLED_Init();
    Serial_Init(115200);
    uint8_t res = mpu_dmp_init();
    if (res == 0) OLED_ShowString(2,1, "Init Successful");
    else {
        OLED_ShowString(2,1, "Init Error:");
        OLED_ShowNum(2, 12, res, 1);
    }
    Delay_s(1);
    OLED_Clear();

    OLED_ShowString(2,1, "pitch:");
    OLED_ShowString(3,1, "roll:");
    OLED_ShowString(4,1, "yaw:");
    while (1){
        uint8_t r = mpu_dmp_get_data(&pitch, &roll, &yaw);
        // this function is in inv_mpu.c, get orientation angle
        if (!r){
            OLED_ShowFloat(2,7,pitch);
            OLED_ShowFloat(3,7, roll);
            OLED_ShowFloat(4,7, yaw);
            MPU6050_GetData(accx_t, accy_t, accz_t, grox_t, groy_t, groz_t);

            send_mpu6050_data(accx_t, accy_t, accz_t, grox_t, groy_t, groz_t);
            send_dmp_data(accx_t, accy_t, accz_t, grox_t, groy_t, groz_t, (int16_t)(roll*100), (int16_t)(pitch * 100), (int16_t)(yaw * 10));
        }
    }
}
