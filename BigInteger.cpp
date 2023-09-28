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

BigInteger::BigInteger(ui64 array[], unsigned size, bool positive)
    : Positive(positive) {
    if(array == NULL || size == 0) {
        digits = new Digit(0);
        last = digits;
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

BigInteger::BigInteger(const char str[]) {

}

BigInteger::BigInteger(const char bytes[], bool positive) {

}

BigInteger::~BigInteger() {
    Digit* aux;
    while(digits != NULL) {
        aux = digits;
        digits = digits->next;
        delete aux;
    }
}

void BigInteger::append(ui64 x) {
    last->next =  new Digit(x);
    last = last->next;
}

ui64 BigInteger::_8bytes_to_int64(const char bytes[8]) {
    ui64 r = 0; // -64 zero bits
    for(int i = 0; i < 8; i++) {
        r <<= 8; // Making room for the bits of the next byte
        // Allocating bits at the right end.
        r |= ui64((unsigned char)bytes[i]);
    }
    return r;
}

void BigInteger::int64_to_8bytes(const ui64 n, char result[8]) {
    int i;
    for(i = 0; i < 8; i++)  // (7-i) * 8
        result[i] = char(n >> ((7-i) << 3));
}

