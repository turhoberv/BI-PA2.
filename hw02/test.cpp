#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <compare>
#include <iostream>
#endif /* __PROGTEST__ */

//=================================================================================================
// date_format manipulator - a dummy implementation. Keep this code unless you implement your
// own working manipulator.
std::ios_base& (*date_format(const char* fmt)) (std::ios_base& x)
{
  return [](std::ios_base& ios) -> std::ios_base& { return ios; };
}

class InvalidDateException : public std::exception {
};
//=================================================================================================
class CDate
{
public:
  static bool isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
  }
  static int daysInMonth(int y, int m) {
    switch (m) {
    case 1: return 31;
    case 2: {
      if (isLeapYear(y))return 29;
      else return 28;
    }
    case 3: return 31;
    case 4: return 30;
    case 5: return 31;
    case 6: return 30;
    case 7: return 31;
    case 8: return 31;
    case 9: return 30;
    case 10: return 31;
    case 11: return 30;
    case 12: return 31;
    default: return 0;
    }
  }

  static bool IsValid(int y, int m, int d) {
    return (y >= 2000 && y <= 2030 && m >= 1 && m <= 12 && d >= 1 && d <= daysInMonth(y, m));
  }

  CDate(int y, int m, int d) {
    if (IsValid(y, m, d)) {
      this->day = d;
      this->month = m;
      this->year = y;
    }
    else throw InvalidDateException();
  }
  CDate(const CDate& d) {
    this->day = d.day;
    this->month = d.month;
    this->year = d.year;
  }


  CDate operator + (int days) const {
    CDate res(*this);
    bool sub = days < 0;
    if (days < 0) days = days * (-1);
    for (int i = 0; i < days; i++) {
      if (IsValid(res.year, res.month, sub ? res.day - 1 : res.day + 1)) {
        res.day += sub ? -1 : 1;
      }
      else if (IsValid(res.year, sub ? res.month - 1 : res.month + 1, sub ? daysInMonth(res.year, res.month - 1) : 1)) {
        res.month += sub ? -1 : 1;
        res.day = sub ? daysInMonth(res.year, res.month) : 1;
      }
      else {
        res.year += sub ? -1 : 1;
        res.month = sub ? 12 : 1;
        res.day = sub ? daysInMonth(res.year, res.month) : 1;
      }
    }
    return res;
  }

  CDate operator - (int days) const {
    CDate res(*this);
    res = res + (-days);
    return res;
  }

  int operator - (const CDate& other) const {
    int counter = 0;
    if (*this == other) return 0;
    if (*this < other) {
      CDate res(*this);
      while (res < other) {
        counter++;
        res = res + 1;
      }
      return -counter;
    }
    else {
      CDate res(other);
      while (res < *this) {
        counter++;
        res = res + 1;
      }
      return counter;

    }

  }

  CDate operator ++ (int) {
    CDate res(*this);
    *this = *this + 1;
    return res;
  }

  CDate operator -- (int) {
    CDate res(*this);
    *this = *this - 1;
    return res;
  }

  CDate operator ++ () {
    *this = *this + 1;
    return *this;
  }

  CDate operator -- () {
    *this = *this - 1;
    return *this;
  }

  bool operator > (const CDate& other) const {
    if (this->year > other.year) return true;
    if (this->year < other.year) return false;
    if (this->month > other.month) return true;
    if (this->month < other.month) return false;
    return this->day > other.day;
  }

  bool operator >= (const CDate& other) const {
    return  *this > other || *this == other;
  }

  bool operator < (const CDate& other) const {
    if (this->year < other.year) return true;
    if (this->year > other.year) return false;
    if (this->month < other.month) return true;
    if (this->month > other.month) return false;
    return this->day < other.day;
  }



  bool operator <= (const CDate& other) const {
    return *this < other || *this == other;
  }

  bool operator == (const CDate& other) const {
    return (this->year == other.year && this->month == other.month && this->day == other.day);
  }
  bool operator != (const CDate& other) const {
    return (this->year != other.year || this->month != other.month || this->day != other.day);
  }

  friend std::ostream& operator << (std::ostream& os, const CDate& d);
  friend std::istream& operator >> (std::istream& is, CDate& d);



public:
  int day;
  int month;
  int year;
};

std::ostream& operator << (std::ostream& os, const CDate& d) {
  std::ostringstream ss;
  ss << std::setfill('0') << std::setw(4) << d.year << "-" << std::setfill('0') << std::setw(2) << d.month << "-" << std::setfill('0') << std::setw(2) << d.day;
  return os << ss.str();
}

std::istream& operator >> (std::istream& is, CDate& date) {
  int y, m, d;
  char c1, c2;

  if (!(is >> y >> c1 >> m >> c2 >> d)) return is;

  if (c1 != '-' || c2 != '-' || !CDate::IsValid(y, m, d)) {
    is.setstate(std::ios::failbit);
    return is;
  }
  date.year = y;
  date.month = m;
  date.day = d;

  return is;
}

#ifndef __PROGTEST__
int main()
{
  std::ostringstream oss;
  std::istringstream iss;

  CDate a(2000, 1, 2);
  CDate b(2010, 2, 3);
  CDate c(2004, 2, 10);
  oss.str("");
  oss << a;
  assert(oss.str() == "2000-01-02");
  oss.str("");
  oss << b;
  assert(oss.str() == "2010-02-03");
  oss.str("");
  oss << c;
  assert(oss.str() == "2004-02-10");
  a = a + 1500;
  oss.str("");
  oss << a;
  assert(oss.str() == "2004-02-10");
  b = b - 2000;
  oss.str("");
  oss << b;
  assert(oss.str() == "2004-08-13");
  assert(b - a == 185);
  assert((b == a) == false);
  assert((b != a) == true);
  assert((b <= a) == false);
  assert((b < a) == false);
  assert((b >= a) == true);
  assert((b > a) == true);
  assert((c == a) == true);
  assert((c != a) == false);
  assert((c <= a) == true);
  assert((c < a) == false);
  assert((c >= a) == true);
  assert((c > a) == false);
  a = ++c;
  oss.str("");
  oss << a << " " << c;
  assert(oss.str() == "2004-02-11 2004-02-11");
  a = --c;
  oss.str("");
  oss << a << " " << c;
  assert(oss.str() == "2004-02-10 2004-02-10");
  a = c++;
  oss.str("");
  oss << a << " " << c;
  assert(oss.str() == "2004-02-10 2004-02-11");
  a = c--;
  oss.str("");
  oss << a << " " << c;
  assert(oss.str() == "2004-02-11 2004-02-10");
  iss.clear();
  iss.str("2015-09-03");
  assert((iss >> a));
  oss.str("");
  oss << a;
  assert(oss.str() == "2015-09-03");
  a = a + 70;
  oss.str("");
  oss << a;
  assert(oss.str() == "2015-11-12");
  oss.str("");

  // std::cout << std::setw(20) << a;


  oss << std::setw(20) << a;
  assert(oss.str() == "          2015-11-12");

  CDate d(2000, 1, 1);
  try
  {
    CDate e(2000, 32, 1);
    assert("No exception thrown!" == nullptr);
  }
  catch (...)
  {
  }
  iss.clear();
  iss.str("2000-12-33");
  assert(!(iss >> d));
  oss.str("");
  oss << d;
  assert(oss.str() == "2000-01-01");
  iss.clear();
  iss.str("2000-11-31");
  assert(!(iss >> d));
  oss.str("");
  oss << d;
  assert(oss.str() == "2000-01-01");
  iss.clear();
  iss.str("2000-02-29");
  assert((iss >> d));
  oss.str("");
  oss << d;
  assert(oss.str() == "2000-02-29");
  iss.clear();
  iss.str("2001-02-29");
  assert(!(iss >> d));
  oss.str("");
  oss << d;
  assert(oss.str() == "2000-02-29");

  //-----------------------------------------------------------------------------
  // bonus test examples
  //-----------------------------------------------------------------------------

  /*
  CDate f(2000, 5, 12);
  oss.str("");
  oss << f;
  assert(oss.str() == "2000-05-12");
  oss.str("");
  oss << date_format("%Y/%m/%d") << f;
  assert(oss.str() == "2000/05/12");
  oss.str("");
  oss << date_format("%d.%m.%Y") << f;
  assert(oss.str() == "12.05.2000");
  oss.str("");
  oss << date_format("%m/%d/%Y") << f;
  assert(oss.str() == "05/12/2000");
  oss.str("");
  oss << date_format("%Y%m%d") << f;
  assert(oss.str() == "20000512");
  oss.str("");
  oss << date_format("hello kitty") << f;
  assert(oss.str() == "hello kitty");
  oss.str("");
  oss << date_format("%d%d%d%d%d%d%m%m%m%Y%Y%Y%%%%%%%%%%") << f;
  assert(oss.str() == "121212121212050505200020002000%%%%%");
  oss.str("");
  oss << date_format("%Y-%m-%d") << f;
  assert(oss.str() == "2000-05-12");
  iss.clear();
  iss.str("2001-01-1");
  assert(!(iss >> f));
  oss.str("");
  oss << f;
  assert(oss.str() == "2000-05-12");
  iss.clear();
  iss.str("2001-1-01");
  assert(!(iss >> f));
  oss.str("");
  oss << f;
  assert(oss.str() == "2000-05-12");
  iss.clear();
  iss.str("2001-001-01");
  assert(!(iss >> f));
  oss.str("");
  oss << f;
  assert(oss.str() == "2000-05-12");
  iss.clear();
  iss.str("2001-01-02");
  assert((iss >> date_format("%Y-%m-%d") >> f));
  oss.str("");
  oss << f;
  assert(oss.str() == "2001-01-02");
  iss.clear();
  iss.str("05.06.2003");
  assert((iss >> date_format("%d.%m.%Y") >> f));
  oss.str("");
  oss << f;
  assert(oss.str() == "2003-06-05");
  iss.clear();
  iss.str("07/08/2004");
  assert((iss >> date_format("%m/%d/%Y") >> f));
  oss.str("");
  oss << f;
  assert(oss.str() == "2004-07-08");
  iss.clear();
  iss.str("2002*03*04");
  assert((iss >> date_format("%Y*%m*%d") >> f));
  oss.str("");
  oss << f;
  assert(oss.str() == "2002-03-04");
  iss.clear();
  iss.str("C++09format10PA22006rulez");
  assert((iss >> date_format("C++%mformat%dPA2%Yrulez") >> f));
  oss.str("");
  oss << f;
  assert(oss.str() == "2006-09-10");
  iss.clear();
  iss.str("%12%13%2010%");
  assert((iss >> date_format("%%%m%%%d%%%Y%%") >> f));
  oss.str("");
  oss << f;
  assert(oss.str() == "2010-12-13");

  CDate g(2000, 6, 8);
  iss.clear();
  iss.str("2001-11-33");
  assert(!(iss >> date_format("%Y-%m-%d") >> g));
  oss.str("");
  oss << g;
  assert(oss.str() == "2000-06-08");
  iss.clear();
  iss.str("29.02.2003");
  assert(!(iss >> date_format("%d.%m.%Y") >> g));
  oss.str("");
  oss << g;
  assert(oss.str() == "2000-06-08");
  iss.clear();
  iss.str("14/02/2004");
  assert(!(iss >> date_format("%m/%d/%Y") >> g));
  oss.str("");
  oss << g;
  assert(oss.str() == "2000-06-08");
  iss.clear();
  iss.str("2002-03");
  assert(!(iss >> date_format("%Y-%m") >> g));
  oss.str("");
  oss << g;
  assert(oss.str() == "2000-06-08");
  iss.clear();
  iss.str("hello kitty");
  assert(!(iss >> date_format("hello kitty") >> g));
  oss.str("");
  oss << g;
  assert(oss.str() == "2000-06-08");
  iss.clear();
  iss.str("2005-07-12-07");
  assert(!(iss >> date_format("%Y-%m-%d-%m") >> g));
  oss.str("");
  oss << g;
  assert(oss.str() == "2000-06-08");
  iss.clear();
  iss.str("20000101");
  assert((iss >> date_format("%Y%m%d") >> g));
  oss.str("");
  oss << g;
  assert(oss.str() == "2000-01-01");
*/

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
