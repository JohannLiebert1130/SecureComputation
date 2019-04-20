#include "basicGate.h"
#include "EncryptedArray.h"
using namespace std;

class CombGate
{
private:
    BasicGate _bg;
    int _size;

public:
    //CombGate(Ctxt v_ones, int size) : _bg(BasicGate(v_ones)), _size(size){}
    CombGate(Ctxt v_ones, int size) : _bg(BasicGate(v_ones)), _size(size){}
    void KSAdder(Ctxt &a, Ctxt &b)
    {
      Ctxt p(a), g(a);
      _bg.XOR(p, b);
      _bg.AND(g, b);
      Ctxt s = p;


      EncryptedArray ea(p.getContext());
      Ctxt p_(p.getPubKey()), g_(p.getPubKey());

      for(int i = 0; i < (int)log(_size); i++)
      {
        p_ = p, g_ = g;
        ea.shift(p_, -(int)pow(2, i));
        ea.shift(g_, -(int)pow(2, i));
        
        _bg.AND(g_, p);
        _bg.OR(g, g_);
        _bg.AND(p, p_);
      }

      
      ea.shift(g, -1);
      _bg.XOR(s, g);
      a = s;
    }
    
};
