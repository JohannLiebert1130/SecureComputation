#include "basicGate.h"
#include "EncryptedArray.h"
#include <string>
#include <cmath>
using namespace std;
using BG = BasicGate;

class CombGate
{
private:
  int _size;

public:
  CombGate(int size) : _size(size){}
  Ctxt KSAdder(Ctxt &a, Ctxt &b, string op="add")
  {
    Ctxt p(a), g(a);
    if(op=="substract")
    {
      Ctxt notB = b;
      BG::NOT(notB);
      BG::XOR(p, notB);
      BG::AND(g, notB);
    }
    else
    {
      BG::XOR(p, b);
      BG::AND(g, b);
    }
    Ctxt s = p;

    EncryptedArray ea(p.getContext());
    Ctxt p_(p.getPubKey()), g_(p.getPubKey());

    cout << log(_size)/log(2) << endl;
    for(int i = 0; i < (int)ceil(log(_size)/log(2)); i++)
    {
      p_ = p, g_ = g;
      ea.shift(p_, -(int)pow(2, i));
      ea.shift(g_, -(int)pow(2, i));
      
      BG::AND(g_, p);
      BG::OR(g, g_);
      BG::AND(p, p_);
    }

    ea.shift(g, -1);
    if(op=="substract")
    {
      vector<long> mask(ea.size(), 0);
      Ctxt encMask(p.getPubKey());
      mask[_size-1] = 1;
      ea.encrypt(encMask, p.getPubKey(), mask);
      BG::OR(g, encMask);
    }
      
    BG::XOR(s, g);
    return s;
  }
};
