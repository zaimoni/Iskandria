#ifndef XCOMLIKE_HPP
#define XCOMLIKE_HPP 1

namespace iskandria {
namespace compass {

	// 2-dimensional compass rose
	enum XCOMlike : unsigned char {
		N = 0,
		NE,
		E,
		SE,
		S,
		SW,
		W,
		NW
	};
	enum {
		NEUTRAL = NW + 1,
		XCOM_STRICT_UB = NW + 1,
		XCOM_EXT_STRICT_UB = NEUTRAL + 1
	};

	inline constexpr XCOMlike rotate(XCOMlike origin, XCOMlike delta) { return XCOMlike((origin + delta) % XCOM_STRICT_UB); }
	inline constexpr XCOMlike inv_rotate(XCOMlike origin, XCOMlike delta) { return XCOMlike((origin - delta) % XCOM_STRICT_UB); }

}
}

#endif
