#pragma once

#define PI 3.14159265358979323846
#define TAU 6.28318530717958647692

constexpr double meter = 1.0;
constexpr double decimeter = meter / 10;
constexpr double centimeter = meter / 100;
constexpr double millimeter = meter / 1000;
constexpr double kilometer = 1000 * meter;
constexpr double inch = 2.54 * centimeter;
constexpr double foot = 12 * inch;
constexpr double yard = 3 * foot;
constexpr double mile = 5280 * foot;

constexpr double operator"" _mm(long double x){return x * millimeter;}
constexpr double operator"" _cm(long double x){return x * centimeter;}
constexpr double operator"" _m(long double x){return x * meter;}
constexpr double operator"" _km(long double x){return x * kilometer;}
constexpr double operator"" _mi(long double x){return x * mile;}
constexpr double operator"" _yd(long double x){return x * yard;}
constexpr double operator"" _ft(long double x){return x * foot;}
constexpr double operator"" _in(long double x){return x * inch;}

constexpr double operator"" _mm(unsigned long long int x){return x * millimeter;}
constexpr double operator"" _cm(unsigned long long int x){return x * centimeter;}
constexpr double operator"" _m(unsigned long long int x){return x * meter;}
constexpr double operator"" _km(unsigned long long int x){return x * kilometer;}
constexpr double operator"" _mi(unsigned long long int x){return x * mile;}
constexpr double operator"" _yd(unsigned long long int x){return x * yard;}
constexpr double operator"" _ft(unsigned long long int x){return x * foot;}
constexpr double operator"" _in(unsigned long long int x){return x * inch;}

constexpr double radian = 1.0;
constexpr double degree = (TAU / 360.0) * radian;

constexpr double operator"" _rad(long double x){return x;}
constexpr double operator"" _deg(long double x){return x * degree;}

constexpr double operator"" _rad(unsigned long long int x){return x;}
constexpr double operator"" _deg(unsigned long long int x){return x * degree;}
