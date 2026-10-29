#pragma once
#include "type_traits.h"


#define _MNC_BEGIN namespace mnc {
#define _MNC_END }

_MNC_BEGIN

//////////////////////////////////////////////////////////////////////
////////////////////////////// CONTENTS //////////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region CONTENTS

// basic functions
constexpr bool isInt(double x);
constexpr bool equal(double x);
constexpr double whole(double x);
constexpr double frac(double x);
constexpr double floor(double x);
constexpr double ceil(double x);
constexpr double abs(double x);
constexpr double sign(double x);
constexpr unsigned long long Pn(unsigned long long n);
constexpr unsigned long long nPk(unsigned long long n, unsigned long long k);
constexpr unsigned long long nCk(unsigned long long n, unsigned long long k);
constexpr long long fibonacci(long long n);

template <int N> 
constexpr long long gcd(long long const (&nums)[N]);
template <int N> 
constexpr long long lcm(long long const (&nums)[N]);
template <int N>
constexpr double mean(double const (&nums)[N]);

constexpr double max(double a, double b);
constexpr double min(double a, double b);

// calculus functions
constexpr double pow(double base, double exponent);
constexpr double sqrt(double x);
constexpr double cbrt(double x);
constexpr double sin(double x);
constexpr double cos(double x);
constexpr double tan(double x);
constexpr double asin(double x);
constexpr double acos(double x);
constexpr double atan(double x);
constexpr double exp(double x);
constexpr double ln(double x);
constexpr double log(double base, double x);
constexpr double log10(double x);
constexpr double sinh(double x);
constexpr double cosh(double x);
constexpr double tanh(double x);
constexpr double asinh(double x);
constexpr double acosh(double x);
constexpr double atanh(double x);
constexpr double erf(double x);
constexpr double fact(double x);

// probability and statistics
double GeomDist(double p, long long lower, long long upper);
double BinomDist(double p, unsigned long long n, long long lower, long long upper);
double PoissDist(double lamda, long long lower, long long upper);
double NegGeomDist(double p, unsigned long long r, long long lower, long long upper);
double HypGeomDist(unsigned long long N, unsigned long long K, unsigned long long n, long long lower, long long upper);
double NormDist(double mean, double sigma, double lower, double upper);
double NormCDF(double mean, double sigma, double x);
double invNormCDF(double mean, double sigma, double p);
double TstudentCDF(unsigned long long df, double x);
double TstudentDist(unsigned long long df, double lower, double upper);

struct Constant
{
    static constexpr double PI = 3.14159265358979323846264338327950288419716939937510L;
    static constexpr double EULER = 2.71828182845904523536028747135266249775724709369995L;
    static constexpr double inf = __builtin_huge_val();
    static constexpr double NaN = __builtin_nan("0");
    static constexpr double epsilon = __DBL_EPSILON__;
};

#pragma endregion
//////////////////////////////////////////////////////////////////////
/////////////////////////// basic functions //////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region basic

constexpr bool isInt(double x)
{
    return x == static_cast<long long>(x);
}

constexpr bool equal(double a, double b)
{
    if (abs(a - b) < Constant::epsilon) return true;
    else return false;
}

constexpr double whole(double x)
{
    return static_cast<long long>(x);
}

constexpr double frac(double x)
{
    return x - static_cast<long long>(x);
}

constexpr double floor(double x)
{
    double u = static_cast<long long>(x);
    if (x == u)
        return x;
    return (x < 0)? (u - 1) : (u);
}

constexpr double ceil(double x)
{
    double u = static_cast<long long>(x);
    if (x == u)
        return x;
    return (x < 0)? (u) : (u + 1);
}

constexpr double abs(double x)
{
    if (x < 0) return -x;
    else return x;
}

constexpr double sign(double x)
{
    if (x < 0) return -1;
    else if (x == 0) return 0;
    else return 1;
}

constexpr unsigned long long Pn(unsigned long long n)
{
    unsigned long long result = 1;
    while (n > 1)
    {
        result *= n;
        n--;
    }
    return result;
}

constexpr unsigned long long nPk(unsigned long long n, unsigned long long k)
{
    if (k > n)
        return 0;

    unsigned long long result = 1;
    for (unsigned long long i = 1; i <= k; i++)
    {
        result *= n;
        n--;
    }
    return result;
}

constexpr unsigned long long nCk(unsigned long long n, unsigned long long k)
{
    if (k > n) 
        return 0;

    if (k > n / 2)
        k = n - k;

    unsigned long long result = 1;
    for (unsigned long long i = 1; i <= k; i++)
    {
        result *= n;
        result /= i;
        n--;
    }
    return result;
}

constexpr long long fibonacci(long long n)
{
    if (n < 0)
    {
        n = -n;
        if (n % 2 == 1)
            return -fibonacci(n);
    }
    
    long long arr[2] = { 0, 1 };
    if (n <= 1)
    {
        return arr[n];
    }
    else
    {
        for (long long i = 2; i <= n; i++)
        {
            arr[1] = arr[0] + arr[1];
            arr[0] = arr[1] - arr[0];
        }
        return arr[1];
    }
}

constexpr long long gcd(long long const (&nums)[0]) = delete;

template <int N>
constexpr long long gcd(long long const (&nums)[N])
{
    struct helper
    {
        static constexpr long long gcd(long long a, long long b)
        {
            if (a < 0)
                a = -a;
            if (b < 0)
                b = -b;

            if (a == 0)
                return b;
            if (b == 0)
                return a;
            
            long long dividend = (b > a) ? b : a;
            long long divisor = (a < b) ? a : b;
            
            while (divisor != 0)
            {
                long long remainder = dividend % divisor;
                dividend = divisor;
                divisor = remainder;
            }

            return dividend;
        }
    };

    long long common = (nums[0] >= 0)? (nums[0]) : (-nums[0]);
    for (int i = 1; i < N; i++)
    {
        common = helper::gcd(common, nums[i]);
    }
    return common;
}

constexpr long long lcm(long long const (&nums)[0]) = delete;

template <int N>
constexpr long long lcm(long long const (&nums)[N])
{
    struct helper
    {
        static constexpr long long lcm(long long a, long long b)
        {
            if (a < 0)
                a = -a;
            if (b < 0)
                b = -b;

            if (a == 0 || b == 0)
                return 0;
            
            long long dividend = (b > a) ? b : a;
            long long divisor = (a < b) ? a : b;
            
            while (divisor != 0)
            {
                long long remainder = dividend % divisor;
                dividend = divisor;
                divisor = remainder;
            }

            return (a / dividend) * (b / dividend) * dividend;
        }
    };

    long long common = (nums[0] >= 0)? (nums[0]) : (-nums[0]);
    for (int i = 1; i < N; i++)
    {
        common = helper::lcm(common, nums[i]);
    }
    return common;
}

constexpr double mean(double const (&nums)[0]) = delete;

template <int N>
constexpr double mean(double const (&nums)[N])
{
    double sum = 0;
    for (int i = 0; i < N; i++)
    {
        sum += nums[i];
    }
    return sum / N;
}

constexpr double max(double a, double b)
{
    return (a > b)? a : b;
}

constexpr double min(double a, double b)
{
    return (a < b)? a : b;
}

#pragma endregion
//////////////////////////////////////////////////////////////////////
//////////////////////// calculus functions //////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region calculus

constexpr double pow(double base, double exponent)
{
    if (exponent == 0)
    {
        if (base != 0)
            return 1;
        else
            return Constant::NaN;
    }

    if (base == 0)
    {
        if (exponent > 0)
            return 0;
        else
            return Constant::NaN;
    }

    if (base == 1)
        return 1;

    if (isInt(exponent))
    {
        if (exponent < 0)
        {
            base = 1 / base;
            exponent = -exponent;
        }

        long long exp = exponent;
        double result = 1;

        // xem số mũ là một chuỗi nhị phân: a.2^0 + b.2^1 + c.2^2 + d.2^3 + EULER.2^4 + ...
        while (exp > 0)
        {
            if (exp & 1)  // tương đương với "exp % 2 == 1"
                result *= base;

            base *= base;
            exp >>= 1;
        }

        return result;
    }

    if (base < 0)
        return Constant::NaN;

    return exp(exponent * ln(base));
}

constexpr double sqrt(double x)
{
    if (x < 0)
        return Constant::NaN;
    if (x == 0)
        return 0;
    if (x == 1)
        return 1;

    double scale = 1;
    while (x < 0.4) // 1 - x > 4 * x - 1 => x < 2 / (1 + 4)
    {
        x *= 4;
        scale *= 0.5;
    }

    while (x > 1.6) // x - 1 > 1 - x / 4 => x < 2 * 4 / (1 + 4)
    {
        x *= 0.25;
        scale *= 2;
    }

    double y = 0.5 * (x + 1);
    for (int i = 0; i < 4; i++)
    {
        y = 0.5 * (y + x / y);
    }
    return y * scale;
}

constexpr double cbrt(double x)
{
    if (x < 0)
        return -cbrt(-x);
    if (x == 0)
        return 0;
    if (x == 1)
        return 1;

    /*
    *   Tương tự như sqrt nhưng nhân/chia hệ số s^3
    *   Quá nhỏ thì nhân s^3
    *   Quá lớn thì chia s^3
    */
    constexpr double s = 1.625, inv = 1 / s;
    constexpr double cube = s * s * s, cube_inv = 1 / cube;
    constexpr double LOWER_BOUND = 2 / (1 + cube);
    constexpr double UPPER_BOUND = LOWER_BOUND * cube;

    double scale = 1;
    while (x < LOWER_BOUND)
    {
        x *= cube;
        scale *= inv;
    }

    while (x > UPPER_BOUND)
    {
        x *= cube_inv;
        scale *= s;
    }

    constexpr double ONE_THIRD = 1.0 / 3.0;
    double y = ONE_THIRD * (x + 2);
    for (int i = 0; i < 4; i++)
    {
        y = ONE_THIRD * (2 * y + x / (y * y));
    }
    return y * scale;
}

constexpr double sin(double x) 
{
    constexpr double PI = Constant::PI;
    // Đưa x về khoảng [-pi; pi]
    if (x < -PI || x > PI)
    {
        constexpr double TWO_PI = 2 * Constant::PI;
        x -= floor((x + PI) / TWO_PI) * TWO_PI;
    }

    // Đưa x về khoảng [-pi/2; pi/2]
    if (x > PI / 2)
        x = PI - x;
    if (x < -PI / 2)
        x = -PI - x;

    // Khởi tạo giá trị ban đầu của chuỗi Taylor
    double result = x, term = x, a = -x * x;
    for (long long k = 1; k < 10; k++) 
    {
        long long u = 2 * k;
        term *= a / (u * (u + 1));
        result += term;
    }

    return result;
}

constexpr double cos(double x) { return sin(Constant::PI / 2 - x); }

constexpr double tan(double x) 
{
    constexpr double PI_OVER_TWO = Constant::PI / 2;
    constexpr double PI_OVER_FOUR = Constant::PI / 4;
    // Đưa x về khoảng [-pi/2; pi/2]
    if (x < -PI_OVER_TWO || x > PI_OVER_TWO)
        x -= floor((x + PI_OVER_TWO) / Constant::PI) * Constant::PI;

    // Kiểm tra các giá trị đặc biệt của x
    if (equal(x, PI_OVER_TWO))
        return Constant::inf;
    if (equal(x, -PI_OVER_TWO))
        return -Constant::inf;

    if (x > PI_OVER_FOUR)
        return 1 / tan(PI_OVER_TWO - x);
    if (x < -PI_OVER_FOUR)
        return -1 / tan(PI_OVER_TWO + x);

    // Khởi tạo giá trị ban đầu của chuỗi Taylor
    double sinx = x, sinterm = x;
    double cosx = 1, costerm = 1;
    double a = -x * x;
    for (long long k = 1; k <= 7; k++)
    {
        long long u = 2 * k, b = u * u;
        double v = a / (u * (u - 1) * (u + 1));
        sinterm *= v * (u - 1);
        sinx += sinterm;
        costerm *= v * (u + 1);
        cosx += costerm;
    }

    return sinx / cosx;
}

constexpr double asin(double x)
{
    struct maclaurin
    {
        static double asin(double x) //chính xác nhưng chậm
        {
            if (x < 0)
                return -asin(-x);
            if (x == 1)
                return Constant::PI / 2;
            if (x > 1)
                return Constant::NaN;
            
            if (x < 0.5)
            {
                /* 
                *                                 1
                *   asin(x) = integral { --------------- dx }
                *                           sqrt(1 - x^2)
                *
                *             = integral { (1 - x^2)^-1/2 dx }
                *
                *             ... áp dụng nhị thức Newton với n = -1/2
                *
                *                                  (2k)!
                *   NOTE: (-1/2 + x)! = sqrt(PI) ----------
                *                                 4^x * x!
                *
                *                      (2k)! * x^(2k + 1)
                *   asin(x) = sum { ------------------------- } 
                *                    4^k * (k!)^2 * (2k + 1)
                */

                double result = x, a = x * x, term = x;
                for (long long k = 1; k <= 23; k++)
                {
                    term *= a * (2 * k - 1) / (2 * k);
                    result += term / (2 * k + 1);
                }
                return result;
            }
            if (x == 0.5) //tránh đệ quy vô tận!
            {
                return Constant::PI / 6;
            }
            else
            {
                return (Constant::PI / 2) - 2 * asin(sqrt(0.5 * (1 - x)));
            }
            /// NOTE: mean error compared to std::asin is 7.22763326610640928813e-17 (23 loops)
        }
    };

    struct chebychev
    {
        static double asin(double x) //độ chính xác thấp! KHÔNG DÙNG!
        {
            /*
            *   Đa thức Chebychev:
            *   Tn(cos(x)) = cos(nx)
            *   T0(x) = 1
            *   T1(x) = x
            *
            *   f(x) = a0 * T0(x) + a1 * T1(x) + a2 * T2(x) + a3 * T3(x) + ...
            *   a0 = 1/pi * integral x: 0 -> pi { f(cos(t)) * dt }
            *   an = 2/pi * integral x: 0 -> pi { f(cos(t)) * cos(nt) * dt }, n > 1
            *
            *   Do arcsin(x) là hàm lẻ nên các hệ số bậc chẵn luôn bằng 0
            *   an = 0, n chẵn
            *   an = 2/pi * integral x: 0 -> pi { arcsin(cos(t)) * cos(nt) * dt }, n lẻ
            *
            *   I  = integral x: 0 -> pi { (pi/2 - arccos(cos(t))) * cos(nt) * dt }
            *      = integral x: 0 -> pi { (pi/2 - t) * cos(nt) * dt }
            *      = (1 - (-1)^n) / n^2
            *
            *                     4
            *    n lẻ => an = ----------
            *                  pi * n^2
            */

            if (x > 1)
                return Constant::NaN;
        
            double result = 0;
            double Tn[2] = { 1, x };
            for (int k = 0; k <= 15; k++)
            {
                int n = 2 * k + 1;
                result += (4 / (Constant::PI * n * n)) * Tn[1];

                // Nhảy một lúc 2 bước
                Tn[0] = 2 * x * Tn[1] - Tn[0]; // Tn+2(x) = 2x * Tn+1(x) - Tn(x), thay thế Tn(x)
                Tn[1] = 2 * x * Tn[0] - Tn[1]; // Tn+3(x) = 2x * Tn+2(x) - Tn+1(x), thay thế Tn+1(x)
            }
            return result;
        }
    };

    struct chebychev_odd
    {
        static double asin(double x)
        {
            if (x < 0)
                return -asin(-x);
            if (x == 0.5)
                return Constant::PI / 6;
            if (x == 1)
                return Constant::PI / 2;
            if (x > 1)
                return Constant::NaN;

            // các hệ số bậc lẻ của asin(x / 2) trên [-0.5; 0.5] (bậc chẵn luôn bằng 0)
            constexpr double coef[] =
            {
                5.17315809222683276047e-01,
                6.07901608607899161113e-03,
                1.95205290965666218919e-04,
                8.31669940680598074919e-06,
                4.05616477754031039543e-07,
                2.14178545084053017148e-08,
                1.19167461058558053476e-09,
                6.88083719162650848772e-11,
                4.08440405230038177055e-12,
                2.47710412060069209064e-13,
                1.52449416688986918691e-14,
                1.01690919276276852120e-15,
                7.86628028053835067768e-17,
            }; //dx = 2^-17

            /*
            *   --------------------------------------------------------------------------------
            *
            *   Tìm các hệ số Chebychev cho asin(x/2)
            *   a_n = 2/pi * integral x: -1 -> 1 { asin(x/2) * Tn(x) * 1/sqrt(1 - x^2) * dx }
            *       = 2/pi * integral t: 0 -> pi { asin(cos(t)/2) * cos(nt) * dt }
            *
            *   Sử dụng các hệ số vừa tìm được để xấp xỉ arcsin(x) từ -1/2 đến 1/2
            *
            *   --------------------------------------------------------------------------------
            *
            *   Arcsin(x) là hàm lẻ 
            *   => Các hệ số bậc chẵn đều bằng 0
            *   => Nhảy hai bậc 1 lần để tiết kiệm số vòng lặp
            *
            *   Đặt Rn(cos(x)) = cos((2n+1)x)
            *   => Rn+2(cos(x)) = 2cos(2x)Rn+1(cos(x)) - Rn(cos(x))
            *                   = (4cos(x)^2 - 2)Rn+1(cos(x)) - Rn(cos(x))
            *
            *   => R-1(x) = x
            *      R0(x) = x
            *      đệ quy: Rn+2(x) = (4x^2 - 2)Rn+1(x) - Rn(x)
            *
            *   --------------------------------------------------------------------------------
            */

            if (x < 0.5)
            {
                double result = 0;
                double y = 2 * x; // Thế [2x] vào arcsin(x/2). Tương đương thế [x] vào arcsin(x)
                double u = 4 * y * y - 2;
                double Rn[2] = { y, y };
                for (int k = 0; k <= 12; k++)
                {
                    result += coef[k] * Rn[1];

                    //Nhảy một lúc 2 bước (bản cải tiến)
                    double R_next = u * Rn[1] - Rn[0];
                    Rn[0] = Rn[1];
                    Rn[1] = R_next;
                }
                return result;
            }
            else
            // inline: pi/2 - 2 * asin(sqrt((1 - x) / 2))
            {
                // inline: x = sqrt((1 - x) / 2)
                {
                    x = 0.5 * (1 - x);
                    double scale = 1;
                    while (x < 0.4)
                    {
                        x *= 4;
                        scale *= 0.5;
                    }

                    double y = 0.5 * (x + 1);
                    for (int i = 0; i < 4; i++)
                    {
                        y = 0.5 * (y + x / y);
                    }
                    x = y * scale;
                }

                // inline: result = asin(sqrt((1 - x) / 2))
                {
                    double result = 0;
                    double y = 2 * x;
                    double u = 4 * y * y - 2;
                    double Rn[2] = { y, y };
                    for (int k = 0; k <= 12; k++)
                    {
                        result += coef[k] * Rn[1];

                        //Nhảy một lúc 2 bước (bản cải tiến)
                        double R_next = u * Rn[1] - Rn[0];
                        Rn[0] = Rn[1];
                        Rn[1] = R_next;
                    }
                    return (Constant::PI / 2) - 2 * result;
                }
            }
            // mean error: 1.52871342218562765883e-16
        }
    };

    struct chebyshev_easy_and_fast
    {
        static double asin(double x)
        {
            if (x < 0)
                return -asin(-x);
            if (x == 0.5)
                return Constant::PI / 6;
            if (x == 1)
                return Constant::PI / 2;
            if (x > 1)
                return Constant::NaN;

            // các hệ số bậc lẻ của asin(x / 2) trên [-0.5; 0.5] (bậc chẵn luôn bằng 0)
            constexpr double coef[] =
            {
                5.17315809222683330583e-01,
                6.07901608607900319219e-03,
                1.95205290965683009044e-04,
                8.31669940681269850050e-06,
                4.05616477781583086567e-07,
                2.14178546439181605095e-08,
                1.19167468174815054541e-09,
                6.88084581722037437425e-11,
                4.08447321429172399664e-12,
                2.47704188887870609666e-13,
                1.52810023201287719778e-14,
                9.55998295187876962563e-16,
                6.05644840685129022974e-17,
            }; //dx = 2^-17

            if (x < 0.5)
            {
                double const y = 2 * x;         // Thế [2x] vào arcsin(x/2). Tương đương thế [x] vào arcsin(x)
                double const u = 4 * y * y - 2;

                double result = 0;
                double Rn[2] = { y, y };
                for (int k = 0; k <= 12; k++)
                {
                    result += coef[k] * Rn[1];

                    //Nhảy một lúc 2 bước (bản cải tiến)
                    double R_next = u * Rn[1] - Rn[0];
                    Rn[0] = Rn[1];
                    Rn[1] = R_next;
                }
                return result;
            }
            else
            // inline: pi/2 - 2 * asin(sqrt((1 - x) / 2))
            {
                double const z = 0.5 * (1 - x);
                double const u = 16 * z - 2;

                // y = sqrt(z)
                double y;
                {
                    double z_copy = z;
                    double scale = 1;
                    while (z_copy < 0.4)
                    {
                        z_copy *= 4;
                        scale *= 0.5;
                    }

                    double t = 0.5 * (z_copy + 1);
                    for (int i = 0; i < 4; i++)
                    {
                        t = 0.5 * (t + z_copy / t);
                    }

                    double x = t * scale;
                    y = 2 * x;
                }

                double result = 0;
                double Rn[2] = { 1, 1 };  // factor y out
                for (int k = 0; k <= 12; k++)
                {
                    result += coef[k] * Rn[1];

                    //Nhảy một lúc 2 bước (bản cải tiến)
                    double R_next = u * Rn[1] - Rn[0];
                    Rn[0] = Rn[1];
                    Rn[1] = R_next;
                }
                result *= y;  // multiply y back in

                return (Constant::PI / 2) - 2 * result;
            }
        }
    };

    if (x < 0)
        return -asin(-x);
    if (x == 0.5)
        return Constant::PI / 6;
    if (x == 1)
        return Constant::PI / 2;
    if (x > 1)
        return Constant::NaN;

    // hệ số chebychev bậc lẻ
    constexpr double coef[] =
    {
        5.17315809222683330583e-01,
        6.07901608607900319219e-03,
        1.95205290965683009044e-04,
        8.31669940681269850050e-06,
        4.05616477781583086567e-07,
        2.14178546439181605095e-08,
        1.19167468174815054541e-09,
        6.88084581722037437425e-11,
        4.08447321429172399664e-12,
        2.47704188887870609666e-13,
        1.52810023201287719778e-14,
        9.55998295187876962563e-16,
        6.05644840685129022974e-17,
    };
    // NOTE: table mới, tính toán bằng long double trên linux

    struct __integrand__
    {
        double operator()(double x, int n)
        {
            return (2 / Constant::PI) * maclaurin::asin(mnc::cos(x) / 2) * mnc::cos(n * x);
        }
    };

    /*
    *   --------------------------------------------------------------------------------
    *
    *   P = c0 + c1 * x + c2 * x^2 + c3 * x^3 + c4 * x^4
    *
    *   P = c0 + c1 * x + c2 * x^2 + (x * c4 + c3) * x^3
    *
    *   P = c0 + c1 * x + (x * (x * c4 + c3) + c2) * x^2
    *
    *   P = c0 + (x * (x * (x * c4 + c3) + c2) + c1) * x
    *
    *   P = (x * (x * (x * (x * c4 + c3) + c2) + c1) + c0) * x^0
    *
    *   --------------------------------------------------------------------------------
    *
    *   Cho P = ... + Cn-3 * Xn-3 + Cn-2 * Xn-2 + Cn-1 * Xn-1 + Cn * Xn
    *   đệ quy: Xn = a * Xn-1 + b * Xn-2
    *   
    *   Xem rk là hệ số của Xk sau khi mở đệ quy tất cả các hạng tử lớn hơn Xk
    *   P = ... + rk * Xk + ... (Xk+1 đã mở đệ quy trước đó)
    *
    *   P = ... + Cn-3 * Xn-3 + Cn-2 * Xn-2 + Cn-1 * Xn-1 + Cn * Xn
    *     = ... + Cn-3 * Xn-3 + Cn-2 * Xn-2 + Cn-1 * Xn-1 + [rn] * Xn
    *   => rn = cn
    *
    *   P = ... + Cn-3 * Xn-3 + Cn-2 * Xn-2 +              Cn-1 * Xn-1 + [rn] * (a * Xn-1 + b * Xn-2)
    *     = ... + Cn-3 * Xn-3 + Cn-2 * Xn-2 + (a * [rn] + Cn-1) * Xn-1                                       + (b * [rn]) * Xn-2
    *     = ... + Cn-3 * Xn-3 + Cn-2 * Xn-2 + [rn-1] * Xn-1                                                  + (b * [rn]) * Xn-2
    *   => rn-1 = a * [rn] + Cn-1
    *
    *   P = ... + Cn-3 * Xn-3 + Cn-2 * Xn-2 +           [rn-1] * (a * Xn-2 + b * Xn-3)        + (b * [rn]) * Xn-2
    *     = ... + Cn-3 * Xn-3 + (a * [rn-1] + b * [rn] + Cn-2) * Xn-2                         + (b * [rn-1]) * Xn-3
    *     = ... + Cn-3 * Xn-3 + [rn-2] * Xn-2                                                 + (b * [rn-1]) * Xn-3
    *   => rn-2 = a * [rn-1] + b * [rn] + Cn-2
    *
    *   P = ... + Cn-3 * Xn-3 +             [rn-2] * (a * Xn-3 + b * Xn-4)        + (b * [rn-1]) * Xn-3
    *     = ... + (a * [rn-2] + b * [rn-1] + Cn-3) * Xn-3                         + (b * [rn-2]) * Xn-4
    *     = ... + [rn-3] * Xn-3                                                   + (b * [rn-2]) * Xn-4
    *   => rn-3 = a * [rn-2] + b * [rn-1] + Cn-3
    *
    *   ...

    *   P = [r0] * X0 + (b * [r1]) * X-1
    *
    *   Vậy: [rk-2] = a * [rk+1] + b * [rk+2] + Ck
    *   [rn] = cn
    *   [rn+1] = 0
    *   [rn+2] = 0
    *
    *   --------------------------------------------------------------------------------
    *
    *   || R-1(x) = x
    *   || R0(x) = x
    *   || Rn+2(x) = (4x^2 - 2) * Rn+1(x) - Rn(x)
    *
    *   => rk = (4x^2 - 2) * rk+1 - rk+2 + Ck
    *   rn+1 = 0
    *   rn+2 = 0
    *
    *   P(x) = r0 * R0(x) + (-1 * r1) * R-1(x)
    *   mà R0(x) = R-1(x) = x
    *   => P(x) = (r0 - r1) * x
    *
    *   --------------------------------------------------------------------------------
    */

    if (x < 0.5)
    {
        double y = 2 * x; // Thế [2x] vào arcsin(x/2). Tương đương thế [x] vào arcsin(x)
        double u = 4 * y * y - 2;
        double rk1 = 0; // rk+1
        double rk2 = 0; // rk+2
        for (int k = 12; k >= 0; k--)
        {
            double rk = u * rk1 - rk2 + coef[k];
            rk2 = rk1;
            rk1 = rk;
        }
        return (rk1 - rk2) * y;
    }
    else
    // inline: pi/2 - 2 * asin(sqrt((1 - x) / 2))
    {
        /*////////////////////////////////////////////////////
        ///////////////////// CODE CŨ ////////////////////////
        //////////////////////////////////////////////////////
        double y = 0.5 * (1 - x);


        // Tính: sqrt(y)
        double scale = 1;
        while (y < 0.4)
        {
            y *= 4;
            scale *= 0.5;
        }

        double t = 0.5 * (y + 1);
        for (int i = 0; i < 4; i++)
        {
            t = 0.5 * (t + y / t);
        }
        double sqrt_y = t * scale;


        // Tính: asin(sqrt(y))
        double z = 2 * sqrt_y;
        double const u = 4 * z * z - 2;
        double rk1 = 0; // rk+1
        double rk2 = 0; // rk+2
        for (int k = 12; k >= 0; k--)
        {
            double rk = u * rk1 - rk2 + coef[k];
            rk2 = rk1;
            rk1 = rk;
        }
        double asin_sqrt_y = (rk1 - rk2) * z;


        // Kết hợp cả hai và trả về
        return (Constant::PI / 2) - 2 * asin_sqrt_y;

        ////////////////////////////////////////////////////*/
        ////////////////////// CODE MỚI //////////////////////
        //////////////////////////////////////////////////////
        double y = 0.5 * (1 - x);


        // Tính: asin(sqrt(y)) (phần đầu)
        double const u = 16 * y - 2;
        double rk1 = 0;
        double rk2 = 0;
        for (int k = 12; k >= 0; k--)
        {
            double rk = u * rk1 - rk2 + coef[k];
            rk2 = rk1;
            rk1 = rk;
        }


        // Tính: sqrt(y)
        double scale = 1;
        while (y < 0.4)
        {
            y *= 4;
            scale *= 0.5;
        }

        double t = 0.5 * (y + 1);
        for (int i = 0; i < 4; i++)
        {
            t = 0.5 * (t + y / t);
        }
        double sqrt_y = t * scale;


        // Tính: asin(sqrt(y)) (phần còn lại)
        double z = 2 * sqrt_y;
        /*double u = 4 * z * z - 2; (đã đem lên trên thành u = 16 * y - 2)*/
        double asin_sqrt_y = (rk1 - rk2) * z;


        // Kết hợp cả hai và trả về
        return (Constant::PI / 2) - 2 * asin_sqrt_y;
    }
    // mean error: 5.20962371853115438745e-17 (12 loops)
}

constexpr double acos(double x) { return (Constant::PI / 2) - asin(x); }

constexpr double atan(double x) { return asin(x / sqrt(x * x + 1)); }

constexpr double exp(double x)
{
    if (x == 0)
        return 1;
    if (x == -Constant::inf)
        return 0;
    if (x == Constant::inf)
        return Constant::inf;

    long long int_part = floor(x + 0.5);
    x = x - int_part;

    // tính toán với phần x không nguyên  
    double term = 1, result = term;
    for (long long k = 1; k <= 13; k++)
    {
        term *= x / k;
        result += term;
    }

    // tính toán với phần x nguyên 
    double a = Constant::EULER;
    if (int_part < 0)
    {
        a = 1 / Constant::EULER;
        int_part = -int_part;
    }

    while (int_part > 0)
    {
        if (int_part & 1)
            result *= a;

        a *= a;
        int_part >>= 1;
    }

    return result;
}

constexpr double ln(double x)
{
    if (x < 0)
        return Constant::NaN;
    if (x == 0)
        return -Constant::inf;
    if (x == 1)
        return 0;

    constexpr double ONE_OVER_EULER = 1 / Constant::EULER;
    int exp = 0;

    /*
    *   Cận dưới của biên (tìm x sao cho e*x vẫn nằm trong phạm vi hội tụ)
    *   e*x - 1 > 1 - x
    */
    while (x < (5.0 / (2.0 + 2.0 * Constant::EULER)))
    {
        x *= Constant::EULER;
        exp--;
    }

    /*
    *   Cận trên của biên (tìm x sao cho x/e vẫn nằm trong phạm vi hội tụ)
    *   1 - x/e < x - 1   
    */
    while (x > (5.0 / (2.0 + 2.0 / Constant::EULER)))
    {
        x *= ONE_OVER_EULER;
        exp++;
    }

    double y = (x - 1) / (x + 1);
    double term = y, result = term, a = y * y;
    for (int k = 1; k < 11; k++)
    {
        term *= a;
        result += term / (2 * k + 1);
    }

    return 2 * result + exp;
}

constexpr double log(double base, double x)
{
    if (x <= 0)
        return -Constant::inf;
    return ln(x) / ln(base);
}

constexpr double log10(double x)
{
    if (x <= 0)
        return -Constant::inf;

    constexpr double ONE_OVER_ln10 = 0.43429448190325176116;
    return ONE_OVER_ln10 * ln(x);
}

constexpr double sinh(double x)
{
    double y = exp(x);
    return 0.5 * (y - 1 / y);
}

constexpr double cosh(double x)
{
    double y = exp(x);
    return 0.5 * (y + 1 / y);
}

constexpr double tanh(double x)
{
    double y = exp(2 * x);
    return (y - 1) / (y + 1);
}

constexpr double asinh(double x) { return ln(x + sqrt(x * x + 1)); }

constexpr double acosh(double x) { return ln(x - sqrt(x * x - 1)); }

constexpr double atanh(double x) { return 0.5 * ln((1 + x) / (1 - x)); }

constexpr double fact(double x)
{
    struct helper 
    {
        static constexpr double logtorial(double x)
        {
            if (x == 1)
                return 0;

            constexpr double EULER_GAMMA = 0.577215664901532860606512;
            constexpr double zeta[] =
            {
                0.000000000000000000000000,
                0.000000000000000000000000,
                1.644934066848226436472415,
                1.202056903159594285399738,
                1.082323233711138191516003,
                1.036927755143369926331367,
                1.017343061984449139714517,
                1.008349277381922826839797,
                1.004077356197944339378685,
                1.002008392826082214417852,
                1.000994575127818085337145,
                1.000494188604119464558702,
                1.000246086553308048298637,
                1.000122713347578489146751,
                1.000061248135058704829409,
                1.000030588236307020493551,
                1.000015282259408651871732,
                1.000007637197637899762273,
                1.000003817293264999839856,
                1.000001908212716553938925,
                1.000000953962033872796114,
                1.000000476932986787806463,
                1.000000238450502727732990,
                1.000000119219926035046987,
                1.000000059608189051259479,
                1.000000029803503514652280,
                1.000000014901554828365041,
                1.000000007450711789835429,
                1.000000003725334024788457,
                1.000000001862659723513049,
                1.000000000931327432419668
            };
            
            double y = x - 1, result = (1 - EULER_GAMMA) * y;
            double term = y * y, a = -y;
            for (long long k = 2; k <= 25; k++)
            {
                /*
                *             zeta[k] - 1
                *   (-1)^k * ------------- * (x - 1)^k
                *			       k
                */
                result += ((zeta[k] - 1) / k) * term;
                term *= a;
            }

            return result;
        }

    };

    // Các giá trị đặc biệt
    if (x == Constant::inf)
        return Constant::inf;
    if (x == -Constant::inf)
        return 0;

    // Đưa x về khoảng [0.5;1.5]
    double y = x - floor(x - 0.5);
    double result = exp(helper::logtorial(y));

    // Suy ngược lại x ban đầu
    while (y < x)
    {
        result *= y + 1;
        y += 1;
    }
    while (y > x)
    {
        result /= y;
        y -= 1;
    }

    return result;
}

constexpr double erf(double x)
{
    struct helper
    {
        static constexpr double original_erf(double x)
        {
            if (x < 0)
                return -original_erf(-x);
            if (x == 0)
                return 0;
            if (x >= 7)
                return 1;

            constexpr double TWO_OVER_SQRT_PI = 1.12837916709551255856;
            constexpr double ONE_OVER_EULER = 1 / Constant::EULER;
            double last_center = mnc::ceil(x) - 1;

            double result = 0;
            long long exp_index = 0;
            double scalar_center = 1; // EULER^(-center^2)
            for (double center = 0; center <= last_center; center += 1) // the expansion center of the taylor series
            {
                double delta = (center == last_center) ? (x - center) : (1);
                double sum = 0, term = 1;
                double coef[2] = { 1,-2 * center }; // derivative[k] = scalar_center * coef[0] as the current derivative
                for (long long k = 1; k <= 35; k++) // result = k{0} => k starts from 1
                {
                    term *= delta / k;
                    sum += coef[0] * term;

                    // calculate order k+2 derivative coefficicient recursively;
                    double next_coef = -2 * (center * coef[1] + k * coef[0]); // d/dx<k+2>f = -2(center * d/dx<k+1>f + k * d/dx<k>f)
                    coef[0] = coef[1];
                    coef[1] = next_coef;
                }

                while (exp_index < center * center)
                {
                    scalar_center *= ONE_OVER_EULER;
                    exp_index++;
                }
                result += scalar_center * sum;
            }

            return TWO_OVER_SQRT_PI * result;
            /// NOTE: mean error compared to std::erf is 6.68115426574348205501e-17 (35 loops)
        }
    };

    //values of erf(x), step = 0.125
    //source: original_erf
    constexpr double erf_table[] = 
    {
        0.00000000000000000000e+00,
        1.40316204801333804797e-01,
        2.76326390168236901701e-01,
        4.04116909434822146885e-01,
        5.20499877813046518682e-01,
        6.23240882188418110488e-01,
        7.11155633653514973602e-01,
        7.84075061059859468315e-01,
        8.42700792949714783120e-01,
        8.88388231701707664634e-01,
        9.22900128256458285314e-01,
        9.48170072782090311847e-01,
        9.66105146475310760934e-01,
        9.78443733239983570726e-01,
        9.86671671219182311319e-01,
        9.91990057670120051192e-01,
        9.95322265018952712090e-01,
        9.97345970640517665018e-01,
        9.98537283413318710323e-01,
        9.99217061782108784129e-01,
        9.99593047982554994135e-01,
        9.99794624263858677793e-01,
        9.99899378077880318649e-01,
        9.99952145160256211653e-01,
        9.99977909503001360925e-01,
        9.99990103265374741248e-01,
        9.99995697220536272631e-01,
        9.99998184718572602492e-01,
        9.99999256901627608940e-01,
        9.99999704859807381219e-01,
        9.99999886272743387927e-01,
        9.99999957486055857103e-01,
        9.99999984582742085237e-01,
        9.99999994576599160467e-01,
        9.99999998149425861627e-01,
        9.99999999387516713689e-01,
        9.99999999803383943231e-01,
        9.99999999938783856734e-01,
        9.99999999981514897662e-01,
        9.99999999994586552532e-01,
        9.99999999998462563155e-01,
        9.99999999999576449916e-01,
        9.99999999999886868274e-01,
        9.99999999999970690112e-01,
        9.99999999999992672528e-01,
        9.99999999999998112621e-01,
        9.99999999999999555911e-01,
        9.99999999999999888978e-01,
        9.99999999999999888978e-01,
        9.99999999999999888978e-01,
        9.99999999999999888978e-01,
        9.99999999999999888978e-01,
        9.99999999999999888978e-01,
        9.99999999999999888978e-01,
        9.99999999999999888978e-01,
        9.99999999999999888978e-01,
        1.00000000000000000000e+00,
    };

    //values of (2/sqrt(PI))*e^(-x^2), step = 0.125
    constexpr double scalar[] = 
    {
        1.12837916709551255856e+00,
        1.11088526959666245020e+00,
        1.06001412937611427800e+00,
        9.80352809545907866173e-01,
        8.78782578935444647072e-01,
        7.63499535760604586265e-01,
        6.42931069195207371969e-01,
        5.24745045290148204309e-01,
        4.15107497420594717674e-01,
        3.18273958500769282853e-01,
        2.36521122447290760293e-01,
        1.70359773687515619889e-01,
        1.18930289223629359618e-01,
        8.04722590225111639173e-02,
        5.27749959301503784870e-02,
        3.35458284242160648114e-02,
        2.06669853540920565871e-02,
        1.23408206143336921651e-02,
        7.14231902201798319468e-03,
        4.00647786167021915937e-03,
        2.17828423035270994645e-03,
        1.14787512588267518431e-03,
        5.86277247093792432640e-04,
        2.90228282862498032386e-04,
        1.39253051946747891110e-04,
        6.47586832347130065624e-05,
        2.91890253835817056315e-05,
        1.27517407997650898216e-05,
        5.39942677738478373910e-06,
        2.21592028463311321854e-06,
        8.81432191231804362467e-07,
        3.39822381780915556688e-07,
        1.26982346718665636369e-07,
        4.59899582884605410949e-08,
        1.61439937195074187292e-08,
        5.49271722885346676737e-09,
        1.81130589590869061591e-09,
        5.78928136658747505955e-10,
        1.79343570343413470120e-10,
        5.38487049209946054477e-11,
        1.56708665310173458260e-11,
        4.42017086933857417807e-12,
        1.20840747160067628572e-12,
        3.20196103824762497539e-13,
        8.22331604526292584145e-14,
        2.04694520842379415920e-14,
        4.93848514096422270614e-15,
        1.15480746435025332183e-15,
        2.61730123924926693493e-16,
        5.74944781793256761017e-17,
        1.22412807925995376175e-17,
        2.52613107147747932360e-18,
        5.05258000306265725052e-19,
        9.79487311756384172224e-20,
        1.84040213158377058888e-20,
        3.35162127184556646640e-21,
        5.91596295800307513893e-22
    };

    if (x < 0)
        return -erf(-x);
    if (x == 0)
        return 0;
    if (x >= 7)
        return 1;

    /*
    *   f(x) = integral { e^(-x^2) * dx }
    *
    *   f'(x) = e^(-x^2)
    *
    *   f''(x) = -2x * e^(-x^2)
    *        = -2x * f'(x)
    *
    *   f'''(x) = -2(x * f''(x) + f'(x))
    *
    *   f''''(x) = -2(x * f'''(x) + 2f''(x))
    *
    *       d^(n+2)                      d^(n+1)                   d^(n)
    *   => ---------- { f(x) } = -2(x * ---------- { f(x) } + n * -------- { f(x) })
    *       dx^(n+2)                     dx^(n+1)                  dx^(n)
    */

    int table_idx = static_cast<int>(8 * x);
    double center = 0.125 * table_idx;
    double delta = x - center;
    
    double result = 0, term = 1;
    double coef[2] = { 1, -2 * center }; //coeficients factored out from derivatives
    for (long long k = 1; k <= 13; k++)
    {
        term *= delta / k;
        result += coef[0] * term;

        // calculate order k+2 derivative coefficicient recursively;
        double next_coef = -2 * (center * coef[1] + k * coef[0]);
        coef[0] = coef[1];
        coef[1] = next_coef;
    }

    return erf_table[table_idx] + result * scalar[table_idx];
    /// NOTE: mean error compared to std::erf is 6.85997947435274731088e-17 (13 loops)
}

#pragma endregion
//////////////////////////////////////////////////////////////////////
////////////////////////// heavy functions ///////////////////////////
//////////////////////////////////////////////////////////////////////
#pragma region heavy


template <typename Function, typename = enable_if<is_callable<Function, double(double)>>>
double sigma_sum(Function const& f, long long lower, long long upper)
{
    long long a = lower, b = upper;
    double result = 0;
    if (b >= a)
    {
        for (long long i = a; i <= b; i++)
        {
            result = result + f(i);
        }
    }
    else
    {
        result = f(a);
        for (long long i = a - 1; i >= b; i--)
        {
            result = result - f(i + 1);
        }
    }
    return result;
}

template <typename Function, typename = enable_if<is_callable<Function, double(double)>>>
double pi_product(Function const& f, long long lower, long long upper)
{
    long long a = lower, b = upper;
    double result = 1;
    if (b >= a)
    {
        for (long long i = a; i <= b; i++)
        {
            result = result * f(i);
        }
    }
    else
    {
        result = f(a);
        for (long long i = a - 1; i >= b; i--)
        {
            result = result / f(i + 1);
        }
    }
    return result;
}

enum class integral_MODE { fixed_Newton_Cote, fixed_Gauss, adaptive_Gauss_unsafe, adaptive_Gauss_safe, Default };
//using Function = double (&)(double);

template <typename Function, typename = enable_if<is_callable<Function, double(double)>>>
double integral(Function const& f, double const lower, double const upper, integral_MODE MODE = integral_MODE::Default)
{
    struct fixed_Newton_Cote
    {
        static double integral(Function const& integrand, double const lower, double const upper)
        {
            double dx = pow(2, -15);
            auto f = [&](double x) -> double { return (lower < x && x < upper)? integrand(x) : (0); };

            // tìm số đoạn chia tối thiểu
            unsigned long long count = ceil((upper - lower) / dx);
            // đảm bảo sô đoạn chia luôn là số chia hết cho 10
            count = 10 * (count / 10 + 1);
            // cập nhật dx 
            dx = (upper - lower) / count;

            double x = lower;
            /*double Sum = 0;
            for (unsigned long long k = 0; k <= count - 10; k += 10)
            {
                x = lower + k * dx;
                Sum += 
                16067 * (f(x) + f(x + 10 * dx)) +
                106300 * (f(x + dx) + f(x + 9 * dx)) + 
                -48525 * (f(x + 2 * dx) + f(x + 8 * dx)) +
                272400 * (f(x + 3 * dx) + f(x + 7 * dx)) +
                -260550 * (f(x + 4 * dx) + f(x + 6 * dx)) +
                427368 * (f(x + 5 * dx));
            }*/

            double Sum = 0.0;
            double rounding_error = 0.0;   // compensation for rounding error
            for (unsigned long long k = 0; k <= count - 10; k += 10)
            {
                double x = lower + k * dx;

                double term =
                16067 * (f(x + 0 * dx) + f(x + 10 * dx)) +
                106300 * (f(x + 1 * dx) + f(x + 9 * dx)) +
                -48525 * (f(x + 2 * dx) + f(x + 8 * dx)) +
                272400 * (f(x + 3 * dx) + f(x + 7 * dx)) +
                -260550 * (f(x + 4 * dx) + f(x + 6 * dx)) +
                427368 *  f(x + 5 * dx);

                // Kahan Summation
                double countered_error_term = term - rounding_error;
                double newSum = Sum + countered_error_term;
                rounding_error = (newSum - Sum) - countered_error_term;
                Sum = newSum;

                /*
                *   1) Attempt to add the corrected term
                *   2) Observe what actually got added
                *   3) Record the discrepancy for later repayment
                */
            }
            return 5 * (Sum / 299376) * dx;
        }
    };

    struct legendre
    {
        long double value;
        long double derivative;

        static legendre calc(int n, long double x)
        {
            /*
            *              2k + 1                   k
            *   Tk+1(x) = -------- * x * Tk(x) - ------- * Tk-1(x)
            *              k + 1                  k + 1
            *
            *   Tk+1'(x) = x * Tk'(x) + (k + 1) * Tk(x)
            *
            */

            long double Tk[2] = { 1, x };  // { Tk-1(x), Tk(x) }
            long double derivative = 1;  // Tk'(x)
            for (int k = 1; k < n; k++)
            {
                long double next = ((2 * k + 1) * x * Tk[1] - k * Tk[0]) / (k + 1);
                derivative = x * derivative + (k + 1) * Tk[1];
                Tk[0] = Tk[1];
                Tk[1] = next;
            }

            return { Tk[1], derivative };
        }
    };

    struct root_weight
    {
        long double root;
        long double weight;

        static root_weight find(int n, long double guess)
        {
            long double weight;
            for (int count = 0; count < 20; count++)
            {
                legendre lg = legendre::calc(n, guess);
                guess = guess - lg.value / lg.derivative;

                /*
                *                    2
                *   ci = -------------------------
                *         (1 - x^2) * (Tn'(xi))^2
                */

                if (count + 1 == 20)
                    weight = 2 / ((1 - guess * guess) * lg.derivative * lg.derivative);
            }

            return { guess, weight };
        }

        static void compute(int n, long double* xi, long double* ci)  // SKIPS xi[0], ci[0]
        {
            if (n < 1)
                return;

            long double dx = pow(2, -15);
            int root_number = 1;
            
            long double x = -1;
            while (x < 1)
            {
                if (legendre::calc(n, x).value * legendre::calc(n, x + 2 * dx).value < 0)
                {
                    root_weight result = find(n, x + dx);
                    xi[root_number] = result.root;
                    ci[root_number] = result.weight;

                    root_number += 1;
                    x += 2 * dx;
                }
                x += dx;
            }
        }

        /*static void table(int n)
        {
            if (n < 1)
            {
                println("<EMPTY>");
                return;
            }

            long double x = -1;
            long double dx = mnc::pow(2, -15);
            int root_number = 1;

            println("TABLE:");
            while (x < 1)
            {
                if (legendre::calc(n, x).value * legendre::calc(n, x + 2 * dx).value < 0)
                {
                    root_weight rw = find(n, x + dx);
                    println("{} {} {} {}",
                        mnc::align(mnc::format("i = {}", root_number), 10),
                        mnc::align(mnc::format("xi = {},", rw.root), 30),
                        mnc::align(mnc::format("ci = {},", rw.weight), 30),
                        mnc::align(mnc::format("Tn(xi) = {}", legendre::calc(n, rw.root).value), 30)
                    );
                    x += 2 * dx;
                    root_number += 1;
                }
                x += dx;
            }
        }*/

        /*static void list(int n)
        {
            if (n < 1)
            {
                println("<EMPTY>");
                return;
            }

            long double dx = mnc::pow(2, -15);
            // compute roots
            println("roots");
            {
                long double x = -1;
                while (x < 1)
                {
                    if (legendre::calc(n, x).value * legendre::calc(n, x + 2 * dx).value < 0)
                    {
                        println("{},", find(n, x + dx).root);
                        x += 2 * dx;
                    }
                    x += dx;
                }
            }

            // compute weights
            println("weights");
            {
                long double x = -1;
                int root_number = 1;
                while (x < 1)
                {
                    if (legendre::calc(n, x).value * legendre::calc(n, x + 2 * dx).value < 0)
                    {
                        println("{},", find(n, x + dx).weight);
                        x += 2 * dx;
                    }
                    x += dx;
                }
            }
        }*/
    };

    static constexpr double xi[] =
    {
        -0.00000000000000000000,
        -0.99971372677344128022,       -0.99849195063959583774,       -0.99629513473312514282,       -0.99312493703744342266,
        -0.98898439524299175396,       -0.98387754070605704104,       -0.97780935848691830081,       -0.97078577576370628233,
        -0.96281365425581555417,       -0.95390078292549174144,       -0.94405587013625602566,       -0.93328853504307951461,
        -0.92160929814533398829,       -0.90902957098252967771,       -0.89556164497072698882,       -0.88121867938501841078,
        -0.86601468849716467524,       -0.84996452787959131392,       -0.8330838798884008245,        -0.81538923833917620332,
        -0.79689789239031449952,       -0.77762790964949546346,       -0.75759811851970715324,       -0.736828089802020747,
        -0.7153381175730564312,        -0.69314919935580199262,       -0.67028301560314096363,       -0.64676190851412929117,
        -0.62260886020370775906,       -0.5978474702471787694,        -0.5725019326213811599,        -0.54659701206509414551,
        -0.52015801988176302295,       -0.49321078920819094726,       -0.46578164977335806407,       -0.43789740217203149131,
        -0.40958529167830154405,       -0.38087298162462995821,       -0.35178852637242169488,       -0.32236034390052914489,
        -0.2926171880384719759,        -0.26258812037150347507,       -0.23230248184497395703,       -0.20178986409573598793,
        -0.17108008053860326858,       -0.14020313723611396717,       -0.10918920358006110882,       -0.078068582813436634971,
        -0.046871682421591634449,      -0.015628984421543083705,      0.015628984421543083705,       0.046871682421591634449,
        0.078068582813436634971,       0.10918920358006110882,        0.14020313723611396717,        0.17108008053860326858,
        0.20178986409573598793,        0.23230248184497395703,        0.26258812037150347507,        0.2926171880384719759,
        0.32236034390052914489,        0.35178852637242169488,        0.38087298162462995821,        0.40958529167830154405,
        0.43789740217203149131,        0.46578164977335806407,        0.49321078920819094726,        0.52015801988176302295,
        0.54659701206509414551,        0.5725019326213811599,         0.5978474702471787694,         0.62260886020370775906,
        0.64676190851412929117,        0.67028301560314096363,        0.69314919935580199262,        0.7153381175730564312,
        0.736828089802020747,          0.75759811851970715324,        0.77762790964949546346,        0.79689789239031449952,
        0.81538923833917620332,        0.8330838798884008245,         0.84996452787959131392,        0.86601468849716467524,
        0.88121867938501841078,        0.89556164497072698882,        0.90902957098252967771,        0.92160929814533398829,
        0.93328853504307951461,        0.94405587013625602566,        0.95390078292549174144,        0.96281365425581555417,
        0.97078577576370628233,        0.97780935848691830081,        0.98387754070605704104,        0.98898439524299175396,
        0.99312493703744342266,        0.99629513473312514282,        0.99849195063959583774,        0.99971372677344128022,
    };

    static constexpr double ci[] =
    {
        0.0000000000000000000000,
        0.00073463449050567173823,     0.0017093926535181051991,      0.002683925371553482625,       0.0036559612013263753823,
        0.0046244500634221195542,      0.005588428003865514998,       0.0065469484508453223634,      0.0074990732554647112143,
        0.0084438714696689707445,      0.0093804196536944571549,      0.010307802574868969725,       0.011225114023185977022,
        0.012131457662979497694,       0.013025947892971541722,       0.013907710703718772702,       0.014775884527441301985,
        0.015629621077546004132,       0.016468086176145212834,       0.01729046056832358319,        0.01809594072212811558,
        0.018883739613374903032,       0.019653087494435304838,       0.020403232646209433498,       0.021133442112527642265,
        0.021843002416247387504,       0.022531220256336272761,       0.023197423185254122807,       0.02384096026596820711,
        0.024461202707957051955,       0.025057544481579589746,       0.025629402910208116256,       0.026176219239545675532,
        0.026697459183570963759,       0.027192613446576880537,       0.027661198220792389268,       0.028102755659101173041,
        0.028516854322395097932,       0.028903089601125201563,       0.029261084110638276023,       0.029590488059912642416,
        0.029890979593332832331,       0.03016226510516914483,        0.030404079526454821231,       0.030616186583980447428,
        0.030798379031152591651,       0.030950478850490986682,       0.0310723374275665154,         0.031163835696209906539,
        0.031224884254849358256,       0.031255423453863354077,       0.031255423453863354077,       0.031224884254849358256,
        0.031163835696209906539,       0.0310723374275665154,         0.030950478850490986682,       0.030798379031152591651,
        0.030616186583980447428,       0.030404079526454821231,       0.03016226510516914483,        0.029890979593332832331,
        0.029590488059912642416,       0.029261084110638276023,       0.028903089601125201563,       0.028516854322395097932,
        0.028102755659101173041,       0.027661198220792389268,       0.027192613446576880537,       0.026697459183570963759,
        0.026176219239545675532,       0.025629402910208116256,       0.025057544481579589746,       0.024461202707957051955,
        0.02384096026596820711,        0.023197423185254122807,       0.022531220256336272761,       0.021843002416247387504,
        0.021133442112527642265,       0.020403232646209433498,       0.019653087494435304838,       0.018883739613374903032,
        0.01809594072212811558,        0.01729046056832358319,        0.016468086176145212834,       0.015629621077546004132,
        0.014775884527441301985,       0.013907710703718772702,       0.013025947892971541722,       0.012131457662979497694,
        0.011225114023185977022,       0.010307802574868969725,       0.0093804196536944571549,      0.0084438714696689707445,
        0.0074990732554647112143,      0.0065469484508453223634,      0.005588428003865514998,       0.0046244500634221195542,
        0.0036559612013263753823,      0.002683925371553482625,       0.0017093926535181051991,      0.00073463449050567162981,
    };

    struct fixed_Gauss
    {
        static double integral(Function const& integrand, double const lower, double const upper)
        {
            struct Kahan 
            {
                double& var;
                double excess; //error

                static Kahan focus(double& variable)
                {
                    return Kahan { variable, 0.0 };
                }

                // var_{k+1} = var_{k} + b_{k}
                void add(double val)
                {
                    double added = val - excess;
                    double kahan_sum = var + added;
                    excess = (kahan_sum - var) - added;
                    var = kahan_sum;
                }
            };

            if (equal(lower, upper))
                return 0;
            if (lower > upper)
                return -integral(integrand, upper, lower);

            constexpr int precision = 8;
            auto f = [&](double x) -> double { return (lower < x && x < upper)? integrand(x) : (0); };
            constexpr double interval = pow(2, -precision);
            unsigned long long count = (upper - lower) * pow(2, precision);

            // Major Sum
            double scale = interval * 0.5;
            double midpoint;

            double Sum = 0;
            Kahan kSum = Kahan::focus(Sum);
            for (unsigned long long k = 0; k < count; k++)
            {
                midpoint = lower + (k + 0.5) * interval;
                
                for (int i = 1; i <= 100; i++)
                {
                    //Sum += ci[i] * f(scale * xi[i] + midpoint);
                    kSum.add(ci[i] * f(scale * xi[i] + midpoint));
                }
            }
            Sum = scale * Sum;


            // Minor Sum
            scale = (upper - (lower + count * interval)) * 0.5;
            midpoint = (upper + (lower + count * interval)) * 0.5;

            double MinorSum = 0;
            Kahan kMinorSum = Kahan::focus(MinorSum);
            for (int i = 1; i <= 100; i++)
            {
                //MinorSum += ci[i] * f(scale * xi[i] + midpoint);
                kMinorSum.add(ci[i] * f(scale * xi[i] + midpoint));
            }
            MinorSum = scale * MinorSum;

            return Sum + MinorSum;
        }
    };

    struct adaptive_Gauss
    {
        struct Kahan 
        {
            double& var;
            double error;

            static Kahan focus(double& variable, double prev_error)
            {
                return Kahan { variable, prev_error };
            }

            // var_{k+1} = var_{k} + b_{k}
            void add(double val)
            {
                double added = val - error;
                double kahan_sum = var + added;
                error = (kahan_sum - var) - added;
                var = kahan_sum;
            }
        };

        static double Gauss(Function const& f, double const lower, double const upper)
        {
            double scale = (upper - lower) * 0.5;
            double midpoint = (upper + lower) * 0.5;

            double Sum = 0;
            Kahan kSum = Kahan::focus(Sum, 0);
            for (int i = 1; i <= 100; i++)
            {
                kSum.add(ci[i] * f(scale * xi[i] + midpoint));
            }

            return scale * Sum;
        }

        static double integral_old(Function const& f, double const lower, double const upper)
        {
            struct helper
            {
                static double adapt(Function const& f, double Sum, double Sum_left, double Sum_right, double lower, double upper, int adapts)
                {
                    if (abs(Sum - (Sum_left + Sum_right)) > Constant::epsilon && adapts > 0)
                    {
                        double midpoint = (lower + upper) * 0.5;
                        bool refine_left = false;
                        bool refine_right = false;

                        double left_midpoint = lower * 0.75 + upper * 0.25;
                        double Sum_left_left = Gauss(f, lower, left_midpoint);
                        double Sum_left_right = Gauss(f, left_midpoint, midpoint);
                        if (abs(Sum_left - (Sum_left_left + Sum_left_right)) > Constant::epsilon)
                        {
                            refine_left = true;
                        }

                        double right_midpoint = lower * 0.25 + upper * 0.75;
                        double Sum_right_left = Gauss(f, midpoint, right_midpoint);
                        double Sum_right_right = Gauss(f, right_midpoint, upper);
                        if (abs(Sum_right - (Sum_right_left + Sum_right_right)) > Constant::epsilon)
                        {
                            refine_right = true;
                        }

                        if (refine_left && refine_right)
                        {
                            Sum_left = adapt(f, Sum_left, Sum_left_left, Sum_left_right, lower, midpoint, adapts - 1);
                            Sum_right = adapt(f, Sum_right, Sum_right_left, Sum_right_right, midpoint, upper, adapts - 1);
                        }
                        else if (refine_left)
                        {
                            Sum_left = adapt(f, Sum_left, Sum_left_left, Sum_left_right, lower, midpoint, adapts);
                        }
                        else if (refine_right)
                        {
                            Sum_right = adapt(f, Sum_right, Sum_right_left, Sum_right_right, midpoint, upper, adapts);
                        }
                    }

                    return Sum_left + Sum_right;
                }
            };

            if (equal(lower, upper))
                return 0;
            if (lower > upper)
                return -integral_old(f, upper, lower);
            
            double Sum = Gauss(f, lower, upper);
            double midpoint = 0.5 * (lower + upper);
            double Sum_left = Gauss(f, lower, midpoint);
            double Sum_right = Gauss(f, midpoint, upper);

            return helper::adapt(f, Sum, Sum_left, Sum_right, lower, upper, 50); //tương đương max depth ~ 100
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////   UNSAFE MODE💣   ///////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        struct value_error 
        {
            double value;
            double error; 
        };

        static value_error Gauss(Function const& f, double lower, double upper, double prev_error)
        {
            double scale = (upper - lower) * 0.5;
            double midpoint = (upper + lower) * 0.5;

            double Sum = 0;
            Kahan kSum = Kahan::focus(Sum, prev_error);
            for (int i = 1; i <= 100; i++)
            {
                kSum.add(scale * ci[i] * f(scale * xi[i] + midpoint));
            }

            return { Sum, kSum.error };
        }

        static double integral_unsafe(Function const& f, double lower, double upper)
        {
            struct helper
            {
                static double adapt(Function const& f, double Sum, double Sum_left, double Sum_right, double const (&prev_error_left_right)[2], double lower, double upper, int adapts)
                {
                    if (abs(Sum - (Sum_left + Sum_right)) > Constant::epsilon && adapts > 0)
                    {
                        double midpoint = (lower + upper) * 0.5;
                        bool refine_left = false;
                        bool refine_right = false;

                        double left_midpoint = lower * 0.75 + upper * 0.25;
                        value_error Sum_left_left = Gauss(f, lower, left_midpoint, prev_error_left_right[0]);
                        value_error Sum_left_right = Gauss(f, left_midpoint, midpoint, Sum_left_left.error);
                        if (abs(Sum_left - (Sum_left_left.value + Sum_left_right.value)) > Constant::epsilon)
                        {
                            refine_left = true;
                        }

                        double right_midpoint = lower * 0.25 + upper * 0.75;
                        value_error Sum_right_left = Gauss(f, midpoint, right_midpoint, prev_error_left_right[1]);
                        value_error Sum_right_right = Gauss(f, right_midpoint, upper, Sum_right_left.error);
                        if (abs(Sum_right - (Sum_right_left.value + Sum_right_right.value)) > Constant::epsilon)
                        {
                            refine_right = true;
                        }

                        if (refine_left && refine_right)
                        {
                            Sum_left = adapt(f, Sum_left, Sum_left_left.value, Sum_left_right.value, { prev_error_left_right[0], Sum_left_left.error }, lower, midpoint, adapts - 1);
                            Sum_right = adapt(f, Sum_right, Sum_right_left.value, Sum_right_right.value, { prev_error_left_right[1], Sum_right_left.error }, midpoint, upper, adapts - 1);
                        }
                        else if (refine_left)
                        {
                            Sum_left = adapt(f, Sum_left, Sum_left_left.value, Sum_left_right.value, { prev_error_left_right[0], Sum_left_left.error }, lower, midpoint, adapts);
                        }
                        else if (refine_right)
                        {
                            Sum_right = adapt(f, Sum_right, Sum_right_left.value, Sum_right_right.value, { prev_error_left_right[1], Sum_right_left.error }, midpoint, upper, adapts);
                        }
                        else
                        {
                            Sum_left = Sum_left_left.value + Sum_left_right.value;
                            Sum_right = Sum_right_left.value + Sum_right_right.value;
                        }
                    }

                    return Sum_left + Sum_right;
                }
            };

            if (equal(lower, upper))
                return 0;
            if (lower > upper)
                return -integral_unsafe(f, upper, lower);
            
            value_error Sum = Gauss(f, lower, upper, 0);
            double midpoint = 0.5 * (lower + upper);
            value_error Sum_left = Gauss(f, lower, midpoint, 0);
            value_error Sum_right = Gauss(f, midpoint, upper, Sum_left.error);

            return helper::adapt(f, Sum.value, Sum_left.value, Sum_right.value, { 0, Sum_left.error }, lower, upper, 50);
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////   SAFE MODE   ///////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        static double integral_safe(Function const& f, double lower, double upper)
        {
            struct helper
            {                
                static double adapt(Function const& f, double Sum, double Sum_left, double Sum_right, double const (&prev_error_left_right)[2], double lower, double upper, int adapts)
                {
                    double interval_inv = 1 / (upper - lower);
                    double NewSum = Sum_left + Sum_right;

                    if (abs(NewSum - Sum) > Constant::epsilon * abs(0.5 * (Sum + NewSum) * interval_inv) && adapts > 0)
                    {
                        double midpoint = (lower + upper) * 0.5;
                        bool refine_left = false;
                        bool refine_right = false;

                        double left_midpoint = lower * 0.75 + upper * 0.25;
                        value_error Sum_left_left = Gauss(f, lower, left_midpoint, prev_error_left_right[0]);
                        value_error Sum_left_right = Gauss(f, left_midpoint, midpoint, Sum_left_left.error);
                        double NewSum_left = Sum_left_left.value + Sum_left_right.value;
                        double mean_Sum_left = (1.0 / 3.0) * (0.5 * Sum + Sum_left + NewSum_left);
                        if (abs(0.5 * Sum - mean_Sum_left) + abs(Sum_left - mean_Sum_left) + abs(NewSum_left - mean_Sum_left) > Constant::epsilon * abs(2 * mean_Sum_left * interval_inv))
                        {
                            refine_left = true;
                        }

                        double right_midpoint = lower * 0.25 + upper * 0.75;
                        value_error Sum_right_left = Gauss(f, midpoint, right_midpoint, prev_error_left_right[1]);
                        value_error Sum_right_right = Gauss(f, right_midpoint, upper, Sum_right_left.error);
                        double NewSum_right = Sum_right_left.value + Sum_right_right.value;
                        double mean_Sum_right = (1.0 / 3.0) * (0.5 * Sum + Sum_right + NewSum_right);
                        if (abs(0.5 * Sum - mean_Sum_right) + abs(Sum_right - mean_Sum_right) + abs(NewSum_right - mean_Sum_right) > Constant::epsilon * abs(2 * mean_Sum_right * interval_inv))
                        {
                            refine_right = true;
                        }

                        if (refine_left && refine_right)
                        {
                            Sum_left = adapt(f, Sum_left, Sum_left_left.value, Sum_left_right.value, { prev_error_left_right[0], Sum_left_left.error }, lower, midpoint, adapts - 1);
                            Sum_right = adapt(f, Sum_right, Sum_right_left.value, Sum_right_right.value, { prev_error_left_right[1], Sum_right_left.error }, midpoint, upper, adapts - 1);
                        }
                        else if (refine_left)
                        {
                            Sum_left = adapt(f, Sum_left, Sum_left_left.value, Sum_left_right.value, { prev_error_left_right[0], Sum_left_left.error }, lower, midpoint, adapts);
                        }
                        else if (refine_right)
                        {
                            Sum_right = adapt(f, Sum_right, Sum_right_left.value, Sum_right_right.value, { prev_error_left_right[1], Sum_right_left.error }, midpoint, upper, adapts);
                        }
                        else
                        {
                            Sum_left = NewSum_left;
                            Sum_right = NewSum_right;
                        }
                    }

                    return Sum_left + Sum_right;
                }
            };

            if (equal(lower, upper))
                return 0;
            if (lower > upper)
                return -integral_safe(f, upper, lower);
            
            value_error Sum = Gauss(f, lower, upper, 0);
            double midpoint = 0.5 * (lower + upper);
            value_error Sum_left = Gauss(f, lower, midpoint, 0);
            value_error Sum_right = Gauss(f, midpoint, upper, Sum_left.error);

            return helper::adapt(f, Sum.value, Sum_left.value, Sum_right.value, { 0, Sum_left.error }, lower, upper, 50);
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////   DEFAULT MODE   //////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        static double Gauss_abs(Function const& f, double lower, double upper)
        {
            double scale = (upper - lower) * 0.5;
            double midpoint = (upper + lower) * 0.5;

            double Sum = 0;
            for (int i = 1; i <= 100; i++)
            {
                Sum += ci[i] * abs(f(scale * xi[i] + midpoint));
            }
            return scale * Sum;
        }

        struct Lam
        {
            Function const& f;
            double a_inv;
            double lower;
            double upper;

            double operator() (double x) const
            {
                if (x <= lower) x = lower + Constant::epsilon;
                if (x >= upper) x = upper - Constant::epsilon;
                return a_inv * f(x);
            }
        };

        static value_error Gauss_lam(Lam const& f, double lower, double upper, double prev_error)
        {
            double scale = (upper - lower) * 0.5;
            double midpoint = (upper + lower) * 0.5;

            double Sum = 0;
            Kahan kSum = Kahan::focus(Sum, prev_error);
            for (int i = 1; i <= 100; i++)
            {
                kSum.add(scale * ci[i] * f(scale * xi[i] + midpoint));
            }

            return { Sum, kSum.error };
        }

        static double integral_default(Function const& f, double lower, double upper)
        {
            struct helper
            {
                struct pair
                {
                    double left;
                    double right;
                };

                static double adapt_unsafe(Lam const& f, double Sum, 
                    double Sum_left, double Sum_right, 
                    pair prev_error, 
                    double lower, double upper, int& count
                )
                {
                    if (abs(Sum - (Sum_left + Sum_right)) > Constant::epsilon)
                    {
                        double midpoint = (lower + upper) * 0.5;
                        bool refine_left = false;
                        bool refine_right = false;

                        double left_midpoint = lower * 0.75 + upper * 0.25;
                        value_error Sum_left_left = Gauss_lam(f, lower, left_midpoint, prev_error.left);
                        value_error Sum_left_right = Gauss_lam(f, left_midpoint, midpoint, Sum_left_left.error);
                        if (abs(Sum_left - (Sum_left_left.value + Sum_left_right.value)) > Constant::epsilon)
                        {
                            refine_left = true;
                        }

                        double right_midpoint = lower * 0.25 + upper * 0.75;
                        value_error Sum_right_left = Gauss_lam(f, midpoint, right_midpoint, prev_error.right);
                        value_error Sum_right_right = Gauss_lam(f, right_midpoint, upper, Sum_right_left.error);
                        if (abs(Sum_right - (Sum_right_left.value + Sum_right_right.value)) > Constant::epsilon)
                        {
                            refine_right = true;
                        }
                        count++;

                        if (refine_left && refine_right && count < 200)
                        {
                            Sum_left = adapt_unsafe(f, Sum_left, 
                                Sum_left_left.value, Sum_left_right.value, 
                                { prev_error.left, Sum_left_left.error }, 
                                lower, midpoint, count
                            );

                            Sum_right = adapt_unsafe(f, Sum_right, 
                                Sum_right_left.value, Sum_right_right.value, 
                                { prev_error.right, Sum_right_left.error }, 
                                midpoint, upper, count
                            );
                        }
                        else if (refine_left && count < 200)
                        {
                            Sum_left = adapt_unsafe(f, Sum_left, 
                                Sum_left_left.value, Sum_left_right.value, 
                                { prev_error.left, Sum_left_left.error }, 
                                lower, midpoint, count
                            );
                        }
                        else if (refine_right && count < 200)
                        {
                            Sum_right = adapt_unsafe(f, Sum_right, 
                                Sum_right_left.value, Sum_right_right.value, 
                                { prev_error.right, Sum_right_left.error }, 
                                midpoint, upper, count
                            );
                        }
                        else
                        {
                            Sum_left = Sum_left_left.value + Sum_left_right.value;
                            Sum_right = Sum_right_left.value + Sum_right_right.value;
                        }
                    }

                    return Sum_left + Sum_right;
                }
            };

            if (equal(lower, upper))
                return 0;
            if (lower > upper)
                return -integral_default(f, upper, lower);

            double midpoint = 0.5 * (lower + upper);
            double scale = Gauss_abs(f, lower, upper);

            if (scale == 0 || scale - scale != 0)
                return scale;

            int count = 0;
            Lam func = { f, 1 / scale, lower - Constant::epsilon, upper + Constant::epsilon};
            value_error Sum = Gauss_lam(func, lower, upper, 0);
            value_error Sum_left = Gauss_lam(func, lower, midpoint, 0);
            value_error Sum_right = Gauss_lam(func, midpoint, upper, Sum_left.error);
            double result = scale * helper::adapt_unsafe(func, Sum.value, 
                Sum_left.value, Sum_right.value, 
                { 0, Sum_left.error }, 
                lower, upper, count
            );
            
            if (result - result == 0)
                return result;

            count = 0;
            func.lower = lower;
            func.upper = upper;
            Sum = Gauss_lam(func, lower, upper, 0);
            Sum_left = Gauss_lam(func, lower, midpoint, 0);
            Sum_right = Gauss_lam(func, midpoint, upper, Sum_left.error);
            return scale * helper::adapt_unsafe(func, Sum.value, 
                Sum_left.value, Sum_right.value, 
                { 0, Sum_left.error }, 
                lower, upper, count
            );
        }
    };

    switch (MODE)
    {
        case integral_MODE::fixed_Newton_Cote:
        return fixed_Newton_Cote::integral(f, lower, upper);
        
        case integral_MODE::fixed_Gauss:
        return fixed_Gauss::integral(f, lower, upper);

        case integral_MODE::adaptive_Gauss_unsafe:
        return adaptive_Gauss::integral_unsafe(f, lower, upper);

        case integral_MODE::adaptive_Gauss_safe:
        return adaptive_Gauss::integral_safe(f, lower, upper);

        case integral_MODE::Default:
        return adaptive_Gauss::integral_default(f, lower, upper);
    }
}

#pragma endregion
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

_MNC_END

#undef _MNC_BEGIN
#undef _MNC_END