#include "Stepper.h"

void Stepper::Init(uint8_t timer_num, void (*func)())
{
    digitalWrite(step_pin_, LOW);
    digitalWrite(dir_pin_, LOW);
    timer_func_ = func;
    timer_ = timerBegin(timer_num, 2, true);
    timerAttachInterrupt(timer_, timer_func_, true);
}

void Stepper::SetSpeed(float spd)
{
    uint32_t us;

    if(spd != 0) {
        if(spd > 0) {
            digitalWrite(dir_pin_, !inv_);
            dir_ = 1;
        } else {
            digitalWrite(dir_pin_, inv_);
            dir_ = -1;
        }

        spd = abs(spd);
        if(spd > max_speed_) spd = max_speed_;
        us = (uint32_t)(40e6 / (spd * ustep_ * 2.0));
        timerAlarmWrite(timer_, us, true);
        if(!timer_enable_) {
            timerAlarmEnable(timer_);
            timer_enable_ = true;
        }
    } else {
        timerAlarmDisable(timer_);
        timer_enable_ = false;
        step_level_ = LOW;
        digitalWrite(step_pin_, LOW);
    }
}

void Stepper::SetMaxSpeed(float max_speed)
{
    max_speed_ = max_speed;
}

void IRAM_ATTR Stepper::TimerStep()
{
    step_level_ = !step_level_;
    if(step_level_) {
        portENTER_CRITICAL(timer_mux_);
        GPIO.out_w1ts = 1 << step_pin_;
        step_ += dir_;
        portEXIT_CRITICAL(timer_mux_);
    } else {
        portENTER_CRITICAL(timer_mux_);
        GPIO.out_w1tc = 1 << step_pin_;
        portEXIT_CRITICAL(timer_mux_);
    }
}

int32_t Stepper::GetStep()
{
    int32_t step;
    portENTER_CRITICAL(timer_mux_);
    step = step_;
    portEXIT_CRITICAL(timer_mux_);
    return step;
}

void Stepper::SetStep(int32_t step)
{
    portENTER_CRITICAL(timer_mux_);
    step_ = step;
    portEXIT_CRITICAL(timer_mux_);
}