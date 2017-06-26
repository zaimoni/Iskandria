// civilization.hpp

#ifndef CIVILIZATION_HPP
#define CIVILIZATION_HPP 1

namespace iskandria {

enum civilization {
	CIV_NONE = 0,
	CIV_TERRAN = 1,	// origin Terra
	CIV_ISKANDRAN = 2,	// origin Iskandra: the Iskandran Badgers
	CIV_WYLKRAI = 3,	// unspecified origin: planet-phobic tigers
	CIV_TZARZ = 4,		// origin unknown: arachnoreptilian
	CIV_ZAIMONIHOME = 5	// the zaimoni, proximate creators of the Iskandran Badgers.
	// above are "current" civilizations, technology closely comparable to that at the time of "Radiation Sensitive", "Ill-Starred Romance", etc.
	// historical civilizations would have their own values if needed.
	// The correct use and repair of technology not "native" to one's civilization is often heavily penalized, and often impossible.
};

}	// namespace iskandria

#endif
