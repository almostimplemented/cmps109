// $Id: bigint.cpp,v 1.61 2014-06-26 17:06:06-07 - - $

#include <cstdlib>
#include <exception>
#include <limits>
#include <stack>
#include <stdexcept>
#include <algorithm>
using namespace std;

#include "bigint.h"
#include "debug.h"

bigint::bigint (long that): long_value (that) {
    DEBUGF ('~', this << " -> " << long_value << " (int ctor)")
}

bigint::bigint (const bigvalue_t& that): big_value (that) {
    DEBUGF ('~', this << " -> " << big_value << " (copy ctor)")
}

bigint::bigint (const string& that) {
    negative = false;
    auto itor = that.cbegin();
    if (itor != that.cend() and *itor == '_') {
        negative = true;
        ++itor;
    }
    int newval = 0;
    while (itor != that.end()) {
        newval = newval * 10 + *itor++ - '0';
    }
    for (auto ritor  = that.crbegin(); 
              ritor != that.crend() && *ritor != '_';
              ritor++)
        big_value.push_back(*ritor);
    long_value = negative ? - newval : + newval;
    DEBUGF ('~', this << " -> " << big_value << " (string ctor)")
}

// Addition algorithm
bigvalue_t bigint::do_bigadd (const bigvalue_t& left, 
                              const bigvalue_t& right) {
    bigvalue_t sum;
    size_t carry(0);
    size_t digit_sum(0);
    size_t min_size = min(left.size(), right.size());
    size_t i;
    for (i = 0; i < min_size; i++) {
        digit_sum = (left.at(i) - '0') + (right.at(i) - '0') + carry;
        if (digit_sum > 9) {
            carry = 1;
            digit_sum -= 10;
        } else {
            carry = 0;
        }
        sum.push_back(digit_sum + '0');
    }
    while (i < left.size()) {
        digit_sum = left.at(i) - '0' + carry;
        if (digit_sum > 9) {
            carry = 1;
            digit_sum -= 10;
        } else {
            carry = 0;
        }
        sum.push_back(digit_sum + '0');
        i++;
    }
    while (i < right.size()) {
        digit_sum = right.at(i) - '0' + carry;
        if (digit_sum > 9) {
            carry = 1;
            digit_sum -= 10;
        } else {
            carry = 0;
        }
        sum.push_back(digit_sum + '0');
        i++;
    }

    if (carry == 1)
        sum.push_back('1');

    return sum;
}

bigint operator+ (const bigint& left, const bigint& right) {
    bigint sum;
    if (left.negative == right.negative) {
        sum.big_value = bigint::do_bigadd(left.big_value, 
                                          right.big_value);
        sum.negative = left.negative;
        return sum;
    } else {
        if (bigint::do_bigless(left.big_value, right.big_value)) {
            sum.big_value = bigint::do_bigsub(right.big_value, 
                                               left.big_value);
            sum.negative = right.negative;
        } else { 
            sum.big_value = bigint::do_bigsub(left.big_value,
                                               right.big_value);
            sum.negative = left.negative;
        }
        return sum;
    }
}

// Subtraction algorithm
bigvalue_t bigint::do_bigsub (const bigvalue_t& left, 
                              const bigvalue_t& right) {
    bigvalue_t diff;
    size_t borrow(0);
    size_t digit_diff(0);
    size_t i;
    for (i = 0; i < right.size(); i++) {
        if (left.at(i) - borrow < right.at(i)) {
            digit_diff = 10 + left.at(i) - right.at(i) - borrow; 
            borrow = 1;
        } else {
            digit_diff = left.at(i) - right.at(i) - borrow;
            borrow = 0;
        }
        diff.push_back(digit_diff + '0');
    }
    while (i < left.size()) {
        if (left.at(i) < '0' + borrow) {
            digit_diff = 10 + left.at(i) - '0' - borrow; 
            borrow = 1;
        } else {
            digit_diff = left.at(i) - '0' - borrow;
            borrow = 0;
        }
        diff.push_back(digit_diff + '0');
        i++;
    }
    while (diff.size() > 0 && diff.back() == '0')
        diff.pop_back();
    return diff;
}

bool bigint::do_bigless (const bigvalue_t& left,
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

bigint operator- (const bigint& left, const bigint& right) {
    bigint diff;
    if (left.negative == right.negative) {
        if (bigint::do_bigless(left.big_value, right.big_value)) {
            diff.big_value = bigint::do_bigsub(right.big_value, 
                                               left.big_value);
            diff.negative = not right.negative;
        } else {
            diff.big_value = bigint::do_bigsub(left.big_value,
                                               right.big_value);
            diff.negative = left.negative;
        }

        return diff;
    } else {
        diff.negative = left.negative;
        diff.big_value = bigint::do_bigadd(left.big_value, 
                                           right.big_value);
        return diff;
    }
}

bigint operator+ (const bigint& right) {
    return +right.long_value;
}

bigint operator- (const bigint& right) {
    return -right.long_value;
}

long bigint::to_long() const {
    if (*this <= bigint (numeric_limits<long>::min())
            or *this > bigint (numeric_limits<long>::max()))
        throw range_error ("bigint__to_long: out of range");
    return long_value;
}

bool abs_less (const long& left, const long& right) {
    return left < right;
}

//
// Multiplication algorithm.
//
bigint operator* (const bigint& left, const bigint& right) {
    return left.long_value * right.long_value;
}

//
// Division algorithm.
//

void multiply_by_2 (bigint::unumber& unumber_value) {
    unumber_value *= 2;
}

void divide_by_2 (bigint::unumber& unumber_value) {
    unumber_value /= 2;
}

bigint::quot_rem divide (const bigint& left, const bigint& right) {
    if (right == 0) throw domain_error ("divide by 0");
    using unumber = unsigned long;
    static unumber zero = 0;
    if (right == 0) throw domain_error ("bigint::divide");
    unumber divisor = right.long_value;
    unumber quotient = 0;
    unumber remainder = left.long_value;
    unumber power_of_2 = 1;
    while (abs_less (divisor, remainder)) {
        multiply_by_2 (divisor);
        multiply_by_2 (power_of_2);
    }
    while (abs_less (zero, power_of_2)) {
        if (not abs_less (remainder, divisor)) {
            remainder = remainder - divisor;
            quotient = quotient + power_of_2;
        }
        divide_by_2 (divisor);
        divide_by_2 (power_of_2);
    }
    return {quotient, remainder};
}

bigint operator/ (const bigint& left, const bigint& right) {
    return divide (left, right).first;
}

bigint operator% (const bigint& left, const bigint& right) {
    return divide (left, right).second;
}

bool operator== (const bigint& left, const bigint& right) {
    return left.long_value == right.long_value;
}

bool operator< (const bigint& left, const bigint& right) {
    return left.long_value < right.long_value;
}

ostream& operator<< (ostream& out, const bigint& that) {
    if (that.negative)
        out << '-';
    out << that.big_value;
    return out;
}

ostream& operator<< (ostream& out, const bigvalue_t& that) {
    for (auto rit  = that.crbegin();
              rit != that.crend();
              ++rit) {
        out << *rit;
    }
    return out;
}

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
