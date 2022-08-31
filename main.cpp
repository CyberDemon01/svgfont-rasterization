#include <bits/stdc++.h>
#include <SDL2/SDL.h>
#include <pugixml.hpp>

using namespace std;

template <typename T>
inline auto sqr(T x) { return x * x; }

constexpr float max_deviation = 0.1f;

template <typename T>
struct vec2 {
	struct {
		union {T x; T s; };
		union {T y; T t; };
	};
	vec2() = default;
	vec2(T _x, T _y) : x(_x), y(_y) {}
	template <typename U>
	vec2(const vec2<U>& _v) : x(_v.x), y(_v.y) {}
	friend auto operator+(const vec2& a, const vec2& b) {
		return vec2(a.x + b.x, a.y + b.y);
	}
	friend auto operator-(const vec2& a, const vec2& b) {
		return vec2(a.x - b.x, a.y - b.y);
	}
	friend auto operator*(const vec2& a, T value) {
		return vec2(a.x * value, a.y * value);
	}
};

using vec2i = vec2<int>;
using vec2f = vec2<float>;

template <typename T>
struct vec3 {
	struct {
		union {T x; T r; };
		union {T y; T g; };
		union {T z; T b; };
	};
	vec3() = default;
	vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
	template <typename U>
	vec3(const vec3<U>& o) : x(o.x), y(o.y), z(o.z) {}
	template <typename U>
	void operator=(const vec3<U>& o) {
		x = static_cast<T>(o.x);
		y = static_cast<T>(o.y);
		z = static_cast<T>(o.z);
	}
	template <typename U>
	friend auto operator+(const vec3<T>& a, const vec3<U>& b) {
		return vec3<decltype(T() + U())>(a.x + b.x, a.y + b.y, a.z + b.z);
	}
	friend auto operator-(const vec3& a, const vec3& b) {
		return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
	}
	template <typename U>
	friend auto operator*(const vec3& a, U value) {
		return vec3<decltype(T() * U())>(a.x * value, a.y * value, a.z * value);
	}
};

using vec3b = vec3<unsigned char>;
using vec3f = vec3<float>;
using vec3i = vec3<int>;

namespace colors {
	vec3i red = {255, 0, 0};
	vec3i green = {0, 255, 0};
	vec3i blue = {0, 0, 255};
	vec3i yellow = {255, 255, 0};
	vec3i pink = {255, 0, 255};
	vec3i cayn = {0, 255, 255};
	vec3i black = {0, 0, 0};
	vec3i white = {255, 255, 255};
}

template <typename T>
struct matrix {
	vector<vector<T>> a = {};
	int n = {};
	int m = {};
	matrix() = default;
	matrix(int _n, int _m) : a(_n, vector<T>(_m, T{})), n(_n), m(_m) {}
	auto& operator[](int i) {
		return a[i];
	}
};

template <typename T>
struct box2D {
	static constexpr T inf = numeric_limits<T>::max();
	static constexpr T m_inf = numeric_limits<T>::min();
	vec2<T> v0 = {inf, inf};
	vec2<T> v1 = {m_inf, m_inf};
	box2D() = default;
	box2D(vec2<T> _v0, vec2<T> _v1) : v0(_v0), v1(_v1) {}
};

auto bounding_box(vec2i a, vec2i b, vec2i c) {
	box2D<int> box = {
		{min({a.x, b.x, c.x}), min({a.y, b.y, c.y})},
		{max({a.x, b.x, c.x}), max({a.y, b.y, c.y})},
	};
	return box;
}

struct sdl2_render {
	SDL_Window* window = {};
	SDL_Renderer* renderer = {};
	int w = {};
	int h = {};
	sdl2_render() = default;
	sdl2_render(const string& t, int _w, int _h) : w(_w), h(_h) {
		assert(!SDL_Init(SDL_INIT_EVERYTHING));
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
		window = SDL_CreateWindow(t.c_str(),
															SDL_WINDOWPOS_UNDEFINED,
															SDL_WINDOWPOS_UNDEFINED,
															w,
															h,
															SDL_WINDOW_SHOWN);
		assert(window != nullptr);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		assert(renderer != nullptr);
	}
	void clear() {
		SDL_RenderClear(renderer);
	}
	void clear(vec3i col) {
		set_color(col);
		SDL_RenderClear(renderer);
	}
	void present() {
		SDL_RenderPresent(renderer);
	}
	void set_color(int r, int g, int b) {
		SDL_SetRenderDrawColor(renderer, r, g, b, 255);
	}
	void set_color(vec3i col) {
		SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
	}
	void paint_pixel(int x, int y) {
		SDL_RenderDrawPoint(renderer, x, y);
	}
	void paint_pixel(float x, float y) {
		SDL_RenderDrawPointF(renderer, x, y);
	}
	void paint_pixel(vec2f p) {
		SDL_RenderDrawPointF(renderer, p.x, p.y);
	}
	void paint_line(vec2f a, vec2f b) {
		SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
	}
	~sdl2_render() {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
};

void set_line(vector<pair<vec2f, vec2f>>& lines,
							const vec2f& a,
							const vec2f& b,
							float scale) {
	lines.push_back({a * scale, b * scale});
}

void set_bezier_curve(vector<pair<vec2f, vec2f>>& lines,
											const vec2f& p0,
											const vec2f& p1,
											const vec2f& p2,
											float scale) {
	auto bezier_point = [&p0, &p1, &p2](float t) {
		auto q0 = p0 + (p1 - p0) * t;
		auto q1 = p1 + (p2 - p1) * t;
		return q0 + (q1 - q0) * t;
	};
	int segments = 1.0f / max_deviation;
	vector<vec2f> curve_points = {};
	for (int i = 0; i <= segments; i++) {
		float t = static_cast<float>(i) / segments;
		auto p = bezier_point(t);
		curve_points.push_back(p);
	}
	for (size_t i = 0; i + 1 < curve_points.size(); i++) {
		set_line(lines, curve_points[i], curve_points[i + 1], scale);
	}
}

void set_bezier_cube_curve(vector<pair<vec2f, vec2f>>& lines,
														vec2f p0,
														vec2f p1,
														vec2f p2,
														vec2f p3,
														float scale) {
	auto bezier_point = [&p0, &p1, &p2, &p3](float t) {
		auto q0 = p0 + (p1 - p0) * t;
		auto q1 = p1 + (p2 - p1) * t;
		auto q2 = p2 + (p3 - p2) * t;
		auto c0 = q0 + (q1 - q0) * t;
		auto c1 = q1 + (q2 - q1) * t;
		return c0 + (c1 - c0) * t;
	};
	int segments = 1.0f / max_deviation;
	vector<vec2f> curve_points = {};
	for (int i = 0; i <= segments; i++) {
		float t = static_cast<float>(i) / segments;
		auto p = bezier_point(t);
		curve_points.push_back(p);
	}
	for (size_t i = 0; i + 1 < curve_points.size(); i++) {
		set_line(lines, curve_points[i], curve_points[i + 1], scale);
	}
}

bool is_polygone_point(const vector<pair<vec2f, vec2f>>& outlines, vec2f v) {
	int cnt = {};
	for (auto [a, b] : outlines) {
		if (a.y > b.y) swap(a, b);
		if (a.y < v.y && b.y >= v.y) {
			auto p = a + (b - a) * ((v.y - a.y) / (b.y - a.y));
			cnt += p.x >= v.x;
		}
	}
	return cnt & 1;
}

int main() {
	ios_base::sync_with_stdio(false);
	cin.tie(nullptr);
	cout.tie(nullptr);

	string xml_file_path = "Consolas.svg";
	pugi::xml_document doc = {};
	pugi::xml_parse_result result = doc.load_file(xml_file_path.c_str());

	if (!result) {
		cerr << "fail to load [" << xml_file_path << "]" << endl;
		return -1;
	}

	auto font = doc.child("svg").child("defs").child("font");
	cout << "Font: " << font.attribute("id").as_string() << endl;
	// font face
	auto font_face = font.child("font-face");
	for (auto ai = font_face.attributes_begin(); ai != font_face.attributes_end(); ai++) {
		cout << ai->name() << ": " << ai->value() << endl;
	}

	int units_per_em = font.child("font-face").attribute("units-per-em").as_int();
	int ascent = font.child("font-face").attribute("ascent").as_int();
	int descent = font.child("font-face").attribute("descent").as_int();

	struct m_args {float x = {}; float y = {};};
	struct v_args {float y = {};};
	struct h_args {float x = {};};
	struct q_args {float xc = {}; float yc = {}; float x = {}; float y = {};};
	struct t_args {float x = {}; float y = {};};
	struct l_args {float x = {}; float y = {};};
	struct c_args {float xc1 = {}; float yc1 = {}; float xc2 = {}; float yc2 = {}; float x = {}; float y = {};};
	struct s_args {float xc = {}; float yc = {}; float x = {}; float y = {};};
	struct z_args {};

	struct arguments {
		union {
			m_args m;
			v_args v;
			h_args h;
			q_args q;
			t_args t;
			l_args l;
			c_args c;
			s_args s;
			z_args z;
		};
		arguments() = default;
		arguments(m_args _m) : m(_m) {}
		arguments(v_args _v) : v(_v) {}
		arguments(h_args _h) : h(_h) {}
		arguments(q_args _q) : q(_q) {}
		arguments(t_args _t) : t(_t) {}
		arguments(l_args _l) : l(_l) {}
		arguments(c_args _c) : c(_c) {}
		arguments(s_args _s) : s(_s) {}
		arguments(z_args _z) : z(_z) {}
	};

	struct command {
		char type = {};
		arguments args;
		command(char t, m_args a) : type(t), args(a) {}
		command(char t, v_args a) : type(t), args(a) {}
		command(char t, h_args a) : type(t), args(a) {}
		command(char t, q_args a) : type(t), args(a) {}
		command(char t, t_args a) : type(t), args(a) {}
		command(char t, l_args a) : type(t), args(a) {}
		command(char t, c_args a) : type(t), args(a) {}
		command(char t, s_args a) : type(t), args(a) {}
		command(char t, z_args a) : type(t), args(a) {}
	};

	unordered_map<char, vector<command>> ch_paths = {};

	for (auto path = font.child("glyph"); path; path = path.next_sibling("glyph")) {
		string unicode = path.attribute("unicode").as_string();
		if (unicode.size() == 1 && (isprint(unicode[0]) || isspace(unicode[0]))) {
			char ch = unicode[0];
			string d = path.attribute("d").as_string();
			stringstream stream(d);
			vector<command> commands = {};
			while (stream) {
				char ch = {};
				stream >> ch;
				if (ch == 'M' || ch == 'm') {
					float x = {};
					float y = {};
					stream >> x >> y;
					commands.push_back({ch, m_args{x, y}});
				}
				else if (ch == 'V' || ch == 'v') {
					float y = {};
					stream >> y;
					commands.push_back({ch, v_args{y}});
				}
				else if (ch == 'H' || ch == 'h') {
					float x = {};
					stream >> x;
					commands.push_back({ch, h_args{x}});
				}
				else if (ch == 'Q' || ch == 'q') {
					float xc = {};
					float yc = {};
					float x = {};
					float y = {};
					stream >> xc >> yc >> x >> y;
					commands.push_back({ch, q_args{xc, yc, x, y}});
				}
				else if (ch == 'T' || ch == 't') {
					float x = {};
					float y = {};
					stream >> x >> y;
					commands.push_back({ch, t_args{x, y}});
				}
				else if (ch == 'L' || ch == 'l') {
					float x = {};
					float y = {};
					stream >> x >> y;
					commands.push_back({ch, l_args{x, y}});
				}
				else if (ch == 'C' || ch == 'c') {
					float xc1 = {}, yc1 = {};
					float xc2 = {}, yc2 = {};
					float x = {}, y = {};
					stream >> xc1 >> yc1 >> xc2 >> yc2 >> x >> y;
					commands.push_back({ch, c_args{xc1, yc1, xc2, yc2, x, y}});
				}
				else if (ch == 'S' || ch == 's') {
					float xc = {}, yc = {};
					float x = {}, y = {};
					stream >> xc >> yc >> x >> y;
					commands.push_back({ch, s_args{xc, yc, x, y}});
				}
				else if (ch == 'Z' || ch == 'z') {
					commands.push_back({ch, z_args{}});
				}
				else if (ch) {
					cout << "[FAIL] unknow command " << ch << "(" << int(ch) << ")" << endl;
					exit(-1);
				}
			}
			ch_paths[ch] = move(commands);
		}
	}
	cout << "commands done" << endl;

	// rasterization
	auto gbuf = [&](char ch, int h, vec3f col) {
		const float aa_scale_factor = 3.0f;
		const float scale = aa_scale_factor * h / static_cast<float>(units_per_em);

		vec2f start = {};

		float x_prev = {};
		float y_prev = {};
		float xc_prev = {};
		float yc_prev = {};

		vector<pair<vec2f, vec2f>> outlines = {};

		for (const auto& [t, arg] : ch_paths[ch]) {
			if (t == 'M') {
				x_prev = arg.m.x;
				y_prev = arg.m.y;
				start = vec2f{arg.m.x, arg.m.y};
			}
			else if (t == 'm') {
				x_prev += arg.m.x;
				y_prev += arg.m.y;
			}
			else if (t == 'V') {
				set_line(outlines, {x_prev, y_prev}, {x_prev, arg.v.y}, scale);
				y_prev = arg.v.y;
			}
			else if (t == 'v') {
				set_line(outlines, {x_prev, y_prev}, {x_prev, y_prev + arg.v.y}, scale);
				y_prev += arg.v.y;
			}
			else if (t == 'H') {
				set_line(outlines, {x_prev, y_prev}, {arg.h.x, y_prev}, scale);
				x_prev = arg.h.x;
			}
			else if (t == 'h') {
				set_line(outlines, {x_prev, y_prev}, {arg.h.x + x_prev, y_prev}, scale);
				x_prev += arg.h.x;
			}
			else if (t == 'L') {
				set_line(outlines, {x_prev, y_prev}, {arg.l.x, arg.l.y}, scale);
				x_prev = arg.l.x;
				y_prev = arg.l.y;
			}
			else if (t == 'l') {
				set_line(outlines, {x_prev, y_prev}, {arg.l.x + x_prev, arg.l.y + y_prev}, scale);
				x_prev += arg.l.x;
				y_prev += arg.l.y;
			}
			else if (t == 'Q') {
				set_bezier_curve(
					outlines,
					{x_prev, y_prev},
					{arg.q.xc, arg.q.yc},
					{arg.q.x, arg.q.y},
					scale
				);
				x_prev = arg.q.x;
				y_prev = arg.q.y;
				xc_prev = arg.q.xc;
				yc_prev = arg.q.yc;
			}
			else if (t == 'q') {
				set_bezier_curve(
					outlines,
					{x_prev, y_prev},
					{arg.q.xc + x_prev, arg.q.yc + y_prev},
					{arg.q.x + x_prev, arg.q.y + y_prev},
					scale
				);
				xc_prev = arg.q.xc + x_prev;
				yc_prev = arg.q.yc + y_prev;
				x_prev += arg.q.x;
				y_prev += arg.q.y;
			}
			else if (t == 'T') {
				set_bezier_curve(
					outlines,
					{x_prev, y_prev},
					{2 * x_prev - xc_prev, 2 * y_prev - yc_prev},
					{arg.t.x, arg.t.y},
					scale
				);
				xc_prev = 2 * x_prev - xc_prev;
				yc_prev = 2 * y_prev - yc_prev;
				x_prev = arg.t.x;
				y_prev = arg.t.y;
			}
			else if (t == 't') {
				set_bezier_curve(
					outlines,
					{x_prev, y_prev},
					{2 * x_prev - xc_prev, 2 * y_prev - yc_prev},
					{arg.t.x + x_prev, arg.t.y + y_prev},
					scale
				);
				xc_prev = 2 * x_prev - xc_prev;
				yc_prev = 2 * y_prev - yc_prev;
				x_prev += arg.t.x;
				y_prev += arg.t.y;
			}
			else if (t == 'C') {
				set_bezier_cube_curve(
					outlines,
					{x_prev, y_prev},
					{arg.c.xc1, arg.c.yc1},
					{arg.c.xc2, arg.c.yc2},
					{arg.c.x, arg.c.y},
					scale
				);
				xc_prev = arg.c.xc2;
				yc_prev = arg.c.yc2;
				x_prev = arg.c.x;
				y_prev = arg.c.y;
			}
			else if (t == 'c') {
				set_bezier_cube_curve(
					outlines,
					{x_prev, y_prev},
					{arg.c.xc1 + x_prev, arg.c.yc1 + y_prev},
					{arg.c.xc2 + x_prev, arg.c.yc2 + y_prev},
					{arg.c.x + x_prev, arg.c.y + y_prev},
					scale
				);
				xc_prev = arg.c.xc2 + x_prev;
				yc_prev = arg.c.yc2 + y_prev;
				x_prev += arg.c.x;
				y_prev += arg.c.y;
			}
			else if (t == 'S') {
				set_bezier_cube_curve(
					outlines,
					{x_prev, y_prev},
					{2 * x_prev - xc_prev, 2 * y_prev - yc_prev},
					{arg.s.xc, arg.s.yc},
					{arg.s.x, arg.s.y},
					scale
				);
				xc_prev = arg.s.xc;
				yc_prev = arg.s.yc;
				x_prev = arg.s.x;
				y_prev = arg.s.y;
			}
			else if (t == 's') {
				set_bezier_cube_curve(
					outlines,
					{x_prev, y_prev},
					{2 * x_prev - xc_prev, 2 * y_prev - yc_prev},
					{arg.s.xc + x_prev, arg.s.yc + y_prev},
					{arg.s.x + x_prev, arg.s.y + y_prev},
					scale
				);
				xc_prev = arg.s.xc + x_prev;
				yc_prev = arg.s.yc + y_prev;
				x_prev += arg.s.x;
				y_prev += arg.s.y;
			}
			else if (t == 'Z' || t == 'z') {
				set_line(outlines, {x_prev, y_prev}, start, scale);
			}
		}

		matrix<float> landscape(units_per_em * scale, units_per_em * scale);
		// for (int y = descent * scale; y <= ascent * scale; y++) {
		// 	for (int x = 0; x < units_per_em * scale; x++) {
		// 		if (is_polygone_point(outlines, {x, y})) {
		// 			landscape[units_per_em * scale - y + descent * scale][x] = 1.0f;
		// 		}
		// 	}
		// }

		vector<vector<float>> yxs(units_per_em * scale);
		for (int y = descent * scale; y <= ascent * scale; y++) {
			for (auto [p0, p1] : outlines) {
				if (p0.y > p1.y) swap(p0, p1);
				if (p0.y < y && p1.y >= y) {
					auto px = p0.x + (p1.x - p0.x) * ((y - p0.y) / (p1.y - p0.y));
					yxs[units_per_em * scale - y + descent * scale].push_back(px);
				}
			}
		}
		for (auto& xs : yxs)
			sort(xs.begin(), xs.end());

		for (int y = 0; y < units_per_em * scale; y++) {
			for (size_t i = 0; i + 1 < yxs[y].size(); i += 2) {
				int x0 = max(yxs[y][i], 0.0f);
				int x1 = min(yxs[y][i + 1], units_per_em * scale);
				for (int x = x0; x <= x1; x++) {
					landscape[y][x] = 1.0f;
				}
			}
		}

		matrix<float> ctb(units_per_em * scale, units_per_em * scale);
		for (int y = 0; y < units_per_em * scale; y++) {
			for (int x = 2; x + 2 < units_per_em * scale; x++) {
				ctb[y][x] = landscape[y][x - 2] * (2.0f / 9.0f) + 
										landscape[y][x - 1] * (1.0f / 9.0f) + 
										landscape[y][x]     * (3.0f / 9.0f) + 
										landscape[y][x + 1] * (1.0f / 9.0f) + 
										landscape[y][x + 2] * (2.0f / 9.0f);
			}
		}

		const vector<int> dy = {-1,  0,  1};
		matrix<float> blur(units_per_em * scale / aa_scale_factor, units_per_em * scale);
		for (int y = 0; y < units_per_em * scale; y++) {
			for (int x = 0; x < units_per_em * scale; x++) {
				float interp = {};
				for (int i = 0; i < 3; i++) {
					int yy = y + dy[i];
					if (yy >= 0 && yy < units_per_em * scale)
						interp += ctb[yy][x] / 3.0f;
				}
				blur[y / aa_scale_factor][x] = interp;
			}
		}

		vector<vec3b> buf(h * h);

		for (int y = 0; y < units_per_em * scale / aa_scale_factor; y++) {
			for (int x = 0; x + 2 < units_per_em * scale; x++) {
				buf[y * h + x / aa_scale_factor] = vec3b(
					blur[y][x + 0] * col.r,
					blur[y][x + 1] * col.g,
					blur[y][x + 2] * col.b
				);
			}
		}
		return buf;
	};

	const int h = 48;

	int width = 1270;
	int height = 720;

	sdl2_render render("Font Rasterization", width, height);

	auto texture = SDL_CreateTexture(
		render.renderer,
		SDL_PIXELFORMAT_RGB24,
		SDL_TEXTUREACCESS_STREAMING,
		h,
		h
	);
	assert(texture);

	unordered_map<char, vector<vec3b>> char_info = {};
	for (int ch = 0; ch < 256; ch++) {
		if (isprint(ch)) {
			if (isdigit(ch))
				char_info[ch] = move(gbuf(ch, h, {255, 255, 0}));
			else
				char_info[ch] = move(gbuf(ch, h, {255, 255, 255}));
		}
	}

	vector<string> text = {
		"func heapify(a[], i, size) {",
		"  ls = (i << 1) + 1",
		"  rs = (i << 1) + 2",
		"  while (ls < size) {",
		"    k = ls",
		"    if (rs < size and a[rs] > a[ls])",
		"      k = rs",
		"    if (a[i] >= a[k])",
		"      break",
		"    swap(a[i], a[k])",
		"    i = k",
		"    ls = (i << 1) + 1",
		"    rs = (i << 1) + 2",
		"  }",
		"}",
	};

	render.clear(colors::black);
	int y = 10;
	for (const auto &s : text) {
		int x = 10;
		for (const auto& ch : s) {
			SDL_UpdateTexture(texture, nullptr, char_info[ch].data(), h * sizeof(vec3b));
			SDL_Rect srect = {x, y, h, h};
			SDL_RenderCopy(render.renderer, texture, nullptr, &srect);
			x += h * 0.6f;
		}
		y += h;
	}

	render.present();

	SDL_Event e = {};
	bool run = true;

	while (run) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT: run = false; break;
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) {
				case SDLK_ESCAPE: run = false; break;
				}
				break;
			}
		}
	}

	return 0;
}
