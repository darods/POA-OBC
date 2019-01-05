//////////////////////////////////////////////////////////////////////////////////
//
//        title        :    Real Matrix class
//                        
//        version        :    v1.99
//        author        :    Jinwook Kim (zinook@kist.re.kr)
//        last update    :    99.8.1
//
//        Note        :
//
//////////////////////////////////////////////////////////////////////////////////

#include "rmatrix.h"
#include <math.h>
#include <fstream>
#include <sys/timeb.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>

#define _EPS 2.2204E-16
#define _TINY 1E-20
#define _INF 1E100

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))

#define _isreal(m) ( m.row == 1 && m.col == 1 )
#define _isvec(m) ( min(m.row, m.col) == 1 )
#define _issamesz(m) ( row == m.row && col == m.col )
#define _isexist(i,j) ( i > 0 && i <= row && j > 0 && j <= col )
#define _isempty(m) ( m.row == 0 || m.col == 0 )
#define _isnull(m) ( m.row == 0 && m.col == 0 )
#define _size(m) ( m.row * m.col)
#define _length(m) ( max(m.row,m.col) )
    
void error_(char* a)
{
    ofstream fout("error.log", ios::ate);
    fout << a << endl;
    fout.close();
    cerr << a << endl;
    exit(-1);
}

RMatrix::RMatrix(int r, int c, const real &d1, const real &d2) : row(r), col(c)
{
    if ( r*c != 2 ) error_("RMatrix(int, int, real, real) : not compatible index");
    element = new real[2];
    element[0] = d1;    element[1] = d2;
}

RMatrix::RMatrix(int r, int c, const real &d1, const real &d2, const real &d3) : row(r), col(c)
{
    if ( r*c != 3 ) error_("RMatrix(int, int, real, real, real) : not compatible index");
    element = new real[3];
    element[0] = d1;    element[1] = d2;    element[2] = d3;
}

RMatrix::RMatrix(int r, int c, const real &d1, const real &d2, const real &d3, const real &d4) : row(r), col(c)
{
    if ( r*c != 4 ) error_("RMatrix(int, int, real, real, real, real) : not compatible index");
    element = new real[4];
    element[0] = d1;    element[1] = d2;
    element[2] = d3;    element[3] = d4;
}

RMatrix::RMatrix(int r, int c, const real &d1, const real &d2, const real &d3, const real &d4, const real &d5, const real &d6) : row(r), col(c)
{
    if ( r*c != 6 ) error_("RMatrix(int, int, real, real, real, real, real, real) : not compatible index");
    element = new real[6];
    element[0] = d1;    element[1] = d2;    element[2] = d3;
    element[3] = d4;    element[4] = d5;    element[5] = d6;
}

RMatrix::RMatrix(int r, int c, const real &d1, const real &d2, const real &d3, const real &d4, const real &d5, const real &d6, const real &d7, const real &d8, const real &d9) : row(r), col(c)
{
    if ( r*c != 9 ) error_("RMatrix(int, int, real, real, real, real, real, real, real, real, real) : not compatible index");
    element = new real[9];
    element[0] = d1;    element[1] = d2;    element[2] = d3;
    element[3] = d4;    element[4] = d5;    element[5] = d6;
    element[6] = d7;    element[7] = d8;    element[8] = d9;
}

RMatrix::RMatrix(int r, int c, real **a) : row(r), col(c)
{
    element = new real[r*c];
    for ( int j = 0; j < r; j++ )
    for ( int i = 0; i < c; i++ ) element[i+j*row] = a[i][j];
}

RMatrix::RMatrix(int n, int m, const RMatrix &m1, const RMatrix &m2)
{
    if ( n*m != 2 ) error_("RMatrix(int,int,RMatrix,RMatrix) : not compatible index");
    if ( _isnull(m1) )
    {
        if ( _isnull(m2) ) { row = col = 0; element = 0; return; }        // [ [] ,; [] ] = []
        else if ( _isempty(m2) )
        {
            if ( n == 1 ) { row = col = 0; element = 0; return; }        // [ [] , empty ] = []
            else { row = m2.row; col = m2.col; element = 0; return; }    // [ [] ; empty ] = empty
        } else                                                            // [ [] ,; m ] = m
        {
            row = m2.row; col = m2.col; 
            element = new real[row*col];
            for ( int i = 0; i < row*col; i++ ) element[i] = m2.element[i];
            return;
        }
    } else if ( _isempty(m1) )
    {
        if ( _isnull(m2) ) { row = m1.row; col = m1.col; element = 0; return; }        // [ empty ,; [] ] = empty
        else if ( _isempty(m2) )
        {
            if ( n == 1 ) 
            {
                if ( !m1.row && !m2.row ) { row = 0; col = m1.col + m2.col; element = 0; return; }    // [ empty1 , empty2 ] : consistancy
                else { row = m1.row; col = m1.col; element = 0; return; }                            // [ empty1 , empty2 ] : inconsistancy
            } else
            {
                if ( !m1.col && !m2.col ) { row = m1.row + m2.row; col = 0; element = 0; return; }  // [ empty1 ; empty2 ] : consistancy
                else { row = m1.row; col = m1.col; element = 0; return; }                            // [ empty1 ; empty2 ] : inconsistancy
            }
        } else                                                            // [ empty ,; m ] = m
        {
            row = m2.row; col = m2.col; 
            element = new real[row*col];
            for ( int i = 0; i < row*col; i++ ) element[i] = m2.element[i];
            return;
        }
    } else
    {
        if ( _isempty(m2) )
        {
            row = m1.row; col = m1.col; 
            element = new real[row*col];
            for ( int i = 0; i < row*col; i++ ) element[i] = m1.element[i];
            return;
        }
    }

    if ( n == 1 )
    {
        if ( m1.row != m2.row ) error_("RMatrix(int,int,RMatrix,RMatrix) : arguments are not compatible");
        row = m1.row;
        col = m1.col + m2.col;
        element = new real [row*col];
        int i;
        for ( i = 0; i < row*m1.col; i++ ) element[i] = m1.element[i];
        for ( i = 0; i < row*m2.col; i++ ) element[i+row*m1.col] = m2.element[i];
    } else
    {
        if ( m1.col != m2.col ) error_("RMatrix(int,int,RMatrix,RMatrix) : arguments are not compatible");
        row = m1.row + m2.row;
        col = m1.col;
        element = new real [row*col];
        int i,j;
        for ( i = 0; i < col; i++ )
        {
            for ( j = 0; j < m1.row; j++ ) element[j+i*row] = m1.element[j+i*m1.row];
            for ( j = 0; j < m2.row; j++ ) element[j+m1.row+i*row] = m2.element[j+i*m2.row];
        }
    } 
    return;
}

RMatrix::RMatrix(int n, int m, const RMatrix &m1, const RMatrix &m2, const RMatrix &m3)
{
    if ( n*m != 3 ) error_("RMatrix(int,int,RMatrix,RMatrix,RMatrix) : not compatible index");
    if ( n == 1 ) 
    {
        RMatrix tmp = RMatrix(1,2, RMatrix(1,2, m1, m2), m3 );
        row = tmp.row;    col = tmp.col;
        element = new real [row*col];
        for ( int i = 0; i < row*col; i++ ) element[i] = tmp.element[i];
    } else 
    {
        RMatrix tmp = RMatrix(2,1, RMatrix(2,1, m1, m2), m3 );
        row = tmp.row;    col = tmp.col;
        element = new real [row*col];
        for ( int i = 0; i < row*col; i++ ) element[i] = tmp.element[i];
    }
}

RMatrix::RMatrix(int n, int m, const RMatrix &m1, const RMatrix &m2,const RMatrix &m3, const RMatrix &m4)
{
    if ( n*m != 4 ) error_("RMatrix(int,int,RMatrix,RMatrix,RMatrix,RMatrix) : not compatible index");
    if ( n == 1 ) 
    {
        RMatrix tmp = RMatrix(1,2, RMatrix(1,2, RMatrix(1,2, m1, m2), m3 ), m4 );
        row = tmp.row;    col = tmp.col;
        element = new real [row*col];
        for ( int i = 0; i < row*col; i++ ) element[i] = tmp.element[i];
    } else if ( n == 4 )
    {
        RMatrix tmp = RMatrix(2,1, RMatrix(2,1, RMatrix(2,1, m1, m2), m3 ), m4 );
        row = tmp.row;    col = tmp.col;
        element = new real [row*col];
        for ( int i = 0; i < row*col; i++ ) element[i] = tmp.element[i];
    } else if ( n == 2 )
    {
        RMatrix tmp = RMatrix(2,1, RMatrix(1,2, m1, m2), RMatrix(1,2, m3, m4) );
        row = tmp.row;    col = tmp.col;
        element = new real [row*col];
        for ( int i = 0; i < row*col; i++ ) element[i] = tmp.element[i];
    }
}

RMatrix Zeros(int r, int c)
{
    RMatrix re(r,c);
    for ( int i = 0; i < r*c; i++) re.element[i] = 0.0;
    return re;
}

RMatrix Ones(int r, int c)
{
    RMatrix re(r,c);
    for ( int i = 0; i < r*c; i++) re.element[i] = 1.0;
    return re;
}

RMatrix ColIndex(const real &start, const real &end)
{
    if ( start > end ) RMatrix();
    int n = int(end-start)+1, i;
    RMatrix re(n,1);
    for ( i = 0; i < n; i++ ) re.element[i] = start+i;
    return re;
}

RMatrix RowIndex(const real &start, const real &end)
{
    if ( start > end ) RMatrix();
    int n = int(end-start)+1, i;
    RMatrix re(1,n);
    for ( i = 0; i < n;  i++ ) re.element[i] = start+i;
    return re;
}

RMatrix ColIndex(const real &start, const real &step, const real &end)
{
    int n, i;
    if ( start == end ) return RMatrix(start);
    if ( start < end )
    {
        if ( step < 0 )  return RMatrix();
        n = int((end-start)/step) + 1;
    } else
    {
        if ( step > 0 )  return RMatrix();
        n = int((start-end)/step) + 1;
    }
    RMatrix re(n,1);
    for ( i = 0; i < n;  i++ ) re.element[i] = start + i*step;
    return re;
}

RMatrix RowIndex(const real &start, const real &step, const real &end)
{
    int n, i;
    if ( start == end ) return RMatrix(start);
    if ( start < end )
    {
        if ( step < 0 )  return RMatrix();
        n = int((end-start)/step) + 1;
    } else
    {
        if ( step > 0 )  return RMatrix();
        n = int((start-end)/step) + 1;
    }
    RMatrix re(1,n);
    for ( i = 0; i < n;  i++ ) re.element[i] = start + i*step;
    return re;
}

RMatrix Eye(int n)
{
    RMatrix re(n,n);
    for ( int i = 0; i < n; i++) 
    for ( int j = 0; j < n; j++) 
    {
        if ( i == j ) re.element[j+i*n] = 1.0;
        else re.element[j+i*n] = 0.0;
    }
    return re;
}

RMatrix Eye(int r, int c)
{
    RMatrix re(r,c);
    for ( int i = 0; i < c; i++) 
    for ( int j = 0; j < r; j++) 
    {
        if ( i == j ) re.element[j+i*r] = 1.0;
        else re.element[j+i*r] = 0.0;
    }
    return re;
}

RMatrix &RMatrix::RowSwap(int i, int j)
{
    if ( i > row || i < 1 || j > row || j < 1 ) error_("RMatrix RowSwap : index exceeds matrix dimensions");
    real temp;
    for ( int k = 0; k < col; k++ )
    {
        temp = element[(i-1)+k*row];
        element[(i-1)+k*row] = element[(j-1)+k*row];
        element[(j-1)+k*row] = temp;
    }
    return *this;
}

RMatrix &RMatrix::ColSwap(int i, int j)
{
    if ( i > col || i < 1 || j > col || j < 1 ) error_("RMatrix ColSwap : index exceeds matrix dimensions");
    real temp;
    for ( int k = 0; k < row; k++ )
    {
        temp = element[k+(i-1)*row];
        element[k+(i-1)*row] = element[k+(j-1)*row];
        element[k+(j-1)*row] = temp;
    }
    return *this;
}

RMatrix ColVector(const RMatrix &m)
{
    RMatrix re(m);
    re.row = _size(m);
    re.col = 1;
    return re;
}
    
double Todouble(const RMatrix &m)
{
    if ( !_isreal(m) ) error_("RMatrix Todouble(): can not cast to double");
    return double(m.element[0]);
}

real &RMatrix::operator () (int i) 
{ 
    if ( i < 1 ) error_("RMatrix (int) : index exceeds matrix dimensions"); 
    else if ( row == 0 || col == 0 ) 
    {
        element = new real [i];
        for ( int j = 0; j < i; j++ ) element[j] = 0.0;
        row = 1;
        col = i;
    } else if ( i > row*col )
    {
        int j;
        real *tmp = new real[i];
        for ( j = 0; j < max(col,row); j++ ) tmp[j] = element[j];
        for ( j = max(col,row); j < i; j++ ) tmp[j] = 0.0;
            
        if ( row == 1 )    col = i;
        else row = i;
        
        delete [] element;
        element = tmp;        
    }
    return element[i-1];
}

real &RMatrix::operator () (int i, int j)
{ 
    if ( i < 1 || j < 1 ) error_("RMatrix (int,int) : index exceeds matrix dimensions");
    else if ( row == 0 || col == 0 )
    {
        element = new real [i*j];
        for ( int k = 0; k < i*j; k++ ) element[k] = 0.0;
        row = i;
        col = j;
    } else if ( i > row || j > col )
    {
        int k, l;
        real *tmp = new real[max(i,row)*max(j,col)];
        
        for ( k = 0; k < col; k++ )
        {
            for ( l = 0; l < row; l++ ) tmp[l+k*max(i,row)] = element[l+k*row];
            for ( l = row; l < max(row,i); l++ ) tmp[l+k*max(i,row)] = 0.0;
        }
        for ( k = col; k < max(col,j); k++ )
            for ( l = 0; l < max(row,i); l++ ) tmp[l+k*max(i,row)] = 0.0;
            
        row = max(i,row);
        col = max(j,col);
        delete [] element;
        element = tmp;
    }
    return element[(i-1)+(j-1)*row]; 
}

RMatrix RMatrix::operator () (const RMatrix &idx) const
{
    int d;
    RMatrix re(idx.row, idx.col);
    
    if ( row == 1 ) 
    {
        re.row = 1;
        re.col = _size(idx);
    } else if ( col == 1 ) 
    {
        re.row = _size(idx);
        re.col = 1;        
    }    

    for ( int i = 0; i < _size(idx); i++ )
    {
        d = int(idx.element[i]);
        if ( d < 1 || d > row*col ) error_("RMatrix (RMatrix) : index exceeds matrix dimensions");
        re.element[i] = element[d-1];
    }
    return re;
}

RMatrix RMatrix::operator () (const RMatrix &ri, const RMatrix &ci) const
{
    if ( _isempty(ri) )
    {
        if ( _isempty(ci) ) return RMatrix();
        else 
        {
            for ( int i = 0; i < _size(ci); i++ ) if ( ci.element[i] <= 0 || ci.element[i] > col ) error_("RMatrix (RMatrix,RMatrix) : index exceeds matrix dimensions");
            return RMatrix(0, _size(ci));
        }
    } 
    int r = _size(ri), c = _size(ci), i, j;
    RMatrix re(r,c);
    for ( i = 0; i < r; i++ )
    for ( j = 0; j < c; j++ ) 
    {
        if ( ri.element[i] <= 0 || ri.element[i] > row || ci.element[j] <= 0 || ci.element[j] > col ) error_("RMatrix (RMatrix,RMatrix) : index exceeds matrix dimensions");
        re.element[i+j*r] = element[(int(ri.element[i])-1)+(int(ci.element[j])-1)*row];
    }
    return re;
}

RMatrix RMatrix::operator == (const RMatrix &m) const
{
    RMatrix re(row, col);
    if ( _isnull(m) || max(row,col) == 0 ) re = 0.0;
    else if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] == m.element[0] );
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] == m.element[i] );
    else error_("RMatrix == (RMatrix) : agrgument is not compatible");
    return re;
}

RMatrix RMatrix::operator == (const real &d) const
{
    RMatrix re(row, col);
    if ( max(row,col) == 0 ) re = 0.0;
    else for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] == d );
    return re;
}

RMatrix RMatrix::operator != (const RMatrix &m) const
{
    RMatrix re(row, col);
    if ( _isnull(m) || max(row,col) == 0 ) re = 1.0;
    else if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] != m.element[0] );
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] != m.element[i] );
    else error_("RMatrix != (RMatrix) : agrgument is not compatible");
    return re;
}

RMatrix RMatrix::operator != (const real &d) const
{
    RMatrix re(row, col);
    if ( max(row,col) == 0 ) re = 1.0;
    else for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] != d );
    return re;
}

RMatrix RMatrix::operator > (const RMatrix &m) const
{
    RMatrix re(row, col);
    if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] > m.element[0] );
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] > m.element[i] );
    else error_("RMatrix > (RMatrix) : agrgument is not compatible");
    return re;
}

RMatrix RMatrix::operator > (const real &d) const
{
    RMatrix re(row, col);
    for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] > d );
    return re;
}

RMatrix RMatrix::operator < (const RMatrix &m) const
{
    RMatrix re(row, col);
    if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] < m.element[0] );
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] < m.element[i] );
    else error_("RMatrix < (RMatrix) : agrgument is not compatible");
    return re;
}

RMatrix RMatrix::operator < (const real &d) const
{
    RMatrix re(row, col);
    for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] < d );
    return re;
}

RMatrix RMatrix::operator >= (const RMatrix &m) const
{
    RMatrix re(row, col);
    if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] >= m.element[0] );
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] > m.element[i] );
    else error_("RMatrix >= (RMatrix) : agrgument is not compatible");
    return re;
}

RMatrix RMatrix::operator >= (const real &d) const
{
    RMatrix re(row, col);
    for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] >= d );
    return re;
}

RMatrix RMatrix::operator <= (const RMatrix &m) const
{
    RMatrix re(row, col);
    if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] <= m.element[0] );
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] <= m.element[i] );
    else error_("RMatrix <= (RMatrix) : agrgument is not compatible");
    return re;
}

RMatrix RMatrix::operator <= (const real &d) const
{
    RMatrix re(row, col);
    for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] <= d );
    return re;
}

RMatrix RMatrix::operator && (const RMatrix &m) const
{
    RMatrix re(row, col);
    if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] && m.element[0] );
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] && m.element[i] );
    else error_("RMatrix && (RMatrix) : agrgument is not compatible");
    return re;
}

RMatrix RMatrix::operator && (const real &d) const
{
    RMatrix re(row, col);
    for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] && d );
    return re;
}

RMatrix RMatrix::operator || (const RMatrix &m) const
{
    RMatrix re(row, col);
    if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] || m.element[0] );
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] || m.element[i] );
    else error_("RMatrix || (RMatrix) : agrgument is not compatible");
    return re;
}

RMatrix RMatrix::operator || (const real &d) const
{
    RMatrix re(row, col);
    for ( int i = 0; i < row*col; i++ ) re.element[i] = ( element[i] || d );
    return re;
}

RMatrix &RMatrix::operator = (const RMatrix &m)
{
    int i;
    if ( _isempty(m) )
    {
        delete [] element;
        row = m.row;
        col = m.col;
        element = 0;
    } else if ( row*col == _size(m) ) 
    {
        row = m.row;
        col = m.col;
        for ( i = 0; i < _size(m); i++) element[i] = m.element[i];
    } else
    {
        delete [] element;
        row = m.row;
        col = m.col;
        element = new real[_size(m)];
        for ( i = 0; i < _size(m); i++) element[i] = m.element[i];
    } 
    return *this;
}

RMatrix &RMatrix::operator = (const real &d)
{
    if ( row*col != 1 )
    { 
        delete [] element;
        element = new real [1];
    }
    *element = d;
    row = col = 1;
    return *this;
}

RMatrix &RMatrix::operator += (const RMatrix &m)
{
    if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) element[i] += m.element[0];
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) element[i] += m.element[i];
    else error_("RMatrix operator += : agrgument is not compatible");
    return *this;
}

RMatrix &RMatrix::operator -= (const RMatrix &m)
{
    if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) element[i] -= m.element[0];
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) element[i] -= m.element[i];
    else error_("RMatrix operator -= : agrgument is not compatible");
    return *this;
}

RMatrix &RMatrix::operator *= (const RMatrix &m)
{
    if ( _issamesz(m) && this != &m )
    {
        real* row_el;
        row_el = new real[col];
        int i, j, k, l;
        real sum;
        for ( i = 0; i < row; i++) 
        {
            for ( l = 0; l < col; l++ ) row_el[l] = element[i+l*row];
            for ( j = 0; j < col; j++) 
            {
                sum = 0.0;
                for ( k = 0; k < col; k++) sum += row_el[k] * m.element[k+j*col];
                element[i+j*row] = sum;
            }
        }
        delete [] row_el;        
    } else *this = *this * m;

    return *this;
}

RMatrix &RMatrix::operator *= (const real &d)
{
    for ( int i = 0; i < row*col; i++ ) element[i] *= d;
    return *this;
}

RMatrix Transpose(const RMatrix &m)
{
    RMatrix re(m.col, m.row);
    for ( int i = 0; i < m.col; i++) 
    for ( int j = 0; j < m.row; j++) re.element[i+j*m.col] = m.element[j+i*m.row];
    return re;
}

RMatrix RMatrix::operator ~ (void) const
{
    return Transpose(*this);
}

RMatrix RMatrix::operator ! (void) const
{
    RMatrix re(row,col);
    for (int i = 0; i < row*col; i++) re.element[i] = !(element[i]);
    return re;
}

RMatrix RMatrix::operator * (const RMatrix &m) const
{
    RMatrix re;
    if ( _isreal(m) )
    {
        re.row = row; re.col = col;
        re.element = new real [re.row*re.col];
        for ( int i = 0; i < row*col; i++ ) re.element[i] = element[i]*m.element[0];
    } else if ( row*col == 1 )
    {
        re.row = m.row; re.col = m.col;
        re.element = new real [re.row*re.col];
        for ( int i = 0; i < m.row*m.col; i++ ) re.element[i] = element[0]*m.element[i];
    } else if ( row == 0 && col == 0 ) 
    {
        if ( m.row == 0 && m.col != 0 ) re.col = m.col;
    } else if ( row == 0 || col == 0 )
    {
        if ( col == m.row )
        {
            re.row = row; re.col = m.col;
            re.element = new real [re.row*re.col];
            for ( int i = 0; i < re.row*re.col; i++ ) re.element[i] = 0.0;
        }
    } else
    {
        if ( col != m.row ) 
        {
            if ( !_isempty(m) ) error_("RMatrix * (RMatrix) : argument is not compatible");
        } else
        {
            re.row = row; re.col = m.col;
            re.element = new real [re.row*re.col];    
            real sum;
            for ( int i = 0; i < row; i++) 
            for ( int j = 0; j < m.col; j++) 
            {
                sum = 0.0;
                for ( int k = 0; k < col; k++) sum += element [i+k*row] * m.element[k+j*m.row];
                re.element[i+j*row] = sum;
            }
        }
    }
    
    return re;    
}

void MultMatrix(RMatrix& re, const RMatrix &a, const RMatrix &b)
{
    int i,j,k;
    real sum;
    if ( re.row != a.row || re.col != b.col || a.col != b.row ) error_("RMatrix MultMatrix: arguments are not compatible");
    else
    {
        for ( i = 0; i < a.row; i++) 
        for ( j = 0; j < b.col; j++) 
        {
            sum = 0.0;
            for ( k = 0; k < a.col; k++) sum += a.element[i+k*a.row] * b.element[k+j*b.row];
            re.element[i+j*a.row] = sum;
        }
    }
}

void AddMatrix(RMatrix& re, const RMatrix &a, const RMatrix &b)
{
    if ( re.row != a.row != b.row || re.col != b.col != a.col ) error_("RMatrix AddRMatrix: arguments are not compatible");
    for ( int i = 0; i < _size(re); i++ ) re.element[i] = a.element[i] + b.element[i];
}

void SubtractMatrix(RMatrix& re, const RMatrix &a, const RMatrix &b)
{
    if ( re.row != a.row != b.row || re.col != b.col != a.col ) error_("RMatrix SubtractMatrix: arguments are not compatible");
    for ( int i = 0; i < _size(re); i++ ) re.element[i] = a.element[i] - b.element[i];
}

RMatrix Rand(int i, int j)
{
    RMatrix re(i,j);
    for ( int k = 0; k < i*j; k++) re.element[k] = double(rand())/double(RAND_MAX);
    return re;
}

RMatrix SymRand(int n)
{
    int i, j;
    RMatrix re(n,n);
    for ( i = 0; i < n; i++) 
    for ( j = 0; j < n; j++) 
    {
        if ( j >= i ) re.element[j+i*n] = double(rand())/double(RAND_MAX);
        else re.element[j+i*n] = re.element[i+j*n];
    }
    return re;
}

RMatrix SkewRand(int n)
{
    int i, j;
    RMatrix re(n,n);
    for ( i = 0; i < n; i++) 
    for ( j = 0; j < n; j++) 
    {
        if ( j > i ) re.element[j+i*n] = double(rand())/double(RAND_MAX);
        else if ( j == i ) re.element[j+i*n] = 0.0;
        else re.element[j+i*n] = -re.element[i+j*n];
    }
    return re;
}

RMatrix Skew(const RMatrix &m)
{
    RMatrix re;
    if ( _length(m) == 3 )
    {
        real data[9] = { 0.0, m(3), -m(2), -m(3), 0.0, m(1), m(2), -m(1), 0.0 };
        re = RMatrix(3, 3, data);        
    } else if ( _length(m) == 6 )
    {
        real data[16] = { 0.0, m(3), -m(2), 0.0, -m(3), 0.0, m(1), 0.0, m(2), -m(1), 0.0, 0.0, m(4), m(5), m(6), 0.0 };
        re = RMatrix(4, 4, data);
    } else error_("RMatrix Skew : not 3 or 6 vector");
    return re;
}

RMatrix InvSkew(const RMatrix &m)
{
    if ( m.row == m.col && m.row == 3 ) 
    {
        real val[3] = { m(6), m(7), m(2) };
        return RMatrix(3, 1, val);
    } else if ( m.row == m.col && m.row == 4 )
    {
        real val[6] = { m(7), m(9), m(2), m(13), m(14), m(15) };
        return RMatrix(6, 1, val);
    } else error_("RMatrix InvSkew : not 3 X 3 or 4 X 4 matrix");
    return RMatrix();
}

RMatrix RMatrix::Sub(int row_s, int row_e, int col_s, int col_e) const
{
    if ( row_s > row_e || col_s > col_e || !_isexist(row_s, col_s) || !_isexist(row_e, col_e) ) error_("RMatrix Sub : index exceeds matrix dimensions");
    int r = (row_e - row_s + 1), c = (col_e - col_s + 1), i, j;
    RMatrix re(r, c);
    for ( i = 0; i < r; i++) 
    for ( j = 0; j < c; j++) re.element[i+j*r] = element[(i+row_s-1)+(j+col_s-1)*row];
    return re;
}

RMatrix &RMatrix::Replace(int r, int c, const RMatrix &m)
{
    int i, j;
    if ( r < 1 || c < 1 || r+m.row-1 > row || c+m.col-1 > col ) error_("RMatrix Replace(int,int,RMatrix) : index exceeds matrix dimensions");
    for ( i = 0; i < m.row; i++) 
    for ( j = 0; j < m.col; j++) element[(j+c-1)*row+(i+r-1)] = m.element[i+j*m.row];
    return *this;
}

RMatrix &RMatrix::Replace(const RMatrix &index, const RMatrix &m)
{
    int i, n = _size(index), idx;
    if ( n != _size(m) ) error_("RMatrix Replace(RMatrix,RMatrix) : arguments are not compatible");
    
    for ( i = 0; i < n; i++ )
    {
        idx = int(index.element[i]) - 1;
        if ( idx < 0 || idx > row*col ) error_("RMatrix Replace(RMatrix,RMatrix) : index exceeds matrix dimensions");
        element[idx] = m.element[i];
    }
    return *this;
}

RMatrix &RMatrix::Replace(const RMatrix &ri, const RMatrix &ci, const RMatrix &m)
{
    int i, j, r, c, szr = _size(ri), szc = _size(ci);
    if ( szr != m.row || szc != m.col ) error_("RMatrix Replace(RMatrix, RMatrix,RMatrix) : arguments are not compatible");
    for ( i = 0; i < szr; i++ )
    for ( j = 0; j < szc; j++ )
    {
        r = int(ri.element[i])-1;
        c = int(ci.element[j])-1;
        if ( r < 0 || r >= row || c < 0 || c >= col ) error_("RMatrix Replace(RMatrix, RMatrix,RMatrix) : index exceeds matrix dimensions");
        element[r+c*row] = m.element[i+j*m.row];
    }
    return *this;
}

RMatrix &RMatrix::Push(int r, int c, const RMatrix &m)
{
    int nr = max(r-1+m.row,row), nc = max(c-1+m.col, col), i, j;
        
    if ( _isempty(m) ) error_("RMatrix Push : empty argument");
    else if ( r < 1 || c < 1 ) error_("RMatrix Push : index exceeds matrix dimensions");
    else if ( r-1+m.row > row || c-1+m.col > col ) this->ReSize(nr, nc);        
    
    for ( i = r-1; i < r-1+m.row; i++)
    for ( j = c-1; j < c-1+m.col; j++) element[i+j*nr] = m.element[i-r+1+(j-c+1)*m.row];
    return *this;
}

RMatrix &RMatrix::Push(const RMatrix &index, const RMatrix &m)
{
    int i, n = _size(index), idx;
    if ( _isempty(m) ) 
    {
        this->Remove(index); 
        return *this; 
    }
    
    if ( n != _size(m) ) error_("RMatrix Push(RMatrix,RMatrix) : arguments are not compatible");
    
    for ( i = 0; i < n; i++ )
    {
        idx = int(index.element[i]);
        if ( idx < 1 ) error_("RMatrix Push(RMatrix,RMatrix) : index exceeds matrix dimensions");
        if ( idx > row*col ) this->ReSize(idx, 1);
        element[idx-1] = m.element[i];
    }
    return *this;
}

RMatrix &RMatrix::Push(const RMatrix &ri, const RMatrix &ci, const RMatrix &m)
{
    int i, j, r, c;
    if ( _size(ri) != m.row || _size(ci) != m.col ) error_("RMatrix Push(RMatrix,RMatrix,RMatrix) : arguments are not compatible");
    for ( i = 0; i < _size(ri); i++ )
    {
        r = int(ri.element[i]);
        for ( j = 0; j < _size(ci); j++ )
        {
            c = int(ci.element[j]);
            if ( r < 1 || c < 1 ) error_("RMatrix Push(RMatrix, RMatrix,RMatrix) : index exceeds matrix dimensions");
            if ( r > row || c > col ) this->ReSize(max(r,row),max(c,col));
            element[(r-1)+(c-1)*row] = m.element[i+j*m.row];
        }
    }
    return *this;
}

RMatrix Diag(const RMatrix &m)
{
    int n, i;
    RMatrix re;
    if ( m.row == 1 || m.col == 1 )
    {
        n = _length(m);
        re = Zeros(n,n);
        for ( i = 0; i < n; i++ ) re.element[i*n+i] = m.element[i];
    } else 
    {
        n = min(m.row, m.col);
        re.ReNew(n,1);
        for ( i = 0; i < n; i++ ) re.element[i] = m.element[i*m.row+i];
    }
    return re;
}

RMatrix Diag(const RMatrix &m, int offset)
{
    if ( m.row < 1 || m.col < 1 ) error_("Diag(RMatrix, int) : empty argument");
    RMatrix re;
    int k, n;
        
    if ( m.col == 1 || m.row == 1 ) // vector case
    {
        n = _length(m);
        re = Zeros(n+abs(offset),n+abs(offset));
        if ( offset > 0 ) for ( int k = 0; k < n; k++ ) re.element[k+(k+offset)*(n+offset)] = m.element[k];
        else for ( k = 0; k < n; k++ ) re.element[k-offset+k*(n-offset)] = m.element[k];
    } else // matrix case
    {
        if ( offset > 0 ) n = m.col;
        else if ( offset < 0 ) n = m.row;
        else n = min(m.row,m.col);
        if ( n <= abs(offset) ) error_("Diag(RMatrix, int) : offset it too big");
        re = Zeros(n-abs(offset),1);
        if ( offset > 0 ) for ( k = 0; k < n-offset; k++ ) re.element[k] = m.element[k+(k+offset)*m.row];
        else for ( k = 0; k < n+offset; k++ ) re.element[k] = m.element[k+abs(offset)+k*m.row];
    }
    return re;
}

RMatrix RMatrix::operator * (const real &d) const
{
    RMatrix re(row, col);
    for (int i = 0; i < row*col; i++) re.element[i] = d * element[i];
    return re;
}

RMatrix RMatrix::operator / (const real &d) const
{
    RMatrix re(row, col);
    for (int i = 0; i < row*col; i++) re.element[i] = element[i] / d;
    return re;
}

RMatrix &RMatrix::operator /= (const real &d)
{
    for (int i = 0; i < row*col; i++) element[i] /= d;
    return *this;
}

RMatrix operator * (const real &d, const RMatrix &m)
{
    RMatrix re(m.row, m.col);
    for (int i = 0; i < _size(m); i++) re.element[i] = d * m.element[i];
    return re;
}

RMatrix RMatrix::operator + (const RMatrix &m) const
{
    RMatrix re(row, col);
    if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = element[i] + m.element[0];
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = element[i] + m.element[i];
    else error_("RMatrix operator + : agrgument is not compatible");
    return re;
}

RMatrix operator + (const real &d, const RMatrix &m)
{
    RMatrix re(m.row, m.col);
    for ( int i = 0; i < _size(m); i++ ) re.element[i] = d + m.element[i];
    return re;    
}

RMatrix RMatrix::operator + (const real &d) const
{    
    RMatrix re(row, col);
    for ( int i = 0; i < row*col; i++ ) re.element[i] = element[i] + d;
    return re;
}
    
RMatrix operator - (const real &d, const RMatrix &m)
{
    RMatrix re(m.row, m.col);
    for ( int i = 0; i < _size(m); i++ ) re.element[i] = m.element[i] - d;
    return re;        
}

RMatrix RMatrix::operator - (const real &d) const
{    
    RMatrix re(row, col);
    for ( int i = 0; i < row*col; i++ ) re.element[i] = element[i] - d;
    return re;
}

RMatrix RMatrix::operator - (const RMatrix &m) const
{
    RMatrix re(row, col);
    if ( _isreal(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = element[i] - m.element[0];
    else if ( _issamesz(m) ) for ( int i = 0; i < row*col; i++ ) re.element[i] = element[i] - m.element[i];
    else error_("RMatrix operator - : agrgument is not compatible");
    return re;
}

real* NewPtr(int r, int c, const RMatrix &m)
{
    real *re = new real[r*c];
    for ( int i = 0; i < r*c; i++ ) re[i] = m.element[i];
    return re;
}

real* NewPtr(int r, int c, const real *elm)
{
    real *re = new real[r*c];
    for ( int i = 0; i < r*c; i++ ) re[i] = *(elm+i);
    return re;
}

int idamax(int n, double *dx)
{
    double dmax;
    int i, idamax = 0;
      
    if ( n < 1 ) return 0;
    if ( n == 1 ) return 1;
    
    dmax = fabs(dx[0]);
    for ( i = 1; i < n; i++ )
    {
        if( fabs(dx[i]) > dmax )
        {
            idamax = i;
            dmax = fabs(dx[i]);
        }
    }
    return idamax;
}

void dgefa(double *x, int lda, int n, int *jpvt, int &info)
{
    double t;
    int i, j, k, l;
    // gaussian elimination with partial pivoting
    info = 0;
    if ( n > 1 )
    {
        for ( k = 0; k < n - 1; k++ )
        {
            // find l = pivot index
            l = idamax(n-k, &x[k+k*lda]) + k;
            jpvt[k] = l;
            // zero pivot implies this column already triangularized
            if ( x[l+k*lda] == 0.0 ) info = k;
            else
            {
                // interchange if necessary
                if ( l != k )
                {
                    t = x[l+k*lda];
                    x[l+k*lda] = x[k+k*lda];
                    x[k+k*lda] = t;
                }
                // compute multipliers
                t = -1.0/x[k+k*lda];
                for ( j = 1; j < n-k; j++ ) x[k+k*lda+j] *= t;
                // row elimination with column indexing
                for ( j = k+1; j < n; j++ )
                {
                    t = x[l+j*lda];
                    if ( l != k )
                    {
                        x[l+j*lda] = x[k+j*lda];
                        x[k+j*lda] = t;
                    }
                    for ( i = 1; i < n-k; i++ ) x[k+j*lda+i] += t*x[k+k*lda+i];
                }
            }
        }
    }
    jpvt[n-1] = n-1;
    if ( x[n-1+(n-1)*lda] == 0.0 ) info = n-1;
    return;
}

void dgedi(double *x, int lda, int n, int *jpvt, double &det, int &power, double *work, int job)
{
    double t;
    int i, j, k;
    // compute determinant
    if ( job == 11 || job == 10 )
    {
        det = 1.0;
        power = 0;
        for ( i = 0; i < n; i++ )
        {
            if ( jpvt[i] != i ) det = -det;
            det *= x[i+i*lda];
            // ...exit
            if ( det == 0.0 ) break;
            while ( fabs(det) < 1.0 )
            {
                det *= 10.0;
                power--;
            }
            while ( fabs(det) >= 10.0 )
            {
                det /= 10.0;
                power++;
            }
        }
    }
    // compute inverse(u)
    if ( job == 10 ) return;
    for ( k = 0; k < n; k++ )
    {
        x[k+k*lda] = 1.0/x[k+k*lda];
        t = -x[k+k*lda];
        for ( i = 0; i < k; i++ ) x[i+k*lda] *= t;
        if ( n >= k+2 )
        {
            for ( j = k+1; j < n; j++ )
            {
                t = x[k+j*lda];
                x[k+j*lda] = 0.0;
                for ( i = 0; i <= k; i++ ) x[i+j*lda] += t*x[i+k*lda];
               }
        }
    }
    // form inverse(u)*inverse(l)
    if ( n >= 2 )
    {
        for ( k = n-2; k >= 0; k-- )
        {
            for ( i = k+1; i < n; i++ )
            {
                work[i] = x[i+k*lda];
                x[i+k*lda] = 0.0;
            }
            for ( i = k+1; i < n; i++ )
            {
                t = work[i];
                for ( j = 0; j < n; j++ ) x[j+k*lda] += t*x[j+i*lda];
            }
            j = jpvt[k];
            if ( j != k ) for ( i = 0; i < n; i++ ) { t = x[i+k*lda]; x[i+k*lda] = x[i+j*lda]; x[i+j*lda] = t; }
        }
    }
    return;
}

void dgesl(double *x, int lda, int n, int *jpvt, double *b, int job)
{
    double t;
    int k, l;

    if ( job == 0 ) 
    {
        // job = 0 , solve  a * x = b
        // first solve  l*y = b
        if ( n >= 2 )
        {
            for( k = 0; k < n-1; k++ )
            {
                l = jpvt[k];
                t = b[l];
                if ( l != k )
                {
                    b[l] = b[k];
                    b[k] = t;
                }
                for ( l = 1; l < n-k; l++ ) b[k+l] += t*x[k+l+k*lda];
            }
        }
        // now solve  u*x = y
        for ( k = n-1; k >= 0; k-- )
        {
            b[k] /= x[k+k*lda];
            t = -b[k];
            for ( l = 0; l < k; l++ ) b[l] += t*x[l+k*lda];
        }
        return;
    }

    // job = nonzero, solve  trans(a) * x = b
    // first solve  trans(u)*y = b
    for ( k = 0; k < n; k++ )
    {
        t = 0.0;
        for ( l = 0; l < k; l++ ) t += x[l+k*lda]*b[l];
        b[k] = (b[k] - t)/x[k+k*lda];
    }
    // now solve trans(l)*x = y
      if ( n >= 2 )
    {
        for ( k = n-1; k >= 0; k-- )
        {
            t = 0.0;
            for ( l = 1; l < n-k; l++ ) t += x[k+l+k*lda]*b[k+l];
            b[k] += t;

            l = jpvt[k];
            if ( l != k )
            {
                t = b[l];
                b[l] = b[k];
                b[k] = t;
            }
        }
    }
    return;
}

RMatrix Inv(const RMatrix &m)
{
    if ( m.row != m.col ) error_("RMatrix Inv : not square");
    int *ipvt = new int[m.row], info, power;
    real det, *mp = NewPtr(m.row, m.row, m), *work = new real[m.row];
    dgefa(mp, m.row, m.row, ipvt, info);
    dgedi(mp, m.row, m.row, ipvt, det, power, work, 1);
    RMatrix re(m.row, m.row, mp);
    delete [] ipvt;
    delete [] work;
    delete [] mp;
    return re;
}

RMatrix pInv(const RMatrix& m)
{
    RMatrix re;
    if ( m.row > m.col ) re = Inv(~m*m)*~m;
    else re = (~m)*Inv(m*~m);
    return re;
}

RMatrix RMatrix::operator % (RMatrix m) const
{
    if ( row*col == 1 ) return m/element[0];
    if ( row != m.row ) error_("RMatrix operator % : agrgument is not compatible");
    if ( row != col ) return pInv(*this)*m;
    
    int *ipvt = new int[m.row], info, i;
    real *a = NewPtr(row, row, element);
    dgefa(a, row, row, ipvt, info);
    for ( i = 0; i < m.col; i++ ) dgesl(a, row, row, ipvt, m.element+row*i, 0);    
    delete [] ipvt;
    delete [] a;
    return m;
}

RMatrix RMatrix::operator / (RMatrix m) const
{
    if ( _isreal(m) ) return (*this)/m.element[0];
    if ( col != m.col ) error_("RMatrix operator / : agrgument is not compatible");
    if ( m.row != m.col ) return (*this)*pInv(m);
    
    int *ipvt = new int[m.row], info, i, j;
    dgefa(m.element, m.row, m.row, ipvt, info);
    RMatrix re(row,col);
    real *row_vec = new real[col];    
    for ( i = 0; i < re.row; i++ ) 
    {
        for ( j = 0; j < re.col; j++ ) row_vec[j] = element[i+j*row];
        dgesl(m.element, m.row, m.row, ipvt, row_vec, 1);
        for ( j = 0; j < re.col; j++ ) re.element[i+j*row] = row_vec[j];
    }    
    delete [] row_vec;
    delete [] ipvt;
    return re;
}

real Det(const RMatrix &m)
{
    if ( m.row != m.col ) error_("RMatrix Det : not square");
    int *ipvt = new int[m.row], info, power;
    real det, *mp = NewPtr(m.row, m.row, m), *work = new real[m.row];
    dgefa(mp, m.row, m.row, ipvt, info);
    dgedi(mp, m.row, m.row, ipvt, det, power, work, 10);
    
    delete [] ipvt;
    delete [] work;
    delete [] mp;
    
    int idx = 0;
    if ( power > 0 ) while ( idx++ != power ) det *= 10.0;
    else if ( power < 0 ) while ( idx-- != power ) det /= 10.0;
    
    return det;
}

real Trace(const RMatrix &m)
{
    if ( m.row != m.col ) error_("RMatrix Trace : not square");
    real d = 0.0;
    for ( int i = 0; i < m.row; i++) d += m.element[i+i*m.row];
    return d;
}

RMatrix Abs(const RMatrix &m)
{
    RMatrix re(m);
    for ( int i = 0; i < _size(m); i++ ) if ( m.element[i] < 0 ) re.element[i] *= -1.0;
    return re;
}

int IsSymmetric(const RMatrix &m)
{
    if ( m.col != m.row ) return 0;
    for ( int i = 0; i < m.row-1; i++ )
    for ( int j = i+1; j < m.col; j++ )    if ( m.element[i+j*m.row] != m.element[j+i*m.row] ) return 1;
    return 1;
}

int IsVector(const RMatrix &m)
{
    return (( m.row > m.col ) ? ( m.col == 1 ? m.row : 0 ) : ( m.row == 1 ? m.col : 0 ));
}

int IsTrue(const RMatrix &m)
{
    if ( _isempty(m) ) return 0;
    for ( int i = 0; i < _size(m); i++ )
        if ( !(m.element[i]) ) return 0;
    return 1;
}

int NNZ(const RMatrix &m)
{
    int i, num = 0;
    for ( i = 0; i < _size(m); i++ ) if ( !m.element[i] ) num++;
    return num;
}

RMatrix Any(const RMatrix &m)
{
    int i, j;
    RMatrix re = Zeros(1, m.col);
    for ( i = 0; i < m.col; i++ )
        for ( j = 0; j < m.row; j++ ) if ( m.element[j+i*m.row] ) re.element[i] = 1;
    return re;
}

RMatrix Rosser(void)
{
    real a[64] = 
    { 
       611,   196,  -192,   407,    -8,   -52,   -49,    29,
       196,   899,   113,  -192,   -71,   -43,    -8,   -44,
      -192,   113,   899,   196,    61,    49,     8,    52,
       407,  -192,   196,   611,     8,    44,    59,   -23,
        -8,   -71,    61,     8,   411,  -599,   208,   208,
       -52,   -43,    49,    44,  -599,   411,   208,   208,
       -49,    -8,     8,    59,   208,   208,    99,  -911,
        29,   -44,    52,   -23,   208,   208,  -911,    99
    };
    return RMatrix(8, 8, a);
}

RMatrix foptions(void)
{
    real val[] = {0,1e-4,1e-4,1e-6,0,0,0,0,0,0,0,0,0,0,0,1e-8,0.1,0};
    return RMatrix(1,18,val);
};

RMatrix foptions(RMatrix opt)
{
    if ( min(opt.row,opt.col) < 18 ) opt.ReSize(1,18);
    opt += ElMult((opt == 0), foptions());
    return opt;
};

void change(real *v, const int i, const int j)
{
    real temp;
    temp = v[i];
    v[i] = v[j];
    v[j] = temp;
}

void change(real *v, const int i, const int j, real *idx)
{
    real temp;
    temp = v[i];
    v[i] = v[j];
    v[j] = temp;
    temp = idx[i];
    idx[i] = idx[j];
    idx[j] = temp;
}

void qsort(real *v, const int n, const int left, const int right)
{
    int i, last; 
    if ( left >= right ) return;
    change(v, left, (left+right)/2 );
    last = left;
    for ( i = left + 1; i <= right; i++ )
        if ( v[i] < v[left] ) change(v, ++last, i);
    change(v,left,last);
    qsort(v, n, left, last-1);
    qsort(v, n, last+1, right);
}

void qsort(real *v, const int n, const int left, const int right, real *idx)
{
    int i, last; 
    if ( left >= right ) return;
    change(v, left, (left+right)/2, idx );
    last = left;
    for ( i = left + 1; i <= right; i++ )
        if ( v[i] < v[left] ) change(v, ++last, i, idx);
    change(v, left, last, idx);
    qsort(v, n, left, last-1, idx);
    qsort(v, n, last+1, right, idx);
}

RMatrix Sort(const RMatrix &x, RMatrix &idx)
{
    int n = x.row, m = x.col, i, j;
    RMatrix re(x);
    idx = RowIndex(1,n*m);
    idx.row = n;
    idx.col = m;
    for ( i = 0; i < m; i++ )
    {
        qsort( (re.element+ i*n), n, 0, n-1, (idx.element+i*n) );
        for ( j = 0; j < n; j++ ) idx.element[i*n+j] -= (i*n);
    }
    return re;
}

RMatrix Sort(const RMatrix &x)
{
    int n = x.row, m = x.col, i;
    RMatrix re(x);
    for ( i = 0; i < m; i++ ) qsort( (re.element+i*n), n, 0, n-1);
    return re;
}

RMatrix ElPower(const RMatrix &m, real p)
{
    RMatrix re(m.row, m.col);
    for ( int i = 0; i < _size(m); i++) re.element[i] = pow(m.element[i], p);
    return re;
}

RMatrix ElMult(const RMatrix &a, const RMatrix &b)
{
    if ( a.row != b.row || a.col != b.col )
    {
        if ( _size(a) != 1 && _size(b) != 1 ) error_("RMatrix ElMut : arguments are not compatible");
        else if ( _size(a) == 1 ) return a.element[0]*b;
        else return a*b.element[0];
    }
    RMatrix re(a.row, a.col);
    for ( int i = 0; i < _size(a); i++ ) re.element[i] = a.element[i] * b.element[i];
    return re;
}

RMatrix ElDivid(const RMatrix &a, const RMatrix &b)
{
    int i;
    if ( a.row != b.row || a.col != b.col ) 
    {
        if ( _size(a) != 1 && _size(b) != 1 ) error_("RMatrix ElDivid : arguments are not compatible");
        else if ( _size(a) == 1 )
        {
            RMatrix re(b.row, b.col);
            for ( i = 0; i < _size(b); i++) re.element[i] = a.element[0] / b.element[i];
            return re;
        } else    return a / b.element[0];
    }
    RMatrix re(a.row, a.col);
    for ( i = 0; i < _size(a); i++ ) re.element[i] = a.element[i] / b.element[i];
    return re;
}

RMatrix Min(const RMatrix &m)
{
    int c = m.col, r = m.row, i, j;
    real temp;
    if ( r == 1 )
    {
        temp = *m.element;
        for ( i = 1; i < c; i++ ) temp = min(temp, m.element[i]);
        return RMatrix( temp );
    }
    RMatrix re(1,c);
    for ( i = 0; i < c; i++ )
    {
        temp = m.element[i*m.row];
        for ( j = 1; j < m.row; j++ ) temp = min(temp, m.element[j+i*m.row]);
        re.element[i] = temp;
    }
    return re;
}

void Min(const RMatrix &m, RMatrix &min, RMatrix &idx)
{
    int c = m.col, r = m.row, i, j;
    real temp;
    if ( r == 1 )
    {
        idx = RMatrix(1);
        temp = m.element[0];
        for ( i = 1; i < c; i++ )
            if ( temp > m.element[i] )
            {
                temp = m.element[i];
                idx.element[0] = i+1;
            }
        
        min = RMatrix(temp);
    }
    min.ReSize(1,c);
    idx.ReSize(1,c);
    for ( i = 0; i < c; i++ )
    {
        temp = m.element[i*m.row];
        idx.element[0] = 1;
        for ( j = 1; j < m.row; j++ ) 
        {
            if ( temp > m.element[j+i*m.row] )
            {
                temp = m.element[j+i*m.row];
                idx.element[i] = j+1;
            }
        }
        min.element[i] = temp;
    }
}
    
void Max(const RMatrix &m, RMatrix &max, RMatrix &idx)
{
    int c = m.col, r = m.row, i, j;
    real temp;
    if ( r == 1 )
    {
        idx = RMatrix(1);
        temp = *m.element;
        for ( i = 1; i < c; i++ )
            if ( temp < m.element[i] )
            {
                temp = m.element[i];
                idx.element[0] = i+1;
            }
        
        max = RMatrix(temp);
    }
    max.ReSize(1,c);
    idx.ReSize(1,c);
    for ( i = 0; i < c; i++ )
    {
        temp = m.element[i*m.row];
        idx.element[0] = 1;
        for ( j = 1; j < m.row; j++ ) 
        {
            if ( temp < m.element[j+i*m.row] )
            {
                temp = m.element[j+i*m.row];
                idx.element[i] = j+1;
            }
        }
        max.element[i] = temp;
    }
}

real MinVec(const RMatrix &m, int *idx)
{
    real tmp = m.element[0];
    if ( idx != 0 ) *idx = 1;
    for ( int i = 1; i < (m.row)*(m.col); i++ )
    {
        if ( m.element[i] < tmp )
        {
            tmp = m.element[i];
            if ( idx != 0 ) *idx = i+1;
        }
    }
    return tmp;
}

real MaxVec(const RMatrix &m, int *idx)
{
    real tmp = -_INF;
    if ( idx != 0 ) *idx = 1;
    for ( int i = 0; i < (m.row)*(m.col); i++ )
    {
        if ( m.element[i] > tmp )
        {
            tmp = m.element[i];
            if ( idx != 0 ) *idx = i+1;
        }
    }
    return tmp;
}

RMatrix Min(const RMatrix &m, real r)
{
    RMatrix re(m);
    for ( int i = 0; i < m.row*m.col; i++ )
    if ( re.element[i] > r ) re.element[i] = r;
    return re;
}

RMatrix Max(const RMatrix &m, real r)
{
    RMatrix re(m);
    for ( int i = 0; i < m.row*m.col; i++ )
    if ( re.element[i] < r ) re.element[i] = r;
    return re;
}

RMatrix Max(const RMatrix &m)
{
    int c = m.col, r = m.row, i, j;
    real temp;
    if ( r == 1 || c == 1 )
    {
        temp = *m.element;
        for ( i = 1; i < max(c,r); i++ ) temp = max(temp, m.element[i]);
        return RMatrix( temp );
    }
    RMatrix re(1,c);
    for ( i = 0; i < c; i++ )
    {
        temp = m.element[i*m.row];
        for ( j = 1; j < m.row; j++ ) temp = max(temp, m.element[j+i*m.row]);
        re(1,i+1) = temp;
    }
    return re;
}

RMatrix Mean(const RMatrix &m)
{
    int c = m.col, r = m.row, i, j;
    real temp;
    if ( r == 1 )
    {
        temp = 0.0;
        for ( i = 0; i < c; i++ ) temp += m.element[i];
        return RMatrix( temp/real(c) );
    }
    RMatrix re(1,c);
    for ( i = 0; i < c; i++ )
    {
        temp = 0.0;
        for ( j = 0; j < m.row; j++ ) temp += m.element[j+i*m.row];
        re(1,i+1) = temp/real(r);
    }
    return re;
}

RMatrix Sign(const RMatrix &m)
{
    RMatrix re(m.row, m.col);
    for ( int i = 0; i < m.col * m.row; i++ )    
    {
        if ( m.element[i] > 0.0 ) re.element[i] = 1.0;
        else if ( m.element[i] < 0.0 ) re.element[i] = -1.0;
        else re.element[i] = 0.0;
    }
    return re;
}

RMatrix Sum(const RMatrix &m)
{
    if ( _isempty(m) ) return 0.0;
    RMatrix re(1,m.col);
    real tmp;
    for ( int i = 0; i < m.col; i++)
    {
        tmp = 0.0;
        for ( int j = 0; j < m.row; j++ ) tmp += m.element[j+i*m.row];
        re.element[i] = tmp;
    }
    return re;
}

real FNorm(const RMatrix &m)
{
    real tmp = 0.0;
    for ( int i = 0; i < m.row*m.col; i++ ) tmp += m.element[i] * m.element[i];
    return sqrt(tmp);
}

real InftyNorm(const RMatrix &m)
{
    return MaxVec(Sum(Abs((~m))));
}
    
RMatrix Find(const RMatrix &a)
{
    if ( _isempty(a) ) return RMatrix();
    RMatrix re(a.row*a.col,1);
    int idx = 0, i;
    for ( i = 0; i < a.row*a.col; i++ ) if ( a.element[i] ) re(++idx) = double(i+1);
    if ( idx == 0 ) return RMatrix();
    re.ReSize(idx);
    return re;
}

void Find(const RMatrix &a, RMatrix &ri, RMatrix &ci)
{
    int i, j, n = a.row*a.col, sz = 0;
    for ( i = 0; i < n; i++ ) if ( a.element[i] ) sz++;

    ri.ReSize(sz,1);
    ci.ReSize(sz,1);
    sz = 0;
    for ( i = 0; i < a.row; i++ )
    for ( j = 0; j < a.col; j++ ) 
        if ( a.element[i+j*a.row] )
        {
            ri.element[sz] = i+1;
            ci.element[sz] = j+1;
            sz++;
        }
}

RMatrix ZeroRemove(const RMatrix &a)
{
    RMatrix idx(Find(a));
    int n = idx.row;
    RMatrix re(n,1);
    for (int i = 1; i <= n; i++) re(i) = a(int(idx(i)));
    return re;
}

RMatrix &RMatrix::RemoveRow(int idx)
{
    if ( idx < 1 || idx > row ) error_("RemoveRow(int) : index exceeds matrix dimensions");
    if ( row*col == 0 ) error_("RemoveRow(int) : cannot remove empty matrix");
    real *tmp = new real[(row-1)*col];
    int i, j;
    for ( i = 0; i < idx-1; i++ )
    for ( j = 0; j < col; j++ ) *(tmp+i+j*(row-1)) = element[i+j*row];
    for ( i = idx; i < row; i++ )
    for ( j = 0; j < col; j++ ) *(tmp+i-1+j*(row-1)) = element[i+j*row];
    delete [] element;
    element = tmp;
    row--;
    return *this;
}

RMatrix &RMatrix::RemoveRow(int si, int ei)
{
    int i, j, tmpr, rmvr;
    if ( si == ei ) return RemoveRow(si);
    if ( si > ei ) { tmpr = si; si = ei; ei = tmpr; }
    
    if ( si < 1 || ei > row ) error_("RemoveRow(int, int) : index exceeds matrix dimensions");
    if ( row*col == 0 ) error_("RemoveRow(int, int) : cannot remove empty matrix");
    rmvr = ei - si + 1;
    tmpr = row - rmvr;
    real *tmp = new real[tmpr*col];
    for ( i = 0; i < si-1; i++ )
    for ( j = 0; j < col; j++ ) *(tmp+i+j*tmpr) = element[i+j*row];
    for ( i = ei; i < row; i++ )
    for ( j = 0; j < col; j++ ) *(tmp+i-rmvr+j*tmpr) = element[i+j*row];
    delete [] element;
    element = tmp;
    row = tmpr;
    return *this;
}

RMatrix &RMatrix::RemoveRow(const RMatrix &idx)
{
    if ( idx.row*idx.col == 0 ) return *this;
    int i, j, r, num_remain = row;
    int *ridx = new int [row];
    
    for ( i = 0; i < row; i++ ) ridx[i] = 0;
    for ( i = 0; i < idx.row*idx.col; i++ )
    {
        r = int(idx.element[i]);
        if ( r < 1 || r > row ) error_("RMatrix RemoveRow(RMatrix) : index exceeds matrix dimensions");
        else ridx[r-1] = 1;
    }
    for ( i = 0; i < row; i++ ) if ( ridx[i] ) num_remain--;

    real *tmp = new real[num_remain*col];
    
    for ( r = i = 0; i < row; i++ )
    {
        if ( !ridx[i] )
        {
            for ( j = 0; j < col; j++ ) *(tmp+r+j*num_remain) = element[i+j*row];
            r++;
        }
    }
    delete [] ridx;
    delete [] element;
    element = tmp;
    row = num_remain;
    return *this;
}

RMatrix &RMatrix::RemoveCol(int idx)
{
    if ( idx < 1 || idx > col ) error_("RemoveCol(int) : index exceeds matrix dimensions");
    if ( row*col == 0 ) error_("RemoveCol(int) : cannot remove empty matrix");
    real *tmp = new real[row*(col-1)];
    int i, j;
    for ( i = 0; i < row; i++ )
    {
        for ( j = 0; j < idx-1; j++) *(tmp+i+j*row) = element[i+j*row];
        for ( j = idx; j < col; j++) *(tmp+i+(j-1)*row) = element[i+j*row];
    }
    delete [] element;
    element = tmp;
    col--;
    return *this;
}

RMatrix &RMatrix::RemoveCol(int si, int ei)
{
    int i, j, rmvc;
    if ( si == ei ) return RemoveCol(si);
    if ( si > ei ) { rmvc = si; si = ei; ei = rmvc; }
    
    if ( si < 1 || ei > col ) error_("RemoveCol(int, int) : index exceeds matrix dimensions");
    if ( row*col == 0 ) error_("RemoveCol(int, int) : cannot remove empty matrix");
    rmvc = ei - si + 1;
    real *tmp = new real[row*(col-rmvc)];
    for ( i = 0; i < row; i++ )
    {
        for ( j = 0; j < si-1; j++ ) *(tmp+i+j*row) = element[i+j*row];
        for ( j = ei; j < col; j++ ) *(tmp+i+(j-rmvc)*row) = element[i+j*row];
    }
    delete [] element;
    element = tmp;
    col -= rmvc;
    return *this;
}

RMatrix &RMatrix::RemoveCol(const RMatrix &idx)
{
    if ( idx.row*idx.col == 0 ) return *this;
    int i, j, c, num_remain = col;
    int *cidx = new int [col];
    
    for ( i = 0; i < col; i++ ) cidx[i] = 0;
    for ( i = 0; i < idx.row*idx.col; i++ )
    {
        c = int(idx.element[i]);
        if ( c < 1 || c > col ) error_("RMatrix RemoveCol(RMatrix,RMatrix) : index exceeds matrix dimensions");
        else cidx[c-1] = 1;
    }
    for ( i = 0; i < col; i++ ) if ( cidx[i] ) num_remain--;

    real *tmp = new real[row*num_remain];
    
    for ( c = j = 0; j < col; j++ )
    {
        if ( !cidx[j] )
        {
            for ( i = 0; i < row; i++ ) *(tmp+i+c*row) = element[i+j*row];
            c++;
        }
    }
    delete [] cidx;
    delete [] element;
    element = tmp;
    col = num_remain;
    return *this;
}

RMatrix &RMatrix::Remove(int idx)
{
    if ( idx < 1 || idx > row*col ) error_("Remove(int) : index exceeds matrix dimensions");
    real *tmp = new real[row*col-1];
    int i;
    for ( i = 0; i < idx-1; i++) tmp[i] = element[i];
    for ( i = idx; i < row*col; i++) tmp[i-1] = element[i];
    delete [] element;
    element = tmp;
    col = row*col - 1;
    row = 1;
    return *this;    
}

RMatrix &RMatrix::Remove(const RMatrix &index)
{
    if ( index.row*index.col == 0 ) return *this;
    int i, j, n = row*col;
    int *idx = new int [row*col];
    
    for ( i = 0; i < n; i++ ) idx[i] = 0;
    for ( i = 0; i < index.row*index.col; i++ )
    {
        j = int(index.element[i]);
        if ( j < 1 || j > n ) error_("RMatrix Remove(RMatrix) : index exceeds matrix dimensions");
        else idx[j-1] = 1;
    }
    for ( i = 0; i < row*col; i++ ) if ( idx[i] ) n--;

    real *tmp = new real[n];
    
    for ( i = j = 0; i < row*col; i++ ) if ( !idx[i] ) *(tmp+j++) = element[i];
    
    delete [] idx;
    delete [] element;
    element = tmp;
    row = 1;
    col = n;
    return *this;
}

RMatrix &RMatrix::ReSize(int r, int c)
{
    if ( r < 0 || c < 0 ) error_("RMatrix ReSize: index exceeds matrix dimensions");
    real *tmp_ele = new real[r*c];
    int i, j;
    for ( i = 0; i < r; i++ )
    for ( j = 0; j < c; j++ )
    {
        if ( i < row && j < col ) tmp_ele[j*r+i] = element[i+j*row];
        else tmp_ele[j*r+i] = 0.0;
    }
    delete [] element;
    element = tmp_ele;
    row = r;
    col = c;
    return *this;
}

RMatrix &RMatrix::ReNew(int r, int c)
{
    if ( r < 0 || c < 0 ) error_("RMatrix ReNew: index exceeds matrix dimensions");
    if ( r*c != row*col )
    {
        delete [] element;
        element = new real [r*c];
    }
    row = r;
    col = c;
    return *this;
}

RMatrix &RMatrix::MakeColVector()
{
    row *= col;
    col = 1;
    return *this;
}

ostream& operator << (ostream &os, const RMatrix &m)
{
    os << m.row << " X " << m.col << " | ";
    os << "[" << endl;
    for(int i = 0; i < m.row; i++)
    for(int j = 0; j < m.col; j++)
    {
        os << m.element[i+j*m.row];
        if ( j == m.col-1 ) os << "; " << endl;
        else os << ", ";
    }
    os << "];" << endl;
    return os;
}

istream& operator >> (istream &is, RMatrix &m)
{
    int r, c;
    char ch;
    is >> r >> ch >> c >> ch ;
    is >> ch;
    m.ReSize(r, c);
    for(int i = 0; i < r; i++)
    for(int j = 0; j < c; j++)    is >> m.element[i+j*r] >> ch;
    is >> ch >> ch;
    return is;
}

static double start_second;
void tic()
{
    struct timeb tstruct;
    time_t seconds;
    time(&seconds);
    ftime( &tstruct );
    start_second = seconds + tstruct.millitm/1000. - 9.01825*1e8;
}

double toc()
{
    struct timeb tstruct;
    time_t seconds;
    time(&seconds);
    ftime( &tstruct );
    return (seconds + tstruct.millitm/1000. - 9.01825*1e8) - start_second;
}

RMatrix Cross(const RMatrix &a, const RMatrix &b)
{
    if ( _length(a) !=3 || _length(b) !=3 ) error_("Cross : Not 3-dim Vector");
    
    return RMatrix(3,1,    a.element[1]* b.element[2] - a.element[2]* b.element[1],
    a.element[2]* b.element[0] - a.element[0]* b.element[2],
    a.element[0]* b.element[1] - a.element[1]* b.element[0] );    
}

real Inner(const RMatrix &a, const RMatrix &b)
{
    if ( !_isvec(a) ) error_("Inner: Not Vector");
    if ( !_isvec(b) ) error_("Inner: Not Vector");
    if ( _length(a) != _length(b) ) error_("Inner: arguments are not compatible");
        
    real sum = 0.0;
    
    for ( int i = 0; i < _length(a); i++ ) sum += a.element[i]* b.element[i];
    return sum;
}

RMatrix RMatrix::GetRow(int i) const
{
    if ( i < 1 || i > row ) error_("RMatrix GetRow(int) : index exceeds matrix dimensions");
    RMatrix re(1,col);
    i--;
    for (int j = 0; j < col; j++) re.element[j] = element[i+j*row];
    return re;
}

RMatrix RMatrix::GetRow(int si, int ei) const
{
    if ( si > ei ) error_("RMatrix GetRow(int,int) : the first argument should be less then the second");
    if ( si < 1 || ei > row ) error_("RMatrix GetRow(int,int) : index exceeds matrix dimensions");
    
    int r = ei-si+1, i, j;
    RMatrix re(r, col);
    for ( i = 0; i < r; i++)
    for ( j = 0; j < col; j++) re.element[i+j*r] = element[i+si-1+j*row];
    return re;
}

RMatrix RMatrix::GetRow(const RMatrix &idx) const
{
    int r = idx.row*idx.col, i, j, index;
    if ( r < 1 ) return RMatrix();
    RMatrix re(r, col);
    for ( i = 0; i < r; i++ )
    {
        index = int(idx.element[i]-1);
        if ( index < 0 || index >= row ) error_("RMatrix GetRow(RMatrix) : index exceeds matrix dimensions");
        for ( j = 0; j < col; j++ ) re.element[i+j*r] = element[index+j*row];
    }
    return re;
    
}

RMatrix RMatrix::GetCol(int i) const
{
    if ( i < 1 || i > col ) error_("RMatrix GetCol(int) : index exceeds matrix dimensions");
    RMatrix re(row,1);
    i--;
    for (int j = 0; j < row; j++) re.element[j] = element[j+i*row];
    return re;
}

RMatrix RMatrix::GetCol(int si, int ei) const
{
    if ( si > ei ) error_("RMatrix GetCol(int,int) : the first argument should be less then the second");
    if ( si < 1 || ei > col ) error_("RMatrix GetCol(int,int) : index exceeds matrix dimensions");
    
    int c = ei-si+1, i, j;
    RMatrix re(row, c);
    for ( i = 0; i < row; i++)
    for ( j = 0; j < c; j++) re.element[i+j*row] = element[i+(j+si-1)*row];
    return re;
}

RMatrix RMatrix::GetCol(const RMatrix &idx) const
{
    int c = idx.row*idx.col, i, j, index;
    if ( c < 1 ) return RMatrix();
    RMatrix re(row, c);
    for ( j = 0; j < c; j++ )
    {
        index = int(idx.element[j]-1);
        if ( index < 0 || index >= col ) error_("RMatrix GetCol(RMatrix) : index exceeds matrix dimensions");
        for ( i = 0; i < row; i++ ) re.element[i+j*row] = element[i+index*row];
    }
    return re;    
}

RMatrix &RMatrix::SetRow(int i, const RMatrix &m)
{
    if ( _isempty(m) )  
    {
        if ( i < 1 || i > row ) error_("RMatrix SetRow(int,RMatrix) : index exceeds matrix dimensions");
        else this->RemoveRow(i);        
    } else if ( !_isvec(m) ) error_("RMatrix SetRow(int,RMatrix) : argument is not vector");
    else if ( row ==0 || col == 0 )
    {
        this->ReSize(i,_size(m));
        for ( int j = 0; j < col; j++ ) element[i-1+j*row] = m.element[j];
    } else
    {
        if ( i < 1 ) error_("RMatrix SetRow(int,RMatrix) : index exceeds matrix dimensions");
        if ( _length(m) != col ) error_("RMatrix SetRow(int,RMatrix) : argument is not compatible");
        if ( i > row ) this->ReSize(i,col);
        for ( int j = 0; j < col; j++ ) element[i-1+j*row] = m.element[j];
    }
    return *this;
}

RMatrix &RMatrix::SetRow(int si, int ei, const RMatrix &m)
{
    if ( si > ei ) error_("RMatrix SetRow(int,int,RMatrix) : the first argument should be less then the second");
    else if ( _isempty(m) )  
    {
        if ( si < 1 || ei > row ) error_("RMatrix SetRow(int,RMatrix) : index exceeds matrix dimensions");
        else this->RemoveRow(si,ei);
    } else if ( row == 0 || col == 0 ) this->Push(si,1,m);
    else if ( ei-si+1 != m.row || col != m.col ) error_("RMatrix SetRow(int,int,RMatrix) : argument is not compatible");
    else this->Push(si,1,m);
    
    return *this;    
}

RMatrix &RMatrix::SetRow(const RMatrix &idx, const RMatrix &m)
{
    int i, j, r;
    if ( _isempty(m) ) this->RemoveRow(idx);
    else if ( row == 0 || col == 0 ) 
        for ( i = 1; i <= _size(idx); i++ )    this->Push(int(idx(i)), 1, m.GetRow(i));
    else if ( _size(idx) != m.row || col != m.col ) error_("RMatrix SetRow(RMatrix,RMatrix) : arguments are not compatible");
    else
    {
        for ( i = 0; i < _size(idx); i++ )
        {
            r = int(idx.element[i]);
            if ( r < 1 ) error_("RMatrix SetRow(RMatrix,RMatrix) : index exceeds matrix dimensions");
            if ( r > row ) 
            {
                cout << "setrow" << endl;
                this->ReSize(r,col);
                cout << "setrow" << endl;
            }
            for ( j = 0; j < col; j++ )    element[r-1+j*row] = m.element[i+j*m.row];        
        }
    }
    return *this;
}

RMatrix &RMatrix::SetCol(int i, const RMatrix &m)
{
    if ( _isempty(m) )  
    {
        if ( i < 1 || i > col ) error_("RMatrix SetCol(int,RMatrix) : index exceeds matrix dimensions");
        else this->RemoveCol(i);
    } else if ( !_isvec(m) ) error_("RMatrix SetCol(int,RMatrix) : argument is not vector");
    else if ( row ==0 || col == 0 )
    {
        this->ReSize(_size(m), i);
        for ( int j = 0; j < row; j++ ) element[j+(i-1)*row] = m.element[j];
    } else
    {
        if ( i < 1 ) error_("RMatrix SetCol(int,RMatrix) : index exceeds matrix dimensions");
        if ( _length(m) != row ) error_("RMatrix SetCol(int,RMatrix) : argument is not compatible");
        if ( i > row ) this->ReSize(i,col);
        for ( int j = 0; j < row; j++ ) element[j+(i-1)*row] = m.element[j];
    }
    return *this;
}

RMatrix &RMatrix::SetCol(int si, int ei, const RMatrix &m)
{
    if ( si > ei ) error_("RMatrix SetCol(int,int,RMatrix) : the first argument should be less then the second");
    else if ( _isempty(m) )
    {
        if ( si < 1 || ei > col ) error_("RMatrix SetCol(int,RMatrix) : index exceeds matrix dimensions");
        else this->RemoveCol(si,ei);
    } else if ( row == 0 || col == 0 ) this->Push(1,si,m);
    else if ( ei-si+1 != m.col || row != m.row ) error_("RMatrix SetCol(int,int,RMatrix) : argument is not compatible");
    else this->Push(1,si,m);
    
    return *this;    
}
    
RMatrix &RMatrix::SetCol(const RMatrix &idx, const RMatrix &m)
{
    int i, j, c;
    if ( _isempty(m) ) this->RemoveCol(idx);
    else if ( row == 0 || col == 0 ) 
        for ( i = 1; i <= _size(idx); i++ )    this->Push(1,int(idx(i)), m.GetCol(i));
    else if ( _size(idx) != m.col || row != m.row ) error_("RMatrix SetCol(RMatrix,RMatrix) : arguments are not compatible");
    else
    {
        for ( i = 0; i < _size(idx); i++ )
        {
            c = int(idx.element[i]);
            if ( c < 1 ) error_("RMatrix SetCol(RMatrix,RMatrix) : index exceeds matrix dimensions");
            if ( c > col ) this->ReSize(c,col);
            for ( j = 0; j < row; j++ )    element[j+(c-1)*row] = m.element[j+i*m.row];        
        }
    }
    return *this;
}

RMatrix Normalize(const RMatrix &m)
{
    if ( !_isvec(m) ) error_("can not Normalize it: not vector");
    real sum = 0.0;
    for ( int i = 0; i < _length(m); i++ ) sum += m.element[i] * m.element[i];
    if ( sum < _EPS ) error_("can not Normalize it: too small");
    sum = sqrt(sum);
    return m/sum;
}

real RMatrix::Normalize()
{
    if ( row*col == 0 ) return 0.0;
    int i;
    real sum = 0.0;
    for ( i = 0; i < row*col; i++ ) sum += element[i]* element[i];
    sum = sqrt(sum);
    for ( i = 0; i < row*col; i++ ) element[i] /= sum;
    return sum;
}

real Quadratic(const RMatrix &a, const RMatrix &m, const RMatrix &b)
{
    if ( _length(a) != _length(b) || _length(a) != m.row || m.row != m.col ) error_("RMatrix friend Quadratic : arguments are not compatible");
    real sum = 0.0;
    for ( int i = 0; i < m.row; i++ )
    for ( int j = 0; j < m.row; j++ ) sum += a.element[i] * m.element[i+j*m.row] * b.element[j];
    return sum;
}

real RMatrix::Quadratic(const RMatrix &a, const RMatrix &b) const
{
    if ( _length(a) != _length(b) || _length(a) != row || row != col ) error_("RMatrix Quadratic : arguments are not compatible");
    real sum = 0.0;
    for ( int i = 0; i < row; i++ )
    for ( int j = 0; j < row; j++ )    sum += a.element[i] * element[i+j*row] * b.element[j];
    return sum;
}

RMatrix Pow(const RMatrix &m, int n )
{
    if ( m.row != m.col ) error_("RMatrix Pow : not square");
    if ( n <= 0 ) return Eye(m.RowSize());
    RMatrix re(m);
    for ( int i = 1; i < n; i++ ) re *= m;
    return re;
}

RMatrix RMatrixd(int m, int n, ...)
{
    va_list marker;
    va_start(marker, n);
    RMatrix re(m,n);
    for ( int j = 0; j < n; j++ ) 
    for ( int i = 0; i < m; i++ ) re.element[i+j*m] = va_arg(marker, real);
    va_end( marker );
    return re;
}
    
RMatrix RMatrixm(int m, int n, ...)
{
    va_list marker;
    va_start(marker, n);
    int flag = 1, r = 0, c = 0, i, j, tmpr, isequal;
    RMatrix* tmp;

    struct matrixsize 
    {
        int r, c; 
    } *psize = new matrixsize [n*m];
    
    // log sizes of argument RMatrices
    for ( j = 0; j < n; j++ )
    for ( i = 0; i < m; i++ )
    {
        tmp = &(va_arg(marker, RMatrix));
        (psize+i+j*m)->r = tmp->RowSize();
        (psize+i+j*m)->c = tmp->ColSize();
    }

    // check that the sizes are compatible
    for ( j = 0; j < n; j++ )
    {
        tmpr = (psize+j*m)->r;
        for ( i = 1; i < m; i++ )
        {
            if ( (psize+i+j*m)->c == 0 ) isequal = 1;
            else isequal = (psize+i+j*m)->c == (psize+i-1+j*m)->c;
            if ( !(flag = ( flag && isequal)) ) error_("makemMatrix : not compatible column size");
             tmpr += (psize+i+j*m)->r;
        }
        if ( j == 0 ) r = tmpr;
        if ( r != tmpr ) error_("makemMatrix : not compatible row size");
        c += (psize+j*m)->c;
    }
    
    RMatrix re(r,c);
    va_end(marker);
    
    va_start(marker, n);
    for ( c = 1, j = 0; j < n; j++ ) 
    {
        for ( r = 1, i = 0; i < m; i++ ) {    
            re.Replace(r,c, va_arg(marker, RMatrix));
            r += ((psize+i+j*m)->r);
        }
        c += (psize+j*m)->c;
    }
    va_end(marker);
    delete [] psize;

    return re;
}
