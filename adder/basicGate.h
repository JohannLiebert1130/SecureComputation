#include "FHE.h"
using namespace NTL;

class BasicGate
{
private:
    Ctxt _v_ones;

public:
    BasicGate(Ctxt v_ones) : _v_ones(v_ones){}

    void AND(Ctxt &c1, Ctxt &c2)
    {
        c1.multiplyBy(c2);
    }
    void XOR(Ctxt &c1, Ctxt &c2)
    {
        c1 += c2;
    }
    void NOT(Ctxt &c)
    {
        c += _v_ones;
    }
    void OR(Ctxt &c1, Ctxt &c2)
    {
        NOT(c1);
        
        Ctxt temp(c2);
        NOT(temp);

        AND(c1, temp);
        NOT(c1);
    }
};