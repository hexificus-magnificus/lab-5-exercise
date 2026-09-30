#include <iostream>
#include <cmath>
#include <optional>
#include <vector>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;

using Point2D = sf::Vector2f;

void drawLine(Point2D from, Point2D to, float width, sf::Color c, sf::RenderWindow *w) {
	Point2D btwn = to - from;
	float m = btwn.y/btwn.x;
	float perpm = -1/m;
	Point2D perp(1, perpm);
	perp = perp.normalized();
	perp *= width;
	// winding is not set cw/ccw, but will be constant within each shape
 	Point2D c0 = from + perp;
	Point2D c1 = from - perp;
	Point2D c2 = to - perp;
	Point2D c3 = to + perp;
	sf::ConvexShape l;
	l.setPointCount(4);
	l.setPoint(0, {c0.x, c0.y});
	l.setPoint(1, {c1.x, c1.y});
	l.setPoint(2, {c2.x, c2.y});
	l.setPoint(3, {c3.x, c3.y});
	l.setFillColor(c);
	w->draw(l);
}

void drawCircle(Point2D pos, float r, sf::RenderWindow *w) {
	sf::CircleShape c(r);
	c.setFillColor(sf::Color(255, 0, 0));
	c.setPosition({pos.x - r, pos.y - r});
	w->draw(c);
}

void drawSquare(Point2D p, float s, float rot, sf::RenderWindow *w) {
	sf::RectangleShape r({s, s});
	r.setOrigin({s/2, s/2});
	r.setFillColor(sf::Color(200, 200, 200));
	r.setPosition({p.x, p.y});
	r.setRotation(sf::radians(rot));
	w->draw(r);
}

int binom(int n, int k) {
	if (k == 0)
		return 1;
	int sol = n-k+1;
	for (int i = 1; i < k; ++i) {
		sol = sol * (n - k + i + 1) / (i + 1);
	}
	return sol;
}

float bernstein(int i, int n, float t) {
	return binom(n, i) * std::pow(t, i) * std::pow(1-t, n-i);
}

Point2D getPoint(const std::vector<sf::Vector2f>& pts, float t) {
	int segcount = (pts.size()-1) / 3;
	int seg = 0;
	if (t < 1) {
		seg = (int)(t*segcount);
		t = t*segcount - seg;
	} else {
		seg = segcount-1;
	}
	int start = seg*3;
	float x, y;
	x = y = 0;
	for (int i = 0; i < 4; i++) {
		float c = bernstein(i, 3, t);
		x += c * pts[start+i].x;
		y += c * pts[start+i].y;
	}
	return {x, y};
}

// TODO: (Part 2) Define a function that returns the curve's slope at t in [0, 1].
Point2D getSlope(const std::vector<sf::Vector2f>& pts, float t) {
	int segcount = (pts.size()-1) / 3;
	int seg = 0;
	if (t < 1) {
		seg = (int)(t*segcount);
		t = t*segcount - seg;
	} else {
		seg = segcount-1;
	}
	int start = seg*3;
	float x = 0;
	float y = 0;
	for (int i = 0; i < 3; i++) {
		sf::Vector2f d = pts[start+i+1] - pts[start+i];
		float c = bernstein(i, 2, t);
		x += c*d.x;
		y += c*d.y;
	}
	x *= 3;
	y *= 3;
	return Point2D{x, y};
}

// TODO: (Part 1) Store four control points for the curve.
std::vector<Point2D> curve(4);
// TODO: (Part 2) Track animation time for the square moving along the curve.
float t = 0;
// TODO: (Part 3) Track the index of the control point being dragged.
int dragPt = -1;

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            // TODO: (Part 3) On left-click, select the closest control point
            // using mouse->position and start dragging it.
	    dragPt = 0;
	    float closel = ((sf::Vector2f)mouse->position - (sf::Vector2f)curve[0]).length();
	    for (int i = 0; i < curve.size(); i++) {
		float l = ((Point2D)mouse->position - curve[i]).length();
	    	if (l < closel) {
				closel = l;
				dragPt = i;
			}
	    }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
            // TODO: (Part 3) On left-button release, stop dragging.
	    	dragPt = -1;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>()) {
            // TODO: (Part 3) Move the selected control point to mouse->position.
            // TODO: (Part 4) Maintain matching slopes at shared endpoints.
            // When moving point 3, move point 5 without changing its distance
            // from point 4 (point numbers here start at 1).
			auto newP = (Point2D)mouse->position;
			if (dragPt >= 0) {
				if (dragPt % 3 == 0) {
					if (dragPt > 0) {
						Point2D d = curve[dragPt-1] - curve[dragPt];
						curve[dragPt-1] = newP + d;
					}
					if (dragPt < curve.size()-1) {
						Point2D d = curve[dragPt+1] - curve[dragPt];
						curve[dragPt+1] = newP + d;
					}
				} else if (dragPt % 3 == 1) {
					int other = dragPt - 2;
					if (other > 0) {
						Point2D d = curve[dragPt - 1] - curve[dragPt];
						float l = (curve[other] - curve[dragPt - 1]).length();
						curve[other] = curve[dragPt-1] + l * d.normalized();
					}
				} else {
					int other = dragPt + 2;
					if (other < curve.size()) {
						Point2D d = curve[dragPt + 1] - curve[dragPt];
						float l = (curve[dragPt + 1] - curve[other]).length();
						curve[other] = curve[dragPt+1] + l * d.normalized();
					}
				}
				curve[dragPt] = newP;
			}
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TODO: (Part 4) '+' adds three control points; '-' removes three,
            // keeping at least four points.
			if (key->scancode == sf::Keyboard::Scan::Equal) {
				Point2D d = curve[curve.size()-1] - curve[curve.size()-2];
				curve.push_back(d + curve.back());
				curve.push_back(getPoint(curve, 1.2));
				curve.push_back(getPoint(curve, 1.3));
			} else if (key->scancode == sf::Keyboard::Scan::Hyphen && curve.size() > 4) {
				curve.pop_back();
				curve.pop_back();
				curve.pop_back();
			}
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    // ====== ====== ======
    // TODO: (Part 1) Sample GetPoint over t in [0, 1] and connect samples using the line-drawing
    // code from your project. Draw all four control points as circles after drawing the curve.
    // ====== ====== ======
    int sampleRes = 100;
    std::vector<Point2D> sample(sampleRes);
    sample[0] = getPoint(curve, 0);
    for (int i = 1; i < sampleRes; i++) {
	float pct = (float)i/(sampleRes-1);
    	sample[i] = getPoint(curve, pct);
		drawLine(sample[i-1], sample[i], 1, sf::Color(50, 50, 255), &window);
    }
    for (auto pt : curve) {
    	drawCircle(pt, 5, &window);
    }

    // ====== ====== ======
    // TODO: (Part 2) Draw a small square moving repeatedly along the curve.
    // Use GetSlope to orient it to the curve at each time step.
    // ====== ====== ======
    Point2D heading = getSlope(curve, t);
    drawSquare(getPoint(curve, t), 30, std::atan2(heading.y, heading.x), &window);

    // ====== ====== ======
    // TODO: (Part 3) Draw control handles from point 1 to 2 and point 3 to 4.
    // TODO: (Part 4) Draw all connected cubic Bezier segments and their handles.
    // ====== ====== ======
    for (int i = 0; i < curve.size(); ++i) {
        if (i%3 == 0) {
			if (i-1 > 0)
				drawLine(curve[i-1], curve[i], 2, sf::Color(50, 255, 50), &window);
			if (i+1 < curve.size())
				drawLine(curve[i+1], curve[i], 2, sf::Color(50, 255, 50), &window);
		}
    }

    // ====== ====== ======
    // TODO: (Bonus) Support multiple curves, a Galaga screen overlay at a 1:2 ratio, and exporting
    // curve points as C++ code for Project 1b.
    // ====== ====== ======

    window.display();
}

int main() {
    sf::RenderWindow window;

    try {
        // Initialize window
        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Bezier Curve Editor");
        window.setFramerateLimit(FPS_LIMIT);
        // Prevent key repeats.
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;
        // Main game loop
	curve[0] = {WINDOW_WIDTH/3, WINDOW_WIDTH/2};
	curve[1] = {WINDOW_WIDTH/3, WINDOW_WIDTH/4};
	curve[2] = {2 * WINDOW_WIDTH/3, WINDOW_WIDTH/4};
	curve[3] = {2 * WINDOW_WIDTH/3, WINDOW_WIDTH/2};
        while (window.isOpen()) {
	    t += .75f/FPS_LIMIT;
	    while (t >= 1)
		    t -= 1;
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
