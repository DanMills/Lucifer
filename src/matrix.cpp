/* $Id: matrix.cpp 19 2010-07-03 16:53:57Z dmills $ */

#if 0

// Matrix operations on 4 * 4 matrices
#include <math.h>
#include <string.h>
#include <assert.h>

#include "matrix.h"

/** Dot product of two 2-element vectors */
#define DOT2(a, b)  ((a)[0]*(b)[0] + (a)[1]*(b)[1])

/** Dot product of two 3-element vectors */
#define DOT3(a, b)  ((a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2])

/** Cross product of two 3-element vectors */
#define CROSS3(n, u, v)             \
do {                        \
   (n)[0] = (u)[1]*(v)[2] - (u)[2]*(v)[1];  \
   (n)[1] = (u)[2]*(v)[0] - (u)[0]*(v)[2];  \
   (n)[2] = (u)[0]*(v)[1] - (u)[1]*(v)[0];  \
} while (0)

#define MAT_FLAG_IDENTITY       0     /**< is an identity matrix flag.
*   (Not actually used - the identity
     *   matrix is identified by the absense
     *   of all other flags.)
*/
#define MAT_FLAG_GENERAL        0x1   /**< is a general matrix flag */
#define MAT_FLAG_ROTATION       0x2   /**< is a rotation matrix flag */
#define MAT_FLAG_TRANSLATION    0x4   /**< is a translation matrix flag */
#define MAT_FLAG_UNIFORM_SCALE  0x8   /**< is an uniform scaling matrix flag */
#define MAT_FLAG_GENERAL_SCALE  0x10  /**< is a general scaling matrix flag */
#define MAT_FLAG_GENERAL_3D     0x20  /**< general 3D matrix flag */
#define MAT_FLAG_PERSPECTIVE    0x40  /**< is a perspective proj matrix flag */
#define MAT_FLAG_SINGULAR       0x80  /**< is a singular matrix flag */
#define MAT_DIRTY_TYPE          0x100  /**< matrix type is dirty */
#define MAT_DIRTY_FLAGS         0x200  /**< matrix flags are dirty */
#define MAT_DIRTY_INVERSE       0x400  /**< matrix inverse is dirty */

/** angle preserving matrix flags mask */
#define MAT_FLAGS_ANGLE_PRESERVING (MAT_FLAG_ROTATION |	   \
				    MAT_FLAG_TRANSLATION |	\
				    MAT_FLAG_UNIFORM_SCALE)

/** geometry related matrix flags mask */
#define MAT_FLAGS_GEOMETRY (MAT_FLAG_GENERAL |	\
			    MAT_FLAG_ROTATION |	   \
			    MAT_FLAG_TRANSLATION |   \
			    MAT_FLAG_UNIFORM_SCALE | \
			    MAT_FLAG_GENERAL_SCALE | \
			    MAT_FLAG_GENERAL_3D |    \
			    MAT_FLAG_PERSPECTIVE |   \
	                    MAT_FLAG_SINGULAR)

/** length preserving matrix flags mask */
#define MAT_FLAGS_LENGTH_PRESERVING (MAT_FLAG_ROTATION |	\
				     MAT_FLAG_TRANSLATION)


/** 3D (non-perspective) matrix flags mask */
#define MAT_FLAGS_3D (MAT_FLAG_ROTATION |    \
		      MAT_FLAG_TRANSLATION |   \
		      MAT_FLAG_UNIFORM_SCALE | \
		      MAT_FLAG_GENERAL_SCALE | \
		      MAT_FLAG_GENERAL_3D)

/** dirty matrix flags mask */
#define MAT_DIRTY          (MAT_DIRTY_TYPE |  \
			    MAT_DIRTY_FLAGS |	\
			    MAT_DIRTY_INVERSE)
/**
 * Test geometry related matrix flags.
 *
 * \param mat a pointer to a Matrix structure.
 * \param a flags mask.
 *
 * \returns non-zero if all geometry related matrix flags are contained within
 * the mask, or zero otherwise.
 */
#define TEST_MAT_FLAGS(mat, a)					\
  ((MAT_FLAGS_GEOMETRY & (~(a)) & ((mat)->flags) ) == 0)


#define MAT_SX 0
#define MAT_SY 5
#define MAT_SZ 10
#define MAT_TX 12
#define MAT_TY 13
#define MAT_TZ 14

#define DEG2RAD (M_PI/180.0)

static const float Identity[16] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

Matrix::Matrix ()
{
    memcpy (m,Identity,sizeof (Identity));
    inv = NULL;
    flags = MAT_FLAG_IDENTITY;
    type = Matrix::MATRIX_IDENTITY;
}

Matrix::Matrix (const Matrix &rhs)
{
    inv = NULL;
    memcpy (m,rhs.m,16 * sizeof (m[0]));
    if (rhs.inv) {
        inv = new float [16];
        memcpy (inv,rhs.inv,16 * sizeof (inv[0]));
    }
    flags = rhs.flags;
    type = rhs.type;
}


Matrix::~Matrix()
{
    delete[] inv;
    inv = NULL;
}

Matrix & Matrix::operator = (const Matrix &rhs)
{
    if (this == &rhs) {
        return *this;
    }
    if ((!rhs.inv) && inv) {
        delete [] inv;
        inv = NULL;
    }
    if (rhs.inv && !inv) {
        inv = new float [16];
    }
    memcpy (m,rhs.m,16 * sizeof (m[0]));
    if (inv) {
        memcpy (inv,rhs.inv,16 * sizeof (inv[0]));
    }
    flags = rhs.flags;
    type = rhs.type;
    return *this;
}


#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

// Matrix multiply
// one of several used for *= depending on circumstances
void Matrix::mul4 (float *product, const float *const a, const float *const b) const
{
    for (unsigned int i = 0; i < 4; i++) {
        const float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
        P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
        P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
        P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
        P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
    }
}

void Matrix::mul34 (float *product, const float *const a, const float *const b) const
{
    for (unsigned int i = 0; i < 3; i++) {
        const float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
        P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0);
        P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1);
        P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2);
        P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3;
    }
    P(3,0) = 0;
    P(3,1) = 0;
    P(3,2) = 0;
    P(3,3) = 1;
}

#undef A
#undef B
#undef P

Matrix & Matrix::multf (float *rhs, unsigned int flags_)
{
    assert (m);
    flags |= (flags_ | MAT_DIRTY_TYPE | MAT_DIRTY_INVERSE);
    if (TEST_MAT_FLAGS(this, MAT_FLAGS_3D))
        mul34 (m,m,rhs);
    else
        mul4(m,m,rhs);
    return *this;
}

Matrix & Matrix::operator *= (const Matrix &rhs)
{
    assert (m);
    flags |= rhs.flags | MAT_DIRTY_TYPE | MAT_DIRTY_INVERSE;

    if (TEST_MAT_FLAGS(this, MAT_FLAGS_3D))
        mul34(m,m,rhs.m);
    else
        mul4(m,m,rhs.m);
    return *this;
}

Matrix::Matrix (const float f[])
{
    memcpy (m,f,sizeof (m[0]) * 16);
    flags = (MAT_FLAG_GENERAL |
             MAT_DIRTY_TYPE |
             MAT_DIRTY_INVERSE |
             MAT_DIRTY_FLAGS);
};

Matrix & Matrix::operator = (const float f[16])
{
    flags = (MAT_FLAG_GENERAL |
             MAT_DIRTY_TYPE |
             MAT_DIRTY_INVERSE |
             MAT_DIRTY_FLAGS);
    memcpy (m,f,16 * sizeof (m[0]));
    return *this;
}


Matrix & Matrix::operator *= (const float *mul)
{
    assert (mul);
    assert (m);
    flags |= (MAT_FLAG_GENERAL |
              MAT_DIRTY_TYPE |
              MAT_DIRTY_INVERSE |
              MAT_DIRTY_FLAGS);

    mul4(m,m,mul);
    return *this;
};

const Matrix Matrix::operator*(const Matrix &other) const
{
    return  Matrix(*this) *= other;
}

#define MAT(m,r,c) (m)[(c)*4+(r)]
#define SWAP_ROWS(a, b) {float *_tmp = a; (a)=(b); (b)=_tmp;}

/*
 * \author
 * Code contributed by Jacques Leroy jle@star.be
 *
 * Calculates the inverse matrix by performing the gaussian matrix reduction
 * with partial pivoting followed by back/substitution with the loops manually
 * unrolled.
 */
bool Matrix::invert_general()
{
    assert (m);
    assert (inv);

    float *out = inv;
    float wtmp[4][8];
    float m0, m1, m2, m3, s;
    float *r0, *r1, *r2, *r3;

    r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];
    r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
		r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
    r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

		r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
		r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
		r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

		r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
		r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
		r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

		r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
		r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
    r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

		/* choose pivot - or die */
    if (fabsf(r3[0])>fabsf(r2[0])) SWAP_ROWS(r3, r2);
    if (fabsf(r2[0])>fabsf(r1[0])) SWAP_ROWS(r2, r1);
    if (fabsf(r1[0])>fabsf(r0[0])) SWAP_ROWS(r1, r0);
    if (0.0 == r0[0])  return false;

    /* eliminate first variable     */
    m1 = r1[0]/r0[0];
    m2 = r2[0]/r0[0];
    m3 = r3[0]/r0[0];
    s = r0[1];
    r1[1] -= m1 * s;
    r2[1] -= m2 * s;
    r3[1] -= m3 * s;
    s = r0[2];
    r1[2] -= m1 * s;
    r2[2] -= m2 * s;
    r3[2] -= m3 * s;
    s = r0[3];
    r1[3] -= m1 * s;
    r2[3] -= m2 * s;
    r3[3] -= m3 * s;
    s = r0[4];
    if (s != 0.0) {
        r1[4] -= m1 * s;
        r2[4] -= m2 * s;
        r3[4] -= m3 * s;
    }
    s = r0[5];
    if (s != 0.0) {
        r1[5] -= m1 * s;
        r2[5] -= m2 * s;
        r3[5] -= m3 * s;
    }
    s = r0[6];
    if (s != 0.0) {
        r1[6] -= m1 * s;
        r2[6] -= m2 * s;
        r3[6] -= m3 * s;
    }
    s = r0[7];
    if (s != 0.0) {
        r1[7] -= m1 * s;
        r2[7] -= m2 * s;
        r3[7] -= m3 * s;
    }

    /* choose pivot - or die */
    if (fabsf(r3[1])>fabsf(r2[1])) SWAP_ROWS(r3, r2);
    if (fabsf(r2[1])>fabsf(r1[1])) SWAP_ROWS(r2, r1);
    if (0.0 == r1[1])  return false;

    /* eliminate second variable */
    m2 = r2[1]/r1[1];
    m3 = r3[1]/r1[1];
    r2[2] -= m2 * r1[2];
    r3[2] -= m3 * r1[2];
    r2[3] -= m2 * r1[3];
    r3[3] -= m3 * r1[3];
    s = r1[4];
    if (0.0 != s) {
        r2[4] -= m2 * s;
        r3[4] -= m3 * s;
    }
    s = r1[5];
    if (0.0 != s) {
        r2[5] -= m2 * s;
        r3[5] -= m3 * s;
    }
    s = r1[6];
    if (0.0 != s) {
        r2[6] -= m2 * s;
        r3[6] -= m3 * s;
    }
    s = r1[7];
    if (0.0 != s) {
        r2[7] -= m2 * s;
        r3[7] -= m3 * s;
    }

    /* choose pivot - or die */
    if (fabsf(r3[2])>fabsf(r2[2])) SWAP_ROWS(r3, r2);
    if (0.0 == r2[2])  return false;

    /* eliminate third variable */
    m3 = r3[2]/r2[2];
    r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
		r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
    r3[7] -= m3 * r2[7];

    /* last check */
    if (0.0 == r3[3]) return false;

    s = 1.0F/r3[3];             /* now back substitute row 3 */
    r3[4] *= s;
    r3[5] *= s;
    r3[6] *= s;
    r3[7] *= s;

    m2 = r2[3];                 /* now back substitute row 2 */
    s  = 1.0F/r2[2];
    r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
    r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
    m1 = r1[3];
    r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
    r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
    m0 = r0[3];
    r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
    r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

    m1 = r1[2];                 /* now back substitute row 1 */
    s  = 1.0F/r1[1];
    r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
    r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
    m0 = r0[2];
    r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
    r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

    m0 = r0[1];                 /* now back substitute row 0 */
    s  = 1.0F/r0[0];
    r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
		r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

    MAT(out,0,0) = r0[4];
    MAT(out,0,1) = r0[5],
    MAT(out,0,2) = r0[6];
    MAT(out,0,3) = r0[7],
    MAT(out,1,0) = r1[4];
    MAT(out,1,1) = r1[5],
    MAT(out,1,2) = r1[6];
    MAT(out,1,3) = r1[7],
    MAT(out,2,0) = r2[4];
    MAT(out,2,1) = r2[5],
    MAT(out,2,2) = r2[6];
    MAT(out,2,3) = r2[7],
    MAT(out,3,0) = r3[4];
    MAT(out,3,1) = r3[5],
    MAT(out,3,2) = r3[6];
    MAT(out,3,3) = r3[7];

    return true;
}
#undef SWAP_ROWS

/*
 * \author Adapted from graphics gems II.
 *
 * Calculates the inverse of the upper left by first calculating its
 * determinant and multiplying it to the symmetric adjust matrix of each
 * element. Finally deals with the translation part by transforming the
 * original translation vector using by the calculated submatrix inverse.
 */

bool Matrix::invert_3d_general()
{
    const float *in = m;
    float *out = inv;
    float pos, neg, t;
    float det;

    assert (m);
    assert (inv);

    /* Calculate the determinant of upper left 3x3 submatrix and
     * determine if the matrix is singular.
     */
    pos = neg = 0.0;
    t =  MAT(in,0,0) * MAT(in,1,1) * MAT(in,2,2);
    if (t >= 0.0) pos += t;
    else neg += t;

    t =  MAT(in,1,0) * MAT(in,2,1) * MAT(in,0,2);
    if (t >= 0.0) pos += t;
    else neg += t;

    t =  MAT(in,2,0) * MAT(in,0,1) * MAT(in,1,2);
    if (t >= 0.0) pos += t;
    else neg += t;

    t = -MAT(in,2,0) * MAT(in,1,1) * MAT(in,0,2);
    if (t >= 0.0) pos += t;
    else neg += t;

    t = -MAT(in,1,0) * MAT(in,0,1) * MAT(in,2,2);
    if (t >= 0.0) pos += t;
    else neg += t;

    t = -MAT(in,0,0) * MAT(in,2,1) * MAT(in,1,2);
    if (t >= 0.0) pos += t;
    else neg += t;

    det = pos + neg;

    if (det*det < 1e-25)
        return false;

    det = 1.0F / det;
    MAT(out,0,0) = (  (MAT(in,1,1)*MAT(in,2,2) - MAT(in,2,1)*MAT(in,1,2) )*det);
    MAT(out,0,1) = (- (MAT(in,0,1)*MAT(in,2,2) - MAT(in,2,1)*MAT(in,0,2) )*det);
    MAT(out,0,2) = (  (MAT(in,0,1)*MAT(in,1,2) - MAT(in,1,1)*MAT(in,0,2) )*det);
    MAT(out,1,0) = (- (MAT(in,1,0)*MAT(in,2,2) - MAT(in,2,0)*MAT(in,1,2) )*det);
    MAT(out,1,1) = (  (MAT(in,0,0)*MAT(in,2,2) - MAT(in,2,0)*MAT(in,0,2) )*det);
    MAT(out,1,2) = (- (MAT(in,0,0)*MAT(in,1,2) - MAT(in,1,0)*MAT(in,0,2) )*det);
    MAT(out,2,0) = (  (MAT(in,1,0)*MAT(in,2,1) - MAT(in,2,0)*MAT(in,1,1) )*det);
    MAT(out,2,1) = (- (MAT(in,0,0)*MAT(in,2,1) - MAT(in,2,0)*MAT(in,0,1) )*det);
    MAT(out,2,2) = (  (MAT(in,0,0)*MAT(in,1,1) - MAT(in,1,0)*MAT(in,0,1) )*det);

    /* Do the translation part */
    MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0) +
                      MAT(in,1,3) * MAT(out,0,1) +
                      MAT(in,2,3) * MAT(out,0,2) );
    MAT(out,1,3) = - (MAT(in,0,3) * MAT(out,1,0) +
                      MAT(in,1,3) * MAT(out,1,1) +
                      MAT(in,2,3) * MAT(out,1,2) );
    MAT(out,2,3) = - (MAT(in,0,3) * MAT(out,2,0) +
                      MAT(in,1,3) * MAT(out,2,1) +
                      MAT(in,2,3) * MAT(out,2,2) );

    return true;
}

/**
 * Compute inverse of a 3d transformation matrix.
 *
 * \return true for success, false for failure (\p singular matrix).
 *
 * If the matrix is not an angle preserving matrix then calls
 * invert_matrix_3d_general for the actual calculation. Otherwise calculates
 * the inverse matrix analyzing and inverting each of the scaling, rotation and
 * translation parts.
 */
bool Matrix::invert_3d()
{
    const float *in = m;
    float *out = inv;

    assert (m);
    assert (inv);

    if (!TEST_MAT_FLAGS(this, MAT_FLAGS_ANGLE_PRESERVING)) {
        return invert_3d_general();
    }

    if (flags & MAT_FLAG_UNIFORM_SCALE) {
        float scale = (MAT(in,0,0) * MAT(in,0,0) +
                       MAT(in,0,1) * MAT(in,0,1) +
                       MAT(in,0,2) * MAT(in,0,2));

        if (scale == 0.0) {
            return false;
        }
        scale = 1.0F / scale;

        /* Transpose and scale the 3 by 3 upper-left submatrix. */
        MAT(out,0,0) = scale * MAT(in,0,0);
        MAT(out,1,0) = scale * MAT(in,0,1);
        MAT(out,2,0) = scale * MAT(in,0,2);
        MAT(out,0,1) = scale * MAT(in,1,0);
        MAT(out,1,1) = scale * MAT(in,1,1);
        MAT(out,2,1) = scale * MAT(in,1,2);
        MAT(out,0,2) = scale * MAT(in,2,0);
        MAT(out,1,2) = scale * MAT(in,2,1);
        MAT(out,2,2) = scale * MAT(in,2,2);
    } else if (flags & MAT_FLAG_ROTATION) {
        /* Transpose the 3 by 3 upper-left submatrix. */
        MAT(out,0,0) = MAT(in,0,0);
        MAT(out,1,0) = MAT(in,0,1);
        MAT(out,2,0) = MAT(in,0,2);
        MAT(out,0,1) = MAT(in,1,0);
        MAT(out,1,1) = MAT(in,1,1);
        MAT(out,2,1) = MAT(in,1,2);
        MAT(out,0,2) = MAT(in,2,0);
        MAT(out,1,2) = MAT(in,2,1);
        MAT(out,2,2) = MAT(in,2,2);
    } else {
        /* pure translation */
        memcpy( out, Identity, sizeof(Identity));
        MAT(out,0,3) = - MAT(in,0,3);
        MAT(out,1,3) = - MAT(in,1,3);
        MAT(out,2,3) = - MAT(in,2,3);
        return true;
    }
    if (flags & MAT_FLAG_TRANSLATION) {
        /* Do the translation part */
        MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0) +
                          MAT(in,1,3) * MAT(out,0,1) +
                          MAT(in,2,3) * MAT(out,0,2) );
        MAT(out,1,3) = - (MAT(in,0,3) * MAT(out,1,0) +
                          MAT(in,1,3) * MAT(out,1,1) +
                          MAT(in,2,3) * MAT(out,1,2) );
        MAT(out,2,3) = - (MAT(in,0,3) * MAT(out,2,0) +
                          MAT(in,1,3) * MAT(out,2,1) +
                          MAT(in,2,3) * MAT(out,2,2) );
    } else {
        MAT(out,0,3) = MAT(out,1,3) = MAT(out,2,3) = 0.0;
    }
    return true;
}

bool Matrix::invert_identity()
{
    memcpy( inv, Identity, sizeof(Identity));
    return true;
}

/**
 * Compute inverse of a no-rotation 3d transformation matrix.
 *
 * The matrix inverse will be
 * stored in the inv attribute.
 *
 * \return true for success, false for failure (\p singular matrix).
 *
 */
bool Matrix::invert_3d_no_rot()
{
    const float *in = m;
    float *out = inv;

    assert (m);
    assert (inv);

    if (MAT(in,0,0) == 0 || MAT(in,1,1) == 0 || MAT(in,2,2) == 0 )
        return false;

    memcpy(out, Identity, sizeof (Identity));
    MAT(out,0,0) = 1.0F / MAT(in,0,0);
    MAT(out,1,1) = 1.0F / MAT(in,1,1);
    MAT(out,2,2) = 1.0F / MAT(in,2,2);

    if (flags & MAT_FLAG_TRANSLATION) {
        MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0));
        MAT(out,1,3) = - (MAT(in,1,3) * MAT(out,1,1));
        MAT(out,2,3) = - (MAT(in,2,3) * MAT(out,2,2));
    }
    return true;
}

/**
 * Compute inverse of a no-rotation 2d transformation matrix.
 *
 * \param mat pointer to a GLmatrix structure. The matrix inverse will be
 * stored in the GLmatrix::inv attribute.
 *
 * \return GL_TRUE for success, GL_FALSE for failure (\p singular matrix).
 *
 * Calculates the inverse matrix by applying the inverse scaling and
 * translation to the identity matrix.
 */
bool Matrix::invert_2d_no_rot()
{
    const float *in = m;
    float *out = inv;

    assert (m);
    assert (inv);

    if (MAT(in,0,0) == 0 || MAT(in,1,1) == 0)
        return false;

    memcpy(out, Identity, sizeof (Identity));
    MAT(out,0,0) = 1.0F / MAT(in,0,0);
    MAT(out,1,1) = 1.0F / MAT(in,1,1);

    if (flags & MAT_FLAG_TRANSLATION) {
        MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0));
        MAT(out,1,3) = - (MAT(in,1,3) * MAT(out,1,1));
    }
    return true;
}

/**
 * Compute inverse of a transformation matrix.
 *
 * \return true for success, false for failure (\p singular matrix).
 *
 * Calls the matrix inversion function corresponding to the
 * given matrix type.  In case of failure, updates the MAT_FLAG_SINGULAR flag,
 * and copies the identity matrix into Matrix::inv.
 */
bool Matrix::invert()
{
    bool res = false;

    analyse(); // make sure the type is right
    switch (type) {
    case Matrix::MATRIX_GENERAL :
        res = invert_general();
        break;
    case Matrix::MATRIX_IDENTITY :
        res = invert_identity ();
        break;
    case Matrix::MATRIX_3D_NO_ROT :
        res = invert_3d_no_rot ();
        break;
    case Matrix::MATRIX_PERSPECTIVE :
        res = invert_general();
        break;
    case Matrix::MATRIX_2D :
        res = invert_3d();
        break;
    case Matrix::MATRIX_2D_NO_ROT :
        res = invert_2d_no_rot ();
        break;
    case Matrix::MATRIX_3D :
        res = invert_3d ();
        break;
    }
    if (res) {
        flags &= ~MAT_FLAG_SINGULAR;
        return true;
    } else {
        flags |= MAT_FLAG_SINGULAR;
        memcpy(inv, Identity, sizeof(Identity));
        return false;
    }
}

Matrix Matrix::inverse ()
{
    if (!inv) {
        inv = new float [16];
    }
    if (invert()) {
        flags &= ~MAT_DIRTY_INVERSE;
        Matrix mat;
        mat.inv = new float[16];
        memcpy (mat.m,inv,sizeof (float) * 16);
        memcpy (mat.inv,m,sizeof (float) * 16);
        return mat;
    } else {
        Matrix mat;
        return mat;
    }
}

/**
 * Generate a 4x4 transformation matrix from Rotate parameters, and
 * post-multiply the matrix by it.
 *
 * \author
 * Based on code from Mesa contributed by Erich Boleyn (erich@uruk.org).
 * Optimizations contributed by Rudolf Opalla (rudi@khm.de).
 */
#define M(row,col)  m_[col*4+row]
Matrix &  Matrix::rotate(float angle, float x, float y, float z )
{
    float s;
    float c;
    float m_[16];
    bool optimized;

    s = sinf( angle * DEG2RAD );
    c = cosf( angle * DEG2RAD );

    memcpy(m_, Identity, sizeof(Identity));
    optimized = false;

    if (x == 0.0F) {
        if (y == 0.0F) {
            if (z != 0.0F) {
                optimized = true;
                /* rotate only around z-axis */
                M(0,0) = c;
                M(1,1) = c;
                if (z < 0.0F) {
                    M(0,1) = s;
                    M(1,0) = -s;
                } else {
                    M(0,1) = -s;
                    M(1,0) = s;
                }
            }
        }
        else if (z == 0.0F) {
            optimized = true;
            /* rotate only around y-axis */
            M(0,0) = c;
            M(2,2) = c;
            if (y < 0.0F) {
                M(0,2) = -s;
                M(2,0) = s;
            } else {
                M(0,2) = s;
                M(2,0) = -s;
            }
        }
    }
    else if (y == 0.0F) {
        if (z == 0.0F) {
            optimized = true;
            /* rotate only around x-axis */
            M(1,1) = c;
            M(2,2) = c;
            if (x < 0.0F) {
                M(1,2) = s;
                M(2,1) = -s;
            } else {
                M(1,2) = -s;
                M(2,1) = s;
            }
        }
    }

    if (!optimized) {
        const float mag = sqrtf(x * x + y * y + z * z);

        if (mag <= 1.0e-4) {
            /* no rotation, leave mat as-is */
            return *this;
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
        M(0,0) = (one_c * xx) + c;
        M(0,1) = (one_c * xy) - zs;
        M(0,2) = (one_c * zx) + ys;
        /*    M(0,3) = 0.0F; */

        M(1,0) = (one_c * xy) + zs;
        M(1,1) = (one_c * yy) + c;
        M(1,2) = (one_c * yz) - xs;
        /*    M(1,3) = 0.0F; */

        M(2,0) = (one_c * zx) - ys;
        M(2,1) = (one_c * yz) + xs;
        M(2,2) = (one_c * zz) + c;
        /*    M(2,3) = 0.0F; */

        /*
          M(3,0) = 0.0F;
          M(3,1) = 0.0F;
          M(3,2) = 0.0F;
          M(3,3) = 1.0F;
        */
    }
    multf(m_, MAT_FLAG_ROTATION);
    return *this;
}
#undef M

/**
 * Apply a perspective projection matrix.
 *
 * \param left left clipping plane coordinate.
 * \param right right clipping plane coordinate.
 * \param bottom bottom clipping plane coordinate.
 * \param top top clipping plane coordinate.
 * \param nearval distance to the near clipping plane.
 * \param farval distance to the far clipping plane.
 *
 * Creates the projection matrix and multiplies it with this matrix,
 * marking the MAT_FLAG_PERSPECTIVE flag.
 */

Matrix & Matrix::frustum( float left, float right,
                          float bottom, float top,
                          float nearval, float farval )
{
    float x, y, a, b, c, d;
    float m_[16];

    x = (2.0F*nearval) / (right-left);
    y = (2.0F*nearval) / (top-bottom);
    a = (right+left) / (right-left);
    b = (top+bottom) / (top-bottom);
    c = -(farval+nearval) / ( farval-nearval);
    d = -(2.0F*farval*nearval) / (farval-nearval);  /* error? */

#define M(row,col)  m_[col*4+row]
    M(0,0) = x;
    M(0,1) = 0.0F;
    M(0,2) = a;
    M(0,3) = 0.0F;
    M(1,0) = 0.0F;
    M(1,1) = y;
    M(1,2) = b;
    M(1,3) = 0.0F;
    M(2,0) = 0.0F;
    M(2,1) = 0.0F;
    M(2,2) = c;
    M(2,3) = d;
    M(3,0) = 0.0F;
    M(3,1) = 0.0F;
    M(3,2) = -1.0F;
    M(3,3) = 0.0F;
#undef M

    multf(m_, MAT_FLAG_PERSPECTIVE);
    return *this;
}

/**
 * Apply an orthographic projection matrix.
 *
 * \param left left clipping plane coordinate.
 * \param right right clipping plane coordinate.
 * \param bottom bottom clipping plane coordinate.
 * \param top top clipping plane coordinate.
 * \param nearval distance to the near clipping plane.
 * \param farval distance to the far clipping plane.
 *
 * Creates the projection matrix and multiplies it with this,
 * marking the MAT_FLAG_GENERAL_SCALE and MAT_FLAG_TRANSLATION flags.
 */
Matrix &  Matrix::ortho (float left, float right,
                         float bottom, float top,
                         float nearval, float farval)
{
    float m_[16];

#define M(row,col)  m_[col*4+row]
    M(0,0) = 2.0F / (right-left);
    M(0,1) = 0.0F;
    M(0,2) = 0.0F;
    M(0,3) = -(right+left) / (right-left);

    M(1,0) = 0.0F;
    M(1,1) = 2.0F / (top-bottom);
    M(1,2) = 0.0F;
    M(1,3) = -(top+bottom) / (top-bottom);

    M(2,0) = 0.0F;
    M(2,1) = 0.0F;
    M(2,2) = -2.0F / (farval-nearval);
    M(2,3) = -(farval+nearval) / (farval-nearval);

    M(3,0) = 0.0F;
    M(3,1) = 0.0F;
    M(3,2) = 0.0F;
    M(3,3) = 1.0F;
#undef M

    multf(m_, (MAT_FLAG_GENERAL_SCALE|MAT_FLAG_TRANSLATION));
    return *this;
}

/**
 * Multiply matrix with a general scaling matrix.
 *
 * \param x x axis scale factor.
 * \param y y axis scale factor.
 * \param z z axis scale factor.
 *
 * Multiplies in-place the elements of matrix by the scale factors. Checks if
 * the scales factors are roughly the same, marking the MAT_FLAG_UNIFORM_SCALE
 * flag, or MAT_FLAG_GENERAL_SCALE. Marks the MAT_DIRTY_TYPE and
 * MAT_DIRTY_INVERSE dirty flags.
 */
Matrix Matrix::scale(float x, float y, float z )
{
		Matrix s;
    s.m[0] *= x;
    s.m[4] *= y;
    s.m[8]  *= z;
    s.m[1] *= x;
    s.m[5] *= y;
    s.m[9]  *= z;
    s.m[2] *= x;
    s.m[6] *= y;
    s.m[10] *= z;
    s.m[3] *= x;
    s.m[7] *= y;
    s.m[11] *= z;

    if (fabsf(x - y) < 1e-8 && fabsf(x - z) < 1e-8)
        s.flags |= MAT_FLAG_UNIFORM_SCALE;
    else
        s.flags |= MAT_FLAG_GENERAL_SCALE;

    s.flags |= (MAT_DIRTY_TYPE |
              MAT_DIRTY_INVERSE);
		return s;
}

/**
 * Multiply a matrix with a translation matrix.
 *
 * \param x translation vector x coordinate.
 * \param y translation vector y coordinate.
 * \param z translation vector z coordinate.
 *
 * Adds the translation coordinates to the elements of matrix in-place.  Marks
 * the MAT_FLAG_TRANSLATION flag, and the MAT_DIRTY_TYPE and MAT_DIRTY_INVERSE
 * dirty flags.
 */
Matrix & Matrix::translate(float x, float y, float z)
{
    m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
    m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
    m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
    m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];

    flags |= (MAT_FLAG_TRANSLATION |
              MAT_DIRTY_TYPE |
              MAT_DIRTY_INVERSE);
    return *this;
}


/**
 * Set matrix to do viewport and depthrange mapping.
 * Transforms Normalized Device Coords to window/Z values.
 */
Matrix & Matrix::viewport(int x, int y, int width, int height,
                          float zNear, float zFar, float depthMax)
{
    m[MAT_SX] = (float) width / 2.0F;
    m[MAT_TX] = m[MAT_SX] + x;
    m[MAT_SY] = (float) height / 2.0F;
    m[MAT_TY] = m[MAT_SY] + y;
    m[MAT_SZ] = depthMax * ((zFar - zNear) / 2.0F);
    m[MAT_TZ] = depthMax * ((zFar - zNear) / 2.0F + zNear);
    flags = MAT_FLAG_GENERAL_SCALE | MAT_FLAG_TRANSLATION;
    type = MATRIX_3D_NO_ROT;
    return *this;
}



/**
 * Set a matrix to the identity matrix.
 *
 * \param mat matrix.
 *
 * Copies ::Identity into matrix and into matrix::inv if not NULL.
 * Sets the matrix type to identity, and clear the dirty flags.
 */
Matrix Matrix::identity()
{
		Matrix i;
    memcpy(i.m, Identity,sizeof(Identity));
    i.type = MATRIX_IDENTITY;
    i.flags &= ~(MAT_DIRTY_FLAGS|
               MAT_DIRTY_TYPE);
    return i;
}

#define ZERO(x) (1<<x)
#define ONE(x)  (1<<(x+16))

#define MASK_NO_TRX (ZERO(12)| ZERO(13)| ZERO(14))
#define MASK_NO_2D_SCALE (ONE(0) | ONE(5))

#define MASK_IDENTITY (ONE(0) |ZERO(4)|ZERO(8) |ZERO(12)  |\
		       ZERO(1)|ONE(5) |ZERO(9) |ZERO(13)  |\
		       ZERO(2)|ZERO(6)|ONE(10) |ZERO(14)  |\
		       ZERO(3)|ZERO(7)|ZERO(11)|ONE(15))

#define MASK_2D_NO_ROT   (           ZERO(4)  | ZERO(8)  |           \
			  ZERO(1)  |            ZERO(9)  |           \
			  ZERO(2)  | ZERO(6)  |  ONE(10) | ZERO(14) |\
			  ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )

#define MASK_2D          (                      ZERO(8)  |           \
			                        ZERO(9)  |           \
			  ZERO(2)  | ZERO(6)  |  ONE(10) | ZERO(14) |\
			  ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )


#define MASK_3D_NO_ROT   (           ZERO(4)  | ZERO(8)  |           \
			  ZERO(1)  |            ZERO(9)  |           \
			  ZERO(2)  | ZERO(6)  |                      \
			  ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )

#define MASK_3D          (                                           \
			                                             \
			                                             \
			  ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )


#define MASK_PERSPECTIVE (           ZERO(4)  |            ZERO(12) |\
			  ZERO(1)  |                       ZERO(13) |\
			  ZERO(2)  | ZERO(6)  |                      \
			  ZERO(3)  | ZERO(7)  |            ZERO(15) )

#define SQ(x) ((x)*(x))
#define LEN_SQUARED_3FV(V) ((V)[0]*(V)[0]+(V)[1]*(V)[1]+(V)[2]*(V)[2])

#define SUB_3V( DST, SRCA, SRCB )        \
do {                                     \
      (DST)[0] = (SRCA)[0] - (SRCB)[0];  \
      (DST)[1] = (SRCA)[1] - (SRCB)[1];  \
      (DST)[2] = (SRCA)[2] - (SRCB)[2];  \
} while (0)

/**
 * Determine type and flags from scratch.
 *
 * This is expensive enough to only want to do it once.
 */
void Matrix::analyse_from_scratch()
{
    unsigned int mask = 0;
    unsigned int i;

    for (i = 0 ; i < 16 ; i++) {
        if (m[i] == 0.0) mask |= (1<<i);
    }

    if (m[0] == 1.0F) mask |= (1<<16);
    if (m[5] == 1.0F) mask |= (1<<21);
    if (m[10] == 1.0F) mask |= (1<<26);
    if (m[15] == 1.0F) mask |= (1<<31);

    flags &= ~MAT_FLAGS_GEOMETRY;

    /* Check for translation - no-one really cares
     */
    if ((mask & MASK_NO_TRX) != MASK_NO_TRX)
        flags |= MAT_FLAG_TRANSLATION;

    /* Do the real work
     */
    if (mask == (unsigned int) MASK_IDENTITY) {
        type = MATRIX_IDENTITY;
    } else if ((mask & MASK_2D_NO_ROT) == (unsigned int) MASK_2D_NO_ROT) {
        type = MATRIX_2D_NO_ROT;

        if ((mask & MASK_NO_2D_SCALE) != MASK_NO_2D_SCALE)
            flags |= MAT_FLAG_GENERAL_SCALE;
    } else if ((mask & MASK_2D) == (unsigned int) MASK_2D) {
        float mm = DOT2(m, m);
        float m4m4 = DOT2(m+4,m+4);
        float mm4 = DOT2(m,m+4);

        type = MATRIX_2D;

        /* Check for scale */
        if (SQ(mm-1) > SQ(1e-6) ||
                SQ(m4m4-1) > SQ(1e-6))
            flags |= MAT_FLAG_GENERAL_SCALE;

        /* Check for rotation */
        if (SQ(mm4) > SQ(1e-6))
            flags |= MAT_FLAG_GENERAL_3D;
        else
            flags |= MAT_FLAG_ROTATION;

    } else if ((mask & MASK_3D_NO_ROT) == (unsigned int) MASK_3D_NO_ROT) {
        type = MATRIX_3D_NO_ROT;

        /* Check for scale */
        if (SQ(m[0]-m[5]) < SQ(1e-6) &&
                SQ(m[0]-m[10]) < SQ(1e-6)) {
            if (SQ(m[0]-1.0) > SQ(1e-6)) {
                flags |= MAT_FLAG_UNIFORM_SCALE;
            }
        } else {
            flags |= MAT_FLAG_GENERAL_SCALE;
        }
    } else if ((mask & MASK_3D) == (unsigned int) MASK_3D) {
        float c1 = DOT3(m,m);
        float c2 = DOT3(m+4,m+4);
        float c3 = DOT3(m+8,m+8);
        float d1 = DOT3(m, m+4);
        float cp[3];

        type = MATRIX_3D;

        /* Check for scale */
        if (SQ(c1-c2) < SQ(1e-6) && SQ(c1-c3) < SQ(1e-6)) {
            if (SQ(c1-1.0) > SQ(1e-6))
                flags |= MAT_FLAG_UNIFORM_SCALE;
            /* else no scale at all */
        } else {
            flags |= MAT_FLAG_GENERAL_SCALE;
        }

        /* Check for rotation */
        if (SQ(d1) < SQ(1e-6)) {
            CROSS3( cp, m, m+4 );
            SUB_3V( cp, cp, (m+8) );
            if (LEN_SQUARED_3FV(cp) < SQ(1e-6))
                flags |= MAT_FLAG_ROTATION;
            else
                flags |= MAT_FLAG_GENERAL_3D;
        } else {
            flags |= MAT_FLAG_GENERAL_3D; /* shear, etc */
        }
    } else if ((mask & MASK_PERSPECTIVE) == MASK_PERSPECTIVE && m[11]==-1.0F) {
        type = MATRIX_PERSPECTIVE;
        flags |= MAT_FLAG_GENERAL;
    } else {
        type = MATRIX_GENERAL;
        flags |= MAT_FLAG_GENERAL;
    }
}

/**
 * Analyze a matrix given that its flags are accurate.
 *
 * This is the more common operation, hopefully.
 */
void Matrix::analyse_from_flags()
{
    if (TEST_MAT_FLAGS(this, 0)) {
        type = MATRIX_IDENTITY;
    } else if (TEST_MAT_FLAGS(this, (MAT_FLAG_TRANSLATION |
                                     MAT_FLAG_UNIFORM_SCALE |
                                     MAT_FLAG_GENERAL_SCALE))) {
        if ( m[10]==1.0F && m[14]==0.0F ) {
            type = MATRIX_2D_NO_ROT;
        } else {
            type = MATRIX_3D_NO_ROT;
        }
    } else if (TEST_MAT_FLAGS(this, MAT_FLAGS_3D)) {
        if (m[ 8]==0.0F &&
                m[ 9]==0.0F &&
                m[2]==0.0F &&
                m[6]==0.0F &&
                m[10]==1.0F &&
                m[14]==0.0F) {
            type = MATRIX_2D;
        } else {
            type = MATRIX_3D;
        }
    } else if ( m[4]==0.0F &&
                m[12]==0.0F&&
                m[1]==0.0F &&
                m[13]==0.0F&&
                m[2]==0.0F &&
                m[6]==0.0F &&
                m[3]==0.0F &&
                m[7]==0.0F &&
                m[11]==-1.0F &&
                m[15]==0.0F) {
        type = MATRIX_PERSPECTIVE;
    } else {
        type = MATRIX_GENERAL;
    }
}

/**
 * Analyze and update a matrix.
 *
 *
 * If the matrix type is dirty then calls either analyse_from_scratch() or
 * analyse_from_flags() to determine its type, according to whether the flags
 * are dirty or not, respectively. If the matrix has an inverse and it's dirty
 * then calls matrix_invert(). Finally clears the dirty flags.
 */
void Matrix::analyse()
{
    if (flags & MAT_DIRTY_TYPE) {
        if (flags & MAT_DIRTY_FLAGS)
            analyse_from_scratch();
        else
            analyse_from_flags();
    }
    if (inv && (flags & MAT_DIRTY_INVERSE)) {
        invert();
        flags &= ~MAT_DIRTY_INVERSE;
    }
    flags &= ~(MAT_DIRTY_FLAGS | MAT_DIRTY_TYPE);
}

/**
 * Test if the given matrix preserves vector lengths.
 */
bool Matrix::is_length_preserving() const
{
    return TEST_MAT_FLAGS(this, MAT_FLAGS_LENGTH_PRESERVING);
}


/**
 * Test if the given matrix does any rotation.
 * (or perhaps if the upper-left 3x3 is non-identity)
 */
bool Matrix::has_rotation() const
{
    return (flags & (MAT_FLAG_GENERAL |
                     MAT_FLAG_ROTATION |
                     MAT_FLAG_GENERAL_3D |
                     MAT_FLAG_PERSPECTIVE));
}

bool Matrix::is_general_scale() const
{
    return (flags & MAT_FLAG_GENERAL_SCALE);
}

bool Matrix::is_dirty() const
{
    return (flags & MAT_DIRTY);
}

#endif