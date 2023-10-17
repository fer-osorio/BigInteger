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

BigInteger::BigInteger(const BigInteger& a) : Positive(a.Positive) {
    Digit *aux = a.digits;
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

// Some facts for operation in the ring of integers:
// ·[0] (Euclidean division theorem) Given a pair of integers a,b, exist
//      unique integers q,r such that a = q·b + r. Using notation for the
//      operations in C++ we can write a = (a/b)*b + a%b.
// ·[1] (x+y)%n = (x%n + y)%n = (x + y%n)%n = (x%n + y%n)%n
// ·[2] (x+y)/n = x/n + y/n + (x%n + y%n)/n
// ·[3] x/2^n = x>>n and x*2^n = x<<n
// ·[4] x%2^n = x&(2^n-1)

// -Set 'k' as any natural number bigger than 1
// -Representation of a and b in digits base b:
//  a == a_{n}a_{n-1}...a_{1}a_{0} and b == b_{m}b_{n-1}...b_{1}b_{0}, where
//  +) 0 <= a_i,b_j <= k-1 for each i\in{1,...,m} and j\in{1,...,m}

//  Set carriage_{0} = 0 and s = max(m,n), call r = a + b, then:
//  -r == r_{s+1}r_{s}...r_{1}r_{0} where
//   1) r_{0} = (a_{0} + b_{0})%k and carriage_{0} = (a_{0} + b_{0})/k
//   2) r_{i} = (a_{i} + b_{i} + carriage_{i-1})%k
//      carriage_{i} = (a_{i}+b_{i}+carriage_{i-1})/k for each i\in{0,...,s}.
//   3) r_{s+1} = carriage_{s}
// -From (1), (2) and (+) we can see that 0 <= carriage_{i} <= 1.

// -Setting k = 2^64 we have
//  i) 0 <= a_{i}+b_{i}+carriage_{i} <= 2^65 - 1.
// -As a result of the last inequality, we'll need 65 bits to hold the result
//  of that sum for arbitrary arguments. Our objective is to determine, using
//  operations in 64 bits, each of the r_{i}'s and each the carriage_{i}'s.

// -From equities 1),2) and inequality i) it follows that
//  I)  ra_{i} is exactly the first 64 bits.
//  II) carriage_{i} is the value of the last bit (left most bit).

// -Rewrite a_{i} as a_{i} = la_{i}^63 + ra_{i}, where
//  la_{i} is its left most bit and ra_{i} if what remains, do the same
//  with b_{i}, then
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
//  x := ra_{i} + rb_{i} + carriage_{i-1}. From [1]
//  x = (x/2^63)·2^63 + (x%2^63) [left most bit, last 62 bits]
// -So:
//  r_{i} = [2^63 + (x/2^63)·2^63 + (x%2^63)] % 2^64
//        = [((1 + [x/2^63])·2^63) % 2^64 + [x%2^63] ] % 2^64
//  First bit of x~~~~^   Last 63 bits of x~~~~^
// -Therefore, the expression ((1 + [x/2^63])·2^63) % 2^64 is zero if and
//  only if the First bit of x (from left to right) is one. In short
//  <1> If the first bit of x is 1, then
//      r_{i} = (ra_{i} + rb_{i} + carriage_{i-1})%2^63
//  <2> If the first bit of x is 0, then
//      r_{i} = 2^63 + ra_{i} + rb_{i} + carriage_{i-1}

// -Now we'll calculate the carriage_{i}'s. From 2), ·), [2] and using the
//  same substitution with x:
//  carriage_{i} =
//      [(la_{i} + lb_{i})·2^63 + (ra_{i} +rb_{i} + carriage_{i-1})]/2^64 =
//      [(la_{i} + lb_{i})·2^63 + x]/2^64 =
//_X_:= [(la_{i} + lb_{i})·2^63]/2^64 + x/2^64 +
//      ([(la_{i} + lb_{i})·2^63]%2^64 + x%2^64)/2^64
// -Case 1: la_{i} = lb_{i} = 1, then _X_= 1 + 0 + (0 + x)/2^64 = 1
// -Case 2: la_{i} = lb_{i} = 0, then _X_= 0 + 0 + (0 + x)/2^64 = 0
// -Case 3: la_{i} != lb_{i}, then _X_= 0 + 0 + (2^63 + x)/2^64.
//  2^63 + x >= 2^64 if and only if x >= 2^63 if and only if the first
//  bit of x is 1, then _X_= 1 if x >= 2^63, _X_ = 0 otherwise.

BigInteger operator + (const BigInteger& a, const BigInteger& b) {
    BigInteger r;
    if(a.digits == NULL || b.digits == NULL) return r; // Nothing to do

    BigInteger::Digit *da = a.digits, *db = b.digits;
    while(da != NULL && db != NULL) {

        da = da->next;
        db = db->next;
    }
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

void BigInteger::print() {
    if(digits == NULL) return ;
    char buffer[8];       // -Needed to safe the hexadecimal
                          //  digits of the number.
    BigInteger::Digit* r = digits;  // -Runner.
    i64 length = 1, i;
    while((r = r->next) != NULL) length++;
    while(length-- > 0) {
        r = digits; i = 0;
        // -Getting to the last unread digit.
        while(i++ < length) {r = r->next;}
        int64_to_8bytes(r->value, buffer);
        for(int i = 0; i < 8; i++) {
            if((ui08)buffer[i] < 16) printf("0");
            printf("%X", (ui08)buffer[i]);
        }
    }
}

void BigInteger::println() {
    print();
    printf("\n");
}

void BigInteger::setAsZero() {
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

void BigInteger::push(ui64 x) {
    Digit* _digits = new Digit(x);
    _digits->next = digits;
    digits = _digits;
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

