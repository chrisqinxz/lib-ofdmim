/**
 ** Copyright (c) 2019 Saulo Jorge Beltrao de Queiroz
 ** Contact: Saulo Queiroz <ssaulojorge@gmail.com> <sauloqueiroz@utfpr.edu.br> <saulo@dei.uc.pt>
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 ** 
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 ** 
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software 
 ** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 **/

#include <immapper.h> 
//========================================================> OFDM IM CLASS
//Default constructor:
IMMapper::IMMapper(): g(1), N(LARGEST_SYMBOL), k(32), ofdmIMSymbol(NULL), p2(NULL), arrayI(NULL), arrayS(NULL)
{
  //the product in the multiplicative formula of C(N,k) produces very large numbers
  //the largest one that fits a 64-bit variable is C(64,32)
  assert(N <= LARGEST_SYMBOL);
  mlut = new MLUT(2);
  allocateOFDMIMSymbol(N);
  allocateKSizeArrays(k);
  //initializing p1 and array I of indexes
  p1 = 0;
  /* the following instructions ensure that the command 
   * indexSelector->*unrank(X, N, k, arrayI) will perform
   * unrank of X in[0,C(N,k)-1] and store results in arrayI
   * Similar to ranking.
   */
  indexSelector = new UnRankingAlgorithmsCallBack;
  unrank = &UnRankingAlgorithmsCallBack::optimalUnranking;
  rank = &UnRankingAlgorithmsCallBack::optimalRanking;
}

IMMapper::IMMapper(int _N): g(1), p2(NULL), arrayI(NULL), arrayS(NULL)
{
  //the product in the multiplicative formula of C(N,k) produces very large numbers
  //the largest one that fits a 64-bit variable is C(64,32)
  assert(N <= LARGEST_SYMBOL);
  N = _N;
  k = N / 2;
  p1 = 0;
  numberOfIMWaveforms = binomialCoefficient(N, k);
  mlut = new MLUT(2);
  allocateOFDMIMSymbol(N);
  allocateKSizeArrays(k);
  /* the following instructions ensure that the command 
   * indexSelector->*unrank(X, N, k, arrayI) will perform
   * unrank of X in[0,C(N,k)-1] and store results in arrayI
   * Similar to ranking.
   */
  indexSelector = new UnRankingAlgorithmsCallBack;
  unrank = &UnRankingAlgorithmsCallBack::optimalUnranking;
  rank = &UnRankingAlgorithmsCallBack::optimalRanking;
}

IMMapper::IMMapper(int _N, int _M): g(1), ofdmIMSymbol(NULL), p2(NULL), arrayI(NULL), arrayS(NULL)
{
  //the product in the multiplicative formula of C(N,k) produces very large numbers
  //the largest one that fits a 64-bit variable is C(64,32)
  assert(N <= LARGEST_SYMBOL);
  N = _N;
  k = N / 2;
  p1 = 0;
  numberOfIMWaveforms = binomialCoefficient(N, k);
  mlut = new MLUT(_M);
  allocateOFDMIMSymbol(N);
  allocateKSizeArrays(k);
  /* the following instructions ensure that the command 
   * indexSelector->*unrank(X, N, k, arrayI) will perform
   * unrank of X in[0,C(N,k)-1] and store results in arrayI
   * Similar to ranking.
   */
  indexSelector = new UnRankingAlgorithmsCallBack;
  unrank = &UnRankingAlgorithmsCallBack::optimalUnranking;
  rank = &UnRankingAlgorithmsCallBack::optimalRanking;
}

IMMapper::~IMMapper()
{
}

void IMMapper::allocateOFDMIMSymbol(int _N)
{
  N = _N;
  ofdmIMSymbol = (complex<double> *) realloc(ofdmIMSymbol, N * sizeof(complex<double>));
  assert(ofdmIMSymbol);
  TypeIndex j;
  for(j=0; j<N; j++)
    ofdmIMSymbol[j] = complex<double>(0.0,0.0); //subcarriers are deactivate unless IxS changes that!
  //after chaning N or k update number of waveforms
  numberOfIMWaveforms = binomialCoefficient(N, getk());
}

void IMMapper::allocateKSizeArrays(int _k)
{
  k = _k;
  int i;
  //allocating k-size array that stores indexes of active subcarriers
  arrayI = (TypeIndex *) realloc(arrayI, k * sizeof(TypeIndex));
  assert(arrayI);
  TypeIndex j;
  for(j=0; j<k; j++)
    arrayI[j] = j;

  //initializing k-size array of data for the active subcarriers. Each is an integer from 0 to M-1
  p2 = (TypeData *)realloc(p2, k * sizeof(TypeData));
  assert(p2);
  for(i=0; i<k; i++)
    p2[i] = 0; //= -1 bpsk

  //reinitializing symbol
  for(i=0; i<getN(); i++)
  { 
    ofdmIMSymbol[i] = complex<double> (0.0, 0.0);
  }
  //initializing array s of M-ary complex baseband samples for the p2 data
  arrayS = (complex<double> *)realloc(arrayS, k * sizeof(complex<double>));
  assert(arrayS);
  for(i=0; i<k; i++)
  {
     arrayS[i] =  mlut->map(p2[i]);
     ofdmIMSymbol[arrayI[i]] = arrayS[i];
  }
  //after chaning N or k update number of waveforms
  numberOfIMWaveforms = binomialCoefficient(getN(), k);
}

void IMMapper::createOFDMIMSymbol()
{
  assert(ofdmIMSymbol);
  assert(arrayI);
  assert(arrayS);
  int i;
  //iterates across all symbol indexes i
  //the ones in arrayI indicates active subcarriers
  for (i=0; i<N; i++)
    ofdmIMSymbol[i] = complex<double>(0.0,0.0); //subcarriers are deactivate unless IxS changes that!

  for (i=0; i<k; i++)  
    ofdmIMSymbol[arrayI[i]] = arrayS[i];  

}

void IMMapper::loadP1(TypeData data)
{
  //assert data is less than 2^floor((log2C(n,k)))-1
  //comment it for performance tests!
  //assert(data <  (1<< (unsigned long int)(log2( binomialCoefficient(getN(), getk()))))-1);
  p1 = data;
}


 
void IMMapper::map()
{
  assert(arrayI);//needed to store indexes
  mapP1();
  mapP2();
  createOFDMIMSymbol();
}


/*
 *  Demap OFDM-IM symbol with values currently stored in array ofdmIMSymbol.
 */
void IMMapper::demap()
{

  oracleOFDMIMDetector(); //populates arrayI of indexes of active subcarriers in ofdmIMSymbol
  demapP1();//updates variable p1 with C(arraI[0],1) +  C(arraI[1],2) + ... + C(arraI[k-1],k)
  demapP2();//demap the complex baseband data of every non-null subcarrier and stores in data array p2
}

/*
 * Iterates across the 'received' OFDM IM symbol.
 * Every time a non-null subcarrier is found in the index i
 * stores i in arrayI.
 * arrayI is indexed from 0 to k-1 (k elements).
*/
void IMMapper::oracleOFDMIMDetector()
{
  assert(arrayI && ofdmIMSymbol);//needed to store indexes
  int i;
  int ik = 0;
  for(i=0; i<getN(); i++)
  {
    if (ofdmIMSymbol[i] != complex<double>(0.0,0.0) )
    { //subcarrier of current index is active
       arrayI[ik] = i;
       ik++;
    }
  }
  //the number of active subcarriers must be k
  assert(ik == getk());
}

/*
 * Employs an user-defined ranking algorithm (depending on the IMMapper constructor)
 * to updated p1 from the values currently stored in arrayI, ie 
 * p1 = C(arraI[k-1],k) + ...+ C(arraI[0],1).
 * It is assumed that the OFDM-IM detector at the receiver populates the arrayI of active indexes
 */
void IMMapper::demapP1()
{
  assert(arrayI);
  //runs user-defined ranking algorithm
  p1 = (indexSelector->*rank) (getN(), getk(), arrayI);
}

/* Reads the k active subcarriers in the ofdm im symbol
 * and maps them into data. The result is stored in data
 * array p2.
 */
void IMMapper::demapP2()
{
  assert(arrayI && p2 && ofdmIMSymbol);
  int i;
  for (i=0; i<k; i++)
    p2[i] = mlut->demap(ofdmIMSymbol[arrayI[i]]);
}


/*
 * the ofdmIMSymbol is initialized with 0+0i by default
 * Therefore, running mapP1 determines which subcarriers remains mapped as null
 * Indexes of non null subcarriers are stored in arrayI.
 */
void IMMapper::mapP1()
{
  assert(arrayI);
  //the unranking algorithms starts from the largest bin. coef candidates to the lowest.
  //then, if P1 is small, the inner loop takes longer to find the first bin. coeff.
  //in this case, it is faster to find the combinadic dualCombP1 of the dual of P1 and, from it, to find
  //the combinadic of P1 with the formula  combP1[k-1-i] = _N-1 - dualCombP1[i], i in 0..k-1
  //see:https://www.developertyrone.com/blog/generating-the-mth-lexicographical-element-of-a-mathematical-combination/
  int _N = getN();
  int _k = getk();
  TypeData dualP1 = binomialCoefficient(_N,_k) - 1 - getP1();
 // if (getP1() >= dualP1)
  //{
     //unranking is a callback (pointer to method) whose value is defined by the IMMapper constructor
     (indexSelector->*unrank) (getP1(), _N, _k, arrayI);
  //}
/*  else
  {
    TypeIndex dualCombP1[_k];//combinadic of the dual of P1
    (indexSelector->*unrank) (dualP1, _N, _k, dualCombP1);
     int i;//, ik = _k - 1;
     for(i=0; i < _k; i++)
     {
        //arrayI[ik] = _N-1 - dualCombP1[i];
        arrayI[k-1-i] = _N-1 - dualCombP1[i]; 
        //ik--;
     }
  }*/
}

TypeData IMMapper::getP1()
{ 
  return p1;
}

int IMMapper::getk()
{
  return k;
}

int IMMapper::getN()
{
  return N;
}

void IMMapper::setN(int _N)
{
  allocateOFDMIMSymbol(_N);
}

void IMMapper::setk(int _k)
{
  allocateKSizeArrays(_k);
}

void IMMapper::setM(int _M)
{
  assert(mlut);
  mlut->setM(_M);
}

/*
 *  Callback pointers _unrank and _rank MUST refer to a method implemented within UnRankingAlgorithmsCallBack
 *  This method assumes that the indexSelector object (which is a pointer to the type UnRankingAlgorithmsCallBack)
 * has already been instanced (in constructor)
 */
void IMMapper::setIxSAlgorithm(void (UnRankingAlgorithmsCallBack::*_unrank)(TypeData, int, int, TypeIndex*), 
                               TypeData (UnRankingAlgorithmsCallBack::*_rank)(int, int, TypeIndex*))
{
  assert(_unrank && _rank && indexSelector);
  unrank = _unrank;
  rank   = _rank;
}



TypeData IMMapper::getNumberOfIMWaveforms()
{
  return numberOfIMWaveforms;
}

void IMMapper::loadP2()
{
  assert(p2);
  int i;
  srand(time(NULL));
  int M = mlut->getM();
  for (i=0; i<k; i++)
    p2[i] = rand() % M;
}
//user-defined data for modulation of active subcarriers
void IMMapper::loadP2(TypeData *_p2)
{
  assert(p2 && _p2);
  int i;
  int M = mlut->getM();
  for (i=0; i<k; i++)
    p2[i] = _p2[i];
}

void IMMapper::mapP2()
{
  assert(p2 && arrayS && arrayI); //DEACTIVATE FOR TIME MEASUREMENT
  int i;
  for (i=0; i<k; i++)
    arrayS[i] = mlut->map(p2[i]);
}


void IMMapper::printSymbol()
{
  assert(ofdmIMSymbol);
  int i;
  for (i=0; i<getN(); i++)
    {
        cout << "Subcarrier " << i << ":\t";
        cout <<real(ofdmIMSymbol[i]) << std::showpos << imag(ofdmIMSymbol[i]) << "j"<<endl;
        cout << std::noshowpos;//deactivate signal print
    }
}

void IMMapper::printP2()
{
  assert(p2);
  int i;
  for (i=0; i<k; i++)
    cout << i << " " << p2[i] << endl;
}

void IMMapper::printArrayS()
{
  assert(arrayS);
  int i;
  for (i=0; i<k; i++)
    cout << i << " " << arrayS[i] << endl;
}

void IMMapper::printArrayI()
{
  assert(arrayI);
  int i;
  for (i=0; i<k; i++)
    cout << "C("<< arrayI[i] << ","<< i+1 << ")" << "+";
  cout << endl;
}


/*
 * This is the best algorithm I know to compute binomial coefficients
 * It is from the ancient book Lilavati, see below link for details
 * REF: https://blog.plover.com/math/choose.html
 * TODO: employ BigInt class to support arbitrarily large symbols
 */
TypeData IMMapper::binomialCoefficient(TypeData N, TypeData K)
{  
  //the product in this multiplicative formula of C(N,k) produces very large numbers
  //the largest one that fits a 64-bit variable is C(64,33)
  assert(N <= LARGEST_SYMBOL && N >= 0 && K >= 0);

  if (N < K)
    return 0;  // special case
  if (N == K || K == 0)
    return 1;

  
  long double r = 1.0, d, n = (long double) N, k = (long double) K;

   for (d=1; d <= k; d++) 
   {
     r *= n--;
     r /= d;
    }
    return (TypeData) r;
}
