#ifndef _PIDCONTROLLER_H_
#define _PIDCONTROLLER_H_

class PIDController
{
    public:
        PIDController(float dt, float max, float min)
        :dt_(dt), max_output_(max), min_output_(min)
        {
        }
        void SetParam(float Kp, float Ki, float Kd);
        float Output(float target, float feedback);
        void ResetIntegral();
        void ResetPreErr();
        void Reset();
    private:
        float dt_;             //制御周期
        float max_output_;     //最大制御量
        float min_output_;     //最小制御量
        float Kp_ = 0.0;
        float Ki_ = 0.0;
        float Kd_ = 0.0;
        float pre_err_ = 0.0;
        float integral_ = 0.0;
};

#endif