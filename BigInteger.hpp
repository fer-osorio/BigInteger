// -Handling numbers of an arbitrary size. The limit is the size of the RAM available.
#ifndef _BIGINTEGER_INCLUDED_
#define  _BIGINTEGER_INCLUDED_

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

union ui32Toui08 { // Cast from 32-bits unsigned integer to 4 unsigned char's
	ui32 ui32int;
	ui08 uchar[4];
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
		ui32   value;		   // Here we set our radix to 2^32
		Digit* next;
		Digit() : value(0), next(NULL) {}
		Digit(ui32 _value) : value(_value), next(NULL) {}
	};
	struct Digit *first = NULL; // First digit is the least significant.
	struct Digit *last = NULL; // Last digit is the most significant.

	bool Positive = true; 	   // Sing. True for positive, false for negative.

	static const ui32 ui32MAX = 0xFFFFFFFF;					  // 32 bits, all 1's.
	static const ui64 ui64MAX = 0xFFFFFFFFFFFFFFFF; 		  // 64 bits, all 1's.
	static const ui64 ui64LeftMost_1 = 0x8000'0000'0000'0000; // 64 bits, only the most significant is 1

	public:
	inline BigInteger(): first(new Digit()), last(first), Positive(true) {}; // -Initialize as zero.
	BigInteger(i64);
	BigInteger(const BigInteger&);
	BigInteger(Digit*, bool); 					// Initializing with a list of digits and sing.
	BigInteger(const char[], ui64, bool = true);// -Initializing with an array of bytes (char's). Little endianess is used.

	BigInteger(const char[], NumberBase = HEXADECIMAL); // -Initializing from a formatted string. The characters will be interpreted accordingly to the number base selected.
	// -The expression is read from left to right, considering the leftmost characters as the most significant digits.
	// -Just valid characters are read.

	BigInteger(const ui32[], unsigned, bool = true);	// Initializing with a ui32 array. Little endianess is used.

	~BigInteger();

	// Assignment
	BigInteger& operator = (const BigInteger&);

	// Arithmetic
	friend BigInteger operator + (const BigInteger&, const BigInteger&);
	friend BigInteger operator - (const BigInteger&, const BigInteger&);
	friend BigInteger operator * (const BigInteger&, const BigInteger&);
	BigInteger operator - ();
	BigInteger& operator ++ ();
	BigInteger& operator -- ();
	BigInteger& operator += (int);

	// Comparison
	bool operator == (int) const;
	bool operator != (int) const;

	friend std::ostream& operator << (std::ostream&, BigInteger);

	void printHex() const;
	void printHexln() const;

	private:
	void setAsZero();	// Calls the destructor and sets this BigInteger to zero
	void setAsOne();	// Calls the destructor and sets this BigInteger to one
	void setAs(int x);	// Calls the destructor and sets this BigInteger to x
	void append(ui32);	// In the list of digits, puts a new element at the end
	void push(ui32);	// In the list of digits, puts a new element at the beginning
	ui32 pop(void);		// In the list of digits, returns the value of the last element and deletes it
	bool isValidDigit(char, NumberBase);// Given a number base...
	void plusEqualPositive(ui32 x); 	// Adds x to this BigInteger. Assuming this BigInteger is positive.
	void minusEqualPositive(ui32 x); 	// Subtracts x from this BigInteger. Assuming this BigInteger is positive.

	// -Addition of non-negative integers. This function will assume both
	//  arguments are positive. The 'Positive' attribute of the result is
	//	never touched.
	// -The result of the addition will be saved in the variable 'result'.
	//  This is made in this way in order to avoid the copy of objects.
	friend BigInteger& additionPositive(const BigInteger& a, const BigInteger& b, BigInteger& result);

	// -Subtraction of non-negative integers. This function will assume both
	//  arguments are positive.
	// -The result of the subtraction will be saved in the variable 'result'.
	//  This is made in this way in order to avoid the copy of objects.
	friend BigInteger& subtractionPositive(const BigInteger& a, const BigInteger& b, BigInteger& result);

	// -Computes the quotient divisor/dividend and the remainder
	//	divisor%dividend, where the divided is a single precision number. The
	//  result is saved in the 'result' array in the form [divisor/dividend,
	//	divisor%dividend].
	// -This function assumes its arguments are positive.
	friend void shortDivisionPositive(const BigInteger& dividend, const ui32 divisor, BigInteger result[2]);

	// -Same as shortDivisionPositive, but know we take in account the sign of the first argument
	friend void shortDivision(const BigInteger& dividend, ui32 divisor, BigInteger result[2]);

	// -Same as shortDivisionPositive, but know we take in account the sign.
	friend void shortDivision(const BigInteger& dividend, int divisor, BigInteger result[2]);

	// -Computes the quotient divisor/dividend and the remainder
	//	divisor%dividend. The result is saved in the 'result' array
	//	in the form [divisor/dividend, divisor%dividend].
	friend void quotientRemainder(const BigInteger& dividend, const BigInteger& divisor, BigInteger result[2]);
};
void ui64Product(ui64, ui64, ui64[2]);
#endif