 /** \file types_3d.h
	*	Types utilises en 3d de facon générale.
	*
	* \author levieux guillaume
	* \date 2005
	*/

#ifndef __NY_3D_H__
#define __NY_3D_H__

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <memory.h> 
#include <string>
#include "ny_utils.h"

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

class NYVert3Df;

void calcNormale(NYVert3Df & vec1, NYVert3Df & vec2, NYVert3Df & normale);
inline bool isNaN(float value)
{
	return (value != value);
}

inline float nanToZero(float value)
{
	if (value != value)
		return 0;
	return value;
}


/** Une couleur en RVBA, 4 floats
	*
	*/
struct NYColor
{

	public:
		float R;
		float V;
		float B;
		float A;

private:
		double xyz_to_lab(double c)
		{
			return c > 216.0 / 24389.0 ? pow(c, 1.0 / 3.0) : c * (841.0/108.0) + (4.0/29.0);
		}

	public:
		NYColor (float r, float v, float b, float a)
		{
			R = r;
			V = v;
			B = b;
			A = a;
		}

		NYColor ()
		{
			R = 0;
			V = 0;
			B = 0;
			A = 1;
		}

		NYColor & operator = (const NYColor & color)
		{
			R = color.R;
			V = color.V;
			B = color.B;
			A = color.A;
			return *this;
		}

		NYColor operator * (float alpha)
		{
			NYColor res;
			res = *this;
			res.R *= alpha;
			res.V *= alpha;
			res.B *= alpha;
			res.A *= alpha;
			return res;
		}

		NYColor operator + (NYColor & color)
		{
			NYColor res;
			res = *this;
			res.R += color.R;
			res.V += color.V;
			res.B += color.B;
			res.A += color.A;
			return res;
		}

		NYColor interpolate(NYColor & target, float alpha)
		{
			alpha = min(1.0f,max(0.0f,alpha));
			NYColor res;
			res = ((*this)*(1.0f-alpha) ) + ((target)*(alpha));
			return res;
		}

		void toHSV(float *h, float *s, float *v, float *a )
		{
			float fmin, fmax, fdelta;
			fmin = min( R, min(V, B) );
			fmax = max( R, max(V, B) );
			*v = fmax;				// v
			fdelta = fmax - fmin;
			if( fmax != 0 )
				*s = fdelta / fmax;		// s
			else {
				// R = V = B = 0		// s = 0, v is undefined
				*s = 0;
				*h = -1;
				return;
			}
			if(fdelta > 0)
			{
				if( R == fmax )
					*h = ( V - B ) / fdelta;		// between yellow & magenta
				else if( V == fmax )
					*h = 2 + ( B - R ) / fdelta;	// between cyan & yellow
				else
					*h = 4 + ( R - V ) / fdelta;	// between magenta & cyan
				*h *= 60;				// degrees
				if( *h < 0 )
					*h += 360;
			}
			else
			{
				*s = 0.0f;
				*h = 0.0f;
			}

			*a = A;
		}

		void fromHSV( float h, float s, float v, float a )
		{
			int i;
			float f, p, q, t;
			if( s == 0 ) {
				// achromatic (grey)
				R = V = B = v;
				return;
			}
			h /= 60;			// sector 0 to 5
			i = (int)floor( h );
			f = h - i;			// factorial part of h
			p = v * ( 1 - s );
			q = v * ( 1 - s * f );
			t = v * ( 1 - s * ( 1 - f ) );
			switch( i ) {
				case 0:
					R = v;
					V = t;
					B = p;
					break;
				case 1:
					R = q;
					V = v;
					B = p;
					break;
				case 2:
					R = p;
					V = v;
					B = t;
					break;
				case 3:
					R = p;
					V = q;
					B = v;
					break;
				case 4:
					R = t;
					V = p;
					B = v;
					break;
				default:		// case 5:
					R = v;
					V = p;
					B = q;
					break;
			}

			 A = a;
		}

		NYColor interpolateHSV(NYColor & target, float alpha)
		{
			alpha = min(1.0f,max(0.0f,alpha));
			float h1,s1,v1,a1;
			this->toHSV(&h1,&s1,&v1,&a1);
			float h2,s2,v2,a2;
			target.toHSV(&h2,&s2,&v2,&a2);

			float higherHue = max(h1,h2);
			float lowerHue = min(h1,h2);
			float maxCCW = higherHue - lowerHue;
			float maxCW = (lowerHue+360.0f) - higherHue;

			float h3,s3,v3,a3;
			if(maxCW > maxCCW)
				h3 = (1.0f-alpha)*h1 + alpha*h2;
			else
			{
				if(h1 > h2)
					h2 += 360.0f;
				else
					h1 += 360.0f;
				h3 = (1.0f-alpha)*h1 + alpha*h2;
				if(h3 > 360.0f)
					h3-=360.0f;
			}
			s3 = (1.0f-alpha)*s1 + alpha*s2;
			v3 = (1.0f-alpha)*v1 + alpha*v2;
			a3 = (1.0f-alpha)*a1 + alpha*a2;
			NYColor res;
			res.fromHSV(h3,s3,v3,a3);
			return res;
		}

		void fromLAB(float l, float a, float b, float al)
		{

			double X, Y, Z;
	
	        // Lab -> normalized XYZ (X,Y,Z are all in 0...1)
	        
	        Y = l * (1.0/116.0) + 16.0/116.0;
	        X = a * (1.0/500.0) + Y;
	        Z = b * (-1.0/200.0) + Y;
	
	        X = X > 6.0/29.0 ? X * X * X : X * (108.0/841.0) - 432.0/24389.0;
	        Y = l > 8.0 ? Y * Y * Y : l * (27.0/24389.0);
	        Z = Z > 6.0/29.0 ? Z * Z * Z : Z * (108.0/841.0) - 432.0/24389.0;
	
	        // normalized XYZ -> linear sRGB
	
	        R = (float)(X * (1219569.0/395920.0)     + Y * (-608687.0/395920.0)    + Z * (-107481.0/197960.0));
	        V = (float)(X * (-80960619.0/87888100.0) + Y * (82435961.0/43944050.0) + Z * (3976797.0/87888100.0));
	        B = (float)(X * (93813.0/1774030.0)      + Y * (-180961.0/887015.0)    + Z * (107481.0/93370.0));
			A = al;
		}

		void toLAB(float *l, float *a, float *b, float *al)
		{

			double X, Y, Z;	        
	        
	        // linear sRGB -> normalized XYZ (X,Y,Z are all in 0...1)
	        
	        X = xyz_to_lab(R * (10135552.0/23359437.0) + V * (8788810.0/23359437.0) + B * (4435075.0/23359437.0));
	        Y = xyz_to_lab(R * (871024.0/4096299.0)    + V * (8788810.0/12288897.0) + B * (887015.0/12288897.0));
	        Z = xyz_to_lab(R * (158368.0/8920923.0)    + V * (8788810.0/80288307.0) + B * (70074185.0/80288307.0));
	
	        // normalized XYZ -> Lab
	
	        *l = (float)(Y * 116.0 - 16.0);
	        *a = (float)((X - Y) * 500.0);
	        *b = (float)((Y - Z) * 200.0);
			*al = A;
		}

		NYColor interpolateLAB(NYColor & target, float alpha)
		{
			alpha = min(1.0f,max(0.0f,alpha));
			float l1,a1,b1,al1;
			this->toLAB(&l1,&a1,&b1,&al1);
			float l2,a2,b2,al2;
			target.toLAB(&l2,&a2,&b2,&al2);

			float l3,a3,b3,al3;

			l3 = (1.0f-alpha)*l1 + alpha*l2;
			a3 = (1.0f-alpha)*a1 + alpha*a2;
			b3 = (1.0f-alpha)*b1 + alpha*b2;
			al3 = (1.0f-alpha)*al1 + alpha*al2;
			NYColor res;
			res.fromLAB(l3,a3,b3,al3);
			return res;
		}

};

class NYPoint
{
	public:
		int X;
		int Y;
	public:
		NYPoint()
		{
			X = 0;
			Y = 0;
		}

		NYPoint(int x, int y)
		{
			X = x;
			Y = y;
		}

		void operator = (const NYPoint & point)
		{
			X = point.X;
			Y = point.Y;
		}

		NYPoint & operator-= (const NYPoint & point)
		{
			X -= point.X;
			Y -= point.Y;

			return *this;
		}

		NYPoint & operator+= (const NYPoint & point)
		{
			X += point.X;
			Y += point.Y;

			return *this;
		}
};

class NYPoint3D
{
public:
	int X;
	int Y;
	int Z;

public:
	NYPoint3D()
	{
		X = 0;
		Y = 0;
		Z = 0;
	}

	NYPoint3D(int x, int y, int z)
	{
		X = x;
		Y = y;
		Z = z;
	}

	void operator = (const NYPoint3D & point)
	{
		X = point.X;
		Y = point.Y;
		Z = point.Z;
	}

	NYPoint3D & operator-= (const NYPoint3D & point)
	{
		X -= point.X;
		Y -= point.Y;
		Z -= point.Z;

		return *this;
	}

	NYPoint3D & operator+= (const NYPoint3D & point)
	{
		X += point.X;
		Y += point.Y;
		Z += point.Z;

		return *this;
	}
};


/** Un point, 2 floats
	*
	*/
class NYVert2Df
{
	public:
		float X;
		float Y;

		NYVert2Df () : X(0), Y(0)
		{
		}
		
		NYVert2Df (float x, float y)
		{
			X = x;
			Y = y;
		}

		NYVert2Df & operator= (const NYVert2Df & vertex)
		{
			X = vertex.X;
			Y = vertex.Y;

			return *this;
		}
};

/** Un point, 4 floats
	*
	*/
class NYVert4Df
{
	public:
		float X;
		float Y;
		float Z;
		float T;

		NYVert4Df() : X(0), Y(0), Z(0), T(0)
		{
		}

		NYVert4Df (float x, float y, float z, float t)
		{
			X = x;
			Y = y;
			Z = z;
			T = t;
		}
};



/** Un point, 3 floats
	*
	*/
class NYVert3Df
{
	public:
		float X;
		float Y;
		float Z;

		NYVert3Df () : X(0), Y(0), Z(0)
		{
		}
		
		NYVert3Df (float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		std::string toStr()
		{
			std::string ret;
			ret += "X:";
			ret += toString(X);
			ret += " Y:";
			ret += toString(Y);
			ret += " Z:";
			ret += toString(Z);
			return ret;
		}

		NYVert3Df & operator= (const NYVert3Df & vertex)
		{
			X = vertex.X;
			Y = vertex.Y;
			Z = vertex.Z;

			return *this;
		}

		NYVert3Df & operator*= (float scalaire)
		{
			X *= scalaire;
			Y *= scalaire;
			Z *= scalaire;

			return *this;
		}

		NYVert3Df & operator/= (float scalaire)
		{
			X /= scalaire;
			Y /= scalaire;
			Z /= scalaire;

			return *this;
		}

		NYVert3Df operator * (float scalaire)
		{
			NYVert3Df temp(X * scalaire,
						   Y * scalaire,
						   Z * scalaire);
			return temp;
		}

		NYVert3Df operator / (float scalaire)
		{
			NYVert3Df temp(X / scalaire,
				Y / scalaire,
				Z / scalaire);
			return temp;
		}

		NYVert3Df operator + (const NYVert3Df & vertex)
		{
			NYVert3Df temp(X + vertex.X,
    					   Y + vertex.Y,
				           Z + vertex.Z);
			return temp;
		}

		NYVert3Df operator - (const NYVert3Df & vertex)
		{
			NYVert3Df temp(X - vertex.X,
				           Y - vertex.Y,
				           Z - vertex.Z);
			return temp;
		}

		NYVert3Df & operator+= (const NYVert3Df & vertex)
		{
			X += vertex.X;
			Y += vertex.Y;
			Z += vertex.Z;

			return *this;
		}

		NYVert3Df & operator-= (const NYVert3Df & vertex)
		{
			X -= vertex.X;
			Y -= vertex.Y;
			Z -= vertex.Z;

			return *this;
		}

		bool operator== (const NYVert3Df & vertex)
		{
			if(X == vertex.X && Y == vertex.Y && Z == vertex.Z)
				return true;
			return false;
		}
		
		float getSize(void)
		{
			return sqrt(X*X+Y*Y+Z*Z);
		}

		float getMagnitude(void)
		{
			return X*X+Y*Y+Z*Z;
		}

		NYVert3Df & normalize(void)
		{
			float longueur = getSize();
			if(longueur)
			{
				X /= longueur;
				Y /= longueur;
				Z /= longueur;
			}
			return *this;
		}

		void createTab(float * tab)
		{
			tab[0] = X;
			tab[1] = Y;
			tab[2] = Z;
		}

		void initFromTab(float * tab)
		{
			X = tab[0];
			Y = tab[1];
			Z = tab[2];
		}

		///Le produit scalaire (dot product)
		float scalProd(const NYVert3Df & vertex)
		{
			return vertex.X*X + vertex.Y*Y + vertex.Z*Z;
		}

		///Le produit vectoriel
		NYVert3Df vecProd(const NYVert3Df & vertex)
		{
			NYVert3Df produit(Y*vertex.Z - Z*vertex.Y, 
			                  Z*vertex.X - X*vertex.Z, 
			                  X*vertex.Y - Y*vertex.X); 
			return produit;
		}
		
		///L'angle que fait ce vecteur sur le plan dont Y est la normale
		float angleY(NYVert3Df & vertex)
		{ 
			NYVert3Df moi(X,0,Z);
			moi.normalize();
			NYVert3Df lui(vertex.X,0,vertex.Z);
			lui.normalize();

			float cosAng = moi.X*lui.X + moi.Z*lui.Z;
			NYVert3Df normale;
			calcNormale(vertex,moi,normale);
			
			float res = acos(cosAng);
			if(isNaN(res))
			{
				if(cosAng > 0.5f)
					res = 0.0f;
				else
					res = 3.14159f;
			}
						
			if(normale.Y >= 0)
				return -res;
			
			return res;
		}

		///L'angle que fait ce vecteur sur le plan dont Z est la normale
		float angleZ(NYVert3Df & vertex)
		{ 
			NYVert3Df moi(X,Y,0);
			moi.normalize();
			NYVert3Df lui(vertex.X,vertex.Y,0);
			lui.normalize();

			float cosAng = moi.X*lui.X + moi.Y*lui.Y;
			NYVert3Df normale;
			calcNormale(vertex,moi,normale);
			
			float res = acos(cosAng);
			if(isNaN(res))
			{
				if(cosAng > 0.5f)
					res = 0.0f;
				else
					res = 3.14159f;
			}
						
			if(normale.Z >= 0)
				return -res;
			
			return res;
		}

		NYVert3Df & rotate (NYVert3Df & axe, float angle)
		{
			float matrice[4][4];

			if(angle < 0.001 && angle > -0.001)
				angle = 0;

			//Calculs fait une fois pour eco
			float cosi = cos(angle);
			float sinu = sin(angle);
			float x = axe.X;
			float y = axe.Y;
			float z = axe.Z;
			float x2 = axe.X*axe.X;
			float y2 = axe.Y*axe.Y;
			float z2 = axe.Z*axe.Z;

			//Creation de la matrice de rot

			//Ligne 1
			matrice[0][0] = x2+(cosi*(1-x2));
			matrice[1][0] = (x*y*(1-cosi))-(z*sinu);
			matrice[2][0] = (x*z*(1-cosi))+(y*sinu);
			matrice[3][0] = 0;

			//Ligne 2
			matrice[0][1] = (x*y*(1-cosi))+(z*sinu);
			matrice[1][1] = y2+(cosi*(1-y2));
			matrice[2][1] = (y*z*(1-cosi))-(x*sinu);
			matrice[3][1] = 0;

			//Ligne 3
			matrice[0][2] = (x*z*(1-cosi))-(y*sinu);
			matrice[1][2] = (y*z*(1-cosi))+(x*sinu);
			matrice[2][2] = z2+(cosi*(1-z2));
			matrice[3][2] = 0;

			//Ligne 4
			matrice[0][3] = 0;
			matrice[1][3] = 0;
			matrice[2][3] = 0;
			matrice[3][3] = 1;

			//Produit du vecteur par la matrice
			NYVert3Df transforme;
			transforme.X = X * matrice[0][0] + Y * matrice[1][0] + Z * matrice[2][0];
			transforme.Y = X * matrice[0][1] + Y * matrice[1][1] + Z * matrice[2][1];
			transforme.Z = X * matrice[0][2] + Y * matrice[1][2] + Z * matrice[2][2];

			*this = transforme;

			//Affectation
			return *this;
		}

		void toSphericalCoordinates(float * dist, float * theta, float * phi)
		{
			*dist = this->getSize();
			*phi = atan2(Y,X);
			*theta = atan2(sqrt(X*X+Y*Y),Z);
		}
};

class NYPlane
{
	public:
		double Xnorm;
		double Ynorm;
		double Znorm;
		double Distance;

	public:
		NYPlane()
		{
			Xnorm = 0;
			Xnorm = 0;
			Xnorm = 0;
			Distance = 0;
		}

		NYPlane(double xNorm, double yNorm, double zNorm, double distance)
		{
			Xnorm = xNorm;
			Xnorm = yNorm;
			Xnorm = zNorm;
			Distance = distance;
		}

		double checkVert(NYVert3Df & vert)
		{
			return Xnorm * vert.X + Xnorm * vert.Y + Xnorm * vert.Z + Distance;
		}

		void normalize(void)
		{
			double longueur = sqrt( Xnorm * Xnorm + Ynorm * Ynorm + Znorm * Znorm);
			if(longueur)
			{
				Xnorm /= longueur;
				Ynorm /= longueur;
				Znorm /= longueur;
				Distance /= longueur;
			}
		}
};



struct NYFloatMatrix
{
	public:
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			}Direct;
			float m[4][4];
			float t[16];
		} Mat;


    NYFloatMatrix() {};
    NYFloatMatrix(const float *pArray)
	{
		Mat.Direct._11 = pArray[0];  Mat.Direct._12 = pArray[1];  Mat.Direct._13 = pArray[2];  Mat.Direct._14 = pArray[3];
		Mat.Direct._21 = pArray[4];  Mat.Direct._22 = pArray[5];  Mat.Direct._23 = pArray[6];  Mat.Direct._24 = pArray[7];
		Mat.Direct._31 = pArray[8];  Mat.Direct._32 = pArray[9];  Mat.Direct._33 = pArray[10]; Mat.Direct._34 = pArray[11];
		Mat.Direct._41 = pArray[12]; Mat.Direct._42 = pArray[13]; Mat.Direct._43 = pArray[14]; Mat.Direct._44 = pArray[15];		
	}

    NYFloatMatrix& operator= (const NYFloatMatrix& matrix)
	{
		Mat.Direct._11 = matrix.Mat.Direct._11;  Mat.Direct._12 = matrix.Mat.Direct._12;  Mat.Direct._13 = matrix.Mat.Direct._13;  Mat.Direct._14 = matrix.Mat.Direct._14;
		Mat.Direct._21 = matrix.Mat.Direct._21;  Mat.Direct._22 = matrix.Mat.Direct._22;  Mat.Direct._23 = matrix.Mat.Direct._23;  Mat.Direct._24 = matrix.Mat.Direct._24;
		Mat.Direct._31 = matrix.Mat.Direct._31;  Mat.Direct._32 = matrix.Mat.Direct._32;  Mat.Direct._33 = matrix.Mat.Direct._33;  Mat.Direct._34 = matrix.Mat.Direct._34;
		Mat.Direct._41 = matrix.Mat.Direct._41;  Mat.Direct._42 = matrix.Mat.Direct._42;  Mat.Direct._43 = matrix.Mat.Direct._43;  Mat.Direct._44 = matrix.Mat.Direct._44;	
		return *this;
	}

	NYFloatMatrix& operator= (const float *pArray)
	{
		Mat.Direct._11 = pArray[0];  Mat.Direct._12 = pArray[1];  Mat.Direct._13 = pArray[2];  Mat.Direct._14 = pArray[3];
		Mat.Direct._21 = pArray[4];  Mat.Direct._22 = pArray[5];  Mat.Direct._23 = pArray[6];  Mat.Direct._24 = pArray[7];
		Mat.Direct._31 = pArray[8];  Mat.Direct._32 = pArray[9];  Mat.Direct._33 = pArray[10]; Mat.Direct._34 = pArray[11];
		Mat.Direct._41 = pArray[12]; Mat.Direct._42 = pArray[13]; Mat.Direct._43 = pArray[14]; Mat.Direct._44 = pArray[15];
		return *this;
	}

	NYFloatMatrix& operator*= (const NYFloatMatrix& matrix)
	{
		float temp[16];
		temp[0] = Mat.Direct._11 * matrix.Mat.Direct._11 + Mat.Direct._12 * matrix.Mat.Direct._21 + Mat.Direct._13 * matrix.Mat.Direct._31 + Mat.Direct._14 * matrix.Mat.Direct._41;  
		temp[1] = Mat.Direct._11 * matrix.Mat.Direct._12 + Mat.Direct._12 * matrix.Mat.Direct._22 + Mat.Direct._13 * matrix.Mat.Direct._32 + Mat.Direct._14 * matrix.Mat.Direct._42;  
		temp[2] = Mat.Direct._11 * matrix.Mat.Direct._13 + Mat.Direct._12 * matrix.Mat.Direct._23 + Mat.Direct._13 * matrix.Mat.Direct._33 + Mat.Direct._14 * matrix.Mat.Direct._43;  
		temp[3] = Mat.Direct._11 * matrix.Mat.Direct._14 + Mat.Direct._12 * matrix.Mat.Direct._24 + Mat.Direct._13 * matrix.Mat.Direct._34 + Mat.Direct._14 * matrix.Mat.Direct._44;

		temp[4] = Mat.Direct._21 * matrix.Mat.Direct._11 + Mat.Direct._22 * matrix.Mat.Direct._21 + Mat.Direct._23 * matrix.Mat.Direct._31 + Mat.Direct._24 * matrix.Mat.Direct._41;  
		temp[5] = Mat.Direct._21 * matrix.Mat.Direct._12 + Mat.Direct._22 * matrix.Mat.Direct._22 + Mat.Direct._23 * matrix.Mat.Direct._32 + Mat.Direct._24 * matrix.Mat.Direct._42;  
		temp[6] = Mat.Direct._21 * matrix.Mat.Direct._13 + Mat.Direct._22 * matrix.Mat.Direct._23 + Mat.Direct._23 * matrix.Mat.Direct._33 + Mat.Direct._24 * matrix.Mat.Direct._43;  
		temp[7]  = Mat.Direct._21 * matrix.Mat.Direct._14 + Mat.Direct._22 * matrix.Mat.Direct._24 + Mat.Direct._23 * matrix.Mat.Direct._34 + Mat.Direct._24 * matrix.Mat.Direct._44;

		temp[8]  = Mat.Direct._31 * matrix.Mat.Direct._11 + Mat.Direct._32 * matrix.Mat.Direct._21 + Mat.Direct._33 * matrix.Mat.Direct._31 + Mat.Direct._34 * matrix.Mat.Direct._41;  
		temp[9]  = Mat.Direct._31 * matrix.Mat.Direct._12 + Mat.Direct._32 * matrix.Mat.Direct._22 + Mat.Direct._33 * matrix.Mat.Direct._32 + Mat.Direct._34 * matrix.Mat.Direct._42;  
		temp[10] = Mat.Direct._31 * matrix.Mat.Direct._13 + Mat.Direct._32 * matrix.Mat.Direct._23 + Mat.Direct._33 * matrix.Mat.Direct._33 + Mat.Direct._34 * matrix.Mat.Direct._43;  
		temp[11] = Mat.Direct._31 * matrix.Mat.Direct._14 + Mat.Direct._32 * matrix.Mat.Direct._24 + Mat.Direct._33 * matrix.Mat.Direct._34 + Mat.Direct._34 * matrix.Mat.Direct._44;

		temp[12] = Mat.Direct._41 * matrix.Mat.Direct._11 + Mat.Direct._42 * matrix.Mat.Direct._21 + Mat.Direct._43 * matrix.Mat.Direct._31 + Mat.Direct._44 * matrix.Mat.Direct._41;  
		temp[13] = Mat.Direct._41 * matrix.Mat.Direct._12 + Mat.Direct._42 * matrix.Mat.Direct._22 + Mat.Direct._43 * matrix.Mat.Direct._32 + Mat.Direct._44 * matrix.Mat.Direct._42;  
		temp[14] = Mat.Direct._41 * matrix.Mat.Direct._13 + Mat.Direct._42 * matrix.Mat.Direct._23 + Mat.Direct._43 * matrix.Mat.Direct._33 + Mat.Direct._44 * matrix.Mat.Direct._43;  
		temp[15] = Mat.Direct._41 * matrix.Mat.Direct._14 + Mat.Direct._42 * matrix.Mat.Direct._24 + Mat.Direct._43 * matrix.Mat.Direct._34 + Mat.Direct._44 * matrix.Mat.Direct._44;

		*this = (float*)temp;
		return *this;
	}

	NYFloatMatrix& operator*= (const float f)
	{
		Mat.Direct._11 *= f;  Mat.Direct._12 *= f;  Mat.Direct._13 *= f;  Mat.Direct._14 *= f;
		Mat.Direct._21 *= f;  Mat.Direct._22 *= f;  Mat.Direct._23 *= f;  Mat.Direct._24 *= f;
		Mat.Direct._31 *= f;  Mat.Direct._32 *= f;  Mat.Direct._33 *= f; Mat.Direct._34 *= f;
		Mat.Direct._41 *= f; Mat.Direct._42 *= f; Mat.Direct._43 *= f; Mat.Direct._44 *= f;		
		return *this;
	}

	NYVert3Df operator* (const NYVert3Df & vert)
	{
		NYVert3Df temp;

		temp.X = Mat.Direct._11 * vert.X + Mat.Direct._12 * vert.Y + Mat.Direct._13 * vert.Z + Mat.Direct._14 ;  
		temp.Y = Mat.Direct._21 * vert.X + Mat.Direct._22 * vert.Y + Mat.Direct._23 * vert.Z + Mat.Direct._24 ;  
		temp.Z = Mat.Direct._31 * vert.X + Mat.Direct._32 * vert.Y + Mat.Direct._33 * vert.Z + Mat.Direct._34 ;    

		return temp;
	}

	void createViewMatrix(NYVert3Df & pos, NYVert3Df & lookat, NYVert3Df & up)
	{
		createIdentite();
		NYVert3Df R2 = lookat - pos;
		R2.normalize();
		NYVert3Df R0(up.vecProd(R2));
		R0.normalize();
		NYVert3Df R1(R2.vecProd(R0));

		NYVert3Df neg(-pos.X,-pos.Y,-pos.Z);
		float D0 = R0.scalProd(neg);
		float D1 = R1.scalProd(neg);
		float D2 = R2.scalProd(neg);

		Mat.Direct._11 = R0.X;
		Mat.Direct._12 = R0.Y;
		Mat.Direct._13 = R0.Z;

		Mat.Direct._21 = R1.X;
		Mat.Direct._22 = R1.Y;
		Mat.Direct._23 = R1.Z;

		Mat.Direct._31 = R2.X;
		Mat.Direct._32 = R2.Y;
		Mat.Direct._33 = R2.Z;

		Mat.Direct._14 = D0;
		Mat.Direct._24 = D1;
		Mat.Direct._34 = D2;
	}

	void createIdentite(void)
	{
		memset(Mat.m,0x00,16*sizeof(float));
		Mat.Direct._11 = 1;
		Mat.Direct._22 = 1;
		Mat.Direct._33 = 1;
		Mat.Direct._44 = 1;
	}

	void createRotateX(float angle)
	{
		createIdentite();
		float cosi = cos(angle);
		float sini = sin(angle);
		Mat.Direct._22 = cosi;
		Mat.Direct._23 = -sini;
		Mat.Direct._32 = sini;
		Mat.Direct._33 = cosi;
	}

	void createRotateY(float angle)
	{
		createIdentite();
		float cosi = cos(angle);
		float sini = sin(angle);
		Mat.Direct._11 = cosi;
		Mat.Direct._13 = sini;
		Mat.Direct._31 = -sini;
		Mat.Direct._33 = cosi;
	}

	void createRotateZ(float angle)
	{
		createIdentite();
		float cosi = cos(angle);
		float sini = sin(angle);
		Mat.Direct._11 = cosi;
		Mat.Direct._12 = -sini;
		Mat.Direct._21 = sini;
		Mat.Direct._22 = cosi;
	}

	void createRotateXYZ(float angleX, float angleY, float angleZ)
	{
		createRotateX(angleX);

		NYFloatMatrix rY;
		rY.createRotateY(angleY);

		NYFloatMatrix rZ;
		rZ.createRotateZ(angleZ);

		(*this) *= rY;
		(*this) *= rZ;
	}

	/*
	// +-           -+   +-                                        -+
	// | r00 r01 r02 |   |  cy*cz           -cy*sz            sy    |
	// | r10 r11 r12 | = |  cz*sx*sy+cx*sz   cx*cz-sx*sy*sz  -cy*sx |
	// | r20 r21 r22 |   | -cx*cz*sy+sx*sz   cz*sx+cx*sy*sz   cx*cy |
	// +-           -+   +-                                        -+

	if (mEntry[2] < (Real)1)
	{
	if (mEntry[2] > -(Real)1)
	{
	// y_angle = asin(r02)
	// x_angle = atan2(-r12,r22)
	// z_angle = atan2(-r01,r00)
	yAngle = (Real)asin((double)mEntry[2]);
	xAngle = Math<Real>::ATan2(-mEntry[5], mEntry[8]);
	zAngle = Math<Real>::ATan2(-mEntry[1], mEntry[0]);
	return EA_UNIQUE;
	}
	else
	{
	// y_angle = -pi/2
	// z_angle - x_angle = atan2(r10,r11)
	// WARNING.  The solution is not unique.  Choosing z_angle = 0.
	yAngle = -Math<Real>::HALF_PI;
	xAngle = -Math<Real>::ATan2(mEntry[3], mEntry[4]);
	zAngle = (Real)0;
	return EA_NOT_UNIQUE_DIF;
	}
	}
	else
	{
	// y_angle = +pi/2
	// z_angle + x_angle = atan2(r10,r11)
	// WARNING.  The solutions is not unique.  Choosing z_angle = 0.
	yAngle = Math<Real>::HALF_PI;
	xAngle = Math<Real>::ATan2(mEntry[3], mEntry[4]);
	zAngle = (Real)0;
	return EA_NOT_UNIQUE_SUM;
	}

	*/

	bool toEulerXYZ(float * xyz)
	{
		float fi = this->Mat.Direct._13;

		if (fi < 1.0f)
		{
			if (fi > -1.0f)
			{
				// y_angle = asin(r02)
				// x_angle = atan2(-r12,r22)
				// z_angle = atan2(-r01,r00)

				xyz[0] = atan2(-this->Mat.Direct._23,this->Mat.Direct._33);
				xyz[1] = asin(this->Mat.Direct._13);
				xyz[2] = atan2(-this->Mat.Direct._12,this->Mat.Direct._11);
				return true;
			}
			else
			{
				// y_angle = -pi/2
				// z_angle - x_angle = atan2(r10,r11)

				// WARNING.  Not unique.  XA - ZA = -atan2(r10,r11)
				xyz[0] = -atan2(-this->Mat.Direct._21,this->Mat.Direct._22);
				xyz[1] = -(float)M_PI_2;
				xyz[2] = 0.0f;
				return false;
			}
		}
		else
		{
			// WARNING.  Not unique.  XAngle + ZAngle = atan2(r10,r11)
			xyz[0] = atan2(-this->Mat.Direct._21,this->Mat.Direct._22);
			xyz[1] = (float)M_PI_2;
			xyz[2] = 0.0;
		}
		return false;
	}

	void createRotateAxe(NYVert3Df & axe, float angle)
	{
		//Creation de la matrice de rot
		if(angle < 0.001 && angle > -0.001)
			angle = 0;

		//Calculs fait une fois pour eco
		float cosi = cos(angle);
		float sinu = sin(angle);
		float x = axe.X;
		float y = axe.Y;
		float z = axe.Z;
		float x2 = axe.X*axe.X;
		float y2 = axe.Y*axe.Y;
		float z2 = axe.Z*axe.Z;

		//Ligne 1
		Mat.m[0][0] = x2+(cosi*(1-x2));
		Mat.m[1][0] = (x*y*(1-cosi))-(z*sinu);
		Mat.m[2][0] = (x*z*(1-cosi))+(y*sinu);
		Mat.m[3][0] = 0;

		//Ligne 2
		Mat.m[0][1] = (x*y*(1-cosi))+(z*sinu);
		Mat.m[1][1] = y2+(cosi*(1-y2));
		Mat.m[2][1] = (y*z*(1-cosi))-(x*sinu);
		Mat.m[3][1] = 0;

		//Ligne 3
		Mat.m[0][2] = (x*z*(1-cosi))-(y*sinu);
		Mat.m[1][2] = (y*z*(1-cosi))+(x*sinu);
		Mat.m[2][2] = z2+(cosi*(1-z2));
		Mat.m[3][2] = 0;

		//Ligne 4
		Mat.m[0][3] = 0;
		Mat.m[1][3] = 0;
		Mat.m[2][3] = 0;
		Mat.m[3][3] = 1;
	}

	void createTranslation(float x, float y, float z)
	{
		createIdentite();
		Mat.Direct._14 = x;
		Mat.Direct._24 = y;
		Mat.Direct._34 = z;
	}

	void getTansposedTab(float * tab)
	{
		tab[0] = this->Mat.Direct._11;
		tab[1] = this->Mat.Direct._21;
		tab[2] = this->Mat.Direct._31;
		tab[3] = this->Mat.Direct._41;

		tab[4] = this->Mat.Direct._12;
		tab[5] = this->Mat.Direct._22;
		tab[6] = this->Mat.Direct._32;
		tab[7] = this->Mat.Direct._42;

		tab[8] = this->Mat.Direct._13;
		tab[9] = this->Mat.Direct._23;
		tab[10] = this->Mat.Direct._33;
		tab[11] = this->Mat.Direct._43;

		tab[12] = this->Mat.Direct._14;
		tab[13] = this->Mat.Direct._24;
		tab[14] = this->Mat.Direct._34;
		tab[15] = this->Mat.Direct._44;
	}

	bool invert(void) 
	{
		float inv[16], det;

		float * m = this->Mat.t;

		inv[0] = m[5]  * m[10] * m[15] - 
				 m[5]  * m[11] * m[14] - 
				 m[9]  * m[6]  * m[15] + 
				 m[9]  * m[7]  * m[14] +
				 m[13] * m[6]  * m[11] - 
				 m[13] * m[7]  * m[10];

		inv[4] = -m[4]  * m[10] * m[15] + 
				  m[4]  * m[11] * m[14] + 
				  m[8]  * m[6]  * m[15] - 
				  m[8]  * m[7]  * m[14] - 
				  m[12] * m[6]  * m[11] + 
				  m[12] * m[7]  * m[10];

		inv[8] = m[4]  * m[9] * m[15] - 
				 m[4]  * m[11] * m[13] - 
				 m[8]  * m[5] * m[15] + 
				 m[8]  * m[7] * m[13] + 
				 m[12] * m[5] * m[11] - 
				 m[12] * m[7] * m[9];

		inv[12] = -m[4]  * m[9] * m[14] + 
				   m[4]  * m[10] * m[13] +
				   m[8]  * m[5] * m[14] - 
				   m[8]  * m[6] * m[13] - 
				   m[12] * m[5] * m[10] + 
				   m[12] * m[6] * m[9];

		inv[1] = -m[1]  * m[10] * m[15] + 
				  m[1]  * m[11] * m[14] + 
				  m[9]  * m[2] * m[15] - 
				  m[9]  * m[3] * m[14] - 
				  m[13] * m[2] * m[11] + 
				  m[13] * m[3] * m[10];

		inv[5] = m[0]  * m[10] * m[15] - 
				 m[0]  * m[11] * m[14] - 
				 m[8]  * m[2] * m[15] + 
				 m[8]  * m[3] * m[14] + 
				 m[12] * m[2] * m[11] - 
				 m[12] * m[3] * m[10];

		inv[9] = -m[0]  * m[9] * m[15] + 
				  m[0]  * m[11] * m[13] + 
				  m[8]  * m[1] * m[15] - 
				  m[8]  * m[3] * m[13] - 
				  m[12] * m[1] * m[11] + 
				  m[12] * m[3] * m[9];

		inv[13] = m[0]  * m[9] * m[14] - 
				  m[0]  * m[10] * m[13] - 
				  m[8]  * m[1] * m[14] + 
				  m[8]  * m[2] * m[13] + 
				  m[12] * m[1] * m[10] - 
				  m[12] * m[2] * m[9];

		inv[2] = m[1]  * m[6] * m[15] - 
				 m[1]  * m[7] * m[14] - 
				 m[5]  * m[2] * m[15] + 
				 m[5]  * m[3] * m[14] + 
				 m[13] * m[2] * m[7] - 
				 m[13] * m[3] * m[6];

		inv[6] = -m[0]  * m[6] * m[15] + 
				  m[0]  * m[7] * m[14] + 
				  m[4]  * m[2] * m[15] - 
				  m[4]  * m[3] * m[14] - 
				  m[12] * m[2] * m[7] + 
				  m[12] * m[3] * m[6];

		inv[10] = m[0]  * m[5] * m[15] - 
				  m[0]  * m[7] * m[13] - 
				  m[4]  * m[1] * m[15] + 
				  m[4]  * m[3] * m[13] + 
				  m[12] * m[1] * m[7] - 
				  m[12] * m[3] * m[5];

		inv[14] = -m[0]  * m[5] * m[14] + 
				   m[0]  * m[6] * m[13] + 
				   m[4]  * m[1] * m[14] - 
				   m[4]  * m[2] * m[13] - 
				   m[12] * m[1] * m[6] + 
				   m[12] * m[2] * m[5];

		inv[3] = -m[1] * m[6] * m[11] + 
				  m[1] * m[7] * m[10] + 
				  m[5] * m[2] * m[11] - 
				  m[5] * m[3] * m[10] - 
				  m[9] * m[2] * m[7] + 
				  m[9] * m[3] * m[6];

		inv[7] = m[0] * m[6] * m[11] - 
				 m[0] * m[7] * m[10] - 
				 m[4] * m[2] * m[11] + 
				 m[4] * m[3] * m[10] + 
				 m[8] * m[2] * m[7] - 
				 m[8] * m[3] * m[6];

		inv[11] = -m[0] * m[5] * m[11] + 
				   m[0] * m[7] * m[9] + 
				   m[4] * m[1] * m[11] - 
				   m[4] * m[3] * m[9] - 
				   m[8] * m[1] * m[7] + 
				   m[8] * m[3] * m[5];

		inv[15] = m[0] * m[5] * m[10] - 
				  m[0] * m[6] * m[9] - 
				  m[4] * m[1] * m[10] + 
				  m[4] * m[2] * m[9] + 
				  m[8] * m[1] * m[6] - 
				  m[8] * m[2] * m[5];

		det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

		if (det == 0)
			return false;

		det = 1.0f / det;

		*this = inv;

		*this *= det;

		return true;
	}
};

/*
* Gere position / rotation d'un objet sous forme de matrice
*/
class Positionator
{
	public:
		NYFloatMatrix _Rot;
		NYVert3Df _Pos;
		Positionator * _Parent;
	
	private:	
		NYFloatMatrix _TempMatrix;

	public:
		Positionator()
		{
			_Rot.createIdentite();
			_Parent = NULL;
		}

		void Rotate(NYVert3Df & axe, float value)
		{
			_TempMatrix.createRotateAxe(axe,value);
			_TempMatrix *= _Rot;
			_Rot = _TempMatrix;
		}

		void Translate(NYVert3Df & move)
		{
			_Pos+=move;
		}

		/*
		Permet de changer un point de repère
		*/
		NYVert3Df Transform(NYVert3Df & point) 
		{
			NYVert3Df res = _Rot * point;
			res += _Pos;
			return res;
		}

		NYVert3Df invTransform(NYVert3Df & point) 
		{
			NYVert3Df res = point;
			res -= _Pos;
			NYFloatMatrix invR = _Rot;
			invR.invert();
			res = invR * res;
			
			return res;
		}

		Positionator &  operator= (const Positionator & pos)
		{
			this->_Pos = pos._Pos;
			this->_Rot = pos._Rot;
			return *this;
		}

		NYVert3Df getWorldPos(NYVert3Df & point)
		{
			NYVert3Df res = this->Transform(point);
			Positionator * parent = this->_Parent;
			while(parent)
			{
				res = parent->Transform(res);
				parent = parent->_Parent;
			}

			return res;
		}

		NYVert3Df getWorldRot(NYVert3Df & point)
		{
			NYVert3Df res = this->_Rot * point;
			Positionator * parent = this->_Parent;
			while(parent)
			{
				res = parent->_Rot * res;
				parent = parent->_Parent;
			}

			return res;
		}
};

/*
* permet de faire tourner un objet en fonction de ses rotations précédentes
*/
class Rotator
{
	public:
		NYVert3Df _AxeX;
		NYVert3Df _AxeY;
		NYVert3Df _AxeZ;
		NYFloatMatrix _Matrix;
		NYFloatMatrix _TempMatrix;


	public:
		Rotator()
		{
			_AxeX = NYVert3Df(1,0,0);
			_AxeY = NYVert3Df(0,1,0);
			_AxeZ = NYVert3Df(0,0,1);
			_Matrix.createIdentite();
		}

		void Rotate(NYVert3Df & axe, float value)
		{
			_AxeX.rotate(axe,value);
			_AxeY.rotate(axe,value);
			_AxeZ.rotate(axe,value);
			_TempMatrix.createRotateAxe(axe,value);
			_Matrix *= _TempMatrix;
		}

		void RotateX(float value)
		{
			Rotate(_AxeX,value);
			_AxeX.normalize();
		}

		void RotateY(float value)
		{
			Rotate(_AxeY,value);
			_AxeY.normalize();
		}

		void RotateZ(float value)
		{
			Rotate(_AxeZ,value);
			_AxeZ.normalize();
		}
};

/** Un quaternion
	*
	*/
class NYQuaternion
{
	public:
		float X;
		float Y;
		float Z;
		float T;

		NYQuaternion() : X(0), Y(0), Z(0), T(0)
		{
		}

		NYQuaternion (float x, float y, float z, float t)
		{
			X = x;
			Y = y;
			Z = z;
			T = t;
		}

		void setIdentite(void)
		{
			X = 0;;
			Y = 0;
			Z = 0;
			T = 1;
		}

		void initFromMat(NYFloatMatrix & matrix)
		{
			float  tr, s;
			float  q[4];
			int    i, j, k;

			int nxt[3] = {1, 2, 0};

			tr = matrix.Mat.m[0][0] + matrix.Mat.m[1][1] + matrix.Mat.m[2][2];

			// check the diagonal

			if (tr > 0.0f) 
			{
				s = sqrt (tr + 1.0f);

				this->T = s / 2.0f;

				s = 0.5f / s;

				this->X = (matrix.Mat.m[1][2] - matrix.Mat.m[2][1]) * s;
				this->Y = (matrix.Mat.m[2][0] - matrix.Mat.m[0][2]) * s;
				this->Z = (matrix.Mat.m[0][1] - matrix.Mat.m[1][0]) * s;

			} else {		
			  
				// diagonal is negative

				i = 0;

				if (matrix.Mat.m[1][1] > matrix.Mat.m[0][0]) i = 1;
				if (matrix.Mat.m[2][2] > matrix.Mat.m[i][i]) i = 2;

				j = nxt[i];
				k = nxt[j];

				s = sqrt ((matrix.Mat.m[i][i] - (matrix.Mat.m[j][j] + matrix.Mat.m[k][k])) + 1.0f);

				q[i] = s * 0.5f;

				if (s != 0.0f) s = 0.5f / s;

				q[3] = (matrix.Mat.m[j][k] - matrix.Mat.m[k][j]) * s;
				q[j] = (matrix.Mat.m[i][j] + matrix.Mat.m[j][i]) * s;
				q[k] = (matrix.Mat.m[i][k] + matrix.Mat.m[k][i]) * s;

				this->X = q[0];
				this->Y = q[1];
				this->Z = q[2];
				this->T = q[3];
			}


		}
};



void checkGlError(char * call);

//Intersection sphère et droite (p1,p2)
bool interDroiteSphere(float p1x, float p1y, float p1z, float p2x, float p2y, float p2z, float pcx, float pcy, float pcz, float rayon, NYVert3Df & inter);


#endif