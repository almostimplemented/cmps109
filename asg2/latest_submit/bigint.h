// Author: Andrew Edwards
// Email:  ancedwar@ucsc.edu
// ID:     1253060
// Date:   2015 Jan 23
//
// bigint.h

#ifndef __BIGINT_H__
#define __BIGINT_H__

#include <exception>
#include <iostream>
#include <utility>
using namespace std;

#include "debug.h"
using digit_t = unsigned char;
using bigvalue_t = vector<digit_t>;

//
// Define class bigint
//
class bigint {
    friend ostream& operator<< (ostream&, const bigint&);
  private:
    void init (const string&);
    long long_value {};
    bool negative;
    bigvalue_t big_value;
    using quot_rem = pair<bigint,bigint>;
    using unumber = unsigned long;
    friend quot_rem divide (const bigint&, const bigint&);
    friend void multiply_by_2 (unumber&);
    friend void divide_by_2 (unumber&);
    friend bigvalue_t do_bigadd (const bigvalue_t&, const bigvalue_t&);
    friend bigvalue_t do_bigsub (const bigvalue_t&, const bigvalue_t&);
    friend bigvalue_t do_bigmul (const bigvalue_t&, const bigvalue_t&);
    friend bool do_bigless (const bigvalue_t&, const bigvalue_t&);
    friend bigvalue_t partial_prod(const bigvalue_t&, size_t);
    friend bigvalue_t partial_quot(const bigvalue_t&, size_t);
    friend bigvalue_t partial_rem(const bigvalue_t&, size_t);
    friend digit_t trialdigit(const bigvalue_t&, const bigvalue_t&,
                              size_t, size_t);
    friend bool smaller(const bigvalue_t&, const bigvalue_t&,
                        size_t, size_t);
    friend bigvalue_t difference(const bigvalue_t&, const bigvalue_t&,
                                 size_t, size_t);
    friend quot_rem longdiv(const bigvalue_t& x, const bigvalue_t& y,
                            size_t n, size_t m);
    friend quot_rem divide(const bigvalue_t& x, const bigvalue_t& y);
  public:

    //
    // Ensure synthesized members are genrated.
    //
    bigint() = default;
    bigint (const bigint&) = default;
    bigint (bigint&&) = default;
    bigint& operator= (const bigint&) = default;
    bigint& operator= (bigint&&) = default;
    ~bigint() = default;

    //
    // Extra ctors to make bigints.
    //
    bigint (const bigvalue_t&);
    bigint (const long);
    bigint (const string&);

    //
    // Basic add/sub operators.
    //
    friend bigint operator+ (const bigint&, const bigint&);
    friend bigint operator- (const bigint&, const bigint&);
    friend bigint operator+ (const bigint&);
    friend bigint operator- (const bigint&);
    long to_long() const;

    //
    // Extended operators implemented with add/sub.
    //
    friend bigint operator* (const bigint&, const bigint&);
    friend bigint operator/ (const bigint&, const bigint&);
    friend bigint operator% (const bigint&, const bigint&);

    //
    // Comparison operators.
    //
    friend bool operator== (const bigint&, const bigint&);
    friend bool operator<  (const bigint&, const bigint&);
};

//
// The rest of the operators do not need to be friends.
// Make the comparisons inline for efficiency.
//

ostream& operator<< (ostream& out, const bigvalue_t& that);

bigint pow (const bigint& base, const bigint& exponent);

inline bool operator!= (const bigint &left, const bigint &right) {
    return not (left == right);
}
inline bool operator>  (const bigint &left, const bigint &right) {
    return right < left;
}
inline bool operator<= (const bigint &left, const bigint &right) {
    return not (right < left);
}
inline bool operator>= (const bigint &left, const bigint &right) {
    return not (left < right);
}
#endif

