#include "PIDController.h"

void PIDController::SetParam(float Kp, float Ki, float Kd)
{
    Kp_ = Kp;
    Ki_ = Ki;
    Kd_ = Kd;
}

float PIDController::Output(float target, float feedback)
{
    float err, p, i, d, output;
    err = target - feedback;
    integral_ += (err + pre_err_) * dt_ / 2.0;
    p = Kp_ * err;
    i = Ki_ * integral_;
    d = Kd_ * (err - pre_err_) / dt_;
    output = p + i + d;
    if(output > max_output_) output = max_output_;
    else if(output < min_output_) output = min_output_;
    pre_err_ = err;
    return output;
}

void PIDController::ResetIntegral()
{
    integral_ = 0.0;
}

void PIDController::ResetPreErr()
{
    pre_err_ = 0.0;
}

void PIDController::Reset()
{
    ResetIntegral();
    ResetPreErr();
}