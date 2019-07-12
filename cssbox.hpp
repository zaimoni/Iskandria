#ifndef CSSBOX_HPP
#define CSSBOX_HPP 1

#include <utility>
#include <memory>
#include <vector>

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
private:
	enum {
		REFLOW = HEIGHT + 1
	};

	std::pair<int, int> _origin;	// (x,y) i.e. (left,top)
	std::pair<int, int> _size;
	std::pair<int, int> _size_min;
	std::pair<int, int> _size_max;
	int _margin[4];
	int _padding[4];
	unsigned char _auto;	// bitmap
	std::weak_ptr<box> _parent;
	std::shared_ptr<box> _self;
	std::vector<std::shared_ptr<box> > _contents;
protected:
	box(const box& src) = default;
	box(box&& src) = default;
	box& operator=(const box& src) = default;
	box& operator=(box&& src) = default;
public:
	// have to allow public default-construction because the top-level box isn't tied to content
	box(bool bootstrap = false);
	virtual ~box() = default;

	// auto values
	bool is_auto(auto_legal src) const { return _auto & (1ULL << src); }
	void set_auto(auto_legal src);

	// width/height
	std::pair<int, int> size() const { return _size; }
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

	// content management
	void append(std::shared_ptr<box>& src);
};

}	// namespace css

#endif
