// Class automatically generated by Dev-C++ New Class wizard

#ifndef utils_MyPoint_h
#define utils_MyPoint_h

namespace utils {

/**
 * No description
 */
class MyPoint
{
	public:
		// class constructor
		MyPoint(double x, double y);
		// class destructor
		~MyPoint();
		
		double getX();
		double getY();
		void add(double x, double y);
		void setX(double x);
		void setY(double y);
		
	private:
		double x;
		double y;

};

}
#endif // utils_MyPoint_h
