#ifndef MATRIX_INCL
#define MATRIX_INCL

#include <boost/numeric/ublas/matrix.hpp>
#include <math.h>


inline boost::numeric::ublas::matrix <float> scale (const float x, const float y, const float z)
{
	boost::numeric::ublas::matrix <float> m(4,4);
	m = boost::numeric::ublas::identity_matrix<float>(4);
	m (0,0) = x;
	m(1,1) = y;
	m(2,2) = z;
	return m;
}

inline boost::numeric::ublas::matrix <float> rotate (const float angle,float x,float y,float z)
{
	float s;
	float c;
	bool optimized;

	s = sinf(angle * M_PI/180.0f);
	c = cosf(angle * M_PI/180.0f);
	boost::numeric::ublas::matrix <float> m(4,4);
	m = boost::numeric::ublas::identity_matrix<float>(4);
	optimized = false;

	if (x == 0.0F) {
		if (y == 0.0F) {
			if (z != 0.0F) {
				optimized = true;
				/* rotate only around z-axis */
				m(0,0) = c;
				m(1,1) = c;
				if (z < 0.0F) {
					m(0,1) = s;
					m(1,0) = -s;
				} else {
					m(0,1) = -s;
					m(1,0) = s;
				}
			}
		}
		else if (z == 0.0F) {
			optimized = true;
			/* rotate only around y-axis */
			m(0,0) = c;
			m(2,2) = c;
			if (y < 0.0F) {
				m(0,2) = -s;
				m(2,0) = s;
			} else {
				m(0,2) = s;
				m(2,0) = -s;
			}
		}
	}
	else if (y == 0.0F) {
		if (z == 0.0F) {
			optimized = true;
			/* rotate only around x-axis */
			m(1,1) = c;
			m(2,2) = c;
			if (x < 0.0F) {
				m(1,2) = s;
				m(2,1) = -s;
			} else {
				m(1,2) = -s;
				m(2,1) = s;
			}
		}
	}

	if (!optimized) {
		const float mag = sqrtf(x * x + y * y + z * z);

		if (mag <= 1.0e-4) {
			/* no rotation, leave mat as-is */
			return m;
		}

		x /= mag;
		y /= mag;
		z /= mag;


		/*
		*     Arbitrary axis rotation matrix.
		*
		*  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
		*  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
		*  (which is about the X-axis), and the two composite transforms
		*  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
		*  from the arbitrary axis to the X-axis then back.  They are
		*  all elementary rotations.
		*
		*  Rz' is a rotation about the Z-axis, to bring the axis vector
		*  into the x-z plane.  Then Ry' is applied, rotating about the
		*  Y-axis to bring the axis vector parallel with the X-axis.  The
		*  rotation about the X-axis is then performed.  Ry and Rz are
		*  simply the respective inverse transforms to bring the arbitrary
		*  axis back to it's original orientation.  The first transforms
		*  Rz' and Ry' are considered inverses, since the data from the
		*  arbitrary axis gives you info on how to get to it, not how
		*  to get away from it, and an inverse must be applied.
		*
		*  The basic calculation used is to recognize that the arbitrary
		*  axis vector (x, y, z), since it is of unit length, actually
		*  represents the sines and cosines of the angles to rotate the
		*  X-axis to the same orientation, with theta being the angle about
		*  Z and phi the angle about Y (in the order described above)
		*  as follows:
		*
		*  cos ( theta ) = x / sqrt ( 1 - z^2 )
		*  sin ( theta ) = y / sqrt ( 1 - z^2 )
		*
		*  cos ( phi ) = sqrt ( 1 - z^2 )
		*  sin ( phi ) = z
		*
		*  Note that cos ( phi ) can further be inserted to the above
		*  formulas:
		*
		*  cos ( theta ) = x / cos ( phi )
		*  sin ( theta ) = y / sin ( phi )
		*
		*  ...etc.  Because of those relations and the standard trigonometric
		*  relations, it is pssible to reduce the transforms down to what
		*  is used below.  It may be that any primary axis chosen will give the
		*  same results (modulo a sign convention) using thie method.
		*
		*  Particularly nice is to notice that all divisions that might
		*  have caused trouble when parallel to certain planes or
		*  axis go away with care paid to reducing the expressions.
		*  After checking, it does perform correctly under all cases, since
		*  in all the cases of division where the denominator would have
		*  been zero, the numerator would have been zero as well, giving
		*  the expected result.
		*/
		float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
		xx = x * x;
		yy = y * y;
		zz = z * z;
		xy = x * y;
		yz = y * z;
		zx = z * x;
		xs = x * s;
		ys = y * s;
		zs = z * s;
		one_c = 1.0F - c;

		/* We already hold the identity-matrix so we can skip some statements */
		m(0,0) = (one_c * xx) + c;
		m(0,1) = (one_c * xy) - zs;
		m(0,2) = (one_c * zx) + ys;
		/*    M(0,3) = 0.0F; */

		m(1,0) = (one_c * xy) + zs;
		m(1,1) = (one_c * yy) + c;
		m(1,2) = (one_c * yz) - xs;
		/*    M(1,3) = 0.0F; */

		m(2,0) = (one_c * zx) - ys;
		m(2,1) = (one_c * yz) + xs;
		m(2,2) = (one_c * zz) + c;
		/*    M(2,3) = 0.0F; */

		/*
		M(3,0) = 0.0F;
		M(3,1) = 0.0F;
		M(3,2) = 0.0F;
		M(3,3) = 1.0F;
		*/
	}
	return m;
}

inline boost::numeric::ublas::matrix <float> translate (float x,float y,float z)
{
	boost::numeric::ublas::matrix <float> m(4,4);
	m = boost::numeric::ublas::identity_matrix<float>(4);
	m(3,0) = x;
	m(3,1) = y;
	m(3,2) = z;
	return m;
}





#if 0
class Matrix
{
	public:
		enum Matrixtype {
			MATRIX_GENERAL,	/**< general 4x4 matrix */
			MATRIX_IDENTITY,	/**< identity matrix */
			MATRIX_3D_NO_ROT,	/**< orthogonal projection and others... */
			MATRIX_PERSPECTIVE,	/**< perspective projection matrix */
			MATRIX_2D,		/**< 2-D transformation */
			MATRIX_2D_NO_ROT,	/**< 2-D scale & translate only */
			MATRIX_3D		/**< 3-D transformation */
		};

		Matrix ();
		Matrix (const Matrix &m);
		Matrix (const float f[16]);
		~Matrix();

		Matrix & operator = (const Matrix &rhs);
		Matrix & operator = (const float f[16]);
		Matrix & operator *= (const Matrix &rhs);
		Matrix & operator *= (const float *mul);
		const Matrix operator*(const Matrix &other) const;

		// Geometry
		Matrix & rotate(float angle, float x, float y, float z);
	  Matrix & translate(float x, float y, float z);
		static Matrix  scale(float x, float y, float z);
		static Matrix  identity();

		// Projection matrices
		Matrix & viewport(int x, int y, int width, int height,
											float zNear, float zFar, float depthMax);
		Matrix &  ortho (float left, float right,
											float bottom, float top,
											float nearval, float farval);
		Matrix & frustum( float left, float right,
											float bottom, float top,
											float nearval, float farval);
		Matrix inverse ();
		void analyse();
		bool is_length_preserving() const;
		bool has_rotation() const;
		bool is_general_scale() const;
		bool is_dirty() const;

	private:
		void mul4 (float *product, const float *const a, const float *const b) const;
		void mul34 (float *product, const float *const a, const float *const b) const;
		Matrix & multf (float *rhs, unsigned int flags_);
		bool invert_general();
		bool invert_3d_general();
		bool invert_3d();
		bool invert_identity();
		bool invert_3d_no_rot();
		bool invert_2d_no_rot();
		bool invert();

		void analyse_from_scratch();
		void analyse_from_flags();

		float m[16];
		float *inv;
		unsigned int flags;
		enum Matrixtype type;
};
#endif

#endif