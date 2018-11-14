#ifndef LIGHTING_ESTIMATOR_H
#define LIGHTING_ESTIMATOR_H

// Asynchronous
#include <thread>
#include <future>

#include <vector>
#include <complex>
#include <osg/Image>

namespace cvr{
    class LightingEstimator{
    protected:
        static LightingEstimator* _myPtr;
//        float* SH_basis;
        float* DATA;
        const static int ch = 3; //CHANNELS
        int const NThreads = std::thread::hardware_concurrency();
        const static int maxl = 2;
        const static int maxtheta = 1000;
        const static int maxphi = 1000;
        const int simpsonflag = 0;

        const float LSR00 = 1.0; // Red Light Scale 00
        const float LSR = 1.0; // Red Light Scale all except 00
        const float LSG00 = 1.0; // Green Light Scale 00
        const float LSG = 1.0; // Green Light Scale all except 00
        const float LSB00 = 1.0; // Blue Light Scale 00
        const float LSB = 1.0; // Blue Light Scale all except 00

        float theta[maxtheta];
        float costheta[maxtheta];
        float sintheta[maxtheta];
        float sintheta2[maxtheta];
        float sinthetafull[maxtheta];
        std::complex<float> expphi[maxphi];
        std::complex<float> expphistar[maxphi];
        std::complex<float> expphim[maxphi][2 * maxl + 1];
        float cosphi[maxphi];
        float sinphi[maxphi];
        float phi[maxphi];
        float plm[maxl + 1][maxl + 1][maxtheta];

        const float rt2 = 1.4142136;

        const int _size = 1000;
        int _size_x = _size; // number of rows
        int _size_y = _size; // number of columns
        const static int maxsize = maxtheta;

        float floatfile[ch][maxsize][maxsize];
        float img[ch][maxsize][maxsize];
        float attenuation[maxl + 1];
//        dim3 lightcoeffs;
        float lightcoeffs[ch][maxl + 1][2 * maxl + 1];
//        float * lightcoeffs;

        float integratephi(int channel, int m, int ii);
        float integratetheta(int l, int m, float fn[maxsize]);
        float arrayval_phi(int channel, int ii, float index);
        float expreal(int m, float j);
        float arrayval_theta(float fn[maxsize], float index);
        float plmvalreal(int l, int m, float i);
        void findcoeffslm_async();
    public:
        static LightingEstimator* instance();
//        float* getSHBasis(osg::Vec3f, int band=4);
        float* getSHLightingParams(osg::Image* image);
        float* getSHLightingParams();
    };
}
#endif