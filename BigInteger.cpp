#include"BigInteger.hpp"

BigInteger::BigInteger() : digits(new Digit()), last(digits), Positive(true) {}

BigInteger::BigInteger(i64 number) {
    if(number < 0) {
        Positive = false;
        number = -number;
    } else
        Positive = true;
    digits = new Digit((ui64)number);
    last = digits;
}

BigInteger::BigInteger(Digit* d, bool pos) : Positive(pos) {
    while(d != NULL) {
        this->append(d->value);
        d = d->next;
    }
}

BigInteger::BigInteger(const BigInteger& a) : Positive(a.Positive) {
    Digit* aux = a.digits;
    while(aux != NULL) {
        this->append(aux->value);
        aux = aux->next;
    }
}

BigInteger::BigInteger(const ui64 array[], unsigned size, bool positive)
    : Positive(positive) {
    if(array == NULL || size == 0) {
        setAsZero();
        return;
    }
    unsigned i = 1;
    digits = new Digit(array[0]);
    last = digits;
    while(i < size) {
        last->next = new Digit(array[i++]);
        last = last->next;
    }
}

BigInteger::BigInteger(const char str[], NumberBase base) {
    int strlen = -1, validCharCount = 0, bitsOccupied = 0;
    int lowerLimit = 0; // -O if no negative sign, 1 in other case
    int value = 0;      // -Holds number value of ascii character.
    if(base > 4) base = (NumberBase)3;
    while(str[++strlen] != 0) {}
    if(strlen == 0 || str == NULL) {
        setAsZero();
        return;
    }
    if(str[0] == '-') { // -Determining the sign.
        Positive = false;
        lowerLimit = 1;
    }
    switch(base) {
        case BINARY :
            while(--strlen >= lowerLimit) {
                if(str[strlen] == '1' || str[strlen] == '0') {
                    if(bitsOccupied == 0) append(0);
                    last->value |= (ui64)(str[strlen] - 48) << bitsOccupied;
                    validCharCount++;
                    bitsOccupied = validCharCount & 63;
                }
            }
            break;
        case QUATERNARY :
            while(--strlen >= lowerLimit) {
                if(str[strlen] > 47 && str[strlen] < 52 ) {
                    if(bitsOccupied == 0) append(0);
                    last->value |= (ui64)(str[strlen] - 48) << bitsOccupied;
                    validCharCount++;
                    bitsOccupied = (validCharCount << 1) & 63;
                }
            }
            break;
        case OCTAL :
            while(--strlen >= lowerLimit) {
                if(str[strlen] > 47 && str[strlen] < 56 ) {
                    if(bitsOccupied == 0) append(0);
                    last->value |= (ui64)(str[strlen] - 48) << bitsOccupied;
                    validCharCount++;
                    bitsOccupied = (validCharCount * 3) & 63;
                }
            }
            break;
        case HEXADECIMAL :
            bool validHexChar;
            while(--strlen >= lowerLimit) {
                if(str[strlen] > 47 && str[strlen] < 57 ) {
                    value = str[strlen] - 48;
                    validHexChar = true;
                }
                if(str[strlen] > 64 && str[strlen] < 91 ) {
                    value = str[strlen] - 55;
                    validHexChar = true;
                }
                if(str[strlen] > 96 && str[strlen] < 123) {
                    value = str[strlen] - 87;
                    validHexChar = true;
                }
                if(validHexChar) {
                    if(bitsOccupied == 0) append(0);
                    last->value |= (ui64)value << bitsOccupied;
                    validCharCount++;
                    bitsOccupied = (validCharCount << 2) & 63;
                }
            }
            break;
        case DECIMAL :
            if(str[strlen] > 47 && str[strlen] < 57 ) {
                // ...
            }
            break;
        default: ;
    }
}

BigInteger::BigInteger(const char bytes[], ui64 size, bool positive)
    : Positive(positive) {
    if(bytes == NULL || size == 0) {
        setAsZero();
        return;
    }
    ui64 q = size >> 3; // q = size / 8;
    ui64 r = size &  7; // r = size % 8;
    ui64 buffer = 0;
    ui64 i = 0;

    if(q == 0) {
        while(i < r) {
            buffer <<= 8;
            buffer |= (ui64)(ui08)bytes[i++];
        }
        digits = new Digit(buffer);
        last = digits;
        return;
    }
    if(r > 0) {
        while(i < r) {
            buffer <<= 8;
            buffer |= (ui64)(ui08)bytes[i++];
        }
        digits = new Digit(buffer);
        last = digits;
        i += r;
    } else {
        digits = new Digit(_8bytes_to_int64(bytes));
        last = digits;
        i += 8;
    }
    for(; i < q; i += 8) push(_8bytes_to_int64(&bytes[i]));
}

BigInteger::~BigInteger() {
    Digit* aux;
    while(digits != NULL) {
        aux = this->digits;
        this->digits = this->digits->next;
        delete aux;
    }
    digits = last = NULL;
}

BigInteger& BigInteger::operator = (const BigInteger& a) {
    if(this != &a) { // Guarding against self assignment.
        this->~BigInteger();
        this->Positive = a.Positive;
        Digit *aux = a.digits;
        while(aux != NULL) {
            this->append(aux->value);
            aux = aux->next;
        }
    }
    return *this;
}

// |||||||||||| Some facts for operation in the ring of integer ||||||||||||||

// ·[0] (Euclidean division theorem) Given a pair of integers a,b, exist
//      unique integers q,r such that a = q·b + r. Using notation for the
//      operations in C++ we can write a = (a/b)*b + a%b.
// ·[1] (x+y)%n = (x%n + y)%n = (x + y%n)%n = (x%n + y%n)%n.
// ·[2] (x+y)/n = x/n + y/n + (x%n + y%n)/n.
// ·[3] Sea 0 <= a,b < n, then (a + b)%n < a,b if and only if a + b > n.
// ·[4] x/2^n = x>>n and x*2^n = x<<n.
// ·[5] x%2^n = x&(2^n - 1).

//----------------------------------------------------------------------------

// |||||| Algorithm for the addition of two positive numbers in base k |||||||

// -Set 'k' as any natural number bigger than 1
// -Representation of a and b in digits base k:
//  a == a_{n}a_{n-1}...a_{1}a_{0} and b == b_{m}b_{n-1}...b_{1}b_{0}, where
//  +) 0 <= a_i,b_j <= k-1 for each i\in{1,...,m} and j\in{1,...,m}
//  ++) a_{n} == 0 if and only if a == 0 and b_{n} == 0 if and only if b == 0

// -Set carriage_{0} = 0, suppose n <= m and call r = a + b, then:
// -r == r_{m+1}r_{m}...r_{1}r_{0} where
//  1) r_{0} = (a_{0} + b_{0})%k and carriage_{0} = (a_{0} + b_{0})/k
//  2) r_{i} = (a_{i} + b_{i} + carriage_{i-1})%k
//     carriage_{i} = (a_{i}+b_{i}+carriage_{i-1})/k for each i\in{0,...,n}.
//  3) r_{i} = (b_{i} + carriage_{i-1})%k and
//     carriage_{i} = (b_{i} + carriage_{i-1})/k for i \in {n+1,...,m}
//  4) r_{m+1} = carriage_{m}
// -From (1), (2), (3) and (+) we can see that 0 <= carriage_{i} <= 1.
// -Notice that, from the definitions, for i\in{n+1,...,m}:
//  ~ If b_{i} = k - 1 and carriage_{i} = 1, then r_{i} = 0 and
//    carriage_{i} = 1; in any other case r_{i} = b_{i} + carriage_{i} and
//    carriage_{i} = 0.

// -Setting k = 2^64 we have
//  i) 0 <= a_{i}+b_{i}+carriage_{i} <= 2^65 - 1.
// -As a result of the last inequality, we'll need 65 bits to hold the result
//  of that sum for arbitrary arguments. Our objective is to determine, using
//  operations in 64 bits, each of the r_{i}'s and each the carriage_{i}'s.

// -From equities 1),2) and inequality i) it follows that
//  I)  ra_{i} is exactly the first 64 bits.
//  II) carriage_{i} is the value of the last bit (left most bit).

// -Rewrite a_{i} as a_{i} = la_{i}^63 + ra_{i}, where la_{i} is its left most
//  bit and ra_{i} is the 63 bits left, do the same with b_{i}, then
//  · a_{i} + b_{i} + carriage_{i-1}
//    = la_{i}^63 + ra_{i} + lb_{i}^63 + rb_{i} + carriage_{i-1}
//    = (la_{i} + lb_{i})·2^63 + (ra_{i} +rb_{i} + carriage_{i-1})

// -From definition and (+) follows that
//  ··)  0 <= ra_{i} + rb_{i} + carriage_{i-1} <= 2^64-1
//  ···) 0 <= la_{i} + lb_{i} <= 2

// -Now we can write:
//  r_{i} = (a_{i} + b_{i} + carriage_{i-1})%2^64
//        = [(la_{i} + lb_{i})·2^63 + (ra_{i} +rb_{i} + carriage_{i-1})]%2^64
//  (**)  = [(la_{i} + lb_{i})·2^63 % 2^64 +
//          ra_{i} + rb_{i} + carriage_{i-1}] % 2^64
// -From (**) follows that if la_{i} = lb_{i} = 1 or la_{i} = lb_{i} = 0 then
//  (la_{i}+lb_{i})·2^63%2^64 = 0 and r_{i} = ra_{i} + rb_{i} + carriage_{i-1}

// -Let la_{i} != lb_{i}, then la_{i} + lb_{i} = 1. Do the substitution
//  x := ra_{i} + rb_{i} + carriage_{i-1}. <~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  From [1] x = (x/2^63)·2^63 + (x%2^63) [left most bit, last 63 bits]
// -So:
//  r_{i} = [2^63 + (x/2^63)·2^63 + (x%2^63)] % 2^64
//        = [((1 + [x/2^63])·2^63) % 2^64 + [x%2^63] ] % 2^64
//  First bit of x~~~~^   Last 63 bits of x~~~~^
// -Therefore, the expression ((1 + [x/2^63])·2^63) % 2^64 is zero if and
//  only if the First bit of x (from left to right) is one. In short
//  <1> If the first bit of x is 1, then
//      r_{i} = (ra_{i} + rb_{i} + carriage_{i-1})%2^63 = x & 2^63 - 1
//  <2> If the first bit of x is 0, then
//      r_{i} = 2^63 + ra_{i} + rb_{i} + carriage_{i-1} = 2^63 + x

// -Now we'll calculate the carriage_{i}'s. From 2), ·), [2] and using the
//  same substitution with x:
//  carriage_{i} =
//      [(la_{i} + lb_{i})·2^63 + (ra_{i} +rb_{i} + carriage_{i-1})]/2^64 =
//      [(la_{i} + lb_{i})·2^63 + x]/2^64 =
//      [(la_{i} + lb_{i})·2^63]/2^64 + x/2^64 +
//      ([(la_{i} + lb_{i})·2^63]%2^64 + x%2^64)/2^64 =: _X_
// -Case 1: la_{i} = lb_{i} = 1, then _X_= 1 + 0 + (0 + x)/2^64 = 1
// -Case 2: la_{i} = lb_{i} = 0, then _X_= 0 + 0 + (0 + x)/2^64 = 0
// -Case 3: la_{i} != lb_{i}, then _X_= 0 + 0 + (2^63 + x)/2^64.
//  2^63 + x >= 2^64 if and only if x >= 2^63 if and only if the first
//  bit of x is 1, then _X_= 1 if first bit of x == 1, _X_ = 0 otherwise.

// ---------------------------------------------------------------------------

BigInteger& additionPositive(const BigInteger& a, const BigInteger& b,
							 BigInteger& result) {
	if(result.digits != NULL) {
	    result.~BigInteger();
	    result.digits = result.last = NULL;
	}
	if(a.digits == NULL || b.digits == NULL) return result; // Exception here

    ui64 la, lb, ra, rb, x;
    ui64 carriage = 0;

    BigInteger::Digit *da = a.digits, *db = b.digits, *largest = NULL;
    while(da != NULL && db != NULL) {
        la = da->value & BigInteger::ui64LeftMost_1; // -Left most bit
        lb = db->value & BigInteger::ui64LeftMost_1; // -Left most bit
        ra = da->value & BigInteger::ui64MAX >> 1; // -All but the left most
        rb = db->value & BigInteger::ui64MAX >> 1; //  bit (in both cases).

        x = ra + rb + carriage;

        if(la == lb) {
            result.append(x);
            if(la > 0) carriage = 1;
            else carriage = 0;
        } else {
            if((x & BigInteger::ui64LeftMost_1) > 0) {
                result.append(x & BigInteger::ui64MAX >> 1);
                carriage = 1;
            } else { //  x + BigInteger::ui64LeftMost_1
                result.append(x | BigInteger::ui64LeftMost_1);
                carriage = 0;
            }
        }
        da = da->next;
        db = db->next;
    }
    // -In case of having to numbers of different length if digits.
    if(da != NULL) largest = da;
    if(db != NULL) largest = db;
    while(largest != NULL) {
        if((carriage == 1) && (largest->value == BigInteger::ui64MAX))
            result.append(0); // Preserve carriage as 1.
        else {
            result.append(largest->value + carriage);
            carriage = 0;
        }
        largest = largest->next;
    }
    if(carriage == 1) result.append(1);
    return result;
}

BigInteger operator + (const BigInteger& a, const BigInteger& b) {
    BigInteger::Digit* nonce = NULL;
    BigInteger r(nonce);

    // Case 1: Operands are negative.
    if(a.Positive == false && b.Positive == false) {
        additionPositive(a, b, r);
        r.Positive = false;
        return r;
    }
    // Case 2: First positive, second negative.
    if(a.Positive == true && b.Positive == false) {
        return subtractionPositive(a, b, r);
    }
    // Case 2: First negative, second positive.
    if(a.Positive == false && b.Positive == true) {
        return subtractionPositive(b, a, r);;
    }
    // Case 4: Operands are positive.
    return additionPositive(a, b, r);
}

// |||| Algorithm for the subtraction of two positive numbers in base k ||||||

// -Set 'k' as any natural number bigger than 1
// -Representation of a and b in digits base k:
//  a == a_{n}a_{n-1}...a_{1}a_{0} and b == b_{m}b_{n-1}...b_{1}b_{0}, where
//  +)  0 <= a_i,b_j <= k-1 for each i\in{1,...,m} and j\in{1,...,m}
//  ++) a_{n} == 0 if and only if a == 0 and b_{n} == 0 if and only if b == 0

// -Set loan_{0} = 0 and suppose m < n, set r = a - b, then
// -r = r_{n}r_{n-1}...r_{0}, where
//  1) r_{0} = a_{0} - b_{0}; if r_{0} < 0, then r_{0} += k and loan_{0} = 1;
//  2) For i\in {1,...,m} : r_{i} = a_{i} - b{i} - loan_{i-1}; if r_{0} < 0,
//     r_{0} += k and loan_{i} = 1, in other case set loan_{i} = 0.
//  3) For i\in{m+1,..,n} : r_{i} = a_{i} - loan_{i-1}; if r_{i} < 0,
//     r_{i} += k and set loan_{i} = 1, in other case set loan_{i} = 0.

// -In our case, we are using unsigned integers of 64 bits, so the comparisons
//  r_{i} < 0 and r_{0} < 0 are useless (they're always false). Fortunately
//  we don't have to use them; from their definition:

//  *  r_{0} < 0 if and only if a_{0} < b_{0} and
//  ** r_{i} < 0 if and only if a_{i} < b_{i} + load

// -(*) can be evaluated easily, (**) is a bit more complicated. If we have
//  b_{i} == 2^64 - 1, then b_{i} + 1 == 2^64 witch is congruent with
//  zero mod 2^64 (Remember that the arithmetic of unsigned integers is the
//  arithmetic of modular integers, where the value of the modulus is the
//  maximum value for the unsigned integer plus one, in this case is 2^64).
//  In other words,if b_{i} == 2^64 - 1, a_{i} == b_{i} and load == 1, then
//  [a_{i} < b_{i}+load] == [2^64 - 1 < 2^64%2^64] == [2^64 - 1 < 0] == false,
//  witch is something we do not want. Luckily, the only values that load
//  takes are 0 and 1, therefore

//  *** a_{i} < b_{i} + load if and only if a_{i} < b_{i} or a_{i} == b_{i}
//      and load == 1.

// -Finally, for (3) we got r_{i} < 0 if and only if a_{i} == 0 and load == 1.

BigInteger& subtractionPositive(const BigInteger& a,
								const BigInteger& b,
								BigInteger& result) {
    if(result.digits != NULL) {
	    result.~BigInteger();
	    result.digits = result.last = NULL;
	}
	if(a.digits == NULL || b.digits == NULL) return result; // Exception here

	ui64 load = 0;
	bool aIsBigger = false;
	BigInteger::Digit *d0 = a.digits, *d1 = b.digits;

	while(d0 != NULL && d1 != NULL) { // Determining the biggest number.
        if(d0->value >= d1->value) aIsBigger = true;
        else aIsBigger = false;
        d0 = d0->next;
        d1 = d1->next;
	}
	if(d0 != NULL && d0->value != 0) aIsBigger = true;
	if(aIsBigger) {
	    result.Positive = true;
	    d0 = a.digits;
	    d1 = b.digits;
	} else {
	    result.Positive = false;
	    d0 = b.digits;
	    d1 = a.digits;
	}
    while(d0 != NULL && d1 != NULL) {
        if((d0->value < d1->value) || (d0->value == d1->value && load == 1)) {
            // Using (a_{i} - b_{i} - load) + k == (k - b_{i} - load) + a_{i}
            // k = (k-1) + 1; in this case 2^64 = (2^64-1) + 1.
            result.append((BigInteger::ui64MAX - d1->value + 1 - load)
                          + d0->value);
            load = 1;
        } else {
            result.append(d0->value - d1->value - load);
            load = 0;
        }
        d0 = d0->next;
        d1 = d1->next;
    }
    if(d0 != NULL) {
        while(d0->next != NULL) {
            if(d0->value == 0 && load == 1) {
                result.append(BigInteger::ui64MAX);
                load = 1;
            } else {
                if(load == 1) {
                    result.append(d0->value - 1);
                    load = 0;
                }
                else result.append(d0->value);
            }
            d0 = d0->next;
        }
        if(d0->value > 1 || load == 0) result.append(d0->value - load);
    }
    // -Erasing the leftmost zeros till find a nonzero element or till
    //  we have just one element in the list.
    while(result.last != result.digits && result.last->value == 0)
          result.pop();

    return result;
}

BigInteger operator - (const BigInteger& a, const BigInteger& b) {
    BigInteger::Digit* nonce = NULL;
    BigInteger r(nonce);

    // Case 1: Operands are negative.
    if(a.Positive == false && b.Positive == false) {
        return subtractionPositive(b, a, r);
    }
    // Case 2: First positive, second negative.
    if(a.Positive == true && b.Positive == false) {
        return additionPositive(a, b, r);
    }
    // Case 3: First negative, second positive.
    if(a.Positive == false && b.Positive == true) {
        additionPositive(a, b, r);
        r.Positive = false;
        return r;
    }
    // Case 4: Operands are positive.
    return subtractionPositive(a, b, r);
}

// ||||||||||||||| Multiplication of integers of 64 bits |||||||||||||||||||||

// -Having two arbitrary integers of 64 bits, compute its product and give
//  the result in an array of two 64-bits integer.

// Define the sets UI64 := {unsigned integers of 64 bits} and
// UI32 := {unsigned integers of 32 bits}. Consider the functions
// u:UI64 -> UI32 and l:UI64 -> UI32 defined as

// *)  u(x) := ux were ux is the upper 32 bits of x.
// **) l(x) := lx were lx is the lower 32 bits of x.

// Then, for all x\in UI64, x can be rewrite as
// ***) x = 2^32·u(x) + l(x)

// Let a and b be two unsigned integers of 64 bits. By (***) we can write
// a = 2^32·u(a) + l(a)
// b = 2^32·u(b) + l(b)

// Lets rename u(a) as ua, l(a) as la, u(b) as ub and l(b) as lb, then
// I) a·b = (2^32·ua + la)·(2^32·ub + lb) =
//        = 2^64·(ua·ub) + 2^32·(ua·lb) + 2^32·(la·ub) + la·lb
//        = 2^64·(ua·ub) + 2^32·(ua·lb + la·ub) + la·lb
//               |--1--|        |------2------|   |-3-|

// Part 1 and 3 can be computed in 64 bits since they are the product of two
// 32 bits integers. Part 2 can be seen as the addition of two 64 bits integer,
// so we need 65 bits in order to know the result for arbitrary arguments. We
// can solve this issue by using the fact the sum of unsigned integers in C++
// the sum modulus 2^w where w can be 8, 16, 32 or 64 (bit length of the word
// containing the integer), and the facts about integers (see above) number 3
// to determine the value of the most significant bit (65th bit) of part 2.
// More in concrete:

// +) Bit 65 of ua·lb + la·ub is 1 if and only if ua·lb + la·ub > 2^64 if and
// only if ua·lb, la·ub > (ua·lb + la·ub)% 2^64, this last expression is sum
// of unsigned integers of 64 bits in C++.

// In order to continue, consider the sets UI65 := {Integers of 65 bits} and
// the functions U:UI65->{0,1}, l64:UI65 -> UI64 defined as

// U(x)   := Ux   were Ux is the most significant bit.
// l64(x) := l64x were l64x is the lower 64 bits of x.

// Define X := ua·lb + la·ub, then we can write X = 2^64·U(X) + l64(X), and we
// can calculate U(x) as mentioned above in (+). Notice that l64(X) = (ua·lb +
// la·ub)%2^64.

// Using (I) and the above results we can write
// a·b = 2^64·(ua·ub) + 2^32·(2^64·U(X) + l64(X)) + la·lb
//     = 2^96·U(X) + 2^64·(ua·ub) + 2^32·l64(X) + la·lb

// Dividing l64(X) into its upper and lower part we obtain
// II) a·b = 2^96·U(X)+2^64·(ua·ub) + 2^64·u(l64(X)) + 2^32·l(l64(X)) + la·lb
//         = 2^64·(2^32·U(X) + (ua·ub) + u(l64(X))) + 2^32·l(l64(X)) + la·lb
//                |---------------r1--------------|  |----------r0----------|

// We almost have the result. From the definition of X we got
// 0 <= X <= 2·(2^64 - 2·2^32 + 1), so 0 <= l64(X) <= 2^64 - 2·2^32 + 1, and
// 0 <= u(l64(X)), l(l64(X)) <= 2^32 - 2·2^16 + 1.
// With this inequalities:
// r1 = 2^32·U(X) + (ua·ub) + u(l64(X)) <= 2^32+(2^64-2·2^32+1)+2^32-2·2^16+1
//                                       = 2^64 - 2^17 + 2 < 2^64
// So 2^32·U(X) + (ua·ub) + u(l64(X)) can be computed in 64 bits. This is not
// the case for r0 because in the worst scenario this is what we got
// 2^32·l(l64(X)) + la·lb = 2^32·(2^32-2·2^16+1) + la·lb
//                        = 2^64 - 2·2^48 + 2^32 + 2^64 - 2·2^32 + 1
//                        = 2^64 + 2^64 - 2·2^48 + 2^32 + 1 > 2^64
// But these won't be a problem because we can use the same technique we used
// with the computation of X; in this case, if 2^32·l(l64(X)) + la·lb > 2^64,
// then we add 1 to r1.

void BigInteger::ui64Product(ui64 a, ui64 b, ui64 result[2]) const {
    ui64Toui32 _a_, _b_, _l64X_;
    _a_.ui64int = a; // ua == _a_.uint[1], la == _a_.uint[0]
    _b_.ui64int = b; // ub == _a_.uint[1], lb == _a_.uint[0]

    // printf("\na = %lX, b = %lX", a, b); //debuggin purposes

    ui64 lalb = (ui64)_a_.uint[0] * _b_.uint[0]; //printf("\nlalb = %lX",lalb);
    ui64 laub = (ui64)_a_.uint[0] * _b_.uint[1]; //printf("\nlaub = %lX",laub);
    ui64 ualb = (ui64)_a_.uint[1] * _b_.uint[0]; //printf("\nualb = %lX",ualb);
    ui64 uaub = (ui64)_a_.uint[1] * _b_.uint[1]; //printf("\nuaub = %lX",uaub);

    ui64 l64X = laub + ualb, UX = 0;
    if(l64X < laub) UX = 1; // case laub + ualb >= 2^64
    _l64X_.ui64int = l64X;  // u(l64X)=_l64X_.uint[0], l(l64X)=_l64X_.uint[1]


    result[1] = (UX << 32) + uaub + (ui64)_l64X_.uint[1];
    result[0] = ((ui64)_l64X_.uint[0] << 32) + lalb;

    //printf("\nresult[0] = (%lX << 32) + %lX + %X", UX, uaub, _l64X_.uint[1]);
    //printf("\nresult[1] = (%lX << 32) + %lX\n", (ui64)_l64X_.uint[0], lalb);

    if(result[0] < lalb) ++result[1]; // case 2^32·l(l64(X)) + la·lb > 2^64
}

// -The following algorithm for multiplication of numbers of radix k was
//  taken from Knut's book "The art of computer programming: Volume 2".

BigInteger operator * (const BigInteger& a, const BigInteger& b) {
    BigInteger::Digit *nonce = NULL, *ad = a.digits, *bd = b.digits;
    BigInteger r(nonce); // Initializing with NULL list of digits.
    r.Positive = (a.Positive && b.Positive) || !(a.Positive || b.Positive);

    if(a.digits == NULL || b.digits == NULL) {return r;} // exception here
    if(a == 0 || b == 0) return BigInteger();            // Returns Zero
    std::cout << '\n';
    ui64 t[2], k = 0, aux;
    while(ad != NULL) { // -Initializing the result.
        // -Supposing a = a[n-1]...a[0] and b = b[m-1]...b[0] (numbers of
        //  'n' and 'm' digits, respectively), then, in this loop, we are
        //  initializing the first 'n' digits. with b[0] * a[i].
        a.ui64Product(ad->value, b.digits->value, t);

        printf("\nt[1,0] = [%lX, %lX]",t[1],t[0]); // Debugging purposes

        // -If t[1] + k >= 2^64 then aux = (t[1] + k)%2^64 (this is how C/C++
        //  handle operations with unsigned integers), so aux < t[1],k; this
        aux = t[0] + k; // last inequality happens if and only if t[1]+k>=2^64
        if(aux < k) ++t[1]; // In casee of t[0] + k > 2^64 - 1
        t[0] = aux;

        printf("\nt[1,0] = [%lX, %lX]\n",t[1],t[0]); // Debugging purposes

        r.append(t[0]); // r[i] = t mod 2^64
        k = t[1];       // k = t / 2^64"
        ad = ad->next;
        std::cout << "\nr = "; r.println();// Debugging purposes
    }
    r.append(k); bd = bd->next;
    while(bd != NULL) { // -Initializing the next 'm' digits with zeros.
        r.append(0);
        bd = bd->next;
    }
    std::cout << "\nr = "; r.println();// Debugging purposes
    ad = a.digits;
    bd = b.digits->next; // In this line we know that b.digits != NULL
    // -Since we now got a * b[0], r.digits it's not NULL and the algorithm
    //  demand to start the sum in the next digit of r, this is r[1].
    // 'rrd' can be interpreted as "running over result digits"
    BigInteger::Digit *rd = r.digits->next, *rrd = rd;
    while(bd != NULL) {
        k = 0;
        while(ad != NULL) {
            a.ui64Product(ad->value, bd->value, t);

            printf("\nt[1,0] = [%lX, %lX]",t[1],t[0]); // Debugging purposes

            aux = t[0] + k;     // Adding k.
            if(aux < k) ++t[1]; // Case t[1] + k >= 2^64
            t[0] = aux;
            printf("\nt[1,0] = [%lX, %lX]",t[1],t[0]); // Debugging purposes

            aux = rrd->value + t[0]; // Adding non-updated value of the result
            if(aux < t[0]) ++t[1];   // In case t[1] + rrd->value >= 2^64
            t[0] = aux;

            printf("\nt[1,0] = [%lX, %lX]\n",t[1],t[0]); // Debugging purposes

            rrd->value = t[0]; // r[i] = t mod 2^64
            k = t[1];          // k = t / 2^64
            ad = ad->next;
            rrd = rrd->next;
            std::cout << "\nr = "; r.println();// Debugging purposes
        }
        rrd->value = k;// Leaving the last value of k in the next digit.
        rd = rd->next; // Next addition starts in the next digit of r.
        rrd = rd;      // Updating the 'runner' variable.
        bd = bd->next; // Next multiplication gonna be with next digit of b.
        ad = a.digits; // Resetting a.
        std::cout << "\nr = "; r.println();// Debugging purposes
    }
    if(r.last->value  == 0) r.pop();
    return r;
}

bool BigInteger::operator == (int x) const{
    if(this->digits == NULL) return false; // No comparison at all.
    if(this->digits->next != NULL && this->digits->next->value != 0)
    return false;// In this point, this is necessarily bigger than x

    bool positive = x > 0; // Signs are equal and values are equal.
    return positive == this->Positive && this->digits->value == (ui64)x;
}

std::ostream& operator << (std::ostream& s, BigInteger x) {
    if(x.digits == NULL) return s << "";
    char buffer[8];       // -Needed to safe the hexadecimal
                          //  digits of the number.
    // -buffer contents gets "unreachable" once out of the
    //  function, so we need to save the content in other place
    char b0, b1, b2, b3, b4, b5, b6, b7;
    BigInteger::Digit* r = x.digits;  // -Runner.
    i64 length = 1, i;
    while((r = r->next) != NULL) length++;
    while(length-- > 0) {
        r = x.digits; i = 0;
        // -Getting to the last unread digit.
        while(i++ < length) {r = r->next;}
        x.int64_to_8bytes(r->value, buffer);
        s << (b0 = buffer[0]) << (b1 =buffer[1]) <<  (b2 = buffer[2])
          << (b3 = buffer[3]) << (b4 = buffer[4]) << (b5 = buffer[5])
          << (b6 = buffer[6]) << (b7 = buffer[7]);
    }
    return s;
}

BigInteger BigInteger::operator - () {
    BigInteger r = *this;
    r.Positive = !this->Positive;
    return r;
}

void BigInteger::print() {
    if(this->digits == NULL) return ;
    char buffer[8];     // -Saves the hexadecimal digits of the number.
    ui08 uVal;          // -Unsigned value.
    Digit* r = this->digits;    // -Runs trough the digits.
    bool nonZeroFoun = false;   // -Tells us if a nonzero character is found.
    bool justZeros = true;      // -Tells us the first zero characters.

    i64 length = 1, i = 0;  // -Length of the number.
    while((r = r->next) != NULL) length++;

    if(this->Positive == false) printf("-");
    // -Printing the first element
    r = this->digits;
    while(++i < length) {r = r->next;}
    int64_to_8bytes(r->value, buffer);
    for(i = 0; i < 8; i++) {
        uVal = (ui08)buffer[i];
        nonZeroFoun = nonZeroFoun || (uVal > 0); // -False till uVal > 0.
        if(nonZeroFoun) {
            // nonZeroFound && justZeros == true means the first non zero
            // value was found.
            if(uVal < 16 && !justZeros) printf("0");
            printf("%X", uVal);
        }
        justZeros = justZeros && (uVal == 0); // -True till uVal > 0.
    } length--;

    while(length-- > 0) {
        r = this->digits; i = 0;
        // -Getting to the last unread digit.
        while(i++ < length) {r = r->next;}
        int64_to_8bytes(r->value, buffer);
        for(i = 0; i < 8; i++) {
            if((i & 7) == 0) printf(",");
            uVal = (ui08)buffer[i];
            if(uVal < 16) printf("0");
            printf("%X", uVal);
        }
    }
    if(!nonZeroFoun) printf("0");
}

void BigInteger::println() {
    print();
    printf("\n");
}

void BigInteger::setAsZero() {
    this->~BigInteger();
    digits = new Digit(0);
    last = digits;
}

void BigInteger::append(ui64 x) {
    if(digits == NULL) {
        digits = new Digit(x);
        last = digits;
        return;
    }
    last->next =  new Digit(x);
    last = last->next;
}

ui64 BigInteger::pop() {
    if(digits == NULL) return 0; // -This shold be handle by an exception.
    ui64 r = this->last->value;
    if(this->digits->next == NULL) {
        if(r != this->digits->value) {/*Some exception here*/}
        delete this->digits;
        this->digits = this->last = NULL;
        return r;
    }
    Digit* aux = this->digits;
    while(aux->next->next != NULL) aux = aux->next;
    delete this->last;
    this->last = aux;
    this->last->next = NULL;
    return r;
}

void BigInteger::push(ui64 x) {
    Digit* _digits = new Digit(x);
    _digits->next = this->digits;
    this->digits = _digits;
}

ui64 BigInteger::_8bytes_to_int64(const char bytes[8]) {
    ui64 r = 0; // -64 zero bits
    for(int i = 0; i < 8; i++) {
        r <<= 8; // Making room for the bits of the next byte
        // Allocating bits at the right end.
        r |= (ui64)(ui08)bytes[i];
    }
    return r;
}

void BigInteger::int64_to_8bytes(const ui64 n, char result[8]) {
    int i;
    for(i = 0; i < 8; i++)  // (7-i) * 8
        result[i] = char(n >> ((7-i) << 3));
}

