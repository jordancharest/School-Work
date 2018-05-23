#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED


class Time {
public:
    // Constructors
    Time();
    Time(int Hour, int Minute, int Second);

    // Accessors
    int getHour() const;
    int getMinute() const;
    int getSecond() const;

    // Modifiers
    void setHour(int Hour);
    void setMinute(int Minute);
    void setSecond(int Second);

    // Output
    void PrintAMPM();

    // Calculations


private:
    int hour_;
    int minute_;
    int second_;

};



#endif // TIME_H_INCLUDED
