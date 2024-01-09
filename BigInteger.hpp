// -Handling numbers of an arbitrary size. The limit is the size of the RAM
//  available.

#ifndef _BIGINTEGER_INCLUDED_
#define _BIGINTEGER_INCLUDED_

#include<cstdint>
#include<iostream>

typedef std::uint64_t ui64;
typedef std::int64_t   i64;
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

struct BigInteger {
	enum NumberBase {
		BINARY,
		QUATERNARY,
		OCTAL,
		HEXADECIMAL,
		DECIMAL
	};
	private: struct Digit {
		ui64 value;
		Digit* next;
		Digit() : value(0), next(NULL) {}
		Digit(ui64 _value) : value(_value), next(NULL) {}
	}*first = NULL, *last = NULL; // First digit is the least significant. Last digit is the most significant.

	bool Positive = true; // Sing. True for positive, false for negative.

	static const ui64 ui64MAX = 0xFFFF'FFFF'FFFF'FFFF; // 64 1's.
	static const ui64 ui64LeftMost_1 = 0x8000'0000'0000'0000; // 100...00

	public:
	BigInteger(); // -Initialize as zero.
	BigInteger(i64);
	BigInteger(const BigInteger&);
	BigInteger(Digit*, bool); // Initializing with a list of digits and sing.
	BigInteger(const char[], ui64, bool = true); // -Initializing with an array of bytes (char's). Little endianess is used.

	// -Initializing from a formafted string. The characters will be
	//  interpreted accordingly to the number base selected.
	// -The expression is read from left to right, considering the leftmost
	//  characters as the most significant digits.
	// -Just valid characters are read.
	BigInteger(const char[], NumberBase = HEXADECIMAL);

	// Initializing with a ui64 array. Little endianess is used.
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

	void printHex() const;
	void printHexln() const;

	private:
	void setAsZero();
	void append(ui64);
	void push(ui64);
	ui64 pop(void);
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
	// -Computes the quotient divisor/dividend and the remainder
	//	divisor%dividend, where the divided is a single precision number. The
	//  result is saved in the 'result' array in the form [divisor/dividend,
	//	divisor%dividend].
	friend void shortDivision(const BigInteger&  divisor,
							  const ui64 dividend,
							  BigInteger result[2]);
	// -Computes the quotient divisor/dividend and the remainder
	//	divisor%dividend. The result is saved in the 'result' array
	//	in the form [divisor/dividend, divisor%dividend].
	friend void quotientRemainder(const BigInteger&  divisor,
								  const BigInteger& dividend,
								  BigInteger result[2]);
};
void ui64Product(ui64, ui64, ui64[2]);
#endif