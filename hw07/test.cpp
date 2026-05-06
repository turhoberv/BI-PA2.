#ifndef __PROGTEST__
#include <cassert>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <vector>

class CRange {
public:
    CRange(size_t from, size_t to) : m_From(from), m_To(to) {
        if (from > to)
            throw std::invalid_argument("invalid range");
    }
    size_t m_From;
    size_t m_To;
};

class CRangeRev {
public:
    CRangeRev(size_t from, size_t to) : m_From(from), m_To(to) {
        if (from < to)
            throw std::invalid_argument("invalid range");
    }
    size_t m_From;
    size_t m_To;
};
#endif /* __PROGTEST__ */

class SliceRule {
public:
    size_t start;
    size_t size;
    int step;
    bool isFixed;

    SliceRule(CRange range) {
        start = range.m_From;
        size = range.m_To - range.m_From + 1;
        step = 1;
        isFixed = false;
    }
    SliceRule(CRangeRev range) {
        start = range.m_From;
        size = range.m_From - range.m_To + 1;
        step = -1;
        isFixed = false;
    }
    SliceRule(size_t x) {
        start = x;
        size = 1;
        step = 0;
        isFixed = true;
    }

    size_t getMaxIndex() const {
        if (step > 0)
            return start + size - 1;
        return start;
    }
};


template<typename T_, size_t M_>
class CTensor;

template<typename T_>
class CTensorView1D_from_2D {
public:
    CTensorView1D_from_2D(CTensor<T_, 2> &tensor, SliceRule rule1, SliceRule rule2) :
        m_Tensor(tensor), m_Rule1(rule1), m_Rule2(rule2) {}

    T_ &operator()(size_t index) {
        size_t max_size = m_Rule1.isFixed ? m_Rule2.size : m_Rule1.size;
        if (index >= max_size) {
            throw std::out_of_range("index out of range");
        }
        size_t origIndex1 = m_Rule1.isFixed ? m_Rule1.start : m_Rule1.start + m_Rule1.step * index;
        size_t origIndex2 = m_Rule2.isFixed ? m_Rule2.start : m_Rule2.start + m_Rule2.step * index;
        return m_Tensor(origIndex1, origIndex2);
    }

    T_ operator()(size_t index) const {
        size_t max_size = m_Rule1.isFixed ? m_Rule2.size : m_Rule1.size;
        if (index >= max_size) {
            throw std::out_of_range("index out of range");
        }
        size_t origIndex1 = m_Rule1.isFixed ? m_Rule1.start : m_Rule1.start + m_Rule1.step * index;
        size_t origIndex2 = m_Rule2.isFixed ? m_Rule2.start : m_Rule2.start + m_Rule2.step * index;
        return m_Tensor(origIndex1, origIndex2);
    }

    // operátor << (tiskne to jako čisté 1D pole)
    friend std::ostream &operator<<(std::ostream &os, const CTensorView1D_from_2D &view) {
        os << '{';
        size_t max_size = view.m_Rule1.isFixed ? view.m_Rule2.size : view.m_Rule1.size;
        for (size_t i = 0; i < max_size; ++i) {
            if (i > 0)
                os << ", ";
            os << view(i);
        }
        os << "}";
        return os;
    }

private:
    CTensor<T_, 2> &m_Tensor;
    SliceRule m_Rule1, m_Rule2;
};


template<typename T_>
class CTensorView1D {
public:
    CTensorView1D(CTensor<T_, 1> &tensor, SliceRule rule1) : m_Tensor(tensor), m_Rule1(rule1) {}
    // operator ()
    T_ &operator()(size_t index1) {
        if (index1 >= m_Rule1.size) {
            throw std::out_of_range("index out of range");
        }
        size_t origIndex = m_Rule1.start + m_Rule1.step * index1;
        return m_Tensor(origIndex);
    }
    const T_ &operator()(size_t index1) const {
        if (index1 >= m_Rule1.size) {
            throw std::out_of_range("index out of range");
        }
        size_t origIndex = m_Rule1.start + m_Rule1.step * index1;
        return m_Tensor(origIndex);
    }
    // operator <<
    friend std::ostream &operator<<(std::ostream &os, const CTensorView1D &view) {
        os << '{';
        for (size_t i = 0; i < view.m_Rule1.size; ++i) {
            os << view(i) << ", ";
        }
        os << "}";
        return os;
    }

private:
    CTensor<T_, 1> &m_Tensor;
    SliceRule m_Rule1;
};

template<typename T_>
class CTensorView2D {
public:
    CTensorView2D(CTensor<T_, 2> &tensor, SliceRule rule1, SliceRule rule2) :
        m_Tensor(tensor), m_Rule1(rule1), m_Rule2(rule2) {}
    T_ &operator()(size_t index1, size_t index2) {
        if (index1 >= m_Rule1.size || index2 >= m_Rule2.size) {
            throw std::out_of_range("index out of range");
        }
        size_t origIndex1 = m_Rule1.start + m_Rule1.step * index1;
        size_t origIndex2 = m_Rule2.start + m_Rule2.step * index2;
        return m_Tensor(origIndex1, origIndex2);
    }
    const T_ &operator()(size_t index1, size_t index2) const {
        if (index1 >= m_Rule1.size || index2 >= m_Rule2.size) {
            throw std::out_of_range("index out of range");
        }
        size_t origIndex1 = m_Rule1.start + m_Rule1.step * index1;
        size_t origIndex2 = m_Rule2.start + m_Rule2.step * index2;
        return m_Tensor(origIndex1, origIndex2);
    }
    // operator <<
    friend std::ostream &operator<<(std::ostream &os, const CTensorView2D &view) {

        os << "{\n";
        for (size_t i = 0; i < view.m_Rule1.size; ++i) {
            os << "  {";
            for (size_t j = 0; j < view.m_Rule2.size; ++j) {
                os << view(i, j);
                if (j + 1 != view.m_Rule2.size) {
                    os << ", ";
                }
            }
            os << '}';
            if (i + 1 != view.m_Rule1.size) {
                os << ",";
            }
            os << '\n';
        }
        os << "}";
        return os;
    }

private:
    CTensor<T_, 2> &m_Tensor;
    SliceRule m_Rule1, m_Rule2;
};


template<typename T_>
class CTensor<T_, 1> {
public:
    // constructor -- create + set all elements
    CTensor(T_ v, size_t size1) : m_Size1(size1) { m_Data.assign(size1, v); }

    // constructor -- from initializer_list
    CTensor(std::initializer_list<T_> list) : m_Size1(list.size()) { m_Data = list; }
    // destructor (opt)
    ~CTensor() {}
    // slice ()
    CTensorView1D<T_> slice(SliceRule rule) { return CTensorView1D<T_>(*this, rule); }
    // operator ()
    T_ &operator()(size_t index1) {
        if (index1 >= m_Size1) {
            throw std::out_of_range("index out of range");
        }
        return m_Data[index1];
    }
    const T_ &operator()(size_t index1) const {
        if (index1 >= m_Size1) {
            throw std::out_of_range("index out of range");
        }
        return m_Data[index1];
    }
    // operator <<

    friend std::ostream &operator<<(std::ostream &os, const CTensor<T_, 1> &tensor) {
        os << '{';
        for (size_t i = 0; i < tensor.m_Size1; ++i) {
            os << tensor(i) << ", ";
        }
        os << "}";
        return os;
    }

private:
    std::vector<T_> m_Data;
    size_t m_Size1;
};


template<typename T_>
class CTensor<T_, 2> {
public:
    // constructor -- create + set all elements
    CTensor(T_ v, size_t size1, size_t size2) : m_Size1(size1), m_Size2(size2) { m_Data.assign(size1 * size2, v); }

    // constructor -- from initializer_list
    CTensor(std::initializer_list<std::initializer_list<T_>> list) {
        m_Size1 = list.size();
        m_Size2 = m_Size1 > 0 ? list.begin()->size() : 0;

        for (const auto &row: list) {
            if (row.size() != m_Size2) {
                throw std::invalid_argument("invalid initializer list");
            }
        }

        m_Data.reserve(m_Size1 * m_Size2);
        for (const auto &row: list) {
            m_Data.insert(m_Data.end(), row.begin(), row.end());
        }
    }
    // destructor (opt)
    ~CTensor() {}
    // slice ()
    CTensorView2D<T_> slice(SliceRule rule1, SliceRule rule2) {
        if (rule1.getMaxIndex() >= m_Size1 || rule2.getMaxIndex() >= m_Size2) {
            throw std::out_of_range("slice out of range");
        }
        return CTensorView2D<T_>(*this, rule1, rule2);
    }

    CTensorView2D<T_> slice(SliceRule rule1) { return CTensorView2D<T_>(*this, rule1, CRange(0, m_Size2 - 1)); }

    CTensorView1D_from_2D<T_> slice(size_t index1, SliceRule rule2) {
        if (rule2.getMaxIndex() >= m_Size2) {
            throw std::out_of_range("slice out of range");
        }
        return CTensorView1D_from_2D<T_>(*this, SliceRule(index1), rule2);
    }

    CTensorView1D_from_2D<T_> slice(SliceRule rule1, size_t index2) {
        if (rule1.getMaxIndex() >= m_Size1) {
            throw std::out_of_range("slice out of range");
        }
        return CTensorView1D_from_2D<T_>(*this, rule1, SliceRule(index2));
    }

    CTensorView1D_from_2D<T_> slice(size_t index1) { return slice(index1, SliceRule(CRange(0, m_Size2 - 1))); }
    // operator ()


    T_ &operator()(size_t index1, size_t index2) {
        if (index1 >= m_Size1 || index2 >= m_Size2) {
            throw std::out_of_range("index out of range");
        }
        return m_Data[index1 * m_Size2 + index2];
    }
    T_ operator()(size_t index1, size_t index2) const {
        if (index1 >= m_Size1 || index2 >= m_Size2) {
            throw std::out_of_range("index out of range");
        }
        return m_Data[index1 * m_Size2 + index2];
    }
    // operator <<
    friend std::ostream &operator<<(std::ostream &os, const CTensor<T_, 2> &tensor) {
        os << "{\n";
        for (size_t i = 0; i < tensor.m_Size1; ++i) {
            os << "  {";
            for (size_t j = 0; j < tensor.m_Size2; ++j) {
                os << tensor(i, j);
                if (j + 1 != tensor.m_Size2) {
                    os << ", ";
                }
            }
            os << '}';
            if (i + 1 != tensor.m_Size1) {
                os << ",";
            }
            os << '\n';
        }
        os << "}";
        return os;
    }

private:
    std::vector<T_> m_Data; // using a one dimensional vector -- better memory management and hopefully will be easier
                            // to implement xd
    size_t m_Size1, m_Size2;
};


#ifndef __PROGTEST__
template<typename T_>
std::string toString(const T_ &x) {
    std::ostringstream oss;
    oss << x;
    return oss.str();
}

int main() {
    CTensor<int, 2> m1(0, 3, 5);
    CTensor<int, 2> m2{{1, 2, 3}, {4, 5, 6}};
    CTensor<std::string, 2> m4{{"test", "progtest"}, {"PA1", "PA2"}};


    assert(toString(m1) == R"({
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0}
})");

    assert(toString(m2) == R"({
  {1, 2, 3},
  {4, 5, 6}
})");

    assert(toString(m4) == R"({
  {test, progtest},
  {PA1, PA2}
})");


    assert(toString(m2.slice(CRange(0, 1), CRange(1, 2))) == R"({
  {2, 3},
  {5, 6}
})");

    assert(toString(m2.slice(0, CRange(1, 2))) == "{2, 3}");
    assert(toString(m2.slice(1)) == "{4, 5, 6}");
    assert(toString(m2.slice(CRange(0, 1), 2)) == "{3, 6}");

    auto v2 = m2.slice(CRange(0, 1), CRange(1, 2));
    assert(v2(0, 0) == 2);
    v2(1, 1) = 666;
    assert(v2(1, 1) == 666);


    assert(toString(v2) == R"({
  {2, 3},
  {5, 666}
})");

    assert(toString(m2) == R"({
  {1, 2, 3},
  {4, 5, 666}
})");


    assert(toString(m2.slice(1, CRange(1, 2))) == "{5, 666}");
    assert(toString(m2.slice(1, CRangeRev(2, 1))) == "{666, 5}");
    assert(toString(m2.slice(1, CRange(2, 2))) == "{666}");
    assert(toString(m2.slice(CRange(1, 1), 2)) == "{666}");
    assert(toString(m2.slice(CRange(1, 1), CRange(2, 2))) == R"###({
  {666}
})###");

    assert(toString(m4.slice(CRangeRev(1, 0), CRangeRev(1, 0))) == R"({
  {PA2, PA1},
  {progtest, test}
})");

    try {
        m2(3, 4)++;
        assert("missing an exception" == nullptr);
    } catch (const std::exception &e) {
    }

    try {
        v2(2, 0)--;
        assert("missing an exception" == nullptr);
    } catch (const std::exception &e) {
    }

    try {
        CTensor<int, 2> m5{{3, 5, 8}, {2, 7}};
        assert("missing an exception" == nullptr);
    } catch (const std::exception &e) {
    }

    try {
        m1.slice(CRange(0, 4), CRange(0, 2));
        assert("missing an exception" == nullptr);
    } catch (const std::exception &e) {
    }

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
