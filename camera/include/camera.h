#ifndef __CAMERA_H__
#define __CAMERA_H__
void Camara_init(void);

#define SPEED_SIZES 52
typedef struct shutter
{
    uint32_t sleep;
    uint32_t speed;
    uint32_t shots;
}Camera;

#endif