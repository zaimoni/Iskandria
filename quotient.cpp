#include "quotient.hpp"
#include "Zaimoni.STL/numeric_error.hpp"
#include "arithmetic.hpp"
#include "Zaimoni.STL/flat_alg2.hpp"
#include "symbolic_fp.hpp"

namespace zaimoni {

enum {
	componentwise_algebraic_evaluation = 1,
	rearrange,
	componentwise_evaluation,
	strict_max_heuristic
};

quotient::quotient(const decltype(_numerator)& numerator, const decltype(_denominator)& denominator) : _numerator(numerator), _denominator(denominator), _heuristic(componentwise_algebraic_evaluation, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(const decltype(_numerator)& numerator, decltype(_denominator)&& denominator) : _numerator(numerator), _denominator(std::move(denominator)), _heuristic(componentwise_algebraic_evaluation, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(decltype(_numerator)&& numerator, const decltype(_denominator)& denominator) : _numerator(std::move(numerator)), _denominator(denominator), _heuristic(componentwise_algebraic_evaluation, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

quotient::quotient(decltype(_numerator)&& numerator, decltype(_denominator)&& denominator) : _numerator(std::move(numerator)), _denominator(std::move(denominator)), _heuristic(componentwise_algebraic_evaluation, 0) {
	if (auto err = _constructor_fatal()) throw zaimoni::math::numeric_error(err);
}

// std::function would be most general solution, but we're not doing anything that intricate
enum {
	is_numerator = 1
};

bool quotient::would_destructive_eval() const {
	if (0 == _heuristic.first) return 0 < _heuristic.second;
	if (   _denominator->is_one()
		|| _numerator->is_zero()) {
		_heuristic = std::pair(0, is_numerator);
		return true;
	}
	return false;
}

// eval_to_ptr
quotient::eval_type quotient::destructive_eval() {
	if (would_destructive_eval()) {
		switch (_heuristic.second)
		{
		case is_numerator: return std::move(_numerator);
		};
	}
	return nullptr;
}

bool quotient::algebraic_self_eval() {
	if (0 == _heuristic.first) return false;

restart:
	switch (_heuristic.first) {
	case componentwise_algebraic_evaluation:
	{
		unsigned int n_state = _heuristic.second % 2;	// chinese remainder theorem encoding
		unsigned int d_state = _heuristic.second / 2;
		switch (n_state) {
		case 0:
			if (!fp_API::algebraic_reduce(_numerator)) ++n_state;
		};
		switch (d_state) {
		case 0:
			if (!fp_API::algebraic_reduce(_denominator)) ++d_state;
		};
		if (3 > (_heuristic.second = n_state + 2 * d_state)) {
			if (auto msg = _transform_fatal(_numerator, _denominator)) throw zaimoni::math::numeric_error(msg);
			would_destructive_eval();
			return true;
		}
		_heuristic = std::pair(_heuristic.first + 1, 0);
		goto restart;
	}
	case rearrange:
	{
		// scalBn of denominator towards 1 (arguably normal-form)
		auto n_scale = _numerator->ideal_scal_bn();
		auto d_scale = _denominator->ideal_scal_bn();
		if (0 < n_scale && 0 < d_scale) {
			if (n_scale != d_scale) {
				do {
					if (n_scale < d_scale) d_scale = _denominator->scal_bn_is_safe(n_scale);
					else n_scale = _numerator->scal_bn_is_safe(d_scale);
				} while (0 > n_scale && 0 > d_scale && n_scale != d_scale);
			}
			if (0 < n_scale && 0 < d_scale) {
				_numerator = scalBn(_numerator, n_scale);
				_denominator = scalBn(_denominator, d_scale);
				return true;
			}
		}
		if (0 > n_scale && 0 > d_scale) {
			if (n_scale != d_scale) {
				do {
					if (n_scale > d_scale) d_scale = _denominator->scal_bn_is_safe(n_scale);
					else n_scale = _numerator->scal_bn_is_safe(d_scale);
				} while (0 > n_scale && 0 > d_scale && n_scale != d_scale);
			}
			if (0 > n_scale && 0 > d_scale) {
				_numerator = scalBn(_numerator, n_scale);
				_denominator = scalBn(_denominator, d_scale);
				return true;
			}
		}

		// \todo: greatest common integer factor exceeds one
		// \todo: mutual cancellation of negative signs
		if (auto d = _denominator.get_rw<API_productinv<fp_API>>()) {
			if (!d->first) {
				_denominator = std::unique_ptr<fp_API>(_denominator->clone());
				d = _denominator.get_rw<API_productinv<fp_API>>();
			}
			if (d) {
				switch (d->first->rearrange_divides(_numerator)) {
				case 2:
					_heuristic = std::pair(componentwise_algebraic_evaluation, 0);
					return true;
				case -1:
				case 1:
					if (!would_destructive_eval()) _heuristic = std::pair(componentwise_algebraic_evaluation, 0);
					return true;
				}
			}
		}
		if (auto n = _numerator.get_rw<API_productinv<fp_API>>()) {
			if (!n->first) {
				_numerator = std::unique_ptr<fp_API>(_numerator->clone());
				n = _numerator.get_rw<API_productinv<fp_API>>();
			}
			if (n) {
				switch (n->first->rearrange_dividedby(_denominator)) {
				case 2:
					_heuristic = std::pair(componentwise_algebraic_evaluation, 0);
					return true;
				case -1:
				case 1:
					if (!would_destructive_eval()) _heuristic = std::pair(componentwise_algebraic_evaluation, 0);
					return true;
				}
			}
		}
		// \todo other adjustments that make sense

		_heuristic = std::pair(_heuristic.first + 1, 0);
	}
	}

	return false;
}

bool quotient::inexact_self_eval() {
	if (0 == _heuristic.first) return false;
	switch (_heuristic.first) {
	case componentwise_evaluation:
	{
		unsigned int n_state = _heuristic.second % 2;	// chinese remainder theorem encoding
		unsigned int d_state = _heuristic.second / 2;
		switch (n_state) {
		case 0:
			if (!fp_API::inexact_reduce(_numerator)) ++n_state;
		};
		switch (d_state) {
		case 0:
			if (!fp_API::inexact_reduce(_denominator)) ++d_state;
		};
		if (3 > (_heuristic.second = n_state + 2 * d_state)) {
			if (auto msg = _transform_fatal(_numerator, _denominator)) throw zaimoni::math::numeric_error(msg);
			_heuristic = std::pair(componentwise_algebraic_evaluation, 0);
			would_destructive_eval();
			return true;
		}
		_heuristic = std::pair(0, 0);
		return false;
	}
	}

	return false;
}


bool quotient::self_eval() {
	if (0 >= _heuristic.first) return false;
	if (algebraic_self_eval()) return true;
	if (inexact_self_eval()) return true;
	return false;
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

std::optional<bool> quotient::_is_finite() const {
	if (const auto test = _numerator->is_finite_kripke()) {
		return *test; // assume defined i.e. not infinity/infinity
	}
	if (const auto test = _denominator->is_finite_kripke()) {
		if (!*test) return true; // presumed not-undefined
		// fallthrough
	}
	return std::nullopt;
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
