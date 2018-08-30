#include <asl/Matrix4.h>
#include <asl/Quaternion.h>

namespace asl {

Matrix4 Matrix4::operator+(const Matrix4& B) const
{
	Matrix4 C;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			C(i,j) = a[i][j] + B(i,j);
	return C;
}

Matrix4 Matrix4::operator*(const Matrix4& B) const
{
	Matrix4 C;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			C(i,j) = a[i][0]*B(0,j)+ a[i][1]*B(1,j) + a[i][2]*B(2,j) + a[i][3]*B(3,j);
	return C;
}

Matrix4 Matrix4::operator*(float t) const
{
	Matrix4 C;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			C(i,j) = t * a[i][j];
	return C;
}

Matrix4& Matrix4::operator*=(float t)
{
	for (int i = 0; i<4; i++)
		for (int j = 0; j<4; j++)
			a[i][j] *= t;
	return *this;
}

Matrix4& Matrix4::operator*=(const Matrix4& B)
{
	Matrix4 C;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			C(i,j) = a[i][0]*B(0,j)+ a[i][1]*B(1,j) + a[i][2]*B(2,j) + a[i][3]*B(3,j);
	*this=C;
	return *this;
}

Matrix4 Matrix4::identity()
{
	return Matrix4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
}

Matrix4 Matrix4::translate(const Vec3& t)
{
	return Matrix4(
		1, 0, 0, t.x,
		0, 1, 0, t.y,
		0, 0, 1, t.z,
		0, 0, 0, 1);
}

Matrix4 Matrix4::scale(const Vec3& s)
{
	return Matrix4(
		s.x, 0,   0,   0,
		0,   s.y, 0,   0,
		0,   0,   s.z, 0,
		0,   0,   0,   1);
}

Matrix4 Matrix4::rotateX(float phi)
{
	return Matrix4(
		1, 0,        0,         0,
		0, cos(phi), -sin(phi), 0,
		0, sin(phi), cos(phi),  0,
		0, 0,        0,         1);
}

Matrix4 Matrix4::rotateY(float phi)
{
	return Matrix4(
		cos(phi),  0, sin(phi), 0,
		0,         1, 0,        0,
		-sin(phi), 0, cos(phi), 0,
		0,         0, 0,        1);
}

Matrix4 Matrix4::rotateZ(float phi)
{
	return Matrix4(
		cos(phi), -sin(phi), 0, 0,
		sin(phi), cos(phi),  0, 0,
		0,        0,         1, 0,
		0,        0,         0, 1);
}

Matrix4 Matrix4::rotate(const Vec3& axis, float angle)
{
	return Quaternion(axis, angle).matrix();
}

Matrix4 Matrix4::reflection(Vec3 p, Vec3 n)
{
	float n2=n.length2(), pn=p*n;

	return Matrix4(
		n2-2*n.x*n.x, -2*n.x*n.y,   -2*n.x*n.z,   2*n.x*pn,
		-2*n.y*n.x,   n2-2*n.y*n.y, -2*n.y*n.z,   2*n.y*pn,
		-2*n.z*n.x,   -2*n.z*n.y,   n2-2*n.z*n.z, 2*n.z*pn,
		0,            0,            0,            n2);
}

Vec3 Matrix4::operator%(const Vec3& p) const
{
	return Vec3(
		a[0][0]*p.x+a[0][1]*p.y+a[0][2]*p.z,
		a[1][0]*p.x+a[1][1]*p.y+a[1][2]*p.z,
		a[2][0]*p.x+a[2][1]*p.y+a[2][2]*p.z );
}

Matrix4 Matrix4::inverse() const
{
	float det = a[0][0]*a[1][1]*a[2][2]-a[0][0]*a[2][1]*a[1][2]-a[1][0]*a[0][1]*a[2][2]+
		a[1][0]*a[2][1]*a[0][2]+a[2][0]*a[0][1]*a[1][2]-a[2][0]*a[1][1]*a[0][2];

	float x = -a[0][1]*a[1][2]*a[2][3]+a[0][1]*a[1][3]*a[2][2]+a[1][1]*a[0][2]*a[2][3]-
		a[1][1]*a[0][3]*a[2][2]-a[2][1]*a[0][2]*a[1][3]+a[2][1]*a[0][3]*a[1][2];
	float y = a[0][0]*a[1][2]*a[2][3]-a[0][0]*a[1][3]*a[2][2]-a[1][0]*a[0][2]*a[2][3]+
		a[1][0]*a[0][3]*a[2][2]+a[2][0]*a[0][2]*a[1][3]-a[2][0]*a[0][3]*a[1][2];
	float z = -a[0][0]*a[1][1]*a[2][3]+a[0][0]*a[1][3]*a[2][1]+a[1][0]*a[0][1]*a[2][3]-
		a[1][0]*a[0][3]*a[2][1]-a[2][0]*a[0][1]*a[1][3]+a[2][0]*a[0][3]*a[1][1];

	Matrix4 m(
		-(-a[1][1] * a[2][2] + a[2][1] * a[1][2]), -a[0][1] * a[2][2] + a[2][1] * a[0][2], a[0][1] * a[1][2] - a[1][1] * a[0][2], x,
		-a[1][0] * a[2][2] + a[2][0] * a[1][2], -(-a[0][0] * a[2][2] + a[2][0] * a[0][2]), -(a[0][0] * a[1][2] - a[1][0] * a[0][2]), y,
		-(-a[1][0] * a[2][1] + a[2][0] * a[1][1]), -a[0][0] * a[2][1] + a[2][0] * a[0][1], a[0][0] * a[1][1] - a[1][0] * a[0][1], z);
	m *= 1.0f/det;
	m(3, 3) = 1.0f;
	return m;
}

Quaternion Matrix4::rotation() const
{
    float t = at(0,0) + at(1,1) + at(2,2), s, r;
    Quaternion q;
    if(t >= 0)
	{
		r = sqrt(1 + t);
		s = 0.5f / r;
		q.w = 0.5f * r;
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
			s = 0.5f / r;
			q.x = 0.5f * r;
			q.y = (at(0,1) + at(1,0)) * s; 
			q.z = (at(2,0) + at(0,2)) * s;
			q.w = (at(2,1) - at(1,2)) * s;
			break;
		case 1:
			r = sqrt(1 + at(1,1) - at(2,2) - at(0,0));
			s = 0.5f / r;
			q.y = 0.5f * r;
			q.z = (at(1,2) + at(2,1)) * s;
			q.x = (at(0,1) + at(1,0)) * s;
			q.w = (at(0,2) - at(2,0)) * s;
			break;
		case 2:
			r = sqrt(1 + at(2,2) - at(0,0) - at(1,1));
			s = 0.5f / r;
			q.z = 0.5f * r;
			q.x = (at(2,0) + at(0,2)) * s;
			q.y = (at(1,2) + at(2,1)) * s;
			q.w = (at(1,0) - at(0,1)) * s;
		}
	}
	return q;
}

float Matrix4::det()
{
	return a[0][0]*a[1][1]*a[2][2]-a[0][0]*a[2][1]*a[1][2]-a[1][0]*a[0][1]*a[2][2]+
		a[1][0]*a[2][1]*a[0][2]+a[2][0]*a[0][1]*a[1][2]-a[2][0]*a[1][1]*a[0][2];
}

}
