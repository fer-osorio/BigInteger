// -Handling numbers of an arbitrary size. The limit is the size of the RAM
//  available.

#ifndef _BIGINTEGER_INCLUDED_
#define _BIGINTEGER_INCLUDED_

#include<cstdint>
#include<iostream>

typedef std::uint64_t ui64;
typedef std::uint32_t ui32;
typedef unsigned char ui08;

union ui64Toui08 { // Cast from 64-bits unsigned integer to 8 unsigned char's
	ui64 ui64int;
	ui08 uchar[8];
};

union ui64Toui32 { // Cast from 64-bits unsigned integer to 2 unsigned int's
	ui64 ui64int;
	ui32 uint[2];
};

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
	BigInteger(); // -Initialize as zero.
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
	friend BigInteger operator - (const BigInteger&, const BigInteger&);
	friend BigInteger operator * (const BigInteger&, const BigInteger&);

	BigInteger operator - ();

	// Comparison
	bool operator == (int) const;

	friend std::ostream& operator << (std::ostream&, BigInteger);

	void print();
	void println();

	private:
	void setAsZero();
	void append(ui64);
	void push(ui64);
	ui64 pop(void);
	ui64 _8bytes_to_int64(const char[8]);
	void int64_to_8bytes(const ui64, char[8]);
	bool isValidDigit(char, NumberBase);

	// -Addition of non-negative integers. This function will assume both
	//  arguments are positive. The 'Positive' attribute of the result is
	//	never touched.
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
	void ui64Addition(ui64, ui64, ui64[2]) const;
	void ui64Product(ui64, ui64, ui64[2]) const;
};

#endif
