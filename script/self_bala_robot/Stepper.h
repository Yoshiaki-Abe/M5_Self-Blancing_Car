#ifndef _STEPPER_H_
#define _STEPPER_H_

#include <Arduino.h>

#define MAX_SPEED 1000.0f

class Stepper
{
    public:
        Stepper(uint8_t step_pin, uint8_t dir_pin, int ustep, portMUX_TYPE *timer_mux, bool inv = false)
        :step_pin_(step_pin), dir_pin_(dir_pin), ustep_(ustep), inv_(inv), timer_mux_(timer_mux)
        {
            pinMode(step_pin_, OUTPUT);
            pinMode(dir_pin_, OUTPUT);
        }
        void Init(uint8_t timer_num, void (*func)()); //タイマと割り込みルーチンを指定
        void SetSpeed(float spd);   //step/secで指定
        void SetMaxSpeed(float max_speed);
        void IRAM_ATTR TimerStep();
        int32_t GetStep();
        void SetStep(int32_t step);

    private:
        uint8_t step_pin_, dir_pin_;
        int ustep_;
        float max_speed_ = MAX_SPEED;
        bool inv_;
        float speed_;
        hw_timer_t *timer_;
        void (*timer_func_)();
        bool step_level_ = LOW;
        int32_t step_ = 0;
        int8_t dir_ = 1;
        portMUX_TYPE *timer_mux_;
        bool timer_enable_ = false;
};

#endif