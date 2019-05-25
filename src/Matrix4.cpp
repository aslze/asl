#include <asl/Matrix4.h>
#include <asl/Quaternion.h>

namespace asl {

template<class T>
Matrix4_<T> Matrix4_<T>::operator+(const Matrix4_<T>& B) const
{
	Matrix4_<T> C;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			C(i,j) = a[i][j] + B(i,j);
	return C;
}

template<class T>
Matrix4_<T> Matrix4_<T>::operator*(const Matrix4_<T>& B) const
{
	Matrix4_<T> C;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			C(i,j) = a[i][0]*B(0,j)+ a[i][1]*B(1,j) + a[i][2]*B(2,j) + a[i][3]*B(3,j);
	return C;
}

template<class T>
Matrix4_<T> Matrix4_<T>::operator*(T t) const
{
	Matrix4_<T> C;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			C(i,j) = t * a[i][j];
	return C;
}

template<class T>
Matrix4_<T>& Matrix4_<T>::operator*=(T t)
{
	for (int i = 0; i<4; i++)
		for (int j = 0; j<4; j++)
			a[i][j] *= t;
	return *this;
}

template<class T>
Matrix4_<T>& Matrix4_<T>::operator*=(const Matrix4_<T>& B)
{
	Matrix4_<T> C;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			C(i,j) = a[i][0]*B(0,j)+ a[i][1]*B(1,j) + a[i][2]*B(2,j) + a[i][3]*B(3,j);
	*this=C;
	return *this;
}

template<class T>
Matrix4_<T> Matrix4_<T>::identity()
{
	return Matrix4_<T>(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::translate(const Vec3_<T>& t)
{
	return Matrix4_<T>(
		1, 0, 0, t.x,
		0, 1, 0, t.y,
		0, 0, 1, t.z,
		0, 0, 0, 1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::scale(const Vec3_<T>& s)
{
	return Matrix4_<T>(
		s.x, 0,   0,   0,
		0,   s.y, 0,   0,
		0,   0,   s.z, 0,
		0,   0,   0,   1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::rotateX(T phi)
{
	return Matrix4_<T>(
		1, 0,        0,         0,
		0, cos(phi), -sin(phi), 0,
		0, sin(phi), cos(phi),  0,
		0, 0,        0,         1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::rotateY(T phi)
{
	return Matrix4_<T>(
		cos(phi),  0, sin(phi), 0,
		0,         1, 0,        0,
		-sin(phi), 0, cos(phi), 0,
		0,         0, 0,        1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::rotateZ(T phi)
{
	return Matrix4_<T>(
		cos(phi), -sin(phi), 0, 0,
		sin(phi), cos(phi),  0, 0,
		0,        0,         1, 0,
		0,        0,         0, 1);
}

template<class T>
Matrix4_<T> Matrix4_<T>::rotate(const Vec3_<T>& axis, T angle)
{
	return Quaternion_<T>::fromAxisAngle(axis, angle).matrix();
}

template<class T>
Matrix4_<T> Matrix4_<T>::reflection(const Vec3_<T>& p, const Vec3_<T>& n)
{
	T n2=n.length2(), pn=p*n;

	return Matrix4_<T>(
		n2-2*n.x*n.x, -2*n.x*n.y,   -2*n.x*n.z,   2*n.x*pn,
		-2*n.y*n.x,   n2-2*n.y*n.y, -2*n.y*n.z,   2*n.y*pn,
		-2*n.z*n.x,   -2*n.z*n.y,   n2-2*n.z*n.z, 2*n.z*pn,
		0,            0,            0,            n2);
}

template<class T>
Vec3_<T> Matrix4_<T>::operator%(const Vec3_<T>& p) const
{
	return Vec3_<T>(
		a[0][0]*p.x+a[0][1]*p.y+a[0][2]*p.z,
		a[1][0]*p.x+a[1][1]*p.y+a[1][2]*p.z,
		a[2][0]*p.x+a[2][1]*p.y+a[2][2]*p.z );
}

template<class T>
Matrix4_<T> Matrix4_<T>::inverse() const
{
	T det = a[0][0]*a[1][1]*a[2][2]-a[0][0]*a[2][1]*a[1][2]-a[1][0]*a[0][1]*a[2][2]+
		a[1][0]*a[2][1]*a[0][2]+a[2][0]*a[0][1]*a[1][2]-a[2][0]*a[1][1]*a[0][2];

	T x = -a[0][1]*a[1][2]*a[2][3]+a[0][1]*a[1][3]*a[2][2]+a[1][1]*a[0][2]*a[2][3]-
		a[1][1]*a[0][3]*a[2][2]-a[2][1]*a[0][2]*a[1][3]+a[2][1]*a[0][3]*a[1][2];
	T y = a[0][0]*a[1][2]*a[2][3]-a[0][0]*a[1][3]*a[2][2]-a[1][0]*a[0][2]*a[2][3]+
		a[1][0]*a[0][3]*a[2][2]+a[2][0]*a[0][2]*a[1][3]-a[2][0]*a[0][3]*a[1][2];
	T z = -a[0][0]*a[1][1]*a[2][3]+a[0][0]*a[1][3]*a[2][1]+a[1][0]*a[0][1]*a[2][3]-
		a[1][0]*a[0][3]*a[2][1]-a[2][0]*a[0][1]*a[1][3]+a[2][0]*a[0][3]*a[1][1];

	Matrix4_<T> m(
		-(-a[1][1] * a[2][2] + a[2][1] * a[1][2]), -a[0][1] * a[2][2] + a[2][1] * a[0][2], a[0][1] * a[1][2] - a[1][1] * a[0][2], x,
		-a[1][0] * a[2][2] + a[2][0] * a[1][2], -(-a[0][0] * a[2][2] + a[2][0] * a[0][2]), -(a[0][0] * a[1][2] - a[1][0] * a[0][2]), y,
		-(-a[1][0] * a[2][1] + a[2][0] * a[1][1]), -a[0][0] * a[2][1] + a[2][0] * a[0][1], a[0][0] * a[1][1] - a[1][0] * a[0][1], z);
	m *= 1/det;
	m(3, 3) = 1;
	return m;
}

template<class T>
Quaternion_<T> Matrix4_<T>::rotation() const
{
    T t = at(0,0) + at(1,1) + at(2,2), s, r;
    Quaternion_<T> q;
    if(t >= 0)
	{
		r = sqrt(1 + t);
		s = (T)0.5 / r;
		q.w = (T)0.5 * r;
		q.x = (at(2,1) - at(1,2)) * s;
		q.y = (at(0,2) - at(2,0)) * s;
		q.z = (at(1,0) - at(0,1)) * s;
	}
    else
	{
		int i = 0; 
		if (at(1,1) > at(0,0))
			i = 1; 
		if (at(2,2) > at(i,i))
			i = 2; 
		switch (i)
		{
		case 0:
			r = sqrt(1 + at(0,0) - at(1,1) - at(2,2));
			s = (T)0.5 / r;
			q.x = (T)0.5 * r;
			q.y = (at(0,1) + at(1,0)) * s; 
			q.z = (at(2,0) + at(0,2)) * s;
			q.w = (at(2,1) - at(1,2)) * s;
			break;
		case 1:
			r = sqrt(1 + at(1,1) - at(2,2) - at(0,0));
			s = (T)0.5 / r;
			q.y = (T)0.5 * r;
			q.z = (at(1,2) + at(2,1)) * s;
			q.x = (at(0,1) + at(1,0)) * s;
			q.w = (at(0,2) - at(2,0)) * s;
			break;
		case 2:
			r = sqrt(1 + at(2,2) - at(0,0) - at(1,1));
			s = (T)0.5 / r;
			q.z = (T)0.5 * r;
			q.x = (at(2,0) + at(0,2)) * s;
			q.y = (at(1,2) + at(2,1)) * s;
			q.w = (at(1,0) - at(0,1)) * s;
		}
	}
	return q;
}

template<class T>
T Matrix4_<T>::det() const 
{
	return a[0][0]*a[1][1]*a[2][2]-a[0][0]*a[2][1]*a[1][2]-a[1][0]*a[0][1]*a[2][2]+
		a[1][0]*a[2][1]*a[0][2]+a[2][0]*a[0][1]*a[1][2]-a[2][0]*a[1][1]*a[0][2];
}

template class Matrix4_<float>;
template class Matrix4_<double>;
}
