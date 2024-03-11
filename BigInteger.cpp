#include"BigInteger.hpp"

BigInteger::BigInteger(i64 number) {
    ui64Toui32 tmp; tmp.ui64int = 0;                                            // We'll divide 'number' in its upper 32 bits and lower 32 bits
    if(number < 0) {                                                            // Saving sign and changing to positive if necessary
        this->NonNegative = false;
        number = -number;
    } else
        this->NonNegative = true;
    if(number > (i64)ui32MAX) {                                                 // In this case, we'll need two Digit objects to store the number.
        tmp.ui64int = (ui64)number;
        this->first = new Digit(tmp.uint[0]);                                    // Saving lower 32 bits
        this->first->next = new Digit(tmp.uint[1]);                              // Saving upper 32 bits
        this->last = this->first->next;
    } else {
        this->first = new Digit((ui32)number);
        this->last = this->first;
    }
}

BigInteger::BigInteger(const ui32 array[], unsigned size, bool nonNegative)
    : NonNegative(nonNegative) {
    if(array == NULL || size == 0) {
        this->setAs(0);
        return;
    }
    unsigned i = 1;
    this->first = new Digit(array[0]);
    this->last = this->first;
    while(i < size) {
        this->last->next = new Digit(array[i++]);
        this->last = last->next;
    }
}

BigInteger::BigInteger(const char str[], NumberBase base) {
    int strlen = -1, validCharCount = 0, bitsOccupied = 0;
    int lowerLimit = 0;             // -0 if no negative sign, 1 in other case
    int value = 0;                  // -Holds number value of ascii character.
    bool validHexChar = false;
    if(base > 4) base = (NumberBase)3;
    while(str[++strlen] != 0) {}    // Calculating length
    if(strlen == 0 || str == NULL) {// Empty string or null pointer case
        this->setAs(0);
        return;
    }
    if(str[0] == '-') {             // -Determining the sign.
        NonNegative = false;
        lowerLimit = 1;
    }
    switch(base) {
        case BINARY :
            while(lowerLimit <= --strlen) {     // Reading the string backwards
                if(str[strlen] == '1' || str[strlen] == '0') {  // Valid character?
                    if(bitsOccupied == 32) {    // In this case we need a new digit
                        this->append(0);
                        bitsOccupied = 0;
                    }
                    this->last->value |= (ui32)(str[strlen] - 48) << bitsOccupied; // Allocating the value represented by the char in the current digit
                    bitsOccupied++;
                }
            }
            break;
        case QUATERNARY :
            while(lowerLimit <= --strlen) { // Here we'll use the fact that a quaternary digit can be represented with two bits
                if(str[strlen] > 47 && str[strlen] < 52 ) {
                    if(bitsOccupied == 0) {
                        this->append(0);
                        bitsOccupied = 0;
                    }
                    this->last->value |= (ui32)(str[strlen] - 48) << bitsOccupied;
                    validCharCount++;
                    bitsOccupied = (validCharCount << 1) & 31; // (validCharCount * 4) % 32
                }
            }
            break;
        case OCTAL :
            while(lowerLimit <= --strlen) {             // Here we'll use the fact that an octal digit can be represented with three bits
                if(str[strlen] > 47 && str[strlen] < 56 ) {
                    if(bitsOccupied == 31) {       // Next three bits won't fit in the current digit
                        this->last->value |= (ui32)(str[strlen] - 48) << 31;// Allocating first
                        this->append(0);                      // New digit
                        this->last->value |= (ui32)(str[strlen] - 48) >> 1; // Allocating next two
                        bitsOccupied = 2;
                    }
                    else if(bitsOccupied == 30) {   // Next three bits won't fit in the current digit
                        this->last->value |= (ui32)(str[strlen] - 48) << 30;// Allocating first two
                        this->append(0);            // New digit
                        this->last->value |= (ui32)(str[strlen] - 48) >> 2; // Allocating last one
                        bitsOccupied = 1;
                    }
                    else {
                        if(bitsOccupied == 0) {
                            this->append(0);
                            bitsOccupied = 0;
                        }
                        this->last->value |= (ui32)(str[strlen] - 48) << bitsOccupied;
                        bitsOccupied += 3;
                        bitsOccupied &= 31; // bitsOccupied &= 31
                    }
                }
            }
            break;
        case HEXADECIMAL :
            while(lowerLimit <= --strlen) {
                validHexChar = false;
                if(str[strlen] > 47 && str[strlen] < 57 ) {
                    value = str[strlen] - 48;
                    validHexChar = true;
                }
                if(str[strlen] > 64 && str[strlen] < 71 ) {
                    value = str[strlen] - 55;
                    validHexChar = true;
                }
                if(str[strlen] > 96 && str[strlen] < 103) {
                    value = str[strlen] - 87;
                    validHexChar = true;
                }
                if(validHexChar) {
                    if(bitsOccupied == 0) {
                        this->append(0);
                        bitsOccupied = 0;
                    }
                    this->last->value |= (ui32)value << bitsOccupied;
                    validCharCount++;
                    bitsOccupied = (validCharCount << 2) & 31; // (validCharCount * 4) % 32
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

BigInteger::BigInteger(const char bytes[], ui64 size, bool nonNegative)
    : NonNegative(nonNegative) {
    if(bytes == NULL || size == 0) {
        this->setAs(0);
        return;
    }
    ui64 q = size >> 2;                                                         // q = size / 4;
    ui64 r = size &  3;                                                         // r = size % 4;
    ui32Toui08 buffer; buffer.ui32int = 0;
    ui64 i = 0, j;

    if(q == 0) {
        while(i < r) {
            buffer.uchar[i] = (ui08)bytes[i]; i++;
        }
        this->first = new Digit(buffer.ui32int);
        this->last = this->first;
        return;
    }
    for(i = 0; i < q; i += 4) {
        for(j = 0; j < 4 ; j++) buffer.uchar[j] = (ui08)bytes[i+j];
        this->append(buffer.ui32int);
    }
    if(r > 0) {
        buffer.ui32int = 0;
        for(j = 0; j < r ; j++) buffer.uchar[j] = (ui08)bytes[i+j];
        this->append(buffer.ui32int);
    }
}

BigInteger& BigInteger::operator = (const BigInteger& a) {
    if(this != &a) {                                                            // Guarding against self assignment.
        this->clean();                                                          // Releasing memory
        this->NonNegative = a.NonNegative;
        Digit *aux = a.first;
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

// |||||| Algorithm for the addition of two non negative numbers in base k |||||||

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
// ---------------------------------------------------------------------------

void BigInteger::addNonnegative(const BigInteger& x, BigInteger& result) const {
	if(result.first != NULL) result.clean();                                     // "Cleaning" the container of the result
	if(this->first == NULL || x.first == NULL) {
	    throw "\nBigInteger.cpp, in function void BigInteger::addNonnegative"
	    "(const BigInteger& x, BigInteger& result); either this->first == NULL "
	    "or x.first == NULL\n";
	}
	if(*this == 0) { result = x; return; }                                      // 0 + b = 0
	if(x == 0) { result = *this; return; }                                      // a + 0 = a
    ui64Toui32 tmp;
    ui64 carriage = 0;

    BigInteger::Digit *dt = this->first, *dx = x.first, *largest = NULL;
    while(dt != NULL && dx != NULL) {
        tmp.ui64int = (ui64)dt->value + (ui64)dx->value + carriage;
        result.append(tmp.uint[0]);
        carriage = tmp.uint[1];
        dt = dt->next;
        dx = dx->next;
    }
    if(dt != NULL) largest = dt;                                                // In case of having to numbers of different amount of digits.
    else largest = dx;
    while(largest != NULL) {
        if((carriage == 1) && (largest->value == ui32MAX))
            result.append(0);                                                   // Preserve carriage as 1.
        else {
            result.append(largest->value + carriage);
            carriage = 0;
        }
        largest = largest->next;
    }
    if(carriage == 1) result.append(1);
    return;
}

BigInteger operator + (const BigInteger& a, const BigInteger& b) {
    BigInteger r(true,0);
    if(a.NonNegative == false && b.NonNegative == false) {                      // Case 1: Operands are negative.
        a.addNonnegative(b, r);                                                 // -a + (-b) = -(a + b)
        r.NonNegative = false;
        return r;
    }
    if(a.NonNegative == true && b.NonNegative == false) {                       // Case 2: First non negative, second negative.
        a.subtractNonnegative(b, r);                                            // a + (-b) = a - b
        return r;
    }
    if(a.NonNegative == false && b.NonNegative == true) {                       // Case 2: First negative, second non negative.
        b.subtractNonnegative(a,r);                                             // -a + b = b - a
        return r;
    }
    a.addNonnegative(b, r);                                                     // Case 4: Operands are non negative.
    return r;
}

// |||| Algorithm for the subtraction of two non negative numbers in base k ||||||

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

void BigInteger::subtractNonnegative(const BigInteger &x, BigInteger &result)
const {
    if(result.first != NULL) result.clean();
	if(this->first == NULL || x.first == NULL) {
	    throw "\nBigInteger.cpp, in function void BigInteger::addNonnegative"
	    "(const BigInteger& x, BigInteger& result); either this->first == NULL "
	    "or x.first == NULL\n";
	}
	if(*this == 0) {result=x; result.NonNegative=!result.NonNegative; return;}  // 0 - b = 0
	if(x == 0) { result = *this; return; }                                      // a - 0 = a

	ui64 load = 0;
	bool thisIsBigger = false;
	BigInteger::Digit *d0 = this->first, *d1 = x.first;                           // Digits of 'this' and digits of x.

	while(d0 != NULL && d1 != NULL) {                                           // Determining the biggest number. Remember: Numbers are non negative
        if(d0->value >= d1->value) thisIsBigger = true;                         // Not using the 'compare' function in order to get a little optimization
        else thisIsBigger = false;                                              // To understand better the algorithm behind this code, go to the definition of
        d0 = d0->next;                                                          // 'compare' function.
        d1 = d1->next;
	}
	if(d0 != NULL) thisIsBigger = true;                                         // this BigInteger has more digits than x (leading digit must be non zero)
	if(thisIsBigger) {
	    result.NonNegative = true; d0 = this->first; d1 = x.first;                // Result is positive. Now d0 points to the digits of the biggest number
	} else {
	    result.NonNegative = false;d0 = x.first; d1 = this->first;                // Result is negative, Now d0 points to the digits of the biggest number
	}
    while(d0 != NULL && d1 != NULL) {
        if((d0->value < d1->value) || (d0->value == d1->value && load == 1)) {  // We need to "ask for a loan". k = (k-1) + 1; in this case 2^32 = (2^32-1) + 1.
            result.append((ui32MAX-d1->value+1-load)+d0->value);                // Using (a_{i} - b_{i} - load) + 2^32 == ((2^32-1) - b_{i} + 1 - load) + a_{i};
            load = 1;
        } else {
            result.append(d0->value - d1->value - load);
            load = 0;
        }
        d0 = d0->next;
        d1 = d1->next;
    }
    if(d0 != NULL) {                                                            // In case of still having digits in the biggest number
        while(d0->next != NULL) {
            if(d0->value == 0 && load == 1) {                                   // We have a zero digit and a non zero load, asking for another loan
                result.append(ui32MAX);
                load = 1;
            } else {                                                            // Found non zero digit, taking the loan from it.
                if(load == 1) {
                    result.append(d0->value - 1);
                    load = 0;
                }
                else result.append(d0->value);                                  // If no loan, we just deposit  the digit.
            }
            d0 = d0->next;
        }
        if(d0->value > 1 || load == 0) result.append(d0->value - load);         // Determining the value of the last digit.
    }
    while(result.last != result.first && result.last->value == 0) result.pop();  // Erasing the leftmost zeros.
    if(result == 0) result.NonNegative = true;
    return;
}

BigInteger operator - (const BigInteger& a, const BigInteger& b) {
    BigInteger r(true,0);                                                       // BigInteger object with empty list of digits

    if(a.NonNegative == false && b.NonNegative == false) {                      // Case 1: Operands are negative.
        b.subtractNonnegative(a, r);                                            // -a - (-b) = -a + b = b - a
        return r;
    }
    if(a.NonNegative == true && b.NonNegative == false) {                       // Case 2: First non negative, second negative.
        a.addNonnegative(b, r);                                                 // a - (-b) = a + b
        return r;
    }
    if(a.NonNegative == false && b.NonNegative == true) {                       // Case 3: First negative, second non negative.
        a.addNonnegative(b, r); r.NonNegative = false;                          // -a - b = -(a + b)
        return r;
    }
    a.subtractNonnegative(b, r);                                                // Case 4: Operands are non negative.
    return r;
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

void ui64Product(ui64 a, ui64 b, ui64 result[2]) {
    ui64Toui32 _a_, _b_, _l64X_;
    _a_.ui64int = a; // ua == _a_.uint[1], la == _a_.uint[0]
    _b_.ui64int = b; // ub == _a_.uint[1], lb == _a_.uint[0]

    // printf("\na = %lX, b = %lX", a, b); //debuggin purposes

    ui64 lalb = (ui64)_a_.uint[0] * _b_.uint[0];//printf("\nlalb = %lX",lalb);
    ui64 laub = (ui64)_a_.uint[0] * _b_.uint[1];//printf("\nlaub = %lX",laub);
    ui64 ualb = (ui64)_a_.uint[1] * _b_.uint[0];//printf("\nualb = %lX",ualb);
    ui64 uaub = (ui64)_a_.uint[1] * _b_.uint[1];//printf("\nuaub = %lX",uaub);

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
    BigInteger::Digit *ad = a.first, *bd = b.first;
    BigInteger r(true,0); // Initializing with NULL list of digits.
    r.NonNegative = (a.NonNegative && b.NonNegative) || !(a.NonNegative || b.NonNegative);

    if(a.first == NULL || b.first == NULL) {return r;}  // exception here
    if(a == 0 || b == 0) return BigInteger();         // Returns Zero

    ui64 k = 0;
    ui64Toui32 t;
    while(ad != NULL) { // -Initializing the result.
        // -Supposing a = a[n-1]...a[0] and b = b[m-1]...b[0] (numbers of
        //  'n' and 'm' digits, respectively), then, in this loop, we are
        //  initializing the first 'n' digits. with b[0] * a[i].
        t.ui64int = (ui64)ad->value * (ui64)b.first->value + k;

        r.append(t.uint[0]); // r[i] = t mod 2^32
        k  = t.uint[1];       // k = t / 2^32
        ad = ad->next;

        //std::cout << "\nr = "; r.printHexln();// Debugging purposes
    }
    r.append((ui32)k); bd = bd->next;
    while(bd != NULL) { // -Initializing the next 'm' digits with zeros.
        r.append(0);
        bd = bd->next;
    }

    //std::cout << "\nr = "; r.printHexln();// Debugging purposes

    ad = a.first;
    bd = b.first->next; // In this line we know that b.digits != NULL
    // -Since we now have a * b[0], r.digits it's not NULL and the algorithm demand to start the sum in the next digit of r, this is r[1].
    // 'rrd' can be interpreted as "running over result digits"
    BigInteger::Digit *rd = r.first->next, *rrd = rd;
    while(bd != NULL) {
        k = 0;
        while(ad != NULL) {
            t.ui64int = (ui64)ad->value*(ui64)bd->value + (ui64)rrd->value + k;

            rrd->value = t.uint[0]; // r[i] = t mod 2^32
            k = t.uint[1];          // k = t / 2^32
            ad = ad->next;
            rrd = rrd->next;

            //std::cout << "\nr = "; r.printHexln();// Debugging purposes
        }
        rrd->value = k;// Leaving the last value of k in the next digit.
        rd = rd->next; // Next addition starts in the next digit of r.
        rrd = rd;      // Updating the 'runner' variable.
        bd = bd->next; // Next multiplication gonna be with next digit of b.
        ad = a.first;   // Resetting a.
        //std::cout << "\nr = "; r.printHexln();// Debugging purposes
    }
    if(r.last->value  == 0) r.pop();
    return r;
}

void BigInteger::shortDivisionNonnegative(ui32 divisor, BigInteger result[2])
const{
	if(result[0].first != NULL) result[0].clean();                               // -Before start, we "clean" what is inside the 'result' array.
	if(result[1].first != NULL) result[1].clean();
	if(divisor  == 0) {
	    throw "Exception in BigInteger.cpp, function void shortDivisionNonnegat"
        "ive(const BigInteger& dividend, const ui32 divisor,BigInteger result[2"
        "]): Division by zero not defined.";
	}
	if(*this == 0) { result[0] = 0; result[1] = 0; return; }                    // 0 = 0·divisor + 0

	Digit* pd;
	BigInteger dividendInverted(true,0);                                        // BigInteger with empty (NULL) digits list.
	for(pd = this->first ; pd != NULL; pd = pd->next) {                          // Inverting the order of the digits.
	    dividendInverted.push(pd->value);
	}
	ui64 r = 0;                                                                 // We'll save the remainder here
	ui64Toui32 t;                                                               // Intended for the 'cast' of a 64-bits integer to an array of two 32-bits integer
	for(pd = dividendInverted.first; pd != NULL; pd = pd->next) {
        t.uint[1] = r; t.uint[0] = pd->value;                                   // t.ui64int = r*2^32 + pd->value
        result[0].push((ui32)(t.ui64int/(ui64)divisor));
        r = t.ui64int % (ui64)divisor;
	}
	while(result[0].last->value == 0 && result[0].first != result[0].last)       // Deleting left zeros (unless quotient == 0)
	    result[0].pop();
	result[1] = BigInteger((i64)r);
}

//  Quotient remainder sign

// -Let a,b be integers, a,b != 0. From euclidean division theorem a = q[a]·b + r where 0 <= r < |b|. Let a,b be positive and suppose r > 0.
// -First  case: -a = (-q[a])·b - r = (-q[a])·b - b + b - r = (-(q[a]+1))·b + (b-r); Since 0 < r < b, then 0 < b-r < b implying -a/b = -(a/b+1) y -a%b = b - a%b.
// -Second case:  a = q[a]·b + r = q[a]·-(-b) + r = (-q[a])·(-b) + r, therefore a/-b = -(a/b) & a%-b = a%b
// -Third  case: -a = -(q[a]·b + r)  = q[a]·(-b) - r = (q[a]+1)·(-b) + (b-r); reasoning similarly to the first case .we have -a/-b = (a/b)+1 and -a%-b = b - a%b.

void BigInteger::shortDivision(ui32 divisor, BigInteger result[2]) const{
    if(divisor == 0) { /*Exception here*/ return;}
    this->shortDivisionNonnegative(divisor, result);                            // Computing the [quotient,reminder] as if the arguments were both non negative
    if(this->NonNegative == false) {                                            // Dividend is negative
        if(result[1] != 0) {                                                    // The remainder is not zero, then...
            ++result[0];                                                        // Using -a/b = -(a/b + 1)
            result[1].NonNegative = false;                                      // Using -a%b = b - a%b.
            result[1].plusEqualNonNegative(divisor);
        }
        if(result[0] != 0) result[0].NonNegative = false;                       // Changing sign of quotient if and only if the quotient is not zero.
    }
}

void BigInteger::shortDivision(int divisor, BigInteger result[2]) const{
    if(divisor == 0) { /*Exception here*/ return;}
    bool divisorPositive;
    if(divisor > 0) divisorPositive = true;                                     // Saving the sign and changing to positive if necessary
    else {
        divisorPositive = false;
        divisor = -divisor;
    }
    this->shortDivisionNonnegative((ui32)divisor, result);                      // Computing the [quotient,reminder] as if the arguments were both positive
    if(this->NonNegative == false) {                                            // Numerator is negative
        if(result[1] != 0) {                                                    // The remainder is not zero, then...
            ++result[0];                                                        // Using -a/b = -(a/b + 1) and -a/-b = (a/b) + 1
            result[1].NonNegative = false;                                      // Using -a%b = b - a%b and -a%-b = b - a%b.
            result[1] += divisor;
        }
        if(divisorPositive && result[0]!=0) result[0].NonNegative = false;      // Changing sign of quotient if and only if divisor is positive and quotient is not
        return;                                                                 // zero.
    }
    if(this->NonNegative && !divisorPositive) {                                 // Dividend is positive and divisor is negative.
        result[0].NonNegative = false;                                          // a/-b = -(a/b) & a%-b = a%b
    }
}

void BigInteger::divisionNonnegative(const BigInteger &divisor, BigInteger
result[2]) const {
    /*  The division algorithm implemented here can be found in the book "(Donald Knuth) The art of computer programming, volume 2. Seminumerical Algorithms. Third
        Edition", in pages 272 and 273.*/
    if(result[0].first != NULL) result[0].clean();                               // -Before start, we "clean" what is inside the 'result' array.
	if(result[1].first != NULL) result[1].clean();
	if(divisor  == 0) {
	    throw "Exception in BigInteger.cpp, function void shortDivisionNonnegat"
        "ive(const BigInteger& dividend, const ui32 divisor,BigInteger result[2"
        "]): Division by zero not defined.";
	}
	if(*this == 0) { result[0] = 0; result[1] = 0; return; }                    // 0 = 0·divisor + 0
    if(divisor.isSinglePrecision()) {                                           // Division by a single precision number (32-bits integer), using short division
        this->shortDivisionNonnegative(divisor.first->value, result);            // for non negative numbers.
        return;
    }                                                                           // At this point of the code we know the divisor is not a single precision number
    if(this->operator<(divisor)) {                                              // If u < v, then u = 0·v + u, therefore int(u/v) = 0 and u mod v = u
        result[0] = 0; result[1] = *this;
        return;
    }

    ui32 _2LeadDigitsDvsor[2]  = {0,0};                                         // Two leading digits of the divisor
    ui32 _3LeadDigitsDvdend[3] = {0,0,0};                                       // Three leading digits of the dividend
    ui32 leftShif = 0;                                                          // Left shift for the virtual normalization
    const ui32 lenDvdend = this->len();                                         // Length of dividend list of digits
    const ui32 lenDvsor = divisor.len();                                        // Length of divisor list of digits
    ui32 lenDiff = lenDvdend - lenDvsor;                                         // Difference between the list of the dividend and the list of the divisor
    ui64 rCap = 0;
    ui64Toui32 qCap = {0}, aux0 = {0};                                          // Trial quotient and trial remainder for each single division cycle
    bool newLeadDigit;                                                          // Flags the introduction of a new digit caused by the normalization process
    ui32 aux1 = 0;                                                              // Auxiliary variable. Will help in small optimizations.
    ui64 aux2 = 0;                                                              // Auxiliary variable. Will help in small optimizations.

    if(lenDvdend == 2) {
        qCap.uint[0] = this->first->value;                                       // Converting from two unsigned int to one unsigned int of 64 bits in each case
        qCap.uint[1] = this->first->next->value;                                 // ...
        aux0.uint[0] = divisor.first->value;                                     // In this case aux0 is just an auxiliary variable
        aux0.uint[1] = divisor.first->next->value;                               // ...
        result[0] = qCap.ui64int / aux0.ui64int;                                // Using 64 bits operation to get the results
        result[1] = qCap.ui64int % aux0.ui64int;                                // ...
        return;
    }                                                                           // At this line we know we have at least three precision places in the dividend

    _2LeadDigitsDvsor[1] = divisor[lenDvsor - 1];                               // Initializing with most significant digit of the divisor
    while(_2LeadDigitsDvsor[1] < _2power31) {                                   // Start of the virtual normalization process for the divisor
        _2LeadDigitsDvsor[1] <<= 1;
        ++leftShif;
    }
    aux1 = divisor[lenDvsor - 2];
    _2LeadDigitsDvsor[1] |= aux1 >> (ui32wordlen - leftShif);                   // Shifting to the left the first two digits
    _2LeadDigitsDvsor[0]  = aux1 << leftShif;
    if(lenDvsor > 2)                                                            // If the divisor has at least three digits, integrate that digit to the shifting
        _2LeadDigitsDvsor[0] |= divisor[lenDvsor-3]>>(ui32wordlen-leftShif);    // End of the virtual normalization process for the divisor

    result[1] = *this;                                                          // Initializing remainder with the value of the dividend
    aux1 = result[1][lenDvsor - 1] >> (ui32wordlen - leftShif);
    newLeadDigit = aux1 > 0;                                                    // Testing if the normalization process creates a new digit in the dividend
    for(; lenDiff != ui32MAX; lenDiff--) {                                      // We are using unsigned int, so lenDiff != ui32MAX is equivalent to lenDiff != -1
        if(newLeadDigit) {                                                      // Virtual normalization introduced a new digit
            _3LeadDigitsDvdend[2]  = aux1;                                      // Obtaining the first three digits of a (virtually) normalized dividend
            _3LeadDigitsDvdend[1]  = result[1][lenDvsor - 1] << leftShif;       // ...
            aux1 = result[1][lenDvsor - 2];                                     // ...
            _3LeadDigitsDvdend[1] |= aux1 >> (ui32wordlen - leftShif);          // ...
            _3LeadDigitsDvdend[0] = aux1 << leftShif;                           // ...
            _3LeadDigitsDvdend[0] |=                                            // ...
                result[1][lenDvsor - 3] >> (ui32wordlen - leftShif);            // ...
            newLeadDigit = false;                                               // This will prevent the entrance to this 'if' in any future cycle
            lenDiff++;                                                          // Adding the new digit to the difference
        } else {                                                                // We didn't have a new digit or the new digit was added in a past cycle
            _3LeadDigitsDvdend[2] = result[1][lenDvsor+lenDiff-1] << leftShif;  // Obtaining the virtual normalized three digits from the leading four digits of
            aux1 = result[1][lenDvsor + lenDiff - 2];                           // the dividend (accordingly to the respective cycle.)
            _3LeadDigitsDvdend[2] |= aux1 >> (ui32wordlen - leftShif);          // ...
            _3LeadDigitsDvdend[1] = aux1 << leftShif;                           // ...
            aux1 = result[1][lenDvsor + lenDiff - 3];                           // ...
            _3LeadDigitsDvdend[1] |= aux1 >> (ui32wordlen - leftShif);          // ...
            _3LeadDigitsDvdend[0] = aux1 << leftShif;                           // ...
            if(lenDvdend > 3) {                                                 // Guarding against a number with just three precision places
                _3LeadDigitsDvdend[0] |=
                result[1][lenDvsor+lenDiff-4] >> (ui32wordlen - leftShif);
            }
        }
        qCap.uint[1] = _3LeadDigitsDvdend[2];                                   // This two lines are equivalent to
        qCap.uint[0] = _3LeadDigitsDvdend[1];                                   // qCap = _3LeadDigitsDvdend[2]*2^32 + _3LeadDigitsDvdend[1]
        rCap = qCap.ui64int %_2LeadDigitsDvsor[1];
        aux0.uint[1] = (ui32)rCap; aux0.uint[0] = _3LeadDigitsDvdend[0];        // Equivalent to aux0 = rCap * 2^32 + _3LeadDigitsDvdend[0]
        qCap.ui64int /= _2LeadDigitsDvsor[1];
        aux2 = qCap.ui64int*_2LeadDigitsDvsor[0];
        while(qCap.ui64int >= _2power32 || aux2 > aux0.ui64int) {               // Condition to eliminate most of the cases where qCap is one too big and all cases
            --qCap.ui64int; rCap += _2LeadDigitsDvsor[1];                       // where qCap is two too big
            if(rCap >= _2power32) break;
            aux0.uint[1] = (ui32)rCap; aux0.uint[0] = _3LeadDigitsDvdend[0];    // Equivalent to aux0 = rCap * 2^32 + _3LeadDigitsDvdend[0]
            aux2 = qCap.ui64int*_2LeadDigitsDvsor[0];
        }
    }
}

int BigInteger::compare(const BigInteger &x) const {                            // We are assuming that the sing of a zero BigInteger is non negative.

    /*  Let u = u[n]b^n + u[n-1]b^{n-1} + ... + u[0] and v = v[n]b^n + v[n-1]b^{n-1} + ... + v[0] be non negative numbers represented in base b, let i be an
        integer in the range 0 <= i <= n, then we have that u[i] < v[i] implies
        u[i]b^{i} + ... + u[0]  <= (v[i] - 1)b^{i} + (u[i-1]b^{i-1} +...+ u[0])
                                <  (v[i] - 1)b^{i} + v[i]b^{i}                  , using x^n > y[n-1]·x^{n-1} +...+ y[1]·x + y[0] for integers x>0, 0 <= y[i] < x
                                <= v[i]b^{i} + v[i-1]b^{i-1} + ... + v[0].
        In resume u[i] < v[i] implies u[i]b^{i} + ... + u[0] < v[i]b^{i} + ... + v[0].
        So, in order to compare u and v, we'll first look at the signs, then at the number of digits and lastly the comparison of each pair of digits   */

    Digit *td = this->first, *xd = x.first;                                       // This variables will run through 'this' digits and 'x' digits
    int  result = 0;                                                            // -1 for this < x, 0 for this == x and 1 for this > x
    bool bothNegative = false;                                                  // If both signs are negative we'll have to invert the result

    if(!this->NonNegative) {                                                    // Checking signs
        if(x.NonNegative) return -1;                                            // 'this' is negative and x is non negative, therefore this < x. Returning -1
        bothNegative = true;                                                    // At this point, both are negative.
    } else if(!x.NonNegative) return 1;                                         // 'this' is non negative and x is negative, therefore this > x. Returning  1

    for(;td != NULL; td = td->next, xd = xd->next) {                            // Comparing digit by digit from the least significant to the most significant
        if(xd == NULL) {
            if(bothNegative) return -1;                                         // x has fewer digits than 'this', this > x if both non negative and this < x if both
            else             return  1;                                         // negative
        }
        if(td->value != xd->value) {                                            // If digits are equal, don't change the value of result
            if(td->value < xd->value) result = -1;                              // Inside this 'If', in this particular cycle, this < x holds true
            else                      result =  1;                              // Inside this 'If', in this particular cycle, this > x holds true
        }
    }
    if(xd != NULL) {
            if(bothNegative) return  1;                                         // x has more digits than 'this', this < x if both non negative and this > x if both
            else             return -1;                                         // negative
    }
    return result;
}

BigInteger& BigInteger::operator++() {
    Digit *td = this->first;                                                     // this digits

    if(this->NonNegative == true) {
        while(td != NULL && td->value == ui32MAX) {                             // Moving the carriage to the next digit
            td->value = 0;
            td = td->next;
        }
        if(td != NULL) ++td->value;                                             // Finally we add the carriage
        else this->append(1);
    }
    else {
        if(*this == 0)  { this->setAs(1); return *this;}                        // Cases where the zero is involved.
        if(*this == -1) { this->setAs(0); return *this;}
        while(td != NULL && td->value == 0) {                                   // Moving the loan to the next digit
            td->value = ui32MAX;
            td = td->next;
        }
        if(td != NULL) {
            --td->value;                                                        // Finally we add the carriage
            if(td == this->last && td->value == 0) this->pop();
        }
        else {/*Exception here: The most significant digit can't be zero*/}
    }
    return *this;
}

void BigInteger::plusEqualNonNegative(ui32 x) {                                 // Assuming this BigInteger is non negative.
    Digit* td = this->first;                                                     // This digits
    ui64Toui32 t = {0};
    t.ui64int = td->value + x;                                                  // Adding to the first element
    td->value = t.uint[0];                                                      // first element = (td->value + v) % 2^32
    if(t.uint[1] > 0) {                                                         // We have a non zero carriage (carriage = 1)
        for(td = td->next; td != NULL && td->value == ui32MAX; td = td->next) { // While current Digit value is ui32MAX we'll change that value to zero and
            td->value = 0;                                                      // preserve the carriage
        }
        if(td == NULL) this->append(1);                                         // Depositing the carriage in a new digit
        else ++td->value;                                                       // Adding the carriage to a digit which value is not 2^32-1
    }
}

void BigInteger::minusEqualNonNegative(ui32 x) {                                // Assuming this BigInteger is non negative.
    Digit *td = this->first;                                                     // This digits
    if(x > td->value) {                                                         // Here we need a loan from the next digit.
        if(td->next == NULL) {                                                  // In case of single precision number.
            td->value = x - td->value;
            this->NonNegative = !this->NonNegative;
            return;
        }
        //td->value = ui32MAX - (ui32)v + td->value + 1;
        td->value += ui32MAX - x;                                               // Adding to the first digit and taking a loan
        td->value++;
        for(td = td->next; td != NULL && td->value == 0; td = td->next) {       // If the next digit is zero, we'll need to take the loan from the next digit to
            td->value = ui32MAX;                                                // that one
        }
        if(td != NULL) {
            --td->value;
            if(td == this->last && td->value == 0) this->pop();
        } else {/*Exception here: The most significant digit can't be zero*/}
    } else {
        td->value -= x;
    }
}

void BigInteger::printHex() const {
    if(this->first == NULL) {
        std::cout << "\nList of digits is a NULL pointer...\n";
        /*Some exception here.*/
        return;
    }
    Digit* r = this->first;                                  // Runs trough the digits.
    ui32Toui08 buffer;                                       // We'll print byte per byte

    i64 length = 1, i = 0;
    if(r->next != NULL) {                                   // In case of having more than one Digit object in the list
        for(length=2,r=r->next; r->next!=NULL; r=r->next)   // At least we have two Digits. Calculating total length
            length++;
    }                                                       // At this point 'r' should point to the last element.
    if(r != this->last) {
        std::cout << "\nException in file BigInteger.cpp, inside function void BigInteger::printHex() const. Last element of the Digit list don't "
                              "coincide with the object pointed by 'this->last'.\n";
        printf("\n[r,this->last] = [%lX, %lX]\n", (ui64)r, (ui64)this->last);
        printf("\nr->value == %X and this->last->value == %X\n", r->value, this->last->value);
    }
    if(!this->NonNegative) printf("-");                        // Negative number
    buffer.ui32int = r->value;                               // From ui32 to ui08[4]
    for(i = 3; i > 0 && buffer.uchar[i] == 0; i--) {}        // Printing first list element. Ignoring left zeros
    if(i == 0) {                                            // In case of having 3 zero bytes
        printf("%X",buffer.uchar[i]);
    } else {
        printf("%X",buffer.uchar[i]);
        for(i--; i >= 0; i--) {                             // Printing non-zero bytes
            if(buffer.uchar[i] < 10) printf("0");
            printf("%X",buffer.uchar[i]);
        }
    }
    --length;

    while(0 < length--) {
        printf(",");
        r = this->first; i = 0;                                                  // -Getting to the last unread digit.
        while(i++ < length) {r = r->next;}
        buffer.ui32int = r->value;
        for(i = 3; i >= 0; i--) {                                               // Printing non-zero bytes
            if(buffer.uchar[i] < 10) printf("0");
            printf("%X",buffer.uchar[i]);
        }
    }
}

void BigInteger::printHexln() const {
    printHex();
    printf("\n");
}

