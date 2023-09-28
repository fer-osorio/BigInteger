// -Handling numbers of an arbitrary size. The limit is the size of the RAM
//  available.

#ifndef _BIGINTEGER_INCLUDED_
#define _BIGINTEGER_INCLUDED_

#include<cstdint>
#include<iostream>

typedef std::uint64_t ui64;
typedef std::int64_t   i64;

struct BigInteger {
	private: struct Digit {
		ui64 value;
		Digit* next;
		Digit() : value(0), next(NULL) {}
		Digit(ui64 _value) : value(_value), next(NULL) {}
	}*digits, *last;

	bool Positive;

	public:
	BigInteger();
	BigInteger(i64 number);
	BigInteger(const char string[]);
	BigInteger(const char bytes[], bool positive = true);
	BigInteger(ui64 array[], unsigned size, bool positive = true);

	~BigInteger();

	friend BigInteger operator + (BigInteger x);
	friend BigInteger operator - (BigInteger x);
	friend BigInteger operator * (BigInteger x);
	std::ostream& operator  <<  (std::ostream&);

	private:
	void append(ui64);
	ui64  pop(void);
	ui64 _8bytes_to_int64(const char bytes[8]);
	void int64_to_8bytes(const ui64 n, char result[8]);
};

#endif
