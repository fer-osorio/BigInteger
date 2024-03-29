#ifndef _BIGINTEGER_INCLUDED_													// -Implementation of multi-precision arithmetic. The limit of the size of the
#define  _BIGINTEGER_INCLUDED_													//  numbers handled is determined by the size of the RAM available.

#include<cstdint>
#include<iostream>

typedef std::uint64_t ui64;
typedef std::int64_t   i64;
typedef std::uint32_t ui32;
typedef std::uint16_t ui16;
typedef unsigned char ui08;

union ui64Toui08 {																// Intended to cast from 64-bits unsigned integer to 8 unsigned char's
	ui64 ui64int;
	ui08 uchar[8];
};

union ui32Toui08 {																// Intended to cast from 32-bits unsigned integer to 4 unsigned char's
	ui32 ui32int;
	ui08 uchar[4];
};

union ui64Toui32 {																// Intended to cast from 64-bits unsigned integer to 2 unsigned int's
	ui64 ui64int;
	ui32 uint[2];
};

union ui32Toui16 {																// Intended to cast from 32-bits unsigned integer to 2 unsigned short's
	ui32 ui32int;
	ui16 ushort[2];
};

static const ui32 _2power31   = 0x80000000;										// Equals to 2^31 = 2^32/2 (half of the base). Intended for division algorithm
static const ui64 _2power32   = 0x100000000;									// Equals to 2^32, witch is the base we're using
static const ui32 ui32MAX     = 0xFFFFFFFF;										// 32 bits, all 1's.
static const ui32 ui32wordlen = 32;
static const ui64 ui64MAX     = 0xFFFFFFFFFFFFFFFF;								// 64 bits, all 1's.
static const ui64 ui64LeftMost_1 = 0x8000'0000'0000'0000;						// 64 bits, only the most significant is 1

struct BigInteger {
	public: enum NumberBase {													// "public" is not necessary here, it's just for readability
		BINARY,																	// Number bases for printing
		QUATERNARY,
		OCTAL,
		HEXADECIMAL,
		DECIMAL
	};
	private: struct Digit {
		ui32   value;															// Here we set our radix base to 2^32
		Digit* next;
		Digit() : value(0), next(NULL) {}
		Digit(ui32 _value) : value(_value), next(NULL) {}
	};
	private:																	// Attributes
	struct Digit *first = NULL;													// First digit is the least significant.
	struct Digit *last = NULL;													// Last digit is the most significant.
	bool NonNegative = true;													// Sign. True for NonNegative, false for negative.

	inline BigInteger(bool empty, ui32 d) {										// Intended to create objects with a null list of digits
		if(!empty) {															// If empty is false this constructor creates a d BigInteger
			this->first = new Digit(d); 											// -Private constructor. The users are not suppose to use BigIntegers with a null
			this->last = this->first;											//  list of digits
			this->NonNegative = true;
		}
	}

	public:
	inline BigInteger(): first(new Digit()), last(first), NonNegative(true) {};		// -Initialize as zero.
	inline ~BigInteger() {this->clean();}
	BigInteger(i64);
	inline BigInteger(int t) {
		if(t < 0) {
			this->NonNegative = false;
			t = -t;
		}
		this->first = new Digit((ui32)t);
		this->last = this->first;
	}
	inline BigInteger(const BigInteger& a) : NonNegative(a.NonNegative) {
    	Digit* aux;
    	for(aux=a.first; aux!=NULL; aux=aux->next) this->append(aux->value);
	}
	BigInteger(const char[], ui64, bool = true);								// -Initializing with an array of bytes (char's). Little endianess is used.

	BigInteger(const char[], NumberBase = HEXADECIMAL);							// -Initializing from a formatted string. The characters will be interpreted
																				//  accordingly to the number base selected.
																				// -The expression is read from left to right, considering the leftmost characters
																				//  as the most significant digits.
																				// -Just valid characters are read.

	BigInteger(const ui32[], unsigned, bool = true);							// Initializing with a ui32 array. Little endianess is used.

	// Assignment
	BigInteger& operator = (const BigInteger&);
	BigInteger& operator = (int t) {
    	this->clean();                                                              // Releasing memory
    	if(t >= 0) this->NonNegative =  true;                                       // Determining sign
    	else {
    	    this->NonNegative = false;                                              // Saving negative sign and making 't' positive
    	    t = -t;
    	}
    	this->last = this->first = new Digit((ui32)t);                               // Digit list with a unique element
    	return *this;
	}

	// Arithmetic
	friend BigInteger operator + (const BigInteger&, const BigInteger&);
	friend BigInteger operator - (const BigInteger&, const BigInteger&);
	friend BigInteger operator * (const BigInteger&, const BigInteger&);
	inline BigInteger 	operator - () const{
    	BigInteger r = *this;
    	if(r != 0) r.NonNegative = !this->NonNegative;							// Guarding against a zero with negative sign
    	else       r.NonNegative = true;
    	return r;
	}
	BigInteger& operator ++ ();
	inline BigInteger& operator -- () {
    	this->NonNegative = !this->NonNegative;									// Using x-1 = -(-x + 1)
    	this->operator++();
    	if(this->operator==(0)) this->NonNegative = !this->NonNegative;			// If it's zero it already has the correct sign (non negative)
    	return *this;
	}
	BigInteger& operator += (int v) {
    	if(v >= 0) {
    	    if(this->NonNegative == true)this->plusEqualNonNegative((ui32)v);
    	    else this->minusEqualNonNegative((ui32) v);							// Using the fact -a + v == -(a - v)
    	} else {
    	    v = -v;
    	    if(this->NonNegative == true)this->minusEqualNonNegative((ui32)v);
    	    else this->plusEqualNonNegative((ui32)v);							// Using the fact -a - v == -(a + v)
    	}
    	return *this;
	}

	// Comparison
	inline bool operator == (int x) const{
    	if(this->first == NULL) return false;                                       // No comparison at all.
    	if(this->first->next != NULL) return false;                                 // At this point, this is necessarily bigger than x
    	bool nonNegative = x >= 0;                                                  // Saving the sign of x
    	if(x < 0) x = -x;                                                           // Saving absolute value of x
    	if(x == 0 && this->first->value == 0) return true;                          // Special case; zero can have any of the signs.
    	return nonNegative == this->NonNegative && this->first->value == (ui32)x;
	}
	inline bool operator != (int x) const{
    	return !(*this == x);
	}
	inline bool operator < (const BigInteger& x) const {
    	if(this->compare(x) == -1) return true;
    	else return false;
	}

	inline ui32 operator [] (ui32 n) const {									// Returns the ui32 in the list place n mod l where l is the length of the list
    	Digit *dt;																// This digits. Supposing we don't have a null digits list
    	for(dt=this->first;n>0;n--,dt=dt->next) if(dt==NULL) dt = this->first;	// If we're at the end, return to the beginning
    	return dt->value;
	};

	friend std::ostream& operator << (std::ostream&, BigInteger);

	void printHex() const;
	void printHexln() const;

	private:
	inline void clean() {														// Frees the memory occupied by the list of digits
		struct Digit* t;
		for(t = this->first; t != NULL; t = this->first) {
			this->first = this->first->next;
			delete t;
		}																		// At this point this->last == NULL
		this->last = NULL;														// Before this line, this->last pointed to a freed memory location
	}
	ui32 len() const{															// Returns the amount of elements in the list of digits
    	ui32 l;																	// Will hold the length
    	Digit* dt;																// Digits of this
    	for(l = 0, dt = this->first; dt != NULL; dt = dt->next) {l++;}			// Computing length
    	return l;
	}
	inline void setAs(int x) {													// Clean and sets this BigInteger to x
    	this->clean();
    	if(x < 0) { this->NonNegative = false; x = -x;}
    	else this->NonNegative = true;
    	this->first = new Digit((ui32)x);
    	this->last = this->first;
	}
	inline void append(ui32 x) {												// In the list of digits, puts a new element at the end
    	if(this->first == NULL) {
    	    this->first = new Digit(x);
    	    this->last = this->first;
    	    return;
    	}
    	this->last->next =  new Digit(x);
    	this->last = this->last->next;
	}
	inline ui32 pop() {
    	if(this->first == NULL) return 0;										// NULL list, this should be handle by an exception.
    	ui64 r = this->last->value;												// Saving last value
    	if(this->first->next == NULL) {											// Single precision number
    	    if(r != this->first->value){/*Some exception here*/}
    	    return r;															// Don't deleting unique list element in order to prevent an empty list
    	}
    	Digit* aux;
    	for(aux = this->first; aux->next->next != NULL; aux = aux->next) {}		// Reaching the element before the last Digit object
    	delete this->last;														// Updating last attribute
    	this->last = aux;
    	this->last->next = NULL;
    	return r;
	}
	inline void push(ui32 x) {													// New element at the beginning of the Digits list
    	Digit* _first = new Digit(x);
    	if(this->last == NULL) this->last = _first;                             // Guarding against BigInteger initialized with NULL pointer.)
    	_first->next = this->first;
    	this->first = _first;
	}
	bool isValidDigit(char, NumberBase);										// Given a number base...
	void plusEqualNonNegative(ui32 x);											// Adds x to this BigInteger. Assuming this BigInteger is non negative.
	void minusEqualNonNegative(ui32 x);											// Subtracts x from this BigInteger. Assuming this BigInteger is non negative.
	int  compare(const BigInteger& x) const;									// Compares two BigIntegers. Returns -1 for this < x, 0 for this == x and 1 for
																				// this > x
	inline bool isSinglePrecision() const {
		if(this->first == NULL) {
			throw "\nException in BigInteger.hpp, function inline bool is"
			"SinglePressition().BigInteger with null list of digits...\n";
		}
		if(this->first->next != NULL) return false;
		return true;
	}

	inline Digit* getDigitptr(unsigned n) const{								// Returns the ui32 in the list place n mod l where l is the length of the list
		Digit *dt;																// This digits. Supposing we don't have a null digits list
    	for(dt=this->first;n>0;n--,dt=dt->next) if(dt==NULL) dt = this->first;	// If we're at the end, return to the beginning
    	return dt;
	}

	void addNonnegative(const BigInteger& x, BigInteger& result) const;			// -Computes the addition of 'this' with x and saves the result in 'result'.
																				//  The addition is conducted as if the arguments were non negatives.

	void subtractNonnegative(const BigInteger& x, BigInteger& result) const;	// -Computes the subtraction of 'this' with x and saves the result in 'result'.
																				//  The subtraction is conducted as if the arguments were non negatives.

	void shortDivisionNonnegative(ui32 divisor, BigInteger result[2]) const;	// -Computes the quotient and remainder of non-negatives 'dividend' and 'divirsor'.
								  												//  Result is saved in 'result' in the form [quotient, remainder]

	void shortDivision(ui32 divisor, BigInteger result[2]) const;				// -Same as void shortDivision, but know we take in account the sign of
																				//  'this'.
	public:
	void shortDivision(int  divisor, BigInteger result[2]) const;				// -Same as shortDivision above, but know we take in account the sign of 'divisor'

	void divisionNonnegative(const BigInteger& divisor, BigInteger result[2])	// -Computes the quotient and the remainder between 'this' and 'divosor'
	const;																		// -The result is saved in the array 'result' in the form [quotient, remainder]
};

void ui64Product(ui64, ui64, ui64[2]);

#endif