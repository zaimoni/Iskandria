#ifndef QUOTIENT_HPP
#define QUOTIENT_HPP 1

#include "Zaimoni.STL/eval.hpp"

namespace zaimoni {

	class quotient final : public fp_API, public eval_shared_ptr<fp_API>
	{
		std::shared_ptr<fp_API> _numerator;
		std::shared_ptr<fp_API> _denominator;
		std::pair<unsigned int, unsigned int> _heuristic;
		enum {
			componentwise_evaluation = 1,
			strict_max_heuristic
		};
	public:
		quotient() = default;
		quotient(const std::shared_ptr<fp_API>& numerator, const std::shared_ptr<fp_API>& denominator);
		quotient(const std::shared_ptr<fp_API>& numerator, std::shared_ptr<fp_API>&& denominator);
		quotient(const std::shared_ptr<fp_API>& numerator, fp_API* denominator);
		quotient(std::shared_ptr<fp_API>&& numerator, const std::shared_ptr<fp_API>& denominator);
		quotient(std::shared_ptr<fp_API>&& numerator, std::shared_ptr<fp_API>&& denominator);
		quotient(std::shared_ptr<fp_API>&& numerator, fp_API* denominator);
		quotient(fp_API* numerator, const std::shared_ptr<fp_API>& denominator);
		quotient(fp_API* numerator, std::shared_ptr<fp_API>&& denominator);
		quotient(fp_API* numerator, fp_API* denominator);
		quotient(const quotient& src) = default;
		quotient(quotient&& src) = default;
		~quotient() = default;
		quotient& operator=(const quotient& src) = default;
		quotient& operator=(quotient&& src) = default;

	private:
		bool would_destructive_eval() const;

	public:
		std::shared_ptr<fp_API> destructive_eval() override; // eval_shared_ptr

		// fp_API
		bool self_eval() override;
		bool is_zero() const override;
		bool is_one() const override;
		int sgn() const override;

		bool is_scal_bn_identity() const override { return false; };	// let evaluation handle this -- pathological behavior

		std::pair<intmax_t, intmax_t> scal_bn_safe_range() const override;
		intmax_t scal_bn_is_safe(intmax_t scale) const override;
		intmax_t ideal_scal_bn() const override;
		const math::type* domain() const override;
		fp_API* clone() const override { return new quotient(*this); };
		std::string to_s() const override;
		int precedence() const override { return 2; }
		bool _is_inf() const override { return _numerator->is_inf(); } // cf. _transform_fatal which requires finite denominator in this case
		bool _is_finite() const override;

	private:
		static const char* _transform_fatal(const std::shared_ptr<fp_API>& n, const std::shared_ptr<fp_API>& d);
		const char* _constructor_fatal() const;
		void _scal_bn(intmax_t scale) override;
		fp_API* _eval() const override;
	};

}

#endif