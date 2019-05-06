#include "basicGate.h"
#include "EncryptedArray.h"
#include "replicate.h"
#include <string>
#include <cmath>
using namespace std;
using BG = BasicGate;

class CombGate
{
private:
  int _size;
  Ctxt encZeros;
  Ctxt encOnes;

public:
  CombGate(int size, Ctxt z, Ctxt o) : _size(size), encZeros(z), encOnes(o){}
  Ctxt KSAdder(Ctxt a, Ctxt b, string op="add")
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


  Ctxt Multiply(Ctxt &a, Ctxt &b)
  {
    EncryptedArray ea(a.getContext());
    Ctxt result(a.getPubKey()), tempA(a.getPubKey()), b_i(b.getPubKey());


    for(int i= 0; i < _size; i++)
    {
      tempA = a;
      b_i = b;
      replicate(ea, b_i, i);
      tempA.multiplyBy(b_i);
      ea.shift(tempA, -(_size - i - 1));
      result = KSAdder(result, tempA);
    }
  }

  Ctxt Cond1(Ctxt b, Ctxt neg_b, Ctxt sign)
  {
    Ctxt part1 = b, part2 = neg_b;
    part1.multiplyBy(sign);
    BG::NOT(sign);
    part2.multiplyBy(sign);
    Ctxt result = part1;
    result += part2;

    return result;
  }

  Ctxt Cond2(Ctxt sign)
  {
    Ctxt result = encOnes;
    BG::NOT(sign);
    result.multiplyBy(sign);
    
    return result;
  }

  void Divide(Ctxt &a, Ctxt &b, Ctxt& r, Ctxt& q)
  {
    Ctxt neg_b = b;
    BG::NOT(neg_b);

    EncryptedArray ea(a.getContext());
    Ctxt ai = a, bi = b;

    replicate(ea, ai, 0);
    replicate(ea, bi, 0);
    ai += bi;
    cout << "yes1" << endl;
    r = a;
    r = KSAdder(r, Cond1(b, neg_b, ai));
        cout << "yes2" << endl;

    for(int i = 0; i < _size - 1; i++)
    {
      ai = r, bi = b;
      replicate(ea, ai, 0);
      replicate(ea, bi, 0);
          cout << "yes3" << endl;

      ai.addCtxt(bi);
            cout << "yes4" << endl;

      q.addCtxt(Cond2(ai));
            cout << "yes5" << endl;

      ea.shift(r, -1);
      ea.shift(q, -1);
       cout << "yes6" << endl;
      r = KSAdder(r, Cond1(b, neg_b, ai));
       cout << "yes7" << endl;
    }
     

    ai = r, bi = b;
    replicate(ea, ai, 0);
    replicate(ea, bi, 0);
    ai += bi;
    q += Cond2(ai);
    ea.shift(r, -1);
    ea.shift(q, -1);
    q += Cond2(encZeros);
  }

};
