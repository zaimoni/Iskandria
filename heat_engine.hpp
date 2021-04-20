// heat_engine.hpp

#include "mass.hpp"

namespace thermodynamic {

class cycle
{
public:
	using interval = interval_shim::interval;
private:
	// units are energy/"unit test mass", but we use the same class for both
	mass _Q_in;		// first step, for our purposes
	mass _Q_out;	// third step, for our purposes
	mass _W_in;		// may be second, or fourth
	mass _W_out;
	// do not represent stage efficiency, yet (later stage ok, or higher level)
	// do not explicitly represent the "test mass", yet (later stage ok)
	bool _W_in_after_Q_in;	// signal which kind of cycle we are using (was advised both options are physical; Rankine is W_out after Q_in)
public:
	cycle() = default;	// so we can use in STL containers
	cycle(const cycle& src) = default;
	// Plan is to build out constructors with higher-level interfaces on an ad-hoc basis
	// at 100% efficiency we have Q_in+W_in = Q_out+W_out; these two constructors are overdetermined
	// decision needs to be made re reversed cycles (we could require all values positive), etc.
	cycle(interval Q_in, interval Q_out, interval W_in, interval W_out, fundamental_constants::units _u, bool W_in_after_Q_in = false)
	: _Q_in(mass::ENERGY, _u, Q_in), _Q_out(mass::ENERGY, _u, Q_out),
	  _W_in(mass::ENERGY, _u, W_in), _W_out(mass::ENERGY, _u, W_out), _W_in_after_Q_in(W_in_after_Q_in)
	{
	}

	cycle(const mass& Q_in, const mass& Q_out, const mass& W_in, const mass& W_out, bool W_in_after_Q_in = false)
	: _Q_in(Q_in), _Q_out((assert(Q_in.system_code() == Q_out.system_code()),Q_out)),
   	  _W_in((assert(Q_in.system_code() == W_in.system_code()), W_in)), _W_out((assert(Q_in.system_code() == W_out.system_code()), W_out)),
	  _W_in_after_Q_in(W_in_after_Q_in)
	{
	}

	~cycle() = default;
	cycle& operator=(const cycle& src) = default;

	// return type of these four not firmly decided; expect to usually would use energy mode i.e. Qin().E()
	const mass& Qin() const { return _Q_in; }
	const mass& Qout() const { return _Q_out; }
	const mass& Win() const { return _W_in; }
	const mass& Wout() const { return _W_out; }

	// circuit view, tracing the edges.  Plain English descriptions provide no guidance here.
	const mass& stage1() const { return _Q_in; }
	const mass& stage2() const { return _W_in_after_Q_in ? _W_in : _W_out; }
	const mass& stage3() const { return _Q_out; }
	const mass& stage4() const { return _W_in_after_Q_in ? _W_out : _W_in; }

	interval efficiency() const {
		auto q_in = _Q_in.E();
		return scalar((q_in - _Q_out.E()) / q_in);
	}

	bool syntax_ok() const
	{	// greater than 100% efficiency is unphysical.
		auto e = efficiency();
		if (1.0 < e.lower()) return false;
		if (1.0 < e.upper() && 1.0 == e.lower()) return false;
		// reality-check in vs. out (related to iterative refinement loop option for overdetermined constructors)
		auto E_in = _Q_in.E() + _W_in.E();
		auto E_out = _Q_out.E() + _W_out.E();
		if (E_out.upper() < E_in.lower()) return false;
		if (E_in.upper() < E_out.lower()) return false;
		return true;	// gross numerical error is not our problem
	}

	bool is_useful() const
	{
		if (_W_out.E().upper() < _W_in.E().lower()) return false;
		if (_Q_in.E().upper() < _Q_out.E().lower()) return false;
		if (_W_in.E().upper() < _W_out.E().lower()) return true;
		if (_Q_out.E().upper() < _Q_in.E().lower()) return true;
		return false;	// assume not useful if gross numerical errors
	}
};

// closer-to-physical heat engine classes go here

}