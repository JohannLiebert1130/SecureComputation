#include "basicGate.h"
#include "EncryptedArray.h"
#include <string>
using namespace std;
using BG = BasicGate;

class CombGate
{
private:
  int _size;

public:
  CombGate(int size) : _size(size){}
  void KSAdder(Ctxt &a, Ctxt &b)
  {
    Ctxt p(a), g(a);
    BG::XOR(p, b);
    BG::AND(g, b);
    Ctxt s = p;

    EncryptedArray ea(p.getContext());
    Ctxt p_(p.getPubKey()), g_(p.getPubKey());

    for(int i = 0; i < (int)ceil(log(_size)); i++)
    {
      p_ = p, g_ = g;
      ea.shift(p_, -(int)pow(2, i));
      ea.shift(g_, -(int)pow(2, i));
      
      BG::AND(g_, p);
      BG::OR(g, g_);
      BG::AND(p, p_);
    }

    ea.shift(g, -1);
    
    BG::XOR(s, g);
    a = s;
  }
};