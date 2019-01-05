//////////////////////////////////////////////////////////////////////////////////
//
//        title        :    Linear Algebra Functions
//                        
//        version        :    v1.99
//        author        :    Jinwook Kim (zinook@kist.re.kr)
//        last update    :    99.8.1
//        
//        Note        :    compatible with RMatrix v2.0 or later
//
//////////////////////////////////////////////////////////////////////////////////

#include "rmatrix.h"
#include <math.h>

#define _EPS 2.2204E-16
#define _TINY 1E-20
#define _INF 1E100

#define dsign(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))

real Norm(const RMatrix &m, real p)
{
    real re, tmp;
    int mx, mn, i;
    if ( m.row > m.col ) { mx = m.row; mn = m.col; }
    else { mx = m.col; mn = m.row; }
    
    if ( mn == 1 )
    {
        tmp = 0.0;
        if ( p == 0.0 )
        {
            for ( i = 0; i < mx; i++ ) tmp += *(m.element+i) * *(m.element+i);
            re = sqrt(tmp);
        } else if ( p > _INF ) re = MaxVec(Abs(m));
        else if ( p < -_INF ) re = MinVec(Abs(m));
        else
        {
            for ( i = 0; i < mx; i++ ) tmp += pow(fabs(*(m.element+i)), p);
            re = pow(tmp, 1.0/p);
        }
    } else
    {
        if ( p == 0.0 || p == 2.0 ) re = MaxVec(SVD(m));
        else if ( p == 1.0 ) re = MaxVec(Sum(Abs(m)));
        else error_("RMatrix Norm(RMatrix, real) : not approapriate second argument");
    }
    return re;
}

real pythag(real a, real b)
{
    real absa, absb;
    absa = fabs(a);
    absb = fabs(b);
    if ( absa > absb ) return absa*sqrt(1.0 + (absb/absa)*(absb/absa));
    else return ( absb == 0 ? 0 : absb*sqrt(1.0 + (absa/absb)*(absa/absb)) );
}

// singular value decomposition in 'numerical recipe'
// modified with EISPACK 'dsvd.f'
void svdcmp(int m, int n, real *a, real *w, real *v, int matu, int matv)
{
    int flag, i, its, j, jj, k, l, nm;
    real anorm, c, f, g, h, s, scale, x, y, z;
    real *rv1 = new real [n];
    g = scale = anorm = 0.0; // Householder reduction to bidiagonal form
    for(i = 0; i < n; i++) {
        l = i+1;
        rv1[i] = scale * g;
        g = s = scale = 0.0;
        if(i < m) {
            for(k = i; k < m; k++) scale += fabs(a[k+m*i]);
            if(scale) {
                for(k = i; k < m; k++) {
                    a[k+m*i] /= scale;
                    s += a[k+m*i] * a[k+m*i];
                }
                f = a[i+m*i];
                g = -dsign(sqrt(s), f);
                h = f * g - s;
                a[i+m*i] = f - g;
                for(j = l; j < n; j++) {
                    for(s = 0.0, k = i; k < m; k++) s += a[k+m*i] * a[k+m*j];
                    f = s / h;
                    for(k = i; k < m; k++) a[k+m*j] += f * a[k+m*i];
                }
                for(k = i; k < m; k++) a[k+m*i] *= scale;
            }
        }
        w[i] = scale * g;
        g = s = scale = 0.0;
        if(i < m && i != n-1) {
            for(k = l; k < n; k++) scale += fabs(a[i+m*k]);
            if(scale) {
                for(k = l; k < n; k++) {
                    a[i+m*k] /= scale;
                    s += a[i+m*k] * a[i+m*k];
                }
                f = a[i+m*l];
                g = -dsign(sqrt(s), f);
                h = f * g - s;
                a[i+m*l] = f - g; 
                for(k = l; k < n; k++) rv1[k] = a[i+m*k] / h;
                for(j = l; j < m; j++) {
                    for(s = 0.0, k = l; k < n; k++) s += a[j+m*k] * a[i+m*k];
                    for(k = l ; k < n; k++) a[j+m*k] += s * rv1[k];
                }
                for(k = l ; k < n; k++) a[i+m*k] *= scale;
            }
        }
        anorm = max(anorm, (fabs(w[i]) + fabs(rv1[i])));
    }
    if ( matv ) 
    {
        for(i = n-1 ; i >= 0; i--) { // Accumulation of right-hand transformations
            if(i < n) {
                if(g) {
                    for(j = l; j < n; j++) // real devision to avoid possible underflow
                        v[j+n*i] = (a[i+m*j] / a[i+m*l]) / g;
                    for(j = l; j < n; j++) {
                        for(s = 0.0, k = l; k < n; k++) s += a[i+m*k] * v[k+n*j];
                        for(k = l; k < n; k++) v[k+n*j] += s * v[k+n*i];
                    }
                }
                for(j = l; j < n; j++) v[i+n*j] = v[j+n*i] = 0.0;
            }
            v[i+n*i] = 1.0;
            g = rv1[i];
            l = i;
        }
    }
    if ( matu )
    {
        for(i = min(m-1, n-1); i >= 0; i--) { // Accumulation of left-hand transformations
            l = i + 1;
            g = w[i];
            for(j = l; j < n; j++) a[i+m*j] = 0.0;
            if(g) {
                g = 1.0 / g;
                for(j = l; j < n; j++) {
                    for(s = 0.0, k = l; k < m; k++) s += a[k+m*i] * a[k+m*j];
                    f = (s / a[i+m*i]) * g;
                    for(k = i; k < m; k++) a[k+m*j] += f * a[k+m*i];
                }
                for(j = i; j < m; j++) a[j+m*i] *= g;
            } else for(j = i; j < m; j++) a[j+m*i] = 0.0;
            ++a[i+m*i];
        }
    }
    for(k = n-1; k >= 0; k--) { // Diagonalization of the bidiagonal form: Loop over
        for(its = 1; its <= 30; its++) { // singular values, and over allowed iterations
            flag=1;
            for(l = k; l >= 0; l--) { // Test for splitting
                nm = l-1; // Note that rv1(1) is always zero
                if((fabs(rv1[l]) + anorm) == anorm) {
                    flag=0;
                    break;
                }
                if((fabs(w[nm]) + anorm) == anorm) break;
            }
            if(flag) {
                c = 0.0;
                s = 1.0;
                for(i = l; i <= k; i++) {
                    f = s * rv1[i];
                    rv1[i] = c * rv1[i];
                    if((fabs(f) + anorm) == anorm) break;
                    g = w[i];
                    h = pythag(f, g);
                    w[i] = h;
                    h = 1.0 / h;
                    c = g * h;
                    s = -f * h;
                    if ( matu ) 
                    {
                        for(j = 0; j < m; j++) {
                            y = a[j+m*nm];
                            z = a[j+m*i];
                            a[j+m*nm] = y * c + z * s;
                            a[j+m*i] = z * c - y * s;
                        }
                    }
                }
            }
            z = w[k];
            if(l == k) { // Convergence
                if(z < 0.0) { // Singular value is made nonnegative
                    w[k] = -z;
                    if ( matv ) for(j = 0; j < n; j++) v[j+n*k] = -v[j+n*k];
                }
                break;
            }
            if(its == 30) error_("svdcmp : no convergence in 30 iterations");
            x = w[l]; // Shift from bottom 2-by-2 minor
            nm = k-1;
            y = w[nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y-z)*(y+z) + (g-h)*(g+h)) / (2.0 * h * y);
            g = pythag(f, 1.0);
            f = ((x-z)*(x+z) + h * ((y / (f + dsign(g, f))) - h)) / x;
            c = s = 1.0; // Next QR transformation
            for(j = l; j <= nm; j++) {
                i = j+1;
                g = rv1[i];
                y = w[i];
                h = s * g;
                g = c * g;
                z = pythag(f, h);
                rv1[j] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y *= c;
                if ( matv ) 
                {
                    for(jj = 0; jj < n; jj++) {
                        x = v[jj+n*j];
                        z = v[jj+n*i];
                        v[jj+n*j] = x * c + z * s;
                        v[jj+n*i] = z * c - x * s;
                    }
                }
                z = pythag(f, h);
                w[j] = z; // Rotation can be arbitary if z = 0
                if(z) {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = c * g + s * y;
                x = c * y - s * g;
                if ( matu ) 
                {
                    for(jj = 0; jj < m; jj++) {
                        y = a[jj+m*j];
                        z = a[jj+m*i];
                        a[jj+m*j] = y * c + z * s;
                        a[jj+m*i] = z * c - y * s;
                    }
                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            w[k] = x;
        }
    }
    delete [] rv1;
}

void _change(real *v, const int i, const int j)
{
    real temp;
    temp = *(v+i);
    *(v+i) = *(v+j);
    *(v+j) = temp;
}

void _sort(real *v, const int n, const int left, const int right)
{
    int i, last; 
    if ( left >= right ) return;
    _change(v, left, (left+right)/2 );
    last = left;
    for ( i = left + 1; i <= right; i++ )
        if ( v[i] > v[left] ) _change(v, ++last, i);
    _change(v,left,last);
    _sort(v, n, left, last-1);
    _sort(v, n, last+1, right);
}

RMatrix SVD(const RMatrix &m)
{
    int i, j;
    real *u, *s;
    RMatrix re;
    if ( m.row > m.col ) 
    {
        u = new real [m.row*m.col];
        s = new real [m.col];
        re.row = m.col;
        for ( i = 0; i < m.row; i++ )
        for ( j = 0; j < m.col; j++ ) u[i+j*m.row] = m.element[i+j*m.row];
        svdcmp(m.row, m.col, u, s, 0, 0, 0);
        _sort(s, m.col, 0, m.col-1);
    } else
    {
        u = new real [m.row*m.col];
        s = new real [m.row];
        re.row = m.row;
        for ( i = 0; i < m.row; i++ )
        for ( j = 0; j < m.col; j++ ) u[j+i*m.col] = m.element[i+j*m.row];
        svdcmp(m.col, m.row, u, s, 0, 0, 0);
        _sort(s, m.row, 0, m.row-1);
    }
    
    delete [] u;
    re.col = 1;
    re.element = s;
    return re;
}

void _change(real *w, int i, int j, real *u, int nu, real *v, int nv)
{
    real _tmp;
    int _idx;
    _tmp = w[i];
    w[i] = w[j];
    w[j] = _tmp;
    for ( _idx = 0; _idx < nu; _idx++ )
    {
        _tmp = u[_idx+i*nu];
        u[_idx+i*nu] = u[_idx+j*nu];
        u[_idx+j*nu] = _tmp;
    }
    for ( _idx = 0; _idx < nv; _idx++ )
    {
        _tmp = v[_idx+i*nv];
        v[_idx+i*nv] = v[_idx+j*nv];
        v[_idx+j*nv] = _tmp;
    }
}

void _sort(real *w, int n, int left, int right, real *u, int nu, real *v, int nv)
{
    int i, last; 
    if ( left >= right ) return;
    _change(w, left, (left+right)/2, u, nu, v, nv);
    last = left;
    for ( i = left + 1; i <= right; i++ )
        if ( w[i] > w[left] ) _change(w, ++last, i, u, nu, v, nv);
    _change(w, left, last, u, nu, v, nv);
    _sort(w, n, left, last-1, u, nu, v, nv);
    _sort(w, n, last+1, right, u, nu, v, nv);
}

void SVD(const RMatrix &m, RMatrix &u, RMatrix &s, RMatrix &v)
{
    int i, j;
    if ( m.row > m.col )
    {
        if ( s.row*s.col != m.row*m.col )
        {
            delete [] s.element;
            s.element = new real [m.row*m.col];
        }
        s.row = m.row;
        s.col = m.col;

        if ( v.row*v.col != m.row*m.col )
        {
            delete [] v.element;
            v.element = new real [m.row*m.col];
        }
        v.row = m.col;
        v.col = m.row;

        for ( i = 0; i < m.row; i++ )
        for ( j = 0; j < m.col; j++ ) v.element[j+i*v.row] = m.element[i+j*m.row];

        if ( u.row*u.col != m.row*m.row )
        {
            delete [] u.element;
            u.element = new real [m.row*m.row];
        }
        u.row = u.col = m.row;

        svdcmp(m.col, m.row, v.element, s.element, u.element, 1, 1);
        
        _sort(s.element, m.row, 0, m.row-1, v.element, m.col, u.element, m.row);
        real *rmv_v = new real[m.col*m.col];
        for ( i = 0; i < m.col; i++ )
        for ( j = 0; j < m.col; j++ ) rmv_v[i+j*m.col] = v.element[i+j*m.col];
        delete [] v.element;
        v.element = rmv_v;
        v.col = v.row;
        
        for ( i = 0; i < m.col; i++ )
        {
            s.element[i+i*m.row] = s.element[i];
            for ( j = 0; j < i; j++ ) s.element[i+j*m.row] = 0.0;
            for ( j = i+1; j < m.col; j++ ) s.element[i+j*m.row] = 0.0;
        }
        for ( i = m.col; i < m.row; i++ )
        for ( j = 0; j < m.col; j++ ) s.element[i+j*m.row] = 0.0;
    } else if ( m.row < m.col )
    {
        if ( s.row*s.col != m.row*m.col )
        {
            delete [] s.element;
            s.element = new real [m.row*m.col];
        }
        s.row = m.row;
        s.col = m.col;
        
        u = m;
        if ( v.row*v.col != m.col*m.col )
        {
            delete [] v.element;
            v.element = new real [m.col*m.col];
        }
        v.row = v.col = m.col;
        
        svdcmp(m.row, m.col, u.element, s.element, v.element, 1, 1);
        
        _sort(s.element, m.col, 0, m.col-1, u.element, m.row, v.element, m.col);
        real *rmv_u = new real[m.row*m.row];
        for ( i = 0; i < m.row; i++ )
        for ( j = 0; j < m.row; j++ ) rmv_u[i+j*m.row] = u.element[i+j*m.row];
        delete [] u.element;
        u.element = rmv_u;
        u.col = u.row;
        
        for ( i = 0; i < m.row; i++ )
        {
            s.element[i+i*m.row] = s.element[i];
            for ( j = 0; j < i; j++ ) s.element[i+j*m.row] = 0.0;
            for ( j = i+1; j < m.col; j++ ) s.element[i+j*m.row] = 0.0;
        }        
    } else
    {
        u = m;
        if ( v.row*v.col != m.row*m.row )
        {
            delete [] v.element;
            v.element = new real [m.row*m.row];
        }
        v.row = v.col = m.row;
        
        if ( s.row*s.col != m.row*m.row )
        {
            delete [] s.element;
            s.element = new real [m.row*m.row];
        }
        s.row = s.col = m.row;

        svdcmp(m.row, m.row, u.element, s.element, v.element, 1, 1);
        _sort(s.element, m.col, 0, m.col-1, u.element, m.row, v.element, m.col);
        for ( i = 0; i < m.row; i++ )
        {
            s.element[i+i*m.row] = s.element[i];
            for ( j = 0; j < i; j++ ) s.element[i+j*m.row] = 0.0;
            for ( j = i+1; j < m.row; j++ ) s.element[i+j*m.row] = 0.0;            
        }
    }    
}

real Cond(const RMatrix &m)
{
    RMatrix s = SVD(m);
    return s.element[0]/s.element[Length(s)-1];
}

//----------------------------------------------------------------------
//
//        title        :    EISPACK C Version
//                        
//        version        :    v1.00
//        author        :    jinwook Kim (zinook@plaza.snu.ac.kr)
//        last update    :    99.2.15
//
//----------------------------------------------------------------------
//     THIS SUBROUTINE CALLS THE RECOMMENDED SEQUENCE OF
//     SUBROUTINES FROM THE EIGENSYSTEM SUBROUTINE PACKAGE (EISPACK)
//     TO FIND THE EIGENVALUES AND EIGENVECTORS (IF DESIRED)
//     OF A REAL GENERAL MATRIX.
//
//     ON INPUT
//
//        NM  MUST BE SET TO THE ROW DIMENSION OF THE TWO-DIMENSIONAL
//        ARRAY PARAMETERS AS DECLARED IN THE CALLING PROGRAM
//        DIMENSION STATEMENT.
//
//        N  IS THE ORDER OF THE MATRIX  A.
//
//        A  CONTAINS THE REAL GENERAL MATRIX.
//
//        MATZ  IS AN INTEGER VARIABLE SET EQUAL TO ZERO IF
//        ONLY EIGENVALUES ARE DESIRED.  OTHERWISE IT IS SET TO
//        ANY NON-ZERO INTEGER FOR BOTH EIGENVALUES AND EIGENVECTORS.
//
//     ON OUTPUT
//
//        WR  AND  WI  CONTAIN THE REAL AND IMAGINARY PARTS,
//        RESPECTIVELY, OF THE EIGENVALUES.  COMPLEX CONJUGATE
//        PAIRS OF EIGENVALUES APPEAR CONSECUTIVELY WITH THE
//        EIGENVALUE HAVING THE POSITIVE IMAGINARY PART FIRST.
//
//        Z  CONTAINS THE REAL AND IMAGINARY PARTS OF THE EIGENVECTORS
//        IF MATZ IS NOT ZERO.  IF THE J-TH EIGENVALUE IS REAL, THE
//        J-TH COLUMN OF  Z  CONTAINS ITS EIGENVECTOR.  IF THE J-TH
//        EIGENVALUE IS COMPLEX WITH POSITIVE IMAGINARY PART, THE
//        J-TH AND (J+1)-TH COLUMNS OF  Z  CONTAIN THE REAL AND
//        IMAGINARY PARTS OF ITS EIGENVECTOR.  THE CONJUGATE OF THIS
//        VECTOR IS THE EIGENVECTOR FOR THE CONJUGATE EIGENVALUE.
//
//        IERR  IS AN INTEGER OUTPUT VARIABLE SET EQUAL TO AN
//        ERROR COMPLETION CODE DESCRIBED IN SECTION 2B OF THE
//        DOCUMENTATION.  THE NORMAL COMPLETION CODE IS ZERO.
//
//        IV1  AND  FV1  ARE TEMPORARY STORAGE ARRAYS.
//----------------------------------------------------------------------

void balanc(int nm, int n, real *a, int& low, int& igh, real* scale)
{
    int i, j, k, l, m, jj, iexc;
    real c, f, g, r, s, b2, radix;
    int noconv;
    radix = 16.0;
    b2 = radix * radix;
    k = 0;
    l = n-1;
    goto L100;
L20: ;
    scale[m] = j+1;
    if ( j == m ) goto L50;
    for ( i = 0; i <= l; i++ )
    {
        f = a[i+j*nm];
        a[i+j*nm] = a[i+m*nm];
        a[i+m*nm] = f;
    }
    for ( i = k; i <= n-1; i++ )
    {
        f = a[j+i*nm];
        a[j+i*nm] = a[m+i*nm];
        a[m+i*nm] = f;
    }
L50: ;
    if ( iexc == 1 ) goto L80;
    else goto L130;
L80: ;
    if ( l == 0 ) goto L280;
    l--;
L100: ;
    for ( jj = 1; jj <= l+1; jj++ )
    {
        j = l+1 - jj;
        for ( i = 0; i <= l; i++ )
        {
            if ( i == j ) goto L110;
            if ( a[j+i*nm] != 0.0 ) goto L120;
L110: ;
        }
        m = l;
        iexc = 1;
        goto L20;
L120: ;
    }
    goto L140;
L130: ;
    k++;
L140: ;
    for ( j = k; j <= l; j++)
    {
        for ( i = k; i <= l; i++ )
        {
            if ( i == j ) goto L150;
            if ( a[i+j*nm] != 0.0 ) goto L170;
L150: ;
        }
        m = k;
        iexc = 2;
        goto L20;
L170: ;
      }
    for( i = k; i <= l; i++ ) scale[i] = 1.0;
    
L190: ;
    noconv = 0;
    for ( i = k; i <= l; i++ )
    {
        c = 0.0;
        r = 0.0;
        for ( j = k; j <= l; j++ )
        {
            if ( j == i ) goto L200;
            c += fabs(a[j+i*nm]);
            r += fabs(a[i+j*nm]);
L200: ;
        }
        if ( c == 0.0 || r == 0.0 ) goto L270;
        g = r / radix;
        f = 1.0;
        s = c + r;
L210: ;
        if ( c >= g ) goto L220;
        f *= radix;
        c *= b2;
        goto L210;
L220: ;
        g = r * radix;
L230: ;
        if ( c < g ) goto L240;
        f /= radix;
        c /= b2;
        goto L230;
L240: ;
        if ( (c + r) / f >= 0.95 * s ) goto L270;
        g = 1.0 / f;
        scale[i] *= f;
        noconv = 1;
        for ( j = k; j <= n-1; j++ ) a[i+j*nm] *= g;
        for ( j = 0; j <= l; j++ ) a[j+i*nm] *= f;
L270: ;
    }
if ( noconv ) goto L190;
L280: ;
    low = k;
    igh = l;
    return;
}

void balbak(int nm, int n, int low, int igh, real* scale, int m, real* z)
{
    int i, j, k, ii;
    real s;
    if ( m == 0 ) return;
    if ( igh != low ) 
    {
        for ( i = low; i <= igh; i++ )
        {
            s = scale[i];
            for ( j = 0; j <= m-1; j++) z[i+j*nm] *= s;
        }
    }
    for ( ii = 1; ii <= n; ii++)
    {
        i = ii-1;
        if ( i >= low && i <= igh ) goto L140;
        if ( i < low ) i = low - ii;
        k = (int)scale[i] - 1;
        if ( k == i ) goto L140;
        for ( j = 0; j <= m-1; j++ )
        {
            s = z[i+j*nm];
            z[i+j*nm] = z[k+j*nm];
            z[k+j*nm] = s;
        }
L140: ;
    }
    return;
}

void elmhes(int nm, int n, int low, int igh, real* a, int* inf)
{
    int im1, jm1, m, mm1;
    real x, y;

    if ( igh < low + 2 ) goto L200;
    for ( m = low+1; m < igh; m++ )
    {
        mm1 = m - 1;
        x = 0.0;
        im1 = m;
        for ( jm1 = m; jm1 <= igh; jm1++)
        {
            if ( fabs(a[jm1+mm1*nm]) <= fabs(x) ) goto L100;
            x = a[jm1+mm1*nm];
            im1 = jm1;
L100: ;
        }
        inf[m] = im1+1;
        if ( im1 == m ) goto L130;
        for ( jm1 = mm1; jm1 < n; jm1++ )
        {
            y = a[im1+jm1*nm];
            a[im1+jm1*nm] = a[m+jm1*nm];
            a[m+jm1*nm] = y;
        }
        for ( jm1 = 0; jm1 <= igh; jm1++ )
        {
            y = a[jm1+im1*nm];
            a[jm1+im1*nm] = a[jm1+m*nm];
            a[jm1+m*nm] = y;
        }
L130: ;
        if ( x == 0.0 ) goto L180;
        
        for ( im1 = m+1; im1 <= igh; im1++ )
        {
            y = a[im1+mm1*nm];
            if (y == 0.0) goto L160;
            y /= x;
            a[im1+mm1*nm] = y;
            for ( jm1 = m; jm1 < n; jm1++ ) a[im1+jm1*nm] -= y * a[m+jm1*nm];
            for ( jm1 = 0; jm1 <= igh; jm1++ ) a[jm1+m*nm] += y * a[jm1+im1*nm];
L160: ;
        }
L180: ;
    }
L200: ;
    return;
}

void eltran(int nm, int n, int low, int igh, real* a, int* inf, real* z)
{
    int i, j, kl, mm, m;

    for ( j = 0; j < n; j++ )
    {
        for ( i = 0; i < n; i++ ) z[i+j*nm] = 0.0;
        z[j+j*nm] = 1.0;
    }
    kl = igh - low - 1;
    if ( kl < 1 ) return;
    for ( mm = 1; mm <= kl; mm++ )
    {
        m = igh - mm;
        for ( i = m+1; i <= igh; i++ ) z[i+m*nm] = a[i+(m-1)*nm];
        i = inf[m]-1; 
        if ( i != m )
        {
            for ( j = m; j <= igh; j++ )
            {
                z[m+j*nm] = z[i+j*nm];
                z[i+j*nm] = 0.0;
            }
            z[i+m*nm] = 1.0;
        }
    }
    return;
}

void hqr(int nm, int n, int low, int igh, real* h, real* wr, real* wi, int& ierr)
{
    int i, j, k, l, ll, m, mm, en, na, itn, its, mp2, enm2;
    real p, q, r, s, t, w, x, y, zz, norm, tst1, tst2;
    int notlas;
    ierr = 0;
    norm = 0.0;
    k = 0;
    for ( i = 0; i < n; i++ )
    {
        for ( j = k; j < n; j++ ) norm += fabs(h[i+j*nm]);
        k = i;
        if ( i >= low && i <= igh ) goto L50;
        wr[i] = h[i+i*nm];
        wi[i] = 0.0;
L50: ;
    }    
    en = igh;
    t = 0.0;
    itn = 30*n;
L60: ;
    if ( en < low ) goto L1001;
    its = 0;
    na = en-1;
    enm2 = en-2;    
L70: ;
    for ( ll = low+1; ll <= en+1; ll++ )
    {
        l = en + low+1 - ll;
        if ( l == low ) goto L100;
        s = fabs(h[l-1+(l-1)*nm]) + fabs(h[l+l*nm]);
        if ( s == 0.0 ) s = norm;
        tst1 = s;
        tst2 = tst1 + fabs(h[l+(l-1)*nm]);
        if ( tst2 == tst1 ) goto L100;
    }
L100: ;
    x = h[en+en*nm];
    if ( l == en ) goto L270;
    y = h[na+na*nm];
    w = h[en+na*nm] * h[na+en*nm];
    if ( l == na ) goto L280;
    if ( itn == 0 ) goto L1000;
    if ( its != 10 && its != 20 ) goto L130;
    t += x;
    for ( i = low; i <= en; i++ ) h[i+i*nm] -= x;
    
    s = fabs(h[en+na*nm]) + fabs(h[na+(enm2)*nm]);
    x = 0.75 * s;
    y = x;
    w = -0.4375 * s * s;
L130: ;
    its++;
    itn--;
    
    for ( mm = l; mm <= enm2; mm++ )
    {
        m = enm2 + l - mm;

        zz = h[m+m*nm];
        r = x - zz;
        s = y - zz;
        p = (r * s - w) / h[m+1+m*nm] + h[m+(m+1)*nm];
        q = h[m+1+(m+1)*nm] - zz - r - s;
        r = h[m+2+(m+1)*nm];
        s = fabs(p) + fabs(q) + fabs(r);
        p /= s;
        q /= s;
        r /= s;
        if ( m == l ) goto L150;
        tst1 = fabs(p)*(fabs(h[m-1+(m-1)*nm]) + fabs(zz) + fabs(h[m+1+(m+1)*nm]));
        tst2 = tst1 + fabs(h[m+(m-1)*nm])*(fabs(q) + fabs(r));
        if ( tst2 == tst1 ) goto L150;
    }
L150: ;
    mp2 = m+2;
    for ( i = mp2; i <= en; i++ )
    {
        h[i+(i-2)*nm] = 0.0;
        if ( i == mp2 ) goto L160;
        h[i+(i-3)*nm] = 0.0;
L160: ;
    }
    for ( k = m; k <= na; k++)
    {
        notlas = (k != na);
        if ( k == m ) goto L170;
        p = h[k+(k-1)*nm];
        q = h[k+1+(k-1)*nm];
        r = 0.0;
        if ( notlas ) r = h[k+2+(k-1)*nm];
        x = fabs(p) + fabs(q) + fabs(r);
        if ( x == 0.0 ) goto L260;
        p /= x;
        q /= x;
        r /= x;
L170: ;
        s = dsign(sqrt(p*p+q*q+r*r), p);
        if ( k == m ) goto L180;
        h[k+(k-1)*nm] = -s * x;
        goto L190;
L180: ;
        if ( l != m ) h[k+(k-1)*nm] = -h[k+(k-1)*nm];
L190: ;
        p += s;
        x = p / s;
        y = q / s;
        zz = r / s;
        q /= p;
        r /= p;
        if ( notlas ) goto L225;
        for ( j = k; j <= en; j++ )
        {
            p = h[k+j*nm] + q * h[k+1+j*nm];
            h[k+j*nm] -= p * x;
            h[k+1+j*nm] -= p * y;
        }
        j = min(en, k+3);
        for ( i = l; i <= j; i++ )
        {
            p = x * h[i+k*nm] + y * h[i+(k+1)*nm];
            h[i+k*nm] -= p;
            h[i+(k+1)*nm] -= p * q;
        }
        goto L255;
L225: ;
         for ( j = k; j <= en; j++ )
        {
            p = h[k+j*nm] + q * h[k+1+j*nm] + r * h[k+2+j*nm];
            h[k+j*nm] -= p * x;
            h[k+1+j*nm] -= p * y;
            h[k+2+j*nm] -= p * zz;
        }
        j = min(en,k+3);
        for ( i = l; i <= j; i++ )
        {
            p = x * h[i+k*nm] + y * h[i+(k+1)*nm] + zz * h[i+(k+2)*nm];
            h[i+k*nm] -= p;
            h[i+(k+1)*nm] -= p * q;
            h[i+(k+2)*nm] -= p * r;
        }
L255: ;
L260: ;
    }
    goto L70;
L270: ;
    wr[en] = x + t;
    wi[en] = 0.0;
    en = na;
    goto L60;
L280: ;
    p = (y - x) / 2.0;
    q = p * p + w;
    zz = sqrt(fabs(q));
    x += t;
    if ( q < 0.0 ) goto L320;
    zz = p + dsign(zz,p);
    wr[na] = x + zz;
    wr[en] = wr[na];
    if ( zz != 0.0 ) wr[en] = x - w / zz;
    wi[na] = 0.0;
    wi[en] = 0.0;
    goto L330;
L320: ;
    wr[na] = x + p;
    wr[en] = x + p;
    wi[na] = zz;
    wi[en] = -zz;
L330: ;
    en = enm2;
    goto L60;
L1000: ;
    ierr = en+1;
L1001: ;
    return;
}

void cdiv(real ar, real ai, real br, real bi, real& cr, real& ci)
{
    real s, ars, ais, brs, bis;
    s = fabs(br) + fabs(bi);
    ars = ar/s;
    ais = ai/s;
    brs = br/s;
    bis = bi/s;
    s = brs*brs + bis*bis;
    cr = (ars*brs + ais*bis)/s;
    ci = (ais*brs - ars*bis)/s;
    return;
}

void hqr2(int nm, int n, int low, int igh, real* h, real* wr, real* wi, real* z, int& ierr)
{
    int i, j, k, l, m, en, na, ii, jj, ll, mm, nn, itn, its, mp2, enm2;
    real p, q, r, s, t, w, x, y, ra, sa, vi, vr, zz, norm, tst1, tst2;
    int notlas;
    ierr = 0;
    norm = 0.0;
    k = 0;
    for ( i = 0; i < n; i++ )
    {
        for ( j = k; j <= n-1; j++ ) norm += fabs(h[i+j*nm]);
        
        k = i;
        if ( i >= low && i <= igh ) goto L50;
        wr[i] = h[i+i*nm];
        wi[i] = 0.0;
L50: ;
    }
    en = igh;
    t = 0.0;
    itn = 30*n;
L60: ;
    if ( en < low ) goto L340;
    its = 0;
    na = en - 1;
    enm2 = en - 1;
L70: ;
    for ( ll = low+1; ll <= en+1; ll++ )
    {
        l = en + low+1 - ll;
        if ( l == low ) goto L100;
        s = fabs(h[l-1+(l-1)*nm]) + fabs(h[l+l*nm]);
        if ( s == 0.0 ) s = norm;
        tst1 = s;
        tst2 = tst1 + fabs(h[l+(l-1)*nm]);
        if ( tst2 == tst1 ) goto L100;
    }
L100: ;
    x = h[en+en*nm];
    if ( l == en ) goto L270;
    y = h[na+na*nm];
    w = h[en+na*nm] * h[na+en*nm];
    if ( l == na ) goto L280;
    if ( itn == 0 ) goto L1000;
    if ( its != 10 && its != 20 ) goto L130;
    t += x;
    for (  i = low; i <= en; i++ ) h[i+i*nm] -= x;
    
    s = fabs(h[en+na*nm]) + fabs(h[na+(enm2-1)*nm]);
    x = 0.75 * s;
    y = x;
    w = -0.4375 * s * s;
L130: ;
    its++;
    itn--;
    for ( mm = l+1; mm <= enm2; mm++ )
    {
        m = enm2 + l - mm;
        zz = h[m+m*nm];
        r = x - zz;
        s = y - zz;
        p = (r * s - w) / h[m+1+m*nm] + h[m+(m+1)*nm];
        q = h[m+1+(m+1)*nm] - zz - r - s;
        r = h[m+2+(m+1)*nm];
        s = fabs(p) + fabs(q) + fabs(r);
        p /= s;
        q /= s;
        r /= s;
        if ( m == l ) goto L150;
        tst1 = fabs(p)*(fabs(h[m-1+(m-1)*nm]) + fabs(zz) + fabs(h[m+1+(m+1)*nm]));
        tst2 = tst1 + fabs(h[m+(m-1)*nm])*(fabs(q) + fabs(r));
        if ( tst2 == tst1 ) goto L150;
    }
L150: ;
    mp2 = m+2;
    for ( i = mp2; i <= en; i++ )
    {
        h[i+(i-2)*nm] = 0.0;
        if ( i == mp2 ) goto L160;
        h[i+(i-3)*nm] = 0.0;
L160: ;
    }
    for ( k = m; k <= na; k++ )
    {
        notlas = (k != na);
        if ( k == m ) goto L170;
        p = h[k+(k-1)*nm];
        q = h[k+1+(k-1)*nm];
        r = 0.0;
        if ( notlas ) r = h[k+2+(k-1)*nm];
        x = fabs(p) + fabs(q) + fabs(r);
        if ( x == 0.0 ) goto L260;
        p /= x;
        q /= x;
        r /= x;
L170: ;
        s = dsign(sqrt(p*p+q*q+r*r),p);
        if ( k == m ) goto L180;
        h[k+(k-1)*nm] = -s * x;
        goto L190;
L180: ;
        if ( l != m ) h[k+(k-1)*nm] = -h[k+(k-1)*nm];
L190: ;
        p += s;
        x = p / s;
        y = q / s;
        zz = r / s;
        q /= p;
        r /= p;
        if ( notlas ) goto L225;
        for ( j = k; j <= n-1; j++ )
        {
            p = h[k+j*nm] + q * h[k+1+j*nm];
            h[k+j*nm] -= p * x;
            h[k+1+j*nm] -= p * y;
        }
        j = min(en,k+3);
        for ( i = 0; i <= j; i++ )
        {
            p = x * h[i+k*nm] + y * h[i+(k+1)*nm];
            h[i+k*nm] -= p;
            h[i+(k+1)*nm] -= p * q;
        }
        for ( i = low; i <= igh; i++ )
        {
            p = x * z[i+k*nm] + y * z[i+(k+1)*nm];
            z[i+k*nm] -= p;
            z[i+(k+1)*nm] -= p * q;
        }
        goto L255;
L225: ;
        for ( j = k; j <= n-1; j++ )
        {
            p = h[k+j*nm] + q * h[k+1+j*nm] + r * h[k+2+j*nm];
            h[k+j*nm] -= p * x;
            h[k+1+j*nm] -= p * y;
            h[k+2+j*nm] -= p * zz;
        }
        j = min(en,k+3);
        for ( i = 0; i <= j; i++ )
        {
            p = x * h[i+k*nm] + y * h[i+(k+1)*nm] + zz * h[i+(k+2)*nm];
            h[i+k*nm] -= p;
            h[i+(k+1)*nm] -= p * q;
            h[i+(k+2)*nm] -= p * r;
        }
        for ( i = low; i <= igh; i++ )
        {
            p = x * z[i+k*nm] + y * z[i+(k+1)*nm] + zz * z[i+(k+2)*nm];
            z[i+k*nm] -= p;
            z[i+(k+1)*nm] -= p * q;
            z[i+(k+2)*nm] -= p * r;
        }
L255: ;
L260: ;
    }
    goto L70;
L270: ;
    h[en+en*nm] = x + t;
    wr[en] = h[en+en*nm];
    wi[en] = 0.0;
    en = na;
    goto L60;
L280: ;
    p = (y - x) / 2.0;
    q = p * p + w;
    zz = sqrt(fabs(q));
    h[en+en*nm] = x + t;
    x = h[en+en*nm];
    h[na+na*nm] = y + t;
    if ( q < 0.0 ) goto L320;
    zz = p + dsign(zz,p);
    wr[na] = x + zz;
    wr[en] = wr[na];
    if ( zz != 0.0 ) wr[en] = x - w / zz;
    wi[na] = 0.0;
    wi[en] = 0.0;
    x = h[en+na*nm];
    s = fabs(x) + fabs(zz);
    p = x / s;
    q = zz / s;
    r = sqrt(p*p+q*q);
    p /= r;
    q /= r;
    for ( j = na; j < n; j++ )
    {
        zz = h[na+j*nm];
        h[na+j*nm] = q * zz + p * h[en+j*nm];
        h[en+j*nm] = q * h[en+j*nm] - p * zz;
    }
    for ( i = 0; i <= en; i++ )
    {
        zz = h[i+na*nm];
        h[i+na*nm] = q * zz + p * h[i+en*nm];
        h[i+en*nm] = q * h[i+en*nm] - p * zz;
    }
    for ( i = low; i <= igh; i++ )
    {
        zz = z[i+na*nm];
        z[i+na*nm] = q * zz + p * z[i+en*nm];
        z[i+en*nm] = q * z[i+en*nm] - p * zz;
    }
    goto L330;
L320: ;
    wr[na] = x + p;
    wr[en] = x + p;
    wi[na] = zz;
    wi[en] = -zz;
L330: ;
    en = enm2-1;
    goto L60;
L340: ;
    if ( norm == 0.0 ) goto L1001;
    for ( nn = 1; nn <= n; nn++ )
    {
        en = n - nn;
        p = wr[en];
        q = wi[en];
        na = en-1;
        if ( q < 0 ) goto L710;
        else if ( q == 0 ) goto L600;
        else goto L800;
L600: ;
        m = en;
        h[en+en*nm] = 1.0;
        if ( na+1 == 0 ) goto L800;
        for ( ii = 1; ii <= na+1; ii++ )
        {
            i = en - ii ;
            w = h[i+i*nm] - p;
            r = 0.0;
            for ( j = m; j <= en; j++ ) r += h[i+j*nm] * h[j+en*nm];
            
            if ( wi[i] >= 0.0 ) goto L630;
            zz = w;
            s = r;
            goto L700;
L630: ;   
            m = i;
            if ( wi[i] != 0.0 ) goto L640;
            t = w;
            if ( t != 0.0 ) goto L635;
            tst1 = norm;
            t = tst1;
L632: ;
            t = 0.01 * t;
            tst2 = norm + t;
            if ( tst2 > tst1 ) goto L632;
L635: ;
            h[i+en*nm] = -r / t;
            goto L680;
L640: ;
            x = h[i+(i+1)*nm];
            y = h[i+1+i*nm];
            q = (wr[i] - p) * (wr[i] - p) + wi[i] * wi[i];
            t = (x * s - zz * r) / q;
            h[i+en*nm] = t;
            if ( fabs(x) <= fabs(zz) ) goto L650;
            h[i+1+en*nm] = (-r - w * t) / x;
            goto L680;
L650: ;
            h[i+1+en*nm] = (-s - y * t) / zz;
L680: ;
            t = fabs(h[i+en*nm]);
            if ( t == 0.0 ) goto L700;
            tst1 = t;
            tst2 = tst1 + 1.0/tst1;
            if ( tst2 > tst1 ) goto L700;
            for ( j = i; j <= en; j++ ) h[j+en*nm] /= t;            
L700: ;
        }
        goto L800;
L710: ;
        m = na;
        if ( fabs(h[en+na*nm]) <= fabs(h[na+en*nm]) ) goto L720;
        h[na+na*nm] = q / h[en+na*nm];
        h[na+en*nm] = -(h[en+en*nm] - p) / h[en+na*nm];
        goto L730;
L720: ;
        cdiv(0.0, -h[na+en*nm], h[na+na*nm]-p, q, h[na+na*nm], h[na+en*nm] );
L730: ;
        h[en+na*nm] = 0.0;
        h[en+en*nm] = 1.0;
        enm2 = na;
        if ( enm2 == 0 ) goto L800;
        for ( ii = 1; ii <= enm2; ii++ )
        {
            i = na - ii;
            w = h[i+i*nm] - p;
            ra = 0.0;
            sa = 0.0;
            for ( j = m; j <= en; j++ )
            {
                ra += h[i+j*nm] * h[j+na*nm];
                sa += h[i+j*nm] * h[j+en*nm];
            }
            if ( wi[i] >= 0.0 ) goto L770;
            zz = w;
            r = ra;
            s = sa;
            goto L795;
L770: ;
            m = i;
            if ( wi[i] != 0.0 ) goto L780;
            cdiv( -ra, -sa, w, q, h[i+na*nm], h[i+en*nm]);
            goto L790;
L780: ;
            x = h[i+(i+1)*nm];
            y = h[i+1+i*nm];
            vr = (wr[i] - p) * (wr[i] - p) + wi[i] * wi[i] - q * q;
            vi = (wr[i] - p) * 2.0 * q;
            if ( vr != 0.0 || vi != 0.0 ) goto L784;
            tst1 = norm * (fabs(w) + fabs(q) + fabs(x) + fabs(y) + fabs(zz));
            vr = tst1;
L783: ;
            vr = 0.01 * vr;
            tst2 = tst1 + vr;
            if ( tst2 > tst1 ) goto L783;
L784: ;
            cdiv(x*r-zz*ra+q*sa, x*s-zz*sa-q*ra, vr, vi, h[i+na*nm], h[i+en*nm]);
            if ( fabs(x) <= fabs(zz) + fabs(q) ) goto L785;
            h[i+1+na*nm] = (-ra - w * h[i+na*nm] + q * h[i+en*nm]) / x;
            h[i+1+en*nm] = (-sa - w * h[i+en*nm] - q * h[i+na*nm]) / x;
            goto L790;
L785: ;
            cdiv(-r-y*h[i+na*nm], -s-y*h[i+en*nm], zz, q, h[i+1+na*nm], h[i+1+en*nm]);
L790: ;
            t = max(fabs(h[i+na*nm]), fabs(h[i+en*nm]));
            if ( t == 0.0 ) goto L795;
            tst1 = t;
            tst2 = tst1 + 1.0/tst1;
            if ( tst2 > tst1 ) goto L795;
            for ( j = i; j <= en; j++ )
            {
                h[j+na*nm] /= t;
                h[j+en*nm] /= t;
            }
L795: ;
        }
L800: ;
    }
    for ( i = 0; i < n; i++ )
    {
        if ( i >= low && i <= igh ) goto L840;
        for ( j = i; j < n; j++ ) z[i+j*nm] = h[i+j*nm];
L840: ;
    }
    for ( jj = low+1; jj <= n; jj++ )
    {
        j = n + low - jj;
        m = min(j,igh);
        for ( i = low; i <= igh; i++ )
        {
            zz = 0.0;
            for ( k = low; k <= m; k++ ) zz += z[i+k*nm] * h[k+j*nm];
            
            z[i+j*nm] = zz;
        }
    }
    goto L1001;
L1000: ;
    ierr = en+1;
L1001: ;
    return;
}

void rg(int nm, int n, real* a, real* wr, real* wi, int matz, real* z, int* iv1, real* fv1, int& ierr)
{
    int is1, is2;
    if ( n > nm )
    {
        ierr = 10 * n;
        return;
    }
    balanc(nm,n,a,is1,is2,fv1);
    elmhes(nm,n,is1,is2,a,iv1);
    if ( matz == 0 )
    {
        hqr(nm,n,is1,is2,a,wr,wi,ierr);
        return;
    }
    eltran(nm, n, is1, is2, a, iv1, z);
    hqr2(nm, n, is1, is2, a, wr, wi, z, ierr);
    if ( ierr != 0 ) return;
    balbak(nm,n,is1,is2,fv1,n,z);
    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// End of EISPACK
//
//////////////////////////////////////////////////////////////////////////////

RMatrix Eig(RMatrix m)
{
    if ( m.row != m.col ) error_("RMatrix Eig : not sqare");
    
    int i, ierr, matz = 0, n = m.row;
    real *z = 0;
    int *iv1 = new int [n];
    real *fv1 = new real [n];
    RMatrix re(n,2);
    
    rg(n, n, m.element, re.element, (re.element+n), matz, z, iv1, fv1, ierr);
    
    real sum = 0.0;
    for ( i = 0; i < n; i++ ) sum += *(re.element+i+n) * *(re.element+i+n);
    if ( sum < _EPS ) re.col = 1;
    
    delete [] iv1;
    delete [] fv1;

    return re;
}

void Eig(RMatrix m, RMatrix &v, RMatrix &d)
{
    int i, n = m.row;
    if ( n != m.col ) error_("RMatrix Eig : not sqare");
    
    int* iv1 = new int [n];
    real* fv1 = new real [n];

    int ierr, matz = 1;
    
    if ( v.row*v.col != n*n )
    {
        delete [] v.element;
        v.element = new real [n*n];
    }
    v.row = v.col = n;

    if ( d.row*d.col != 2*n )
    {
        delete [] d.element;
        d.element = new real [2*n];
    }
    d.row = n;
    d.col = 2;

    rg(n, n, m.element, d.element, d.element+n, matz, v.element, iv1, fv1, ierr);
    
    real sum = 0.0;
    for ( i = 0; i < n; i++ ) sum += *(d.element+i+n) * *(d.element+i+n);
    if ( sum < _EPS ) d.col = 1;

    delete [] iv1;
    delete [] fv1;
}

RMatrix Expm(const RMatrix& a)
{
    if ( a.row != a.col ) error_("RMatrix Expm : not square");
    RMatrix A(a);
    int n = a.row, e, k;
    real f = frexp(InftyNorm(A), &e);
    int s = max(0,e+1);
    int p2s = 2;
    for ( int i = 1; i < s; i++ ) p2s *= 2;
    A = A/real(p2s);

    RMatrix X(A); 
    real c = 1.0/2.0;
    RMatrix E(Eye(n) + c*A);
    RMatrix D(Eye(n) - c*A);
    int q = 6, p = 1;
    for ( k = 2; k <= q; k++ )
    {
        c *= ((real(q)-real(k)+1.0) / (real(k)*(2.0*real(q)-real(k)+1.0)));
        X = A*X;
        RMatrix cX(c*X);
        E += cX;
        if ( p ) D += cX;
        else D -= cX;
        p = !p;
    }
    E = D%E;
    for ( k = 1; k <= s; k++ ) E *= E;
    return E;
}

void complex_log(real a, real b, real &x, real &y)
{
    y = atan2(b,a);
    x = log(a/cos(y));
}

// Cholesky factorization in 'numerical recipe'
// modified as x = ~L*L, where L is upper triangular
int choldc(real *x, int nm, real *p)
{
    int i, j, k;
    real sum;
    for ( i = 0; i < nm; i++ )
    {
        for ( j = i; j < nm; j++ )
        {
            for ( sum = x[j+i*nm], k = i-1; k >= 0; k-- ) sum -= x[k+i*nm]*x[k+j*nm];
            if ( i == j ) 
            {
                if ( sum <= 0.0 ) return 1; // x, with rounding errors, is not positive definite. 
                p[i] = sqrt(sum); 
            } else x[i+j*nm] = sum / p[i];
        }
    }
    return 0;
}

int IsPositive(const RMatrix &m)
{
    int i, n = m.row, er;
    if ( n != m.col ) error_("RMatrix IsPositive : not sqaure matrix");
    real *a, *p; 
    a = new real[n*n];
    p = new real[n];
    for ( i = 0; i < n*n; i++ ) a[i] = m.element[i];
    er = choldc(a, n, p);
    delete [] a;
    delete [] p;
    if ( er ) return 0;
    return 1;
}

RMatrix Chol(RMatrix m)
{
    int i, j, n = m.row, er;
    if ( n != m.col ) error_("RMatrix Chol : not sqaure matrix");
    real *p = new real [n];
    
    er = choldc(m.element, n, p);
    if ( er ) error_("RMatrix Chol : not positive definite");
    
    for ( j = 0; j < n; j++ )
    for ( i = 0; i < n; i++ )
    {
        if ( i == j ) m.element[i+j*n] = p[i];
        else if ( i < j ) m.element[j+i*n] = 0.0;
    }
    delete [] p;
    return m;
}

void Chol(const RMatrix &m, RMatrix &r, int &d)
{
    int i, j, n = m.row;
    if ( n != m.col ) error_("RMatrix Chol : not sqaure matrix");
    real *p = new real [n];

    r = m;
    d = choldc(r.element, n, p);
    
    for ( j = 0; j < n; j++ )
    for ( i = 0; i < n; i++ )
    {
        if ( i == j ) r.element[i+j*n] = p[i];
        else r.element[j+i*n] = 0.0;
    }
    delete [] p;
}

RMatrix Companion(const RMatrix &m)
{
    int r = m.row, c = m.col;
    if ( r > 1 && c > 1 ) error_("Companion : not vector");
    int n = max(r, c) - 1;
    RMatrix re(n, n);
    for ( int i = 0 ; i < n; i++ )
    for ( int j = 0 ; j < n; j++ )
    {
        if ( i == 0 ) re.element[j*n] = -m.element[j+1] / m.element[0];
        else if ( i == j+1 ) re.element[i+j*n] = 1.0;
        else re.element[i+j*n] = 0.0;
    }
    return re;
}
    
RMatrix Conv(const RMatrix &u, const RMatrix &v)
{
    int r = u.row, c = u.col, m = max(r,c), n, i, j;
    if ( r > 1 && c > 1 ) error_("Conv(u,v) : u is not vector");
    r = v.row, c = v.col, n = max(r,c);
    if ( r > 1 && c > 1 ) error_("Conv(u,v) : v is not vector");
    
    RMatrix re(m+n-1,1);
    if ( r == 1 ) 
    {
        re.row = 1; 
        re.col = m+n-1;
    }
    
    real sum;

    for ( i = 0; i < m+n-1; i++ )
    {
        sum = 0.0;
        for ( j = max(0,1+i-n); j <= min(i,m-1); j++ ) sum += u.element[j] * v.element[i-j];
        re.element[i] = sum;
    }
    return re;
}

RMatrix Roots(const RMatrix& cof)
{
    int r = cof.row, c = cof.col, n = max(r,c);
    if ( r > 1 && c > 1 ) error_("Roots : not vector");
    int flag = 1, i = 0, j;
    while ( flag )
        if ( *(cof.element+i) != 0.0 || ++i >= n ) flag = 0;
    if ( i == 0 ) return Eig(Companion(cof));
    if ( i == n ) return RMatrix();
    RMatrix cof2(n-i,1);
    for ( j = 0; j < n-i; j++ ) *(cof2.element+j) = *(cof.element+j+i);
    return Eig(Companion(cof2));
}

// START OF LINPACK DQRDC
void dqrdc(real *x, int ldx, int n, int p, real *qraux, int *jpvt, real *work, int job)
/*    
    dqrdc uses householder transformations to compute the qr factorization of an n by p matrix x.  column pivoting
    based on the 2-norms of the reduced columns may be performed at the users option.

    on entry

    x        real precision(ldx,p), where ldx .ge. n. x contains the matrix whose decomposition is to be computed.
    ldx        int. ldx is the leading dimension of the array x.
    n        int. n is the number of rows of the matrix x.
    p        int. p is the number of columns of the matrix x.
    jpvt    int(p). jpvt contains ints that control the selection of the pivot columns.  the k-th column x(k) of x
            is placed in one of three classes according to the value of jpvt(k).
            if jpvt(k) .gt. 0, then x(k) is an initial column.
            if jpvt(k) .eq. 0, then x(k) is a free column.
            if jpvt(k) .lt. 0, then x(k) is a final column.
            before the decomposition is computed, initial columns are moved to the beginning of the array x and final
            columns to the end.  both initial and final columns are frozen in place during the computation and only
            free columns are moved.  at the k-th stage of the reduction, if x(k) is occupied by a free column
            it is interchanged with the free column of largest reduced norm.  jpvt is not referenced if
            job .eq. 0.
    work    real precision(p). work is a work array.  work is not referenced if job .eq. 0.
    job        int. job is an int that initiates column pivoting.
            if job .eq. 0, no pivoting is done.
            if job .ne. 0, pivoting is done.

    on return

    x        x contains in its upper triangle the upper triangular matrix r of the qr factorization.
            below its diagonal x contains information from which the orthogonal part of the decomposition
            can be recovered.  note that if pivoting has been requested, the decomposition is not that
            of the original matrix x but that of x with its columns permuted as described by jpvt.
    qraux    real precision(p). qraux contains further information required to recover the orthogonal part of the decomposition.
    jpvt    jpvt(k) contains the index of the column of the original matrix that has been interchanged into the k-th column, 
            if pivoting was requested.
*/
{
    int j, l, pl, pu, maxj, jp, lup, negj, swapj, jj, k;
    real maxnrm, tt, nrmxl, t, tmp;
    pl = 0;
    pu = -1;
    if ( job != 0 )
    {
        // pivoting has been requested.  rearrange the columns according to jpvt.
        for ( j = 0; j < p; j++ )
        {
            swapj = ( jpvt[j] > 0 );
            negj = ( jpvt[j] < 0 );
            jpvt[j] = j+1;
            if ( negj ) jpvt[j] = -j+1;
            if ( swapj )
            {
                if ( j != pl ) for ( k = 0; k < n; k++ ) { tmp = x[k+pl*ldx]; x[k+pl*ldx] = x[k+j*ldx]; x[k+j*ldx] = tmp; }
                jpvt[j] = jpvt[pl];
                jpvt[pl++] = j+1;
            }
        }
        pu = p-1;
        for ( jj = 1; jj <= p; jj++ )
        {
            j = p - jj;
            if ( jpvt[j] < 0 )
            {
                jpvt[j] = -jpvt[j];
                if ( j != pu )
                {
                    for ( k = 0; k < n; k++ ) { tmp = x[k+pu*ldx]; x[pu+pl*ldx] = x[k+j*ldx]; x[k+j*ldx] = tmp; }
                    jp = jpvt[pu];
                    jpvt[pu] = jpvt[j];
                    jpvt[j] = jp;
                }
                pu--;
            }
        }
    }
    // compute the norms of the free columns.
    if ( pu >= pl )
    {
        for ( j = pl; j <= pu; j++ )
        {
            tmp = 0.0; 
            for ( k = 0; k < n; k++ ) tmp += x[k+j*ldx]*x[k+j*ldx];
            qraux[j] = sqrt(tmp);
            work[j] = qraux[j];
        }
    }
    // perform the householder reduction of x.
    lup = ( n > p ? p : n );
    for ( l = 0; l < lup; l++ )
    {
        if ( l >= pl && l < pu )
        {
            // locate the column of largest norm and bring it into the pivot position.
            maxnrm = 0.0;
            maxj = l;
            for ( j = l; j <= pu; j++ )
            {
                if ( qraux[j] > maxnrm )
                {
                    maxnrm = qraux[j];
                    maxj = j;
                }
            }
            if ( maxj != l )
            {
                for ( k = 0; k < n; k++ ) { tmp = x[k+l*ldx]; x[k+l*ldx] = x[k+maxj*ldx]; x[k+maxj*ldx] = tmp; }
                qraux[maxj] = qraux[l];
                work[maxj] = work[l];
                jp = jpvt[maxj];
                jpvt[maxj] = jpvt[l];
                jpvt[l] = jp;
            }
        }
        qraux[l] = 0.0;
        if ( l != n-1 )
        {
            // compute the householder transformation for column (l+1).
            tmp = 0.0; 
            for ( k = 0; k < n-l; k++ ) tmp += x[l+k+l*ldx]*x[l+k+l*ldx];
            nrmxl = sqrt(tmp);
            if ( nrmxl != 0.0 )
            {
                if ( x[l+l*ldx] != 0.0 ) nrmxl = dsign(nrmxl,x[l+l*ldx]);
                for ( k = 0; k < n-l; k++ ) x[l+k+l*ldx] *= 1.0/nrmxl;
                x[l+l*ldx] += 1.0;
                // apply the transformation to the remaining columns,
                // updating the norms.
                if ( p >= l+2 )
                {
                    for ( j = l+1; j < p; j++ )
                    {
                        tmp = 0.0; 
                        for ( k = 0; k < n-l; k++ ) tmp += x[l+k+l*ldx]*x[l+k+j*ldx];
                        t = -tmp/x[l+l*ldx];
                        for ( k = 0; k < n-l; k++ ) x[l+k+j*ldx] += t*x[l+k+l*ldx];
                        if ( j >= pl && j <= pu && qraux[j] != 0.0 )
                        {
                            tt = 1.0 - pow(fabs(x[l+j*ldx])/qraux[j], 2);
                            tt = ( tt > 0.0 ? tt : 0.0 );
                            t = tt;
                            tt = 1.0 + 0.05*tt*pow(qraux[j]/work[j],2);
                            if ( tt == 1.0 )
                            {
                                tmp = 0.0; 
                                for ( k = 0; k < n-l-1; k++ ) tmp += x[l+1+k+j*ldx]*x[l+1+k+j*ldx];
                                qraux[j] = sqrt(tmp);
                                work[j] = qraux[j];
                            } else qraux[j] *= sqrt(t);
                        }
                    }
                }
                // save the transformation.
                qraux[l] = x[l+l*ldx];
                x[l+l*ldx] = -nrmxl;
            }
        }
    }
    return;
}

void dqrsl(real *x, int ldx, int n, int k, real *qraux, real *y, real *qy, real *qty, real *b, real *rsd, real *xb, int job, int &info)
/*
    dqrsl applies the output of dqrdc to compute coordinate transformations, projections, and least squares solutions.
    for k .le. min(n,p), let xk be the matrix 

        xk = (x(jpvt(1)),x(jpvt(2)), ... ,x(jpvt(k)))

    formed from columnns jpvt(1), ... ,jpvt(k) of the original n x p matrix x that was input to dqrdc (if no pivoting was
    done, xk consists of the first k columns of x in their original order).  dqrdc produces a factored orthogonal matrix q
    and an upper triangular matrix r such that

        xk = q * (r)
                 (0)

    this information is contained in coded form in the arrays x and qraux.

    on entry

    x        real precision(ldx,p). x contains the output of dqrdc.
    ldx        integer. ldx is the leading dimension of the array x.
    n        integer. n is the number of rows of the matrix xk.  it must have the same value as n in dqrdc.
    k        integer. k is the number of columns of the matrix xk.  k must nnot be greater than min(n,p), where p is the
            same as in the calling sequence to dqrdc.
    qraux    real precision(p). qraux contains the auxiliary output from dqrdc.
    y        real precision(n). y contains an n-vector that is to be manipulatedby dqrsl.
    job        integer. job specifies what is to be computed.  job has the decimal expansion abcde, with the following meaning.
                if a.ne.0, compute qy.
                if b,c,d, or e .ne. 0, compute qty.
                if c.ne.0, compute b.
                if d.ne.0, compute rsd.
                if e.ne.0, compute xb.
            note that a request to compute b, rsd, or xb automatically triggers the computation of qty, for
            which an array must be provided in the calling sequence.

     on return

    qy        real precision(n). qy conntains q*y, if its computation has been requested.
    qty        real precision(n). qty contains trans(q)*y, if its computation has been requested.  here trans(q) is the
            transpose of the matrix q.
    b        real precision(k) b contains the solution of the least squares problem
            minimize norm2(y - xk*b),

    if its computation has been requested.  (note that if pivoting was requested in dqrdc, the j-th
    component of b will be associated with column jpvt(j) of the original matrix x that was input into dqrdc.)
    rsd    real precision(n). rsd contains the least squares residual y - xk*b, if its computation has been requested.  rsd is
    also the orthogonal projection of y onto the orthogonal complement of the column space of xk.
    xb     real precision(n). xb contains the least squares approximation xk*b, if its computation has been requested.  xb is also
    the orthogonal projection of y onto the column space of x.
    info   integer. info is zero unless the computation of b has been requested and r is exactly singular.  
    in this case, info is the index of the first zero diagonal element of r and b is left unaltered.

    the parameters qy, qty, b, rsd, and xb are not referenced if their computation is not requested and in this case
    can be replaced by dummy variables in the calling program. to save storage, the user may in some cases use the same
    array for different parameters in the calling sequence.  a frequently occuring example is when one wishes to compute
    any of b, rsd, or xb and does not need y or qty.  in this case one may identify y, qty, and one of b, rsd, or xb, while
    providing separate arrays for anything else that is to be computed.  thus the calling sequence

        call dqrsl(x,ldx,n,k,qraux,y,dum,y,b,y,dum,110,info)

    will result in the computation of b and rsd, with rsd overwriting y.  more generally, each item in the following
    list contains groups of permissible identifications for a single callinng sequence.
        1. (y,qty,b) (rsd) (xb) (qy)
        2. (y,qty,rsd) (b) (xb) (qy)
        3. (y,qty,xb) (b) (rsd) (qy)
        4. (y,qy) (qty,b) (rsd) (xb)
        5. (y,qy) (qty,rsd) (b) (xb)
        6. (y,qy) (qty,xb) (b) (rsd)
    in any group the value returned in the array allocated to the group corresponds to the last member of the group.
*/
{
    int i, j, jj, ju, l;
    real t,temp;
    int cb, cqy, cqty, cr, cxb;
    info = 0;
    // determine what is to be computed.

    cqy = (job/10000.0 != 0 );
    cqty = ( ( job % 10000 ) != 0 );
    cb = ( ( job % 1000 )/100.0 != 0 );
    cr = ( ( job % 100 )/10.0 != 0 );
    cxb = ( ( job % 10 ) !=  0 );
    ju = ( k > n-1 ? n-1 : k );

    // special action when n=1.

    if ( ju == 0 )
    {
        if ( cqy ) qy[0] = y[0];
        if ( cqty ) qty[0] = y[0];
        if ( cxb ) xb[0] = y[0];
        if ( cb )
        {
            if ( x[0] != 0.0 ) b[0] = y[0]/x[0];
            else info = 1;
        }
        if ( cr ) rsd[0] = 0.0;
        return;
    }

    // set up to compute qy or qty.

    if ( cqy ) for ( l = 0; l < n; l++ ) qy[l] = y[l];
    if ( cqty ) for ( l = 0; l < n; l++ ) qty[l] = y[l];
    if ( cqy )
    {
        // compute qy.
        for ( jj = 1; jj <= ju; jj++ )
        {
            j = ju - jj;
            if ( qraux[j] != 0.0 )
            {
                temp = x[j+j*ldx];
                x[j+j*ldx] = qraux[j];
                t = 0.0; 
                for ( l = 0; l < n-j; l++ ) t += x[j+l+j*ldx]*qy[j+l];
                t /= -x[j+j*ldx];
                for ( l = 0; l < n-j; l++ ) qy[j+l] += t*x[j+l+j*ldx];
                x[j+j*ldx] = temp;
            }
        }
    }
    if ( cqty )
    {
        // compute trans(q)*y
        for ( j = 0; j+1 <= ju; j++ )
        {
            if ( qraux[j] != 0.0 )
            {
                temp = x[j+j*ldx];
                x[j+j*ldx] = qraux[j];
                t = 0.0; 
                for ( l = 0; l < n-j; l++ ) t += x[j+l+j*ldx]*qty[j+l];
                t /= -x[j+j*ldx];
                for ( l = 0; l < n-j; l++ ) qty[j+l] += t*x[j+l+j*ldx];
                x[j+j*ldx] = temp;
            }
        }
    }
    // set up to compute b, rsd, or xb.
    if ( cb ) for ( l = 0; l < k; l++ ) b[l] = qty[l];
    if ( cxb ) for ( l = 0; l < k; l++ ) xb[l] = qty[l];
    if ( cr && k < n) for ( l = 0; l < n-k; l++ ) rsd[l] = qty[l];
    if ( cxb && k < n )
        for ( i = k; i < n; i++ ) xb[i] = 0.0;
    
    if ( cr )
        for ( i = 0; i < k; i++ ) rsd[i] = 0.0;

    if ( cb )
    {
        // compute b.
        for ( jj = 1; jj <= k; jj++ )
        {
            j = k - jj;
            if ( x[j+j*ldx] == 0.0 )
            {
                  info = j+1;
                  goto L180;
            }
            b[j] /= x[j+j*ldx];
            if ( j+1 != 1 )
            {
                t = -b[j];
                for ( l = 0; l < j; l++ ) b[l] += t*x[l+j*ldx];
            }
        }
L180:
        ;
    }
    if ( cr || cxb )
    {
        // compute rsd or xb as required.
        for ( jj = 1; jj <= ju; jj++ )
        {
            j = ju - jj;
            if ( qraux[j] != 0.0 )
            {
                temp = x[j+j*ldx];
                x[j+j*ldx] = qraux[j];
                if ( cr )
                {
                    t = 0.0; 
                    for ( l = 0; l < n-j; l++ ) t += x[j+l+j*ldx]*rsd[j+l];
                    t /= -x[j+j*ldx];
                    for ( l = 0; l < n-j; l++ ) rsd[j+l] += t*x[j+l+j*ldx];
                }
                if ( cxb )
                {
                    t = 0.0; 
                    for ( l = 0; l < n-j; l++ ) t += x[j+l+j*ldx]*xb[j+l];
                    t /= -x[j+j*ldx];
                    for ( l = 0; l < n-j; l++ ) xb[j+l] += t*x[j+l+j*ldx];
                }
                x[j+j*ldx] = temp;
            }
        }
    }
    return;
}

// END OF LINPACK DQRDC

void QR(const RMatrix &m, RMatrix &q, RMatrix &r, RMatrix &e)
{
    real *qraux, *work, *qptr;
    int n = m.row, p = m.col, i, j, info, q_flag, *jpvt;

    qraux = new real [p];
    work = new real [max(p,n)];
    jpvt = new int [p];

    if ( q.row*q.col == n*n )
    {
        qptr = q.element;
        q_flag = 0;
    } else
    {
        qptr = new real [n*n];
        q_flag= 1;
    }
    
    for ( i = 0; i < p; i++ ) jpvt[i] = 0;

    r = m;

    dqrdc(r.element, n, n, p, qraux, jpvt, work, 1);
    
    for ( i = 0; i < m.row; i++ )
    {
        for ( j = 0; j < n; j++ ) work[j] = ( i == j ? 1.0 : 0.0 );
        dqrsl(r.element, n, n, p, qraux, work, work, work, work, work, work, 10000, info);
        for ( j = 0; j < n; j++ ) qptr[j+n*i] = work[j];
    }

    for ( i = 0; i < p; i++ )
    for ( j = 0; j < n; j++ ) if ( j > i ) *(r.element+j+n*i) = 0.0;
    
    if ( e.row*e.col != p*p )
    {
        delete [] e.element;
        e.element = new real [p*p];
    }
    e.row = e.col = p;
    for ( i = 0; i < p*p; i++ ) *(e.element+i) = 0.0;
    for ( i = 0; i < p; i++ ) *(e.element+jpvt[i]-1+i*p) = 1.0;
    q.row = q.col = n;
    if ( q_flag )
    {
        delete [] q.element;
        q.element = qptr;
    } 
    delete [] jpvt;
    delete [] qraux;
    delete [] work;
}

void QR(const RMatrix &m, RMatrix& q, RMatrix& r)
{
    real *qraux, *work, *qptr;
    int n = m.row, p = m.col, i, j, info, q_flag;

    qraux = new real [p];
    work = new real [max(p,n)];
    if ( q.row*q.col == n*n )
    {
        qptr = q.element;
        q_flag = 0;
    } else
    {
        qptr = new real [n*n];
        q_flag= 1;
    }
    
    r = m;

    dqrdc(r.element, n, n, p, qraux, 0, work, 0);
    for ( i = 0; i < m.row; i++ )
    {
        for ( j = 0; j < n; j++ ) work[j] = ( i == j ? 1.0 : 0.0 );
        dqrsl(r.element,n,n,p,qraux,work,work,work,work,work,work,10000,info);
        for ( j = 0; j < n; j++ ) qptr[j+n*i] = work[j];
    }

    for ( i = 0; i < p; i++ )
    for ( j = 0; j < n; j++ ) if ( j > i ) *(r.element+j+n*i) = 0.0;
    
    q.row = q.col = n;
    if ( q_flag )
    {
        delete [] q.element;
        q.element = qptr;
    } 
    delete [] qraux;
    delete [] work;
}

RMatrix QR(RMatrix m)
{
    real *qraux, *work;
    int n = m.row, p = m.col;

    qraux = new real [p];
    work = new real [p];
    
    dqrdc(m.element, n, n, p, qraux, 0, work, 0);
    
    delete [] qraux;
    delete [] work;
    return m;
}
