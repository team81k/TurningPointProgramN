#include "main.h"

#pragma  once

class pid
{
    public:
        double min = 0, max = 0;
        double Kp = 0, Ki = 0, Kd = 0;
        double slew = 0;
        double negativeSlew = 0;

        int updateMin = 100; //in milliseconds

        pid(double Kp, double Ki, double Kd, double min, double max, double slew);
        double calculate(double value = 0);

        void setTarget(double target);
        void setPowerPreSlew(double powerPreSlew);
        void setPower(double power);

        double getPower();
        double getError();
        double getChange();

        void clear();

    private:
        double target = 0;

        int step = 0;

        double error = 0, integral = 0, derivative = 0;

        double powerPreSlew = 0;
        double power = 0;

        double preError = 0;
        long lastUpdate = -1;
};
