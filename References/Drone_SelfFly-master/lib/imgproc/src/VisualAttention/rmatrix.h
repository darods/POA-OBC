//////////////////////////////////////////////////////////////////////////////////
//
//        title        :    header for Real Matrix Class and Linear Algebra Functions
//                        
//        version        :    v1.99
//        author        :    Jinwook Kim (zinook@kist.re.kr)
//        last update    :    99.8.1
//
//        Note        :
//
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RMatrix_
#define _RMatrix_

#include <iostream>

using namespace std;

typedef double real;

void error_(char*);    // report error to 'error.log'
void tic();            // start a stopwatch timer.
double toc();        // read the stopwatch timer and return ellapsed seconds since tic() was called.

class RMatrix{
private:
    int row, col;
    real *element;
    
public:
    // constructor
    RMatrix() : row(0), col(0), element(0) { };                            // default constructor
    RMatrix(int r, int c) : row(r), col(c), element(new real[r*c]) { }; // constructor with dimension
    
    RMatrix(const RMatrix &m) : row(m.row), col(m.col), element(new real[row*col]) { for ( int i = 0; i < row*col; i++) *(element+i) = *(m.element+i); };
    // copy constructor

    RMatrix(int r, int c, const RMatrix &m1, const RMatrix &m2);
    RMatrix(int r, int c, const RMatrix &m1, const RMatrix &m2, const RMatrix &m3);
    RMatrix(int r, int c, const RMatrix &m1, const RMatrix &m2, const RMatrix &m3, const RMatrix &m4);
    
    RMatrix(const real &d) : row(1), col(1) { element = new real[1]; *element = d; };    // constructor of one component
    RMatrix(const int &d) : row(1), col(1) { element = new real[1]; *element = real(d); };    // constructor of one component
    RMatrix(int r, int c, const real &d1, const real &d2);
    RMatrix(int r, int c, const real &d1, const real &d2, const real &d3);
    RMatrix(int r, int c, const real &d1, const real &d2, const real &d3, const real &d4);
    RMatrix(int r, int c, const real &d1, const real &d2, const real &d3, const real &d4, const real &d5, const real &d6);
    RMatrix(int r, int c, const real &d1, const real &d2, const real &d3, const real &d4, const real &d5, const real &d6, const real &d7, const real &d8, const real &d9);
    RMatrix(int r, int c, real **d);                                        // constructor from double pointer, not double array.
    RMatrix(int r, int c, real *d) : row(r), col(c), element(new real[r*c])    // constructor from single pointer, element is arranged along column
    { for (int i = 0; i < r*c; i++ ) *(element+i) = *(d+i);    };                // for example, m=[1,2;3,4] -> real a[4]={1,3,2,4]; RMatrix m(2,2,a);
    RMatrix(int r, int c, int *d) : row(r), col(c), element(new real[r*c])
    { for (int i = 0; i < r*c; i++ ) *(element+i) = *(d+i);    };
    
    // destructor
    ~RMatrix() { delete [] element; };

    //member functions
    RMatrix Sub(int rs, int re, int cs, int ce) const;        // return submatrix of given range based m-file notation
    
    RMatrix &Replace(int i, int j, const RMatrix &m);        // replace some part of matrix with m
    RMatrix &Replace(const RMatrix &index, const RMatrix &m);    // replace m into *this
    RMatrix &Replace(const RMatrix &ri, const RMatrix &ci, const RMatrix &m);    // replace some part of matrix with m
    
    RMatrix &Push(int i, int j, const RMatrix &m);            // push m into *this
    RMatrix &Push(const RMatrix &index, const RMatrix &m);    // push m into *this
    RMatrix &Push(const RMatrix &ri, const RMatrix &ci, const RMatrix &m);        // push m into *this
    
    int RowSize(void) const { return row; };                // return number of row
    int ColSize(void) const { return col; };                // return number of column
        
    RMatrix &ReSize(int r, int c = 1);                        // resize matrix
    RMatrix &ReNew(int r, int c);                            // if size of memory allocaated is different from r*c, then reallocate memory
    RMatrix &RowSwap(int i, int j);                            // swap ith & jth row
    RMatrix &ColSwap(int i, int j);                            // swap ith & jth column
    
    RMatrix GetRow(int i) const;                            // get i'th row vector
    RMatrix GetRow(int si, int ei) const;                    // get m(si:ei,:)
    RMatrix GetRow(const RMatrix &idx) const;                // get i'th row
    
    RMatrix &SetRow(int i, const RMatrix &m);                // set i'th row with m, m should be vector
    RMatrix &SetRow(int si, int ei, const RMatrix &m);        // means that a(si:ei,:) = m
    RMatrix &SetRow(const RMatrix &idx, const RMatrix &m);    // set idx'th row

    RMatrix GetCol(int i) const;                            // get i'th column vector
    RMatrix GetCol(int si, int ei) const;                    // get m(:,si:ei)
    RMatrix GetCol(const RMatrix &idx) const;                // get i'th row
    
    RMatrix &SetCol(int i, const RMatrix &m);                // get i'th column with m, m should be vector
    RMatrix &SetCol(int si, int ei, const RMatrix &m);        // means that a(:,si:ei) = m
    RMatrix &SetCol(const RMatrix &idx, const RMatrix &m);    // set idx'th row
    
    RMatrix &RemoveRow(int idx);                            // remove ith row
    RMatrix &RemoveRow(const RMatrix &idx);                    // remove idx matrix
    RMatrix &RemoveRow(int si, int ei);                        // remove from si'th row to ei'th row
    
    RMatrix &RemoveCol(int idx);                            // remove ith column
    RMatrix &RemoveCol(const RMatrix &idx);                    // remove idx matrix
    RMatrix &RemoveCol(int si, int ei);                        // remove from si'th row to ei'th row
    
    RMatrix &Remove(int i);                                    // remove i'th element and make row vector
    RMatrix &Remove(const RMatrix &idx);                    // remove idx'th element and make row vector
    
    RMatrix &MakeColVector();                                // make column vector and return
    real Normalize();                                        // normalize (*this) and return its norm
    real Quadratic(const RMatrix &a, const RMatrix &b) const; // return a * (*this) * b : a,b are assumed to be vectors

    //operators
    
    real &operator () (int i);                                // ith element in column order
    const real &operator () (int i) const { if ( i < 1 || i > row*col ) error_("RMatrix (int) : index over range"); return *(element+i-1); };

    real &operator () (int i, int j);                        // (i,j) element
    const real &operator () (int i, int j) const { if ( i < 1 || j < 1 || i > row || j > col ) error_("RMatrix (int,int) : index over range"); return *(element+(j-1)*row+i-1); };
    
    RMatrix operator () (const RMatrix &idx) const;                        // a_k = a(idx(i,j))
    RMatrix operator () (const RMatrix &ri, const RMatrix &ci) const;    // a_ij = a(ri(i), ci(j))
    RMatrix operator + (void) const { return RMatrix(*this); };            // annary plus RMatrix
    RMatrix operator - (void) const { return -1.0*RMatrix(*this); };    // annary minus RMatrix
    
    RMatrix &operator = (const RMatrix &m);            // substitute operator
    RMatrix &operator = (const real &d);            // substitute operator
    RMatrix &operator -= (const RMatrix &m);        // -= substitute operator
    RMatrix &operator += (const RMatrix &a);        // += operator
    RMatrix &operator *= (const RMatrix &m);        // *= operator
    RMatrix &operator *= (const real &d);            // *= substitute operator
    RMatrix &operator /= (const real &d);            // /= operator
    RMatrix operator == (const RMatrix &m) const;    // relational operator, equal
    RMatrix operator == (const real &d) const;        // relational operator, equal
    RMatrix operator != (const RMatrix &m) const;    // relational operator, not equal
    RMatrix operator != (const real &d) const;        // relational operator, not equal
    RMatrix operator > (const RMatrix &m) const;    // relational operator, greater than
    RMatrix operator > (const real &d) const;        // relational operator, greater than
    RMatrix operator < (const RMatrix &m) const;    // relational operator, less than
    RMatrix operator < (const real &d) const;        // relational operator, less than
    RMatrix operator >= (const RMatrix &m) const;    // relational operator, greater than or equal
    RMatrix operator >= (const real &d) const;        // relational operator, greater than or equal
    RMatrix operator <= (const RMatrix &m) const;    // relational operator, less than or equal
    RMatrix operator <= (const real &d) const;        // relational operator, less than or equal
    RMatrix operator && (const RMatrix &m) const;    // relational operator, and
    RMatrix operator && (const real &d) const;        // relational operator, and
    RMatrix operator || (const RMatrix &m) const;    // relational operator, or
    RMatrix operator || (const real &d) const;        // relational operator, or
    RMatrix operator ! (void) const;                // not operator
    

    RMatrix operator + (const RMatrix &m) const;        // + operator : (*this) + m
    RMatrix operator - (const RMatrix &m) const;        // - operator : (*this) - m
    RMatrix operator * (const RMatrix &m) const;        // * operator : product with RMatrix (*this) * m
    RMatrix operator / (RMatrix m) const;                // / operator : fast matrix inversion (*this) * Inv(m)
    //RMatrix operator / (const RMatrix &m) const;                // / operator : fast matrix inversion (*this) * Inv(m)
    RMatrix operator % (RMatrix m) const;                // % operator : fast matrix inversion Inv(*this) * m
    RMatrix operator + (const real &c) const;            // + operator : +c component wise
    RMatrix operator - (const real &c) const;            // - operator : -c component wise
    RMatrix operator * (const real &c) const;            // * operator : *c compoenet wise
    RMatrix operator / (const real &c) const;            // / operator : /c component wise
    RMatrix operator ~ (void) const;                    // ~ operator : Transpose
    
    //friend functions
    friend int RowSize(const RMatrix &m) { return m.row; };                        // return number of row 
    friend int ColSize(const RMatrix &m) { return m.col; };                        // return number of column 
    friend void Size(const RMatrix &m, int *r, int *c) { *r = m.row; *c = m.col; };
    friend int Length(const RMatrix &m) { if ( m.col && m.row ) return m.col>m.row?m.col:m.row; else return 0; };    // return max(column,row)
    friend RMatrix ColVector(const RMatrix &m);                                    // return as column vector
    friend RMatrix Normalize(const RMatrix &m);                                    // normalize m
    friend real *NewPtr(int n, int m, const RMatrix &M);                        // allocate memory and initialize its element with M
    friend real *NewPtr(int n, int m, const real *elm);                            // allocate memory and initialize its element with elm
    friend double Todouble(const RMatrix &m);                                    // casting to double

    friend RMatrix operator + (const real &c, const RMatrix &a);    // + operator : +c component wise
    friend RMatrix operator - (const real &c, const RMatrix &a);    // + operator : -c component wise
    friend RMatrix operator * (const real &c, const RMatrix &a);    // * operator : *c compoenet wise
    friend ostream &operator << (ostream &os, const RMatrix &m);    // ostream standard output
    friend istream &operator >> (istream &is, RMatrix &m);            // istream standart input
    
    friend RMatrix ElMult(const RMatrix &a, const RMatrix &b);        // return (a_ij*b_ij)
    friend RMatrix ElDivid(const RMatrix &a, const RMatrix &b);        // return (a_ij/b_ij)
    friend RMatrix ElPower(const RMatrix &m, real p);                // return pow(a_ij, p)
    
    // for optimal speed use this methods, return is 're', size of 're' must be same to result
    friend void MultMatrix(RMatrix &re, const RMatrix &a, const RMatrix &b); // multiplication re = a*b
    friend void AddMatrix(RMatrix &re, const RMatrix &a, const RMatrix &b);  // addition re = a+b
    friend void SubtractMatrix(RMatrix &re, const RMatrix &a, const RMatrix &b); // subtraction re = a-b
    
    friend RMatrix Zeros(int r, int c);                                // return zero matrix
    friend RMatrix Ones(int r, int c);                                // return 1 matrix
    friend RMatrix Eye(int n);                                        // return identity matrix
    friend RMatrix Eye(int r, int c);                                // return identity matrix
    friend RMatrix RowIndex(const real &start, const real &end);    // return start:end in row vector
    friend RMatrix RowIndex(const real &start, const real &step, const real &ene);    // return start:step:end
    friend RMatrix ColIndex(const real &start, const real &end);    // return start:end in column vector
    friend RMatrix ColIndex(const real &start, const real &step, const real &ene);    // return start:step:end
    friend RMatrix Rand(int n, int m);                                // return n*m random matrix
    friend RMatrix SymRand(int n);                                    // return n*n random symmetric matrix
    friend RMatrix SkewRand(int n);                                    // return n*n random skew symmetric matrix
    friend RMatrix Rosser(void);                                    // return 8X8 rosser matrix - test matrix for eig algorithm
    friend RMatrix foptions(void);                                    // return options matrix used at optimization    
    friend RMatrix foptions(RMatrix opt);
    friend RMatrix Skew(const RMatrix &m);                            // return skew symmetric matrix of 3 of 6 vector m
    friend RMatrix InvSkew(const RMatrix &m);                        // return vector from skew symmetric matrix
    friend RMatrix Companion(const RMatrix &u);                        // return companion matrix of u
    friend RMatrix RMatrixd(int n, int m, ...);                        // make nXm matrix from varialbe number of  real arguments
                                                                    // Be cautious. Type of optional arguments should be real.
                                                                    // Also n & m should be compatible with the number of optional arguments.
                                                                    // For example, RMatrix(2, 3, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0) = [1.0, 3.0 5.0; 2.0, 4.0 6.0]
                                                                    // , where it is not permitted That the third argument is '1' under the case of real = double. Becuase '1' is type of int.
    friend RMatrix RMatrixm(int n, int m, ...);                        // make matrix from varialbe number of RMatrix arguments
                                                                    // columns of each matrices in the same row should be equal
    
    friend int IsEmpty(const RMatrix &m) { return ( m.col*m.row == 0 ) ? 1 : 0; };    // validate check
    friend int IsSqaure(const RMatrix &m) { return ( m.row == m.col ) ? 1 : 0; };    // if m is sqaure matrix, return 1 else 0
    friend int IsTrue(const RMatrix &m);                            // if m has all non-zero elements, return1 else 0
    friend int IsVector(const RMatrix &m);                            // if m is vector, return its size else 0
    friend int IsSymmetric(const RMatrix &m);                        // if m is symmetric, return 1 else 0
    friend int IsPositive(const RMatrix &m);                        // if m is positive, return 1 else 0. m is assumed to be symmetric.
    friend int NNZ(const RMatrix &m);                                // number of nonzero elements
    friend RMatrix Any(const RMatrix &m);                            // True if any element of a vector is nonzero
    friend RMatrix Min(const RMatrix &m);                            // find minimum column wise
    friend void Min(const RMatrix &m, RMatrix &min, RMatrix &idx);    // return minimum with its index
    friend RMatrix Min(const RMatrix &m, real r);                    // return min(m_ij, r)
    friend RMatrix Max(const RMatrix &m);                            // find maximum column wise
    friend RMatrix Max(const RMatrix &m, real r);                    // return max(m_ij, r)
    friend void Max(const RMatrix &m, RMatrix &min, RMatrix &idx);    // return maximum with its index
    friend real MinVec(const RMatrix &m, int *idx = 0);                // return min(m), m is assumed to be vector, min(m) is idx'th element
    friend real MaxVec(const RMatrix &m, int *idx = 0);                // return max(m), m is assumed to be vector, max(m) is idx'th element
    
    friend real Norm(const RMatrix &m, real p=0.0);                    // matrix norm
    friend real FNorm(const RMatrix &m);                            // matrix Frobenius norm
    friend real InftyNorm(const RMatrix &m);                        // matrix Infinity norm
    
    friend RMatrix Sum(const RMatrix &m);                            // matrix Sum
    friend RMatrix Abs(const RMatrix &m);                            // return fabs of each element
    friend RMatrix Mean(const RMatrix &m);                            // find mean value column wise
    friend RMatrix Sort(const RMatrix &m);                            // quick sort, same to matlab 'sort'
    friend RMatrix Sort(const RMatrix &m, RMatrix &idx);            // quick sort, same to matlab 'sort'
    friend RMatrix Sign(const RMatrix &m);                            // return sign
    
    friend RMatrix Find(const RMatrix &a);                            // find non-zero element location
    friend void Find(const RMatrix &a, RMatrix &ri, RMatrix &ci);    // find non-zero element location
    
    friend RMatrix ZeroRemove(const RMatrix &a);                    // non-zero element
    friend RMatrix Pow(const RMatrix &m, int n );                    // power of m by order n
    
    friend RMatrix Cross(const RMatrix &a, const RMatrix &b);        // cross product of 3X1 matrices
    friend real Inner(const RMatrix &a, const RMatrix &b);            // inner product of n-vectors
    friend real Quadratic(const RMatrix &a, const RMatrix &m, const RMatrix &b); // return a' * m * b : a,b are assumed to be vectors
    
    friend RMatrix Diag(const RMatrix &m);                            // return Diagonalized matrix
    friend RMatrix Diag(const RMatrix &m, int n);                    // return Diagonalized matrix
    friend RMatrix Transpose(const RMatrix &m);                        // transpose of m
    friend real Det(const RMatrix &m);                                // Determinant of m
    friend real Trace(const RMatrix &m);                            // Trace
    friend real Cond(const RMatrix &m);                                // condition number, (minimum singular value)/(maximum singular value)
    
    friend RMatrix Inv(const RMatrix &m);                            // Inverse of a
    friend RMatrix pInv(const RMatrix &m);                            // pseudo inverse of a
    friend RMatrix SVD(const RMatrix &m);                                    // Singular value decomposition
    friend void SVD(const RMatrix &M, RMatrix &U, RMatrix &S, RMatrix &V);    // M = U*S*~V
    friend void ESVD(const RMatrix &M, RMatrix &U, RMatrix &S, RMatrix &V);    // M = U*S*~V
    
    friend RMatrix Eig(RMatrix m);                                    // get eigen-values of general real matrix m
    friend void Eig(RMatrix m, RMatrix &v, RMatrix &d);                // get eigen-values and eigen-vectors of general real matrix m

    friend RMatrix Expm(const RMatrix &m);                            // exponential of general real matrix
    friend void QR(const RMatrix &m, RMatrix &q, RMatrix &r);        // QR decomposition, m = q*r, where q is orthogonal and r is upper-triangular matrix
    friend void QR(const RMatrix &m, RMatrix &q, RMatrix &r, RMatrix &e);    // QR decomposition, m*e = q*r, where q is orthogonal and r is upper-triangular matrix and e is permutation matrix
    friend RMatrix QR(RMatrix m);                                    // QR decomposition, return result of dqrdc
    friend RMatrix Chol(RMatrix m);                                    // Cholesky factorization, m = ~re*re, where re is upper-triangular and m is assumed to be positive definite and symmetric.
    friend void Chol(const RMatrix &m, RMatrix &r, int &d);            // Cholesky factorization, if not positive definite, d = 1, else 0

    friend RMatrix Conv(const RMatrix &u, const RMatrix &v);        // convolution of vectors.
    friend RMatrix Roots(const RMatrix &cof);                        // roots of a polynomial of which coefficients are cof.
};

#endif
