#include <iostream>
#include <ctime>
#include <sys/time.h>
#include "combGate.h"
using namespace std;


// Simple class to measure time for each method
// class Timer
// {
// public:
//     void start() { m_start = my_clock(); }
//     void stop() { m_stop = my_clock(); }
//     double elapsed_time() const {
//         return m_stop - m_start;
//     }

// private:
//     double m_start, m_stop;
//     double my_clock() const {
//         struct timeval tv;
//         gettimeofday(&tv, NULL);
//         return tv.tv_sec + tv.tv_usec * 1e-6;
//     }
// };

void InitVector(vector<long>& v)
{
    for(int i = 0; i < v.size(); i++)
        v[i] = rand() % 2;
}

void PrintVector(const vector<long>& v)
{
    for(int i = 0; i < v.size(); i++)
        cout << v[i];
    cout << endl;
}
int main()
{
    long m = 0;                   // Specific modulus
	long p = 2;                   // Plaintext base [default=2], should be a prime number
	long r = 1;                   // Lifting [default=1]
	long L = 800;                 // Number of levels in the modulus chain [default=heuristic]
	long c = 3;                   // Number of columns in key-switching matrix [default=2]
	long w = 64;                  // Hamming weight of secret key
	long d = 1;                   // Degree of the field extension [default=1]
	long k = 80;                  // Security parameter [default=80] 
    long s = 0;                   // Minimum number of slots [default=0]

    
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

    int num = 4;
    vector<long> v1(num), v2(num);
    InitVector(v1);
    InitVector(v2);
    PrintVector(v1);
    PrintVector(v2);

    v1.resize(ea.size());
    v2.resize(ea.size());
    Ctxt encV1(publicKey), encV2(publicKey);
    ea.encrypt(encV1, publicKey, v1);
    ea.encrypt(encV2, publicKey, v2);

    vector<long> vOnes(ea.size(), 0);
    vector<long> vZeros(ea.size(), 0);
    vOnes[num - 1] = 1;
    Ctxt encZeros(publicKey), encOnes(publicKey);
    ea.encrypt(encZeros, publicKey, vZeros);
    ea.encrypt(encOnes, publicKey, vOnes);
     
    CombGate cb(num, encZeros, encOnes);
    vector<long> result(ea.size());

    Timer timer;
    timer.start();
    Ctxt total = cb.KSAdder(encV1, encV2);
    timer.stop();
    std::cout << "Time taken: " << timer.elapsed_time() << std::endl;
    ea.decrypt(total, secretKey, result);
    result.resize(num);
    PrintVector(result);

    result.resize(ea.size());
    Timer timer2;
    timer2.start();
    Ctxt enc = cb.Multiply(encV1, encV2);
    timer2.stop();
    std::cout << "Time taken: " << timer2.elapsed_time() << std::endl;
    ea.decrypt(enc, secretKey, result);
    cout << "ok" << endl; 
    result.resize(num);
    PrintVector(result);


    result.resize(ea.size());
    Timer timer4;
    timer4.start();
    enc = cb.Multiply2(encV1, encV2);
    timer4.stop();
    std::cout << "Time taken: " << timer4.elapsed_time() << std::endl;
    ea.decrypt(enc, secretKey, result);
    result.resize(num);
    PrintVector(result);

    result.resize(ea.size());
    Ctxt remainder(publicKey), quotient(publicKey);
    Timer timer3;
    timer3.start();
    cb.Divide(encV1, encV2, remainder, quotient);
    timer3.stop();
    std::cout << "Time taken: " << timer3.elapsed_time() << std::endl;
    ea.decrypt(quotient, secretKey, result);
    result.resize(num);
    PrintVector(result);
 
    return 0;
}
