// Author: Andrew Edwards
// Email:  ancedwar@ucsc.edu
// ID:     1253060
// Date:   2015 Jan 23
//
// bigint.cpp

#include <cstdlib>
#include <exception>
#include <limits>
#include <stack>
#include <stdexcept>
#include <algorithm>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "util.h"

#define LINE_LIMIT 69

// Constructors

bigint::bigint (const bigvalue_t& that): big_value (that) {
    DEBUGF ('~', this << " -> " << big_value << " (copy ctor)")
}

bigint::bigint (long that): long_value (that) {
    init(to_string(that));
    DEBUGF ('~', this << " -> " << long_value << " (int ctor)")
}

bigint::bigint (const string& that) {
    init(that);
    DEBUGF ('~', this << " -> " << big_value << " (string ctor)")
}

// Initialization method
// Takes a string representing a number and instantiates the
// bigvalue_t vector with the corresponding digits.
// 
// The input can indicate negativity via '_' (if it comes from
// the user input) or '-' (if it comes from the to_string method
// when the bigint(long) construtor is used)

void bigint::init (const string& that) {
    negative = false;
    auto itor = that.cbegin();
    if (itor != that.cend() and (*itor == '_' || *itor == '-')) {
        negative = true;
        ++itor;
    }
    int newval = 0;
    while (itor != that.end()) {
        newval = newval * 10 + *itor++ - '0';
    }
    for (auto ritor  = that.crbegin(); 
            ritor != that.crend() && *ritor != '_';
            ritor++) {
        big_value.push_back(*ritor - '0');
    }
    long_value = negative ? - newval : + newval;
}

// Adds two digit vectors. 
// Same logic as addition by hand.
bigvalue_t do_bigadd (const bigvalue_t& left, 
                      const bigvalue_t& right) {
    bigvalue_t sum;
    digit_t carry(0);
    digit_t digit_sum(0);
    size_t min_size = min(left.size(), right.size());
    size_t i;
    for (i = 0; i < min_size; i++) {
        // Compute digit sum. If greater than 9, take note
        // with the carry bit and deduct 10 from the sum.
        digit_sum = left.at(i) + right.at(i) + carry;
        if (digit_sum > 9) {
            carry = 1;
            digit_sum -= 10;
        } else {
            carry = 0;
        }
        sum.push_back(digit_sum);
    }
    while (i < left.size()) {
        digit_sum = left.at(i) + carry;
        if (digit_sum > 9) {
            carry = 1;
            digit_sum -= 10;
        } else {
            carry = 0;
        }
        sum.push_back(digit_sum);
        i++;
    }
    while (i < right.size()) {
        digit_sum = right.at(i) + carry;
        if (digit_sum > 9) {
            carry = 1;
            digit_sum -= 10;
        } else {
            carry = 0;
        }
        sum.push_back(digit_sum);
        i++;
    }

    // Last step: if the carry bit is set, we need to
    // push back a 1 to be the new highest digit.
    if (carry == 1)
        sum.push_back(1);

    return sum;
}

// Subtracts two digit vectors. 
// Same logic as subtraction by hand. 
// Precondition:  left >= right
// Postcondition: result >= 0
bigvalue_t do_bigsub (const bigvalue_t& left, 
                      const bigvalue_t& right) {
    bigvalue_t diff;
    digit_t borrow(0);
    digit_t digit_diff(0);
    size_t i;
    for (i = 0; i < right.size(); i++) {
        // Check if we need to borrow from the next highest digit
        if (left.at(i) - borrow < right.at(i)) {
            digit_diff = 10 + left.at(i) - right.at(i) - borrow; 
            borrow = 1;
        } else {
            digit_diff = left.at(i) - right.at(i) - borrow;
            borrow = 0;
        }
        diff.push_back(digit_diff);
    }
    while (i < left.size()) {
        if (left.at(i) < borrow) {
            digit_diff = 10 + left.at(i) - borrow; 
            borrow = 1;
        } else {
            digit_diff = left.at(i) - borrow;
            borrow = 0;
        }
        diff.push_back(digit_diff);
        i++;
    }
    // Remove leading zeroes
    while (diff.size() > 1 && diff.back() == 0)
        diff.pop_back();
    return diff;
}

// Returns true if left < right, assumes both are positive
// integers.
bool do_bigless (const bigvalue_t& left,
                 const bigvalue_t& right) {
    // if the vectors' sizes differ the answer is trivial
    if (left.size() < right.size())
        return true;
    else if (left.size() > right.size())
        return false;

    // iterate from highest order digits to find smaller input
    auto lit = left.crbegin();
    auto rit = right.crbegin();
    for (; lit != left.crend(); lit++, rit++)
        if (*lit < *rit)
            return true;
        else if (*lit > *rit)
            return false;

    // in this case they are equal
    return false;
}

// Overloading the addition and subtraction operators.  
// Determines proper sign and makes the appropriate call to 
// do_bigadd or do_bigless.

bigint operator+ (const bigint& left, const bigint& right) {
    bigint sum;
    if (left.negative == right.negative) {
        sum.big_value = do_bigadd(left.big_value, right.big_value);
        sum.negative = left.negative;
        return sum;
    } else {
        if (do_bigless(left.big_value, right.big_value)) {
            sum.big_value = do_bigsub(right.big_value, left.big_value);
            sum.negative = right.negative;
        } else { 
            sum.big_value = do_bigsub(left.big_value, right.big_value);
            sum.negative = left.negative;
        }
        return sum;
    }
}

bigint operator- (const bigint& left, const bigint& right) {
    bigint diff;
    if (left.negative == right.negative) {
        if (do_bigless(left.big_value, right.big_value)) {
            diff.big_value = do_bigsub(right.big_value, 
                    left.big_value);
            diff.negative = not right.negative;
        } else {
            diff.big_value = do_bigsub(left.big_value,
                    right.big_value);
            diff.negative = left.negative;
        }

        return diff;
    } else {
        diff.negative = left.negative;
        diff.big_value = do_bigadd(left.big_value, 
                right.big_value);
        return diff;
    }
}

// Overloading the positive and negative unary operators.

bigint operator+ (const bigint& right) {
    bigint pos_bigint(right);
    pos_bigint.negative = false;
    return pos_bigint;
}

bigint operator- (const bigint& right) {
    bigint pos_bigint(right);
    pos_bigint.negative = true;
    return pos_bigint;
}

// Multiplies two digit vectors. 
// Same logic as long multiplication 
bigvalue_t do_bigmul (const bigvalue_t& left, const bigvalue_t& right) {
    bigvalue_t product(left.size() + right.size(), 0);
    digit_t c, d;
    for (size_t i = 0; i < left.size(); i++) {
        c = 0;
        for (size_t j = 0; j < right.size(); j++) {
            d = product.at(i+j) + (left.at(i) * right.at(j)) + c;
            product.at(i+j) = d % 10;
            c = d / 10;
        }
        product.at(i + right.size()) = c;
    }
    while (product.size() > 1 && product.back() == 0)
        product.pop_back();
    return product;
}

bigint operator* (const bigint& left, const bigint& right) {
    bigint product;
    product.negative = (left.negative != right.negative);
    product.big_value = do_bigmul(left.big_value, 
            right.big_value);
    return product;
}

//
// Long division algorithm.
//
// From P. Brinch Hansen,
// Multiple-length division revisited: A tour of the minefield.

// Partial product, quotient, and remainder assume that
// x is a multiple length integer and k is a single digit
// i.e. 0 <= k < 10

bigvalue_t partial_prod(const bigvalue_t& x, size_t k) {
    int temp, carry;
    size_t size = x.size();
    bigvalue_t product(x.size() + 1, 0);
    carry = 0;
    for (size_t i = 0; i < size; i++) {
        temp = x.at(i) * k + carry;
        product.at(i) = temp % 10;
        carry = temp / 10;
    }
    product.at(size) = carry;
    while (product.size() > 1 && product.back() == 0)
        product.pop_back();
    DEBUGF ('/', "partial_prod(" << x << ", "
                 << (int) k << ") = " << product)
    return product;
}

bigvalue_t partial_quot(const bigvalue_t& x, size_t k) {
    int temp, carry;
    size_t size = x.size();
    bigvalue_t quotient(x.size(), 0);
    carry = 0;
    for (size_t i = size - 1; i < size; i--) {
        temp = x.at(i) + 10 * carry;
        quotient.at(i) = temp / k;
        carry = temp % k;
    }
    while (quotient.size() > 1 && quotient.back() == 0)
        quotient.pop_back();
    DEBUGF ('/', "partial_quot(" << x << ", " 
                 << k << ") = " << quotient)
    return quotient;
}

bigvalue_t partial_rem(const bigvalue_t& x, size_t k) {
    int carry;
    size_t size = x.size();
    carry = 0;
    for (size_t i = size - 1; i < size; i--) {
        carry = (x.at(i) + 10 * carry) % k;
    }
    // Since k is a digit, the remainder must also be a digit
    DEBUGF ('/', "partial_rem(" << x << ", " 
                 << k << ") = " << carry)
    return bigvalue_t(1, carry);
}

//
// The computation of a quotient digit q_k breaks down into the
// simpler prefix operations. The assignment
//          q_t = trialdigit(r, d, k, m)
// defines a trial digit, q_t = q_e, which is an inital estimate
// of q_k. The operands of the trial digit function are prefixes
// of the remainder r and the divisor d
//      r[k + m - 2 ... k + m]    d[m - 1 ... m - 2]
// where
//          2 <= m <= k + m
//

digit_t trialdigit(const bigvalue_t& r, const bigvalue_t& d,
                   size_t k, size_t m) {
    DEBUGF ('/', "trialdigit(" << r << ", " << d << ", " << 
                 (int) k << ", " << (int) m << ")") 
    int d2, r3;
    size_t km;
    km = k + m;
    if (r.size() > km)
        r3 = (r.at(km)*10 + r.at(km - 1))*10 + r.at(km - 2);
    else if (r.size() > km - 1)
        r3 = r.at(km - 1)*10 + r.at(km - 2);
    else if (r.size() > km - 2)
        r3 = r.at(km - 2);
    else
        r3 = 0;

    if (d.size() > m - 1)
        d2 = d.at(m - 1)*10 + d.at(m - 2);
    else if (d.size() > m - 2)
        d2 = d.at(m - 2);
    else
        d2 = 0;

    DEBUGF ('/', "trialdigit = " << min(r3 / d2, 9))
    return min(r3 / d2, 9);
}


// Returns r[k ... k + m] < dq 
// (Note dq = dq[m ... 0])
bool smaller(const bigvalue_t& r, const bigvalue_t& dq,
                size_t k, size_t m) {
    DEBUGF ('/', "smaller(" << r << ", " << dq << ", " <<
                (int) k << ", " << (int) m << ")")
    int i, j;

    // Add leading zeroes if necessary, to make comparison easier
    bigvalue_t r_copy(r);
    while (r_copy.size() <= m + k)
        r_copy.push_back(0);

    bigvalue_t dq_copy(dq);
    while (dq_copy.size() <= m)
        dq_copy.push_back(0);

    i = m;
    j = 0;
    while (i != j)
        if (r_copy.at(i + k) != dq_copy.at(i))
            j = i;
        else
            i = i - 1;

    return r_copy.at(i + k) < dq_copy.at(i);
}

// Returns r - dq * 10^k, corresponding to the long divison step of
// subtracting from the high order digits of the current remainder.
bigvalue_t difference(const bigvalue_t& r, const bigvalue_t& dq,
                      size_t k, size_t m) {
    bigvalue_t dq_shifted;
    // Do the multiplication locally, since it is trivial
    for (size_t i = 0; i < k; i++)
        dq_shifted.push_back(0);
    for (auto it = dq.cbegin(); it != dq.cend(); it++)
        dq_shifted.push_back(*it);

    DEBUGF ('/', "difference(" << r << ", " << dq << ", " <<
                (int) k << ", " << (int) m << ")" << " = " 
                << do_bigsub (r, dq_shifted))

    return do_bigsub (r, dq_shifted); 
}

// Auxiliary function used by divide(x, y). The call to divide
// checks for the simple cases where x < y or y.size() == 1.
//
// If neither of those cases apply, this function is called. 
// n and m are the size of x and y, respectively.
//
// The procedure mainly works by estimating the quotient digits,
// correcting as necessary, and then subtracting the most recently
// computed quotient digit (scaled to the apropriate power of 10)
// from the current remainder.
//
// There is also a scaling step that reduces the expected number
// of digit corrections.
//
// See the referenced paper for a full description of the algorithm.
bigint::quot_rem longdiv(const bigvalue_t& x, const bigvalue_t& y,
                      size_t n, size_t m) {
    DEBUGF ('/', "longdiv(" << x << ", " << y << ", " <<
                (int) n << ", " << (int) m << ")")
    bigvalue_t d, dq, q(n, 0), r;
    int f, qt;
    int k;

    f = 10 / (y.at(m - 1) + 1);
    r = partial_prod(x, f);
    d = partial_prod(y, f);
    for (k = n - m; k >= 0; k--) {
        qt = trialdigit(r, d, k, m);
        dq = partial_prod(d, qt);
        if (smaller(r, dq, k, m)) {
            qt = qt - 1;
            dq = partial_prod(d, qt);
        }
        DEBUGF ('/', "accessing q(" << (int) k << ")"
                     << "and q has size " << q.size())
        q.at(k) = qt;
        r = difference(r, dq, k, m);
    }
    while (q.size() > 1 && q.back() == 0)
        q.pop_back();
    while (r.size() > 1 && r.back() == 0)
        r.pop_back();
    return make_pair(bigint(q), bigint(partial_quot(r,f)));
}

// Main division function. Checks for corner cases and then calls
// longdiv(x, y, x.size(), y.size()) if necessary.
bigint::quot_rem divide(const bigvalue_t& x, const bigvalue_t& y) {
    DEBUGF ('/', "divide(" << x << ", " << y << ")")
    int n, m, y1;
    m = y.size();
    if (m == 1) {
        y1 = y.at(m - 1);
        return make_pair(bigint(partial_quot(x, y1)),
                         bigint(partial_rem(x, y1)));
    } else {
        n = x.size();
        if (m > n)
            return make_pair(bigint(0), x);
        return longdiv(x, y, n, m);
    }
}

// Overloading the division operator. Determines sign of quotient and
// then calls divide(left, right).
bigint operator/ (const bigint& left, const bigint& right) {
    if (right == 0) throw ydc_exn ("ydc: divid by zero");
    bigint result = divide (left.big_value, right.big_value).first;
    result.negative = left.negative ^ right.negative;
    return result;
}

// Overloading the modulus operator. Determines sign of quotient and
// then calls divide(left, right).
bigint operator% (const bigint& left, const bigint& right) {
    bigint result = divide (left.big_value, right.big_value).second;
    result.negative = left.negative ^ right.negative;
    return result;
}

// Overloading the equality operator. Checks sizes and signs, and if
// necessary it steps through the digits to determine equality.
bool operator== (const bigint& left, const bigint& right) {
    if (left.negative != right.negative)
        return false;
    if (left.big_value.size() != right.big_value.size())
        return false;

    for (size_t i = 0; i < left.big_value.size(); i++) 
        if (left.big_value.at(i) != right.big_value.at(i))
            return false;

    return true;
}

// Overloading the less than operator. Checks sign and then will 
// make call to do_bigless depending on the situation.
bool operator< (const bigint& left, const bigint& right) {
    bool retval;
    if (left.negative) {
        if (!right.negative)
            retval = true;
        else 
            retval = do_bigless(right.big_value, left.big_value);
    } else {
        if (right.negative)
            retval = false;
        else 
            retval = do_bigless(left.big_value, right.big_value);
    }
    DEBUGF ('l', "operator<(" << left << ", " << right 
                       << ") = " << retval) 
    return retval;
}

ostream& operator<< (ostream& out, const bigint& that) {
    if (that.negative)
        out << '-';
    out << that.big_value;
    return out;
}

ostream& operator<< (ostream& out, const bigvalue_t& that) {
    size_t i = 0;
    for (auto rit  = that.crbegin();
            rit != that.crend();
            ++rit) {
        out << (int) *rit;
        i++;
        if (i == LINE_LIMIT) {
            out << "\\" << endl;
            i = 0;
        }
    }
    return out;
}

// Tries to convert the bigint to a long and fails if out of range
long bigint::to_long() const {
    DEBUGF ('^', "this.long_value = " << long_value);
    if (*this <= bigint (numeric_limits<long>::min())
            or *this > bigint (numeric_limits<long>::max()))
        throw range_error ("bigint__to_long: out of range");
    return long_value;
}

// Implementation of the power operation. Not overloading because
// we require that the exponent be able to fit inside a long.
bigint pow (const bigint& base, const bigint& exponent) {
    DEBUGF ('^', "base = " << base << ", exponent = " << exponent);
    if (base == 0) return 0;
    bigint base_copy = base;
    long expt = exponent.to_long();
    bigint result = 1;
    if (expt < 0) {
        base_copy = 1 / base_copy;
        expt = - expt;
    }
    while (expt > 0) {
        if (expt & 1) { //odd
            result = result * base_copy;
            --expt;
        }else { //even
            base_copy = base_copy * base_copy;
            expt /= 2;
        }
    }
    DEBUGF ('^', "result = " << result);
    return result;
}
