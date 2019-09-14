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
protected:
	enum {
		REFLOW = HEIGHT + 1
	};

	unsigned char _auto;	// bitmap
	unsigned char _auto_recalc;	// margin or height/width
private:
	std::pair<int, int> _origin;	// (x,y) i.e. (left,top)
	std::pair<int, int> _size;
	std::pair<int, int> _size_min;
	std::pair<int, int> _size_max;
	int _margin[4];
	int _padding[4];
	std::weak_ptr<box> _parent;
	std::shared_ptr<box> _self;
protected:
	box(bool bootstrap = false);
	box(const box& src) = default;
	box(box&& src) = default;
	box& operator=(const box& src) = default;
	box& operator=(box&& src) = default;
public:
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

protected:
	void set_parent(std::shared_ptr<box>& src) {
		if (src) {
			src->_parent = _self;
			src->_self = src;
		}
	}

	void _width(int w);
	void _height(int h);
};

class box_dynamic : public box
{
private:
	std::vector<std::shared_ptr<box> > _contents;
public:
	// have to allow public default-construction because the top-level box isn't tied to content
	box_dynamic(bool bootstrap = false) : box(bootstrap) {}
	box_dynamic(const box_dynamic& src) = default;
	box_dynamic(box_dynamic&& src) = default;
	box_dynamic& operator=(const box_dynamic& src) = default;
	box_dynamic& operator=(box_dynamic&& src) = default;
	~box_dynamic() = default;

	// content management
	void append(std::shared_ptr<box>& src);
};

}	// namespace css

#endif
