#include <stdlib.h>
#include <array>

/*
 The state codes.
 */
enum states {
	_9 = -9,
	_8 = -8,
	_7 = -7,
	_6 = -6,
	_5 = -5,
	_4 = -4,
	_3 = -3,
	_2 = -2,
	__ = -1, /* the universal error code */

	GO = 0, /* start    */
	OK, /* ok       */
	OB, /* object   */
	KE, /* key      */
	CO, /* colon    */
	VA, /* value    */
	AR, /* array    */
	ST, /* string   */
	ES, /* escape   */
	U1, /* u1       */
	U2, /* u2       */
	U3, /* u3       */
	U4, /* u4       */
	MI, /* minus    */
	ZE, /* zero     */
	IN, /* integer  */
	FR, /* fraction */
	E1, /* e        */
	E2, /* ex       */
	E3, /* exp      */
	T1, /* tr       */
	T2, /* tru      */
	T3, /* true     */
	F1, /* fa       */
	F2, /* fal      */
	F3, /* fals     */
	F4, /* false    */
	N1, /* nu       */
	N2, /* nul      */
	N3, /* null     */
	NR_STATES
};

/*
 These modes can be pushed on the stack.
 */
enum modes {
	MODE_ARRAY, MODE_DONE, MODE_KEY, MODE_OBJECT
};

template<unsigned Depth>
struct JSON_checkerT {
	constexpr JSON_checkerT() :
			state(GO), depth(Depth), top(-1) {
		push(MODE_DONE);
	}

	constexpr int reject() {
		/*
		 Delete the JSON_checker object.
		 */
		//    free((void*)jc->stack);
		//    free((void*)jc);
		return false;
	}

	constexpr int push(const int mode) {
		/*
		 Push a mode onto the stack. Return false if there is overflow.
		 */
		top += 1;
		if (top >= depth) {
			return false;
		}
		stack[top] = mode;
		return true;
	}

	constexpr int pop(int mode) {
		/*
		 Pop the stack, assuring that the current mode matches the expectation.
		 Return false if there is underflow or if the modes mismatch.
		 */
		if ((top < 0) || (stack[top] != mode) ) {
			return false;
		}
		top -= 1;
		return true;
	}

	states state;
	int depth;
	int top;
	int stack[Depth] { };
};

using JSON_checker = JSON_checkerT<20>;

extern constexpr bool JSON_checker_char(JSON_checker& jc, int next_char);
extern constexpr bool JSON_checker_done(JSON_checker& jc);

/* Characters are mapped into these 31 character classes. This allows for
 a significant reduction in the size of the state transition table. */

enum classes {
	_1 = -1, C_SPACE = 0,/* space */
	C_WHITE, /* other whitespace */
	C_LCURB, /* {  */
	C_RCURB, /* } */
	C_LSQRB, /* [ */
	C_RSQRB, /* ] */
	C_COLON, /* : */
	C_COMMA, /* , */
	C_QUOTE, /* " */
	C_BACKS, /* \ */
	C_SLASH, /* / */
	C_PLUS, /* + */
	C_MINUS, /* - */
	C_POINT, /* . */
	C_ZERO, /* 0 */
	C_DIGIT, /* 123456789 */
	C_LOW_A, /* a */
	C_LOW_B, /* b */
	C_LOW_C, /* c */
	C_LOW_D, /* d */
	C_LOW_E, /* e */
	C_LOW_F, /* f */
	C_LOW_L, /* l */
	C_LOW_N, /* n */
	C_LOW_R, /* r */
	C_LOW_S, /* s */
	C_LOW_T, /* t */
	C_LOW_U, /* u */
	C_ABCDF, /* ABCDF */
	C_E, /* E */
	C_ETC, /* everything else */
	NR_CLASSES
};


/* This array maps the 128 ASCII characters into character classes.
 The remaining Unicode characters should be mapped to C_ETC.
 Non-whitespace control characters are errors. */
constexpr const classes ascii_class[128] = {
		_1, _1, _1, _1, _1, _1, _1, _1, _1, C_WHITE, C_WHITE, _1, _1, C_WHITE, _1, _1,
		_1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1, _1,

		C_SPACE, C_ETC, C_QUOTE, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC,
		C_ETC, C_ETC, C_PLUS, C_COMMA, C_MINUS, C_POINT, C_SLASH, C_ZERO,
		C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT,
		C_DIGIT, C_COLON, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC,

		C_ETC, C_ABCDF, C_ABCDF, C_ABCDF, C_ABCDF, C_E, C_ABCDF, C_ETC, C_ETC,
		C_ETC, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC,
		C_ETC, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC, C_ETC, C_LSQRB,
		C_BACKS, C_RSQRB, C_ETC, C_ETC,

		C_ETC, C_LOW_A, C_LOW_B, C_LOW_C, C_LOW_D, C_LOW_E, C_LOW_F, C_ETC,
		C_ETC, C_ETC, C_ETC, C_ETC, C_LOW_L, C_ETC, C_LOW_N, C_ETC, C_ETC,
		C_ETC, C_LOW_R, C_LOW_S, C_LOW_T, C_LOW_U, C_ETC, C_ETC, C_ETC, C_ETC,
		C_ETC, C_LCURB, C_ETC, C_RCURB, C_ETC, C_ETC };


/*
 The state transition table takes the current state and the current symbol,
 and returns either a new state or an action. An action is represented as a
 negative number. A JSON text is accepted if at the end of the text the
 state is OK and if the mode is MODE_DONE.*/
constexpr states state_transition_table[NR_STATES][NR_CLASSES] = {
 /*
                 white                                      1-9                                   ABCDF  etc
             space |  {  }  [  ]  :  ,  "  \  /  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
/*start  GO*/ {GO,GO,_6,__,_5,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ok     OK*/ {OK,OK,__,_8,__,_7,__,_3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*object OB*/ {OB,OB,__,_9,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*key    KE*/ {KE,KE,__,__,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*colon  CO*/ {CO,CO,__,__,__,__,_2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*value  VA*/ {VA,VA,_6,__,_5,__,__,__,ST,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*array  AR*/ {AR,AR,_6,__,_5,_7,__,__,ST,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*string ST*/ {ST,__,ST,ST,ST,ST,ST,ST,_4,ES,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST},
/*escape ES*/ {__,__,__,__,__,__,__,__,ST,ST,ST,__,__,__,__,__,__,ST,__,__,__,ST,__,ST,ST,__,ST,U1,__,__,__},
/*u1     U1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U2,U2,U2,U2,U2,U2,U2,U2,__,__,__,__,__,__,U2,U2,__},
/*u2     U2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U3,U3,U3,U3,U3,U3,U3,U3,__,__,__,__,__,__,U3,U3,__},
/*u3     U3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U4,U4,U4,U4,U4,U4,U4,U4,__,__,__,__,__,__,U4,U4,__},
/*u4     U4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ST,ST,ST,ST,ST,ST,ST,ST,__,__,__,__,__,__,ST,ST,__},
/*minus  MI*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ZE,IN,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*zero   ZE*/ {OK,OK,__,_8,__,_7,__,_3,__,__,__,__,__,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*int    IN*/ {OK,OK,__,_8,__,_7,__,_3,__,__,__,__,__,FR,IN,IN,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*frac   FR*/ {OK,OK,__,_8,__,_7,__,_3,__,__,__,__,__,__,FR,FR,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*e      E1*/ {__,__,__,__,__,__,__,__,__,__,__,E2,E2,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ex     E2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*exp    E3*/ {OK,OK,__,_8,__,_7,__,_3,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*tr     T1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T2,__,__,__,__,__,__},
/*tru    T2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T3,__,__,__},
/*true   T3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*fa     F1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*fal    F2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F3,__,__,__,__,__,__,__,__},
/*fals   F3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F4,__,__,__,__,__},
/*false  F4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*nu     N1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N2,__,__,__},
/*nul    N2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N3,__,__,__,__,__,__,__,__},
/*null   N3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__},

};

constexpr bool JSON_checker_char(JSON_checker& jc, const int next_char) {
	classes next_class {  };
	states next_state { };

	/*  Determine the character's class. */
	if (next_char < 0) {
		return jc.reject();
	}
	if (next_char >= 128) {
		next_class = C_ETC;
	} else {
		next_class = ascii_class[next_char];
		if (static_cast<int>(next_class) <= static_cast<int>(__)) {
			return jc.reject();
		}
	}

	/*	 Get the next state from the state transition table. */
	next_state = state_transition_table[jc.state][next_class];
	if (next_state >= 0) {
		jc.state = next_state;
	} else {
		/* Or perform one of the actions. */
		switch (next_state) {
		/* empty } */
		case -9:
			if (!jc.pop(MODE_KEY)) {
				return jc.reject();
			}
			jc.state = OK;
			break;

			/* } */case -8:
			if (!jc.pop(MODE_OBJECT)) {
				return jc.reject();
			}
			jc.state = OK;
			break;

			/* ] */case -7:
			if (!jc.pop(MODE_ARRAY)) {
				return jc.reject();
			}
			jc.state = OK;
			break;

			/* { */case -6:
			if (!jc.push(MODE_KEY)) {
				return jc.reject();
			}
			jc.state = OB;
			break;

			/* [ */case -5:
			if (!jc.push(MODE_ARRAY)) {
				return jc.reject();
			}
			jc.state = AR;
			break;

			/* " */case -4:
			switch (jc.stack[jc.top]) {
			case MODE_KEY:
				jc.state = CO;
				break;
			case MODE_ARRAY:
			case MODE_OBJECT:
				jc.state = OK;
				break;
			default:
				return jc.reject();
			}
			break;

			/* , */case -3:
			switch (jc.stack[jc.top]) {
			case MODE_OBJECT:
				/* A comma causes a flip from object mode to key mode. */
				if (!jc.pop(MODE_OBJECT) || !jc.push(MODE_KEY)) {
					return jc.reject();
				}
				jc.state = KE;
				break;
			case MODE_ARRAY:
				jc.state = VA;
				break;
			default:
				return jc.reject();
			}
			break;

			/* : */case -2:
			/* A colon causes a flip from key mode to object mode. */
			if (!jc.pop(MODE_KEY) || !jc.push(MODE_OBJECT)) {
				return jc.reject();
			}
			jc.state = VA;
			break;
			/* Bad action. */
		default:
			return jc.reject();
		}
	}
	return true;
}


constexpr bool JSON_checker_done(JSON_checker& jc) {
	return jc.state == OK && jc.pop(MODE_DONE);
}

constexpr bool operator""_jsonchecker(const char* js, unsigned long len) {
	JSON_checker jc;
	while (len--) {
		int next_char = *js;
		js++;
		if (next_char <= 0) {
			break;
		}
		if (!JSON_checker_char(jc, next_char)) {
			return false;
		}
	}
	return (JSON_checker_done(jc));
}

/* compile time unit tests */
static_assert (  R"( {} )"_jsonchecker, "FAIL0");
static_assert (  R"( { } )"_jsonchecker, "FAIL1");
static_assert (! R"( { )"_jsonchecker, "FAIL2");
static_assert (  R"( {"key":42, "k2":"value", "a1":[1,2,3] } )"_jsonchecker, "FAIL3");

static_assert (  R"( 
[
    "JSON Test Pattern pass1",
    {"object with 1 member":["array with 1 element"]},
    {},
    [],
    -42,
    true,
    false,
    null,
    {
        "integer": 1234567890,
        "real": -9876.543210,
        "e": 0.123456789e-12,
        "E": 1.234567890E+34,
        "":  23456789012E66,
        "zero": 0,
        "one": 1,
        "space": " ",
        "quote": "\"",
        "backslash": "\\",
        "controls": "\b\f\n\r\t",
        "slash": "/ & \/",
        "alpha": "abcdefghijklmnopqrstuvwyz",
        "ALPHA": "ABCDEFGHIJKLMNOPQRSTUVWYZ",
        "digit": "0123456789",
        "0123456789": "digit",
        "special": "`1~!@#$%^&*()_+-={':[,]}|;.</>?",
        "hex": "\u0123\u4567\u89AB\uCDEF\uabcd\uef4A",
        "true": true,
        "false": false,
        "null": null,
        "array":[  ],
        "object":{  },
        "address": "50 St. James Street",
        "url": "http://www.JSON.org/",
        "comment": "// /* <!-- --",
        "# -- --> */": " ",
        " s p a c e d " :[1,2 , 3

,

4 , 5        ,          6           ,7        ],"compact":[1,2,3,4,5,6,7],
        "jsontext": "{\"object with 1 member\":[\"array with 1 element\"]}",
        "quotes": "&#34; \u0022 %22 0x22 034 &#x22;",
        "\/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"
: "A key can be any string"
    },
    0.5 ,98.6
,
99.44
,

1066,
1e1,
0.1e1,
1e-1,
1e00,2e+00,2e-00
,"rosebud"]

 )"_jsonchecker, "FAIL4");
