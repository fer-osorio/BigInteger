#include"BigInteger.hpp"

BigInteger::BigInteger() : digits(new Digit()), last(digits),
                           Positive(true) {}

BigInteger::BigInteger(i64 number) {
    if(number < 0) {
        Positive = false;
        number = -number;
    } else
        Positive = true;
    digits = new Digit((ui64)number);
    last = digits;
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
    int strlen = -1, validCharCount = 0, i = 0, value = -1;
    while(str[++strlen] != 0) {}
    if(strlen == 0 || str == NULL) {
        setAsZero();
        return;
    }
    if(str[0] == '-') { // -Determining the sign.
        Positive = false; i++;
    }
    while(i < strlen) { // Valid character for any of the number bases.
        switch(base) {
            case BINARY :
                if(str[i] == '0' || str[i] == '1') {

                }
                break;
            case QUATERNARY :
                if(str[i] > 47 && str[i] < 52 ) {

                }
                break;
            case OCTAL :
                if(str[i] > 47 && str[i] < 56 ) {

                }
                break;
            case HEXADECIMAL :
                if(str[i] > 47 && str[i] < 57 ) value = str[i] - 48;
                if(str[i] > 64 && str[i] < 91 ) value = str[i] - 55;
                if(str[i] > 96 && str[i] < 123) value = str[i] - 87;
                break;
            case DECIMAL :
                if(str[i] > 47 && str[i] < 57 ) value = str[i] - 48;
                break;
            default: ;
        }
        i++;
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
        aux = digits;
        digits = digits->next;
        delete aux;
    }
}

void BigInteger::setAsZero() {
    digits = new Digit(0);
    last = digits;
}

void BigInteger::append(ui64 x) {
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

