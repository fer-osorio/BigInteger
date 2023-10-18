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

	static const ui64 ui64MAX = 0xFFFF'FFFF'FFFF'FFFF; // 64 1's.
	static const ui64 ui64LeftMost_1 = 0x8000'0000'0000'0000; // 100...00

	public:
	BigInteger();
	BigInteger(i64);
	BigInteger(const BigInteger&);
	BigInteger(Digit*, bool = true);
	BigInteger(const char[], ui64, bool = true);
	BigInteger(const char[], NumberBase = HEXADECIMAL);
	BigInteger(const ui64[], unsigned, bool = true);

	~BigInteger();

	// Assignment
	BigInteger& operator = (const BigInteger&);

	// Arithmetic
	friend BigInteger operator + (const BigInteger&, const BigInteger&);
	friend BigInteger operator - (BigInteger);
	friend BigInteger operator * (BigInteger);

	// Comparison
	bool operator == (int) const;

	friend std::ostream& operator << (std::ostream&, BigInteger);

	void print();
	void println();

	private:
	void setAsZero();
	void append(ui64);
	void push(ui64);
	ui64  pop(void);
	ui64 _8bytes_to_int64(const char[8]);
	void int64_to_8bytes(const ui64, char[8]);
	bool isValidDigit(char, NumberBase);

	// -Addition of non-negative integers. This function will assume both
	//  arguments are positive.
	// -The result of the addition will be saved in the variable 'result'.
	//  This is made in this way in order to avoid the copy of objects.
	friend BigInteger& additionPositive(const BigInteger& a,
										const BigInteger& b,
										BigInteger& result);
	// -Subtraction of non-negative integers. This function will assume both
	//  arguments are positive.
	// -The result of the subtraction will be saved in the variable 'result'.
	//  This is made in this way in order to avoid the copy of objects.
	friend BigInteger& subtractionPositive(const BigInteger& a,
										   const BigInteger& b,
										   BigInteger& result);
};

#endif
