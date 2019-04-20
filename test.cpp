#include <iostream>
#include <ctime>
#include<sys/time.h>
#include "combGate.h"
using namespace std;


// Simple class to measure time for each method
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

void initVector(vector<long>& v)
{
  for(int i = 0; i < v.size(); i++)
    v[i] = rand() % 2;
}


int main()
{
  long m = 0;                   // Specific modulus
	long p = 2;                   // Plaintext base [default=2], should be a prime number
	long r = 1;                   // Lifting [default=1]
	long L = 400;                 // Number of levels in the modulus chain [default=heuristic]
	long c = 3;                   // Number of columns in key-switching matrix [default=2]
	long w = 64;                  // Hamming weight of secret key
	long d = 1;                   // Degree of the field extension [default=1]
	long k = 80;                  // Security parameter [default=80] 
  long s = 1024;                   // Minimum number of slots [default=0]

    
    m = FindM(k, L, c, p, d, s, 0);

    std::cout << "Initializing context... " << std::flush;
	FHEcontext context(m, p, r); 	          // Initialize context
	buildModChain(context, L, c);             // Modify the context, adding primes to the modulus chain
	std::cout << "OK!" << std::endl;
    cout<<"securitylevel="<<context.securityLevel()<<endl;

	std::cout << "Generating keys... " << std::flush;
	FHESecKey secretKey(context);                    // Construct a secret key structure
	const FHEPubKey& publicKey = secretKey;                 // An "upcast": FHESecKey is a subclass of FHEPubKey
	secretKey.GenSecKey(w);                          // Actually generate a secret key with Hamming weight
	addSome1DMatrices(secretKey);                    // Extra information for relinearization

    ZZX G =  context.alMod.getFactorsOverZZ()[0]; 
    EncryptedArray ea(context, G);

    cout << "m: " << m << endl;
    cout << "nslots: " << ea.size() << endl;   

    int num = 1024;
    vector<long> v1(num), v2(num);
    initVector(v1);
    initVector(v2);
    v1.resize(ea.size());
    v2.resize(ea.size());
    Ctxt encV1(publicKey), encV2(publicKey);
    ea.encrypt(encV1, publicKey, v1);
    ea.encrypt(encV2, publicKey, v2);

    vector<long> ones(ea.size(), 1);

    Ctxt encOnes(publicKey);
    ea.encrypt(encOnes, publicKey, ones);
    

    CombGate cb(encOnes, num);
    Timer timer;
    timer.start();
    cb.KSAdder(encV1, encV2);
    timer.stop();
    std::cout << "Time taken: " << timer.elapsed_time() << std::endl;

    vector<long> result;
    ea.decrypt(encV1, secretKey, result);
    for(int i = 0; i < num; i++)
      cout << result[i] << ' ';
    cout << endl;

    return 0;
}