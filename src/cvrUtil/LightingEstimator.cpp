#include <cvrUtil/LightingEstimator.h>
#include <math.h>
#include <cvrUtil/ARCoreManager.h>
#include <cvrUtil/AndroidStdio.h>

using namespace cvr;
using namespace std;
LightingEstimator* LightingEstimator::_myPtr = nullptr;
LightingEstimator* LightingEstimator::instance() {
    if(!_myPtr) _myPtr = new LightingEstimator;
    return _myPtr;
}

float LightingEstimator::plmvalreal(int l, int m, float i)
{
    //assert(l >= 0 && l <= maxl && m >= -l && m <= l);
    int mindex = m; if (m < 0) mindex *= -1;
    int iint = (int)i;
    float fraci = i - iint;
    float t;
    //assert(i >= 0 && i <= maxtheta - 1);
    if (fabs(fraci)<1.0e-4)		t = plm[l][mindex][iint];
    else						t = fraci*plm[l][mindex][iint + 1] + (1 - fraci)*plm[l][mindex][iint];
    if (m != 0) t *= rt2;
    return t;
}

float LightingEstimator::arrayval_theta(float fn[maxsize], float index)
{
    int a, b;
    a = (int)index; b = a + 1;
    if (b == _size_x) b = _size_x - 1;
    //assert(a >= 0 && a < _size_x && b >= 0 && b < _size_x);
    float frac = index - a;
    return frac * fn[b] + (1 - frac)*fn[a];
}


float LightingEstimator::arrayval_phi(int channel, int ii, float index)
{
    int a, b;
    a = (int)index;
    b = a + 1;
    b = b % _size_y;

    //assert(a >= 0 && a < _size_y && b >= 0 && b < _size_y);
    float frac = index - a;
    return frac * DATA[ch*(_size_y*ii + b) + 2 - channel] + (1 - frac)* DATA[ch*(_size_y*ii + a) + 2 - channel];
}
float LightingEstimator::expreal(int m, float j)
{
    int iint = (int)j;
    float fraci = j - iint;
    float t;
    //  assert(j >= 0 && j <= maxphi - 1) ;
    //assert(j >= 0 && j <= maxphi);
    if (fabs(fraci)<1.0e-4)
    {
        if (m >= 0) t = expphim[iint][m + maxl].real();
        else		t = expphim[iint][-m + maxl].imag();
    }
    else {
        if (m >= 0) t = (1 - fraci)*expphim[iint][m + maxl].real() + fraci *expphim[(iint + 1) % maxphi][m + maxl].real();
        else		t = (1 - fraci)*expphim[iint][-m + maxl].imag() + fraci *expphim[(iint + 1) % maxphi][-m + maxl].imag();
    }
    return t;
}
float LightingEstimator::integratephi(int channel, int m, int ii)
{

    float retval = 0;
    float mulfac = 2.0*M_PI / maxphi;

    for (int i = 0; i < maxphi; i++)
    {
        float iposn = (float)i * (float)_size_y / (float)maxphi;
        float simpsons = 1.0;
        if (simpsonflag)
        {
            if (i == 0) simpsons = 1.0 / 3.0;
            else if (i == maxphi - 1) simpsons = 1.0;
            else if (i == maxphi - 2) simpsons = 4.0 / 3.0;
            else if (i % 2 == 0) simpsons = 2.0 / 3.0;
            else simpsons = 4.0 / 3.0;
        }
        retval += expreal(m, i)*mulfac*arrayval_phi(channel, ii, iposn)*simpsons;
    }

    return retval;
}

float LightingEstimator::integratetheta(int l, int m, float fn[maxsize])
{
    float retval = 0;
    float mulfac = M_PI / maxtheta;
    float iposn, simpsons;
    for (int i = 0; i < maxtheta; i++)
    {
        iposn = (float)i * (float)_size_x / (float)maxtheta;
        simpsons = 1.0;
        if (simpsonflag)
        {
            if (i == 0) simpsons = 1.0 / 3.0;
            else if (i == maxtheta - 1) simpsons = 1.0;
            else if (i == maxtheta - 2) simpsons = 4.0 / 3.0;
            else if (i % 2 == 0) simpsons = 2.0 / 3.0;
            else simpsons = 4.0 / 3.0;
        }
        retval += arrayval_theta(fn, iposn)*sintheta[i] * mulfac*plmvalreal(l, m, i)*simpsons;
    }
    return retval;
}

void LightingEstimator::findcoeffslm_async()
{
    float SCALE = 0.03;
    float fnt[ch][2 * maxl + 1][maxtheta];
    int l;
    int PerThread = _size_x / NThreads;
    vector<future<void>> futures;
    for (unsigned int iTh = 0; iTh < NThreads; ++iTh)
    {
        future<void> result(async([this, iTh, &PerThread, &fnt]()
                                  {
                                      int end = (iTh + 1) * PerThread;
                                      if (iTh == NThreads - 1) end = _size_x;
                                      for (int i = iTh * PerThread; i < end; i++)
                                          for (int channel = 0; channel < ch; channel++)
                                              for (int m = -maxl; m <= maxl; m++)
                                                  fnt[channel][m + maxl][i] = integratephi(channel, m, i);
                                  }
        ));

        futures.push_back(move(result));
    }
    for (vector<future<void>>::iterator it = futures.begin(); it < futures.end(); it++)
    it->get();
    for (int channel = 0; channel < ch; channel++)
        for (l = 0; l <= maxl; l++)
            for (int m = -l; m <= l; m++)
                lightcoeffs[channel][l][m + maxl] = SCALE * integratetheta(l, m, fnt[channel][m + maxl]);
}

float* LightingEstimator::getSHLightingParams(osg::Image* image){
    image->scaleImage(32,64,image->r());
    DATA = (float*) image->data();
    _size_x = image->t(); _size_y = image->s();
    LOGE("===size: %d, %d", _size_x, _size_y);
    findcoeffslm_async();
    float back[]={LSR00*lightcoeffs[0][0][2], LSG00*lightcoeffs[1][0][2], LSB00*lightcoeffs[2][0][2], //L00
    LSR*lightcoeffs[0][1][1], LSG*lightcoeffs[1][1][1], LSB*lightcoeffs[2][1][1], //L1m1
    LSR*lightcoeffs[0][1][2], LSG*lightcoeffs[1][1][2], LSB*lightcoeffs[2][1][2], //L10
    LSR*lightcoeffs[0][1][3], LSG*lightcoeffs[1][1][3], LSB*lightcoeffs[2][1][3], //L11
    LSR*lightcoeffs[0][2][0], LSG*lightcoeffs[1][2][0], LSB*lightcoeffs[2][2][0], //L2m2
    LSR*lightcoeffs[0][2][1], LSG*lightcoeffs[1][2][1], LSB*lightcoeffs[2][2][1], //L2m1
    LSR*lightcoeffs[0][2][2], LSG*lightcoeffs[1][2][2], LSB*lightcoeffs[2][2][2], //L20
    LSR*lightcoeffs[0][2][3], LSG*lightcoeffs[1][2][3], LSB*lightcoeffs[2][2][3], //L21
    LSR*lightcoeffs[0][2][4], LSG*lightcoeffs[1][2][4], LSB*lightcoeffs[2][2][4]};
    return &back[0];
}

float* LightingEstimator::getSHLightingParams(){
    float back[]={LSR00*lightcoeffs[0][0][2], LSG00*lightcoeffs[1][0][2], LSB00*lightcoeffs[2][0][2], //L00
                  LSR*lightcoeffs[0][1][1], LSG*lightcoeffs[1][1][1], LSB*lightcoeffs[2][1][1], //L1m1
                  LSR*lightcoeffs[0][1][2], LSG*lightcoeffs[1][1][2], LSB*lightcoeffs[2][1][2], //L10
                  LSR*lightcoeffs[0][1][3], LSG*lightcoeffs[1][1][3], LSB*lightcoeffs[2][1][3], //L11
                  LSR*lightcoeffs[0][2][0], LSG*lightcoeffs[1][2][0], LSB*lightcoeffs[2][2][0], //L2m2
                  LSR*lightcoeffs[0][2][1], LSG*lightcoeffs[1][2][1], LSB*lightcoeffs[2][2][1], //L2m1
                  LSR*lightcoeffs[0][2][2], LSG*lightcoeffs[1][2][2], LSB*lightcoeffs[2][2][2], //L20
                  LSR*lightcoeffs[0][2][3], LSG*lightcoeffs[1][2][3], LSB*lightcoeffs[2][2][3], //L21
                  LSR*lightcoeffs[0][2][4], LSG*lightcoeffs[1][2][4], LSB*lightcoeffs[2][2][4]};
    return &back[0];
}