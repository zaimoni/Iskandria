#ifndef CSSBOX_HPP
#define CSSBOX_HPP 1

#include <utility>
#include <memory>
#include <vector>
#include "Zaimoni.STL/Compiler.h"

#define MULTITHREAD_DRAW 1

namespace css {

class box {
#undef LEFT
#undef TOP
#undef RIGHT
#undef BOTTOM
#undef WIDTH
#undef HEIGHT
#undef REFLOW
public:
	enum auto_legal {
		LEFT = 0,
		TOP,
		RIGHT,
		BOTTOM,	// highest index for margin, padding
		WIDTH,
		HEIGHT,	// highest index for auto flagging
	};

	enum clear_float_legal {
		CF_NONE = 0,
		CF_LEFT,
		CF_RIGHT,
		CF_BOTH_INHERIT
	};
	static_assert(3 == CF_BOTH_INHERIT, "3!=CF_BOTH_INHERIT");
protected:
	enum {
		REFLOW = HEIGHT + 1
	};

	unsigned char _auto;	// bitmap: margins, width, height
	unsigned char _auto_recalc;	// margin or height/width
	unsigned char _clear_float;	// clear, float encodings
	std::pair<int, int> _origin;	// (x,y) i.e. (left,top): relative to container
	std::pair<int, int> _screen;	// (x,y) i.e. (left,top): global
private:
	std::pair<int, int> _size;
	std::pair<int, int> _size_min;
	std::pair<int, int> _size_max;
	int _margin[4];
	int _padding[4];
	std::weak_ptr<box> _parent;
	std::shared_ptr<box> _self;
#if MULTITHREAD_DRAW
	static unsigned int _recalc_fakelock;
#endif
protected:
	box(bool bootstrap = false);
	ZAIMONI_DEFAULT_COPY_ASSIGN(box);
public:
	virtual ~box() = default;

	// auto values
	bool is_auto(auto_legal src) const { return _auto & (1ULL << src); }
	void set_auto(auto_legal src);

	// width/height
	auto size() const { return _size; }
	int width() const { return _size.first; }
	int height() const { return _size.second; }
	int min_width() const { return _size_min.first; }
	int min_height() const { return _size_min.second; }
	int max_width() const { return _size_max.first; }
	int max_height() const { return _size_max.second; }

	void width(int w);
	void height(int h);
	void min_width(int w);
	void min_height(int h);
	void max_width(int w);
	void max_height(int h);

	// padding, margin
	template<int src> int padding() const {
		static_assert(0 <= src && WIDTH > src, "0 <= src && WIDTH > src");
		return _padding[src];
	}

	template<int src> int margin() const {
		static_assert(0 <= src && WIDTH > src, "0 <= src && WIDTH > src");
		return _margin[src];
	}
	template<int src> void _set_margin(int x) {
		static_assert(0 <= src && WIDTH > src, "0 <= src && WIDTH > src");
		_auto_recalc &= ~(1ULL << src);
		_margin[src] = x;
	}
	template<int src> void set_margin(int x) {
		static_assert(0 <= src && WIDTH > src, "0 <= src && WIDTH > src");
		_auto &= ~(1ULL << src);
		_set_margin<src>(x);
	}

	int full_width() const { return width() + padding<LEFT>() + padding<RIGHT>(); }
	int full_height() const { return height() + padding<TOP>() + padding<BOTTOM>(); }

	// layout recalculation (unsure about access control here)
	std::shared_ptr<box> parent() const { return _parent.lock(); }
	auto origin() const { return _origin; }
	virtual void remove(std::shared_ptr<box> gone);
	void disconnect() { _self.reset(); };

	// these should be abstract, but bringing up a new implementation may need waiving that
#ifdef PROTOTYPING_CSS
	virtual bool flush() { return false; };
	virtual int need_recalc() const { return 0; };	// return value is C error code convention; 0 no-action, negative error, positive action code
	virtual void draw() const {};
#else
	virtual bool flush() = 0;
	virtual int need_recalc() const = 0;	// return value is C error code convention; 0 no-action, negative error, positive action code
	virtual void draw() const = 0;
#endif
	virtual void force_size(int w, int h) {};	// should be abstract

	void recalc();
	virtual void set_origin(std::pair<int, int> logical_origin);
	virtual void screen_coords(std::pair<int, int> logical_origin);

	void horizontal_centering(int ub, std::pair<int, int> origin);
	void vertical_centering(int ub, std::pair<int, int> origin);
	bool request_horz_margins();
	bool request_vert_margins();
protected:
	virtual void schedule_reflow();
	void set_parent(std::shared_ptr<box>& src);
	void _width(int w);
	void _height(int h);

private:
#ifdef PROTOTYPING_CSS
	virtual void _recalc(int code) {};
#else
	virtual void _recalc(int code) = 0;
#endif
};

class box_dynamic : public box
{
private:
	std::vector<std::shared_ptr<box> > _contents;
public:
	// have to allow public default-construction because the top-level box isn't tied to content
	box_dynamic(bool bootstrap = false) : box(bootstrap) {}
	ZAIMONI_DEFAULT_COPY_ASSIGN(box_dynamic);
	~box_dynamic();

	// content management
	void append(std::shared_ptr<box> src);
	virtual void remove(std::shared_ptr<box> gone);

	virtual void draw() const;
	virtual void screen_coords(std::pair<int, int> logical_origin);
	virtual void force_size(int w, int h);
protected:
	virtual void schedule_reflow();
private:
	virtual bool flush();
	virtual int need_recalc() const;
	virtual void _recalc(int code);
};

}	// namespace css

#endif
