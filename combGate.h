#include "basicGate.h"
#include "EncryptedArray.h"
#include "replicate.h"
#include <string>
#include <cmath>
using namespace std;
using BG = BasicGate;
class Timer
{
public:
    void start() { m_start = my_clock(); }
    void stop() { m_stop = my_clock(); }
    double elapsed_time() const {
        return m_stop - m_start;
    }

private:
    double m_start, m_stop;
    double my_clock() const {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }
};
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
    return result;
  }

  Ctxt Multiply2(Ctxt &a, Ctxt &b, FHESecKey secretKey)
  {
    EncryptedArray ea(a.getContext());
    Ctxt tempA(a.getPubKey()), b_i(b.getPubKey());

    vector<Ctxt> middleSums(_size, a);
    vector<long> plain(ea.size());
    for(int i= 0; i < _size; i++)
    {
      b_i = b;
      Timer timer1;
      timer1.start();
      replicate(ea, b_i, i);
      timer1.stop();
      std::cout << "Time taken for replicate: " << timer1.elapsed_time() << std::endl;

      middleSums[i].multiplyBy(b_i);
      ea.shift(middleSums[i], -(_size - i - 1));
    }

    int level = log(_size)/log(2), db = 1;
    cout << "level:" << level << endl;
    while(level > 0)
    {
      for(int i = 0; i < _size/(2*db); i++)
      {
        Timer timer;
        timer.start();
        middleSums[2*i*db] = KSAdder(middleSums[2*i*db], middleSums[(2*i+1)*db]);
        timer.stop();
        std::cout << "Time taken for ksadder: " << timer.elapsed_time() << std::endl;
      }
      db *= 2, level--;
    }
    return middleSums[0];
  }
  Ctxt Cond1(Ctxt b, Ctxt neg_b, Ctxt sign)
  {
    Ctxt part1 = b, part2 = neg_b;
    Timer m;
    m.start();
    part1.multiplyBy(sign);
    m.stop();
    std::cout << "Time taken for multiply: " << m.elapsed_time() << std::endl;
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
    Timer timer1;
    timer1.start();
    Ctxt neg_b = b;
    BG::NOT(neg_b);
    neg_b = KSAdder(neg_b, encOnes);

    EncryptedArray ea(a.getContext());
    Ctxt ai = a, bi = b;
    timer1.stop();
    std::cout << "Time taken: " << timer1.elapsed_time() << std::endl;

    Timer timer2;
    timer2.start();
    replicate(ea, ai, 0);
    replicate(ea, bi, 0);
    timer2.stop();
    std::cout << "Time taken for replicate: " << timer2.elapsed_time() << std::endl;

    Timer timer3;
    timer3.start();
    ai += bi;
    r = a;
    r = KSAdder(r, Cond1(b, neg_b, ai));
        cout << "yes2" << endl;

    timer3.stop();
    std::cout << "Time taken for middle add: " << timer3.elapsed_time() << std::endl;

    for(int i = 0; i < _size - 1; i++)
    {
      Timer timer;
      timer.start();
      ai = r, bi = b;
      replicate(ea, ai, 0);
      replicate(ea, bi, 0);
      ai.addCtxt(bi);
      q.addCtxt(Cond2(ai));

      ea.shift(r, -1);
      ea.shift(q, -1);
      cout << "yes6" << endl;
      r = KSAdder(r, Cond1(b, neg_b, ai));
      cout << "yes7" << endl;

      timer.stop();
      std::cout << "Time taken for  loop: " << timer.elapsed_time() << std::endl;
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
