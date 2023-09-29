// -Handling numbers of an arbitrary size. The limit is the size of the RAM
//  available.

#ifndef _BIGINTEGER_INCLUDED_
#define _BIGINTEGER_INCLUDED_

#include<cstdint>
#include<iostream>

typedef std::uint64_t ui64;
typedef std::int64_t   i64;
typedef unsigned char ui08;

enum NumberBase {
	BINARY,
	QUATERNARY,
	OCTAL,
	HEXADECIMAL,
	DECIMAL
};

struct BigInteger {
	private: struct Digit {
		ui64 value;
		Digit* next;
		Digit() : value(0), next(NULL) {}
		Digit(ui64 _value) : value(_value), next(NULL) {}
	}*digits = NULL, *last = NULL;

	bool Positive = true; // Sing. True for positive, false for negative.

	public:
	BigInteger();
	BigInteger(i64);
	BigInteger(const char[], NumberBase = HEXADECIMAL);
	BigInteger(const char[], ui64, bool = true);
	BigInteger(const ui64[], unsigned, bool = true);

	~BigInteger();

	friend BigInteger operator + (BigInteger);
	friend BigInteger operator - (BigInteger);
	friend BigInteger operator * (BigInteger);
	std::ostream& operator  <<  (std::ostream&);

	private:
	void setAsZero();
	void append(ui64);
	void push(ui64);
	ui64  pop(void);
	ui64 _8bytes_to_int64(const char[8]);
	void int64_to_8bytes(const ui64, char[8]);
};

#endif
