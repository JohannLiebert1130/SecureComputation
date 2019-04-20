#include "FHE.h"
using namespace NTL;

class BasicGate
{
public:
    static void AND(Ctxt &c1, Ctxt &c2)
    {
        c1.multiplyBy(c2);
    }
    static void XOR(Ctxt &c1, Ctxt &c2)
    {
        c1 += c2;
    }
    static void NOT(Ctxt &c)
    {
        c.addConstant(to_ZZX(1));
    }
    static void OR(Ctxt &c1, Ctxt &c2)
    {
        NOT(c1);
        
        Ctxt temp(c2);
        NOT(temp);

        AND(c1, temp);
        NOT(c1);
    }
};