#ifndef __NY_UTILS__
#define __NY_UTILS__

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN		
#endif
#include <windows.h>
#include <process.h>
#include <psapi.h>
#include <math.h>
#include <assert.h>
#include <conio.h>
#include <stdio.h>

#include "types.h"

#include <string>
#include <strstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <vector>
using namespace std;

#define NAN(x) (x!=x)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

///Avoir une valeur aléatoire entre 0 et max
inline double randf(void)
{
	double rnd = rand();
	rnd /= RAND_MAX;
	return rnd;
}

///Message d'erreur avec sortie direct tfacon c foutu
inline void CRITICAL(char * texte, sint32 erreur)
{
	char numeroErreur[10];
	sprintf(numeroErreur,"%ld",erreur);
	MessageBoxA(NULL,texte,numeroErreur,0);
	exit(0);
}
template<class T>
inline T round(T val)
{
	T rest;
	modf(val,&rest);
	if (rest > 0.5)
		return ceil(val);
	return floor(val);
}

/// Nettoyage d'un pointeur (cree avec new)
#define SAFEDELETE(a) { if (a) {delete a; a = NULL; }}

/// Nettoyage d'un tableau (cree par new[])
#define SAFEDELETE_TAB(a) { if (a) {delete [] a; a = NULL; }}

///Connaitre la taille d'un fichier
uint32 getFileSize(const char* name);

template<class T>
string toString(const T& val)
{
    stringstream strm;
    strm << val;
    return strm.str();
}
template<typename T>
T stringTo( const std::string& s )
{
	std::istringstream iss(s);
	T x;
	iss >> x;
	return x;
};

inline void splitString (std::vector<string> & elements, string & toSplit, char delim)
{
	std::stringstream ss(toSplit);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elements.push_back(item);
	}
}


uint16 fgetu16 (FILE * fe);

uint32 fgetu32 (FILE * fe);

bool isPowerOfTwo(uint32 value);

unsigned long getMemoryUsage(void);

void nyStartTimer(void);

float nyGetElapsedTimer(void);

template<typename T>
float ny_sign(T val)
{
	if(val >= 0)
		return 1.0f;
	else
		return -1.0f;

}

#endif