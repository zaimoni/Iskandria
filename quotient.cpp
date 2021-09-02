#include "quotient.hpp"
#include "Zaimoni.STL/numeric_error.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/flat_alg2.hpp"
#include "symbolic_fp.hpp"

namespace zaimoni {

quotient::quotient(const decltype(_numerator)& numerator, const decltype(_denominator)& denominator) : _numerator(numerator), _denominator(denominator), _heuristic(strict_max_heuristic - 1, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(const decltype(_numerator)& numerator, decltype(_denominator)&& denominator) : _numerator(numerator), _denominator(std::move(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(const decltype(_numerator)& numerator, fp_API* denominator) : _numerator(numerator), _denominator(denominator), _heuristic(strict_max_heuristic - 1, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(decltype(_numerator)&& numerator, const decltype(_denominator)& denominator) : _numerator(std::move(numerator)), _denominator(denominator), _heuristic(strict_max_heuristic - 1, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(decltype(_numerator)&& numerator, decltype(_denominator)&& denominator) : _numerator(std::move(numerator)), _denominator(std::move(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(decltype(_numerator)&& numerator, fp_API* denominator) : _numerator(std::move(numerator)), _denominator(denominator), _heuristic(strict_max_heuristic - 1, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(fp_API* numerator, const decltype(_denominator)& denominator) : _numerator(numerator), _denominator(denominator), _heuristic(strict_max_heuristic - 1, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(fp_API* numerator, decltype(_denominator)&& denominator) : _numerator(numerator), _denominator(std::move(denominator)), _heuristic(strict_max_heuristic - 1, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(fp_API* numerator, fp_API* denominator) : _numerator(numerator), _denominator(denominator), _heuristic(strict_max_heuristic - 1, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

bool quotient::would_destructive_eval() const {
	if (_denominator->is_one()) return true;
	if (_numerator->is_zero()) return true;
	return false;
}

// eval_to_ptr
quotient::eval_type quotient::destructive_eval() {
	if (_denominator->is_one()) return std::move(_numerator);
	if (_numerator->is_zero()) return std::move(_numerator);
	return nullptr;
}

bool quotient::self_eval() {
	if (0 >= _heuristic.first) return false;
	// \todo: greatest common integer factor exceeds one
	// \todo: mutual cancellation of negative signs
	// \todo: scalBn of denominator towards 1 (arguably normal-form)
	switch (_heuristic.first) {
	case componentwise_evaluation:
	{
		unsigned int n_state = _heuristic.second % 3;	// chinese remainder theorem encoding
		unsigned int d_state = _heuristic.second / 3;
		switch (n_state) {
		case 0:
			if (_numerator->self_eval()) break;
			n_state = 1;
			// intentional fall-through
		case 1:
			n_state = fp_API::eval(_numerator) ? 0 : 2;
		};
		switch (d_state) {
		case 0:
			if (_denominator->self_eval()) break;
			d_state = 1;
			// intentional fall-through
		case 1:
			d_state = fp_API::eval(_denominator) ? 0 : 2;
		};
		if (8 > (_heuristic.second = 3 * d_state + n_state)) {
			if (auto msg = _transform_fatal(_numerator, _denominator)) throw zaimoni::math::numeric_error(msg);
			if (would_destructive_eval()) _heuristic.first = 0;
			return true;
		}
	}
	// intentional fall-through
	default:
		_heuristic.first = 0;
		return false;
	}
}

bool quotient::is_zero() const {
	if (_numerator->is_zero()) return true;
	if (_denominator->is_inf()) return true;
	return false;
}

bool quotient::is_one() const {
	if (_numerator == _denominator) return true;	// we assume that if two std::shared_ptrs are binary-equal that they are the same, even if they are intervals
	if (_numerator->is_one() && _denominator->is_one()) return true;
	return false;
}

int quotient::sgn() const {
	const auto n_sgn = _numerator->sgn();
	if (const auto d_sgn = _denominator->sgn()) return n_sgn * d_sgn;
	else return n_sgn;	// division by zero hard-errors so more like "don't know"
}

intmax_t quotient::scal_bn_is_safe(intmax_t scale) const
{
	auto probe_numerator = _numerator->scal_bn_is_safe(scale);
	if (probe_numerator == scale) return scale;
	auto to_account_for = scale - probe_numerator;
	auto neg_to_account_for = (-INTMAX_MAX <= to_account_for ? -to_account_for : INTMAX_MAX);
	auto probe_denominator = -_denominator->scal_bn_is_safe(neg_to_account_for);
	if (probe_denominator == to_account_for) return scale;
	return probe_numerator + probe_denominator;
}

intmax_t quotient::ideal_scal_bn() const {
	if (_numerator->is_scal_bn_identity() || _denominator->is_scal_bn_identity()) return 0;
	intmax_t ret = _numerator->ideal_scal_bn();
	clamped_diff_assign(ret, _denominator->ideal_scal_bn());
	return ret;
}

const math::type* quotient::domain() const
{
	decltype(auto) d_domain = _denominator->domain();
	if (d_domain) d_domain = d_domain->inverse(_type_spec::Multiplication);
	if (!d_domain) return nullptr;
	if (decltype(auto) n_domain = _numerator->domain()) {
		if (decltype(auto) ret = math::type::defined(*n_domain, _type_spec::Multiplication, *d_domain)) return ret;
	}
	return nullptr;
}

std::string quotient::to_s() const {
	auto n = _numerator->to_s();
	if (_precedence >= _numerator->precedence_to_s()) n = std::string("(") + n + ')';
	auto d = _denominator->to_s();
	if (_precedence >= _denominator->precedence_to_s()) d = std::string("(") + d + ')';
	return n + '/' + d;
}

bool quotient::_is_finite() const {
	if (_numerator->is_finite()) return true;
	else if (_numerator->is_inf()) return false;
	else if (_denominator->is_inf()) return true;	// presumed not-undefined
	return false;
}

const char* quotient::_transform_fatal(const decltype(_numerator)& n, const decltype(_denominator)& d)
{
	if (d->is_zero()) return "zero denominator";
	if (n->is_inf() && d->is_inf()) return "infinity/infinity";
	return nullptr;
}

const char* quotient::_constructor_fatal() const {
	if (!_numerator) return "numerator null";
	if (!_denominator) return "denominator null";
	if (!domain()) return "division not defined";
	return _transform_fatal(_numerator, _denominator);
}

void quotient::_scal_bn(intmax_t scale) {
	// normalize denominator
	if (const auto denominator_scale = _denominator->ideal_scal_bn()) {
		if (0 > denominator_scale) {
			if (0 < scale && -scale <= denominator_scale) {
				_denominator->scal_bn(denominator_scale);
				if (0 == (scale += denominator_scale)) return;
			}
		} else /* if (0 < denominator_scale) */ {
			if (0 > scale && scale <= -denominator_scale) {
				_denominator->scal_bn(denominator_scale);
				if (0 == (scale += denominator_scale)) return;
			}
		}
	}
	// normalize numerator
	if (const auto numerator_scale = _numerator->ideal_scal_bn()) {
		if (0 < numerator_scale) {
			if (0 < scale && numerator_scale <= scale) {
				_numerator->scal_bn(numerator_scale);
				if (0 == (scale -= numerator_scale)) return;
			}
		}
		else /* if (0 > numerator_scale) */ {
			if (0 > scale && numerator_scale >= scale) {
				_numerator->scal_bn(numerator_scale);
				if (0 == (scale -= numerator_scale)) return;
			}
		}
	}

	if (const auto numerator_soak = _numerator->scal_bn_is_safe(scale)) {
		_numerator->scal_bn(numerator_soak);
		if (0 == (scale -= numerator_soak)) return;
	}
	if (const auto denominator_soak = _denominator->scal_bn_is_safe(-INTMAX_MAX > scale ? INTMAX_MAX : -scale )) {
		_denominator->scal_bn(denominator_soak);
		if (0 == (scale += denominator_soak)) return;
	}
	// didn't fit: install residual into numerator
	std::unique_ptr<symbolic_fp> stage(new symbolic_fp(_numerator));
	stage->scal_bn(scale);
	_numerator = std::shared_ptr<fp_API>(stage.release());
}

fp_API* quotient::_eval() const { return zaimoni::math::eval_quotient(_numerator, _denominator); }

}
