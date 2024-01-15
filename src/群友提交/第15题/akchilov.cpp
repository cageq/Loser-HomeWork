#include <algorithm>
#include <functional>
#include <iostream>
#include <ranges>
#include <vector>


//ps宏展开在clang gcc下失败，以下是展开版
// https://gcc.godbolt.org/z/6Y37drM5K
// 为std::vector增加一个自定义的赋值函数
template <typename T>
    requires std::disjunction_v<std::is_integral<T>, std::is_floating_point<T>>
class vector : public std::vector<T> {
public:
    using std::vector<T>::vector;
    using std::vector<T>::size;
    using std::vector<T>::operator[];
    template <typename E>
    vector<T>& operator=(const E& e)
    {
        const auto count = std::min(size(), e.size());
        this->resize(count);
        for (std::size_t idx{ 0 }; idx < count; ++idx) {
            this->operator[](idx) = e[idx];
        }
        return *this;
    }
};


template<class E1, class E2, class F>
struct vector_expr {

    std::remove_cvref_t<E1> const& e1;
    std::remove_cvref_t<E2> const& e2;

    vector_expr(const E1& e1_, const E2& e2_) :e1(e1_), e2(e2_){}
    std::size_t size() const { return 6; }
    double operator[](std::size_t i) const
    {
        return F{}(e1[i], e2[i]);
    }
    template< class Outer_FFF, class EE2> auto invoke_operate(EE2 const& rhs) const
    {
        return vector_expr<vector_expr<E1, E2, F>, EE2, Outer_FFF>{*this, rhs};
    }
    
};

auto plus_f = [](auto const& e1, auto const& e2) {return e1 + e2; };
auto sub_f = [](auto const& e1, auto const& e2) {return e1 - e2; };
auto mul_f = [](auto const& e1, auto const& e2) {return e1 * e2; };
auto div_f = [](auto const& e1, auto const& e2) {return e1 / e2; };


template<class E1, class E2, class F> auto operator+ (vector_expr<E1, E2, F> const& A, auto const& B)
{
    return A. template invoke_operate<decltype(plus_f)>(B);
}
template<class E1, class E2, class F> auto operator- (vector_expr<E1, E2, F> const& A, auto const& B)
{
    return A.template invoke_operate<decltype(sub_f)>(B);
}
template<class E1, class E2, class F> auto operator* (vector_expr<E1, E2, F> const& A, auto const& B)
{
    return A.template invoke_operate<decltype(mul_f)>(B);
}
template<class E1, class E2, class F> auto operator/ (vector_expr<E1, E2, F> const& A, auto const& B)
{
    return A.template invoke_operate<decltype(div_f)>(B);
}

template<class T> auto operator+(vector<T> const& lhs, auto const& rhs)
{
    return vector_expr <vector<T>, std::remove_cvref_t<decltype(rhs)>, decltype(plus_f)>{lhs, rhs};
}
template<class T> auto operator-(vector<T> const& lhs, auto const& rhs)
{
    return vector_expr <vector<T>, std::remove_cvref_t<decltype(rhs)>, decltype(sub_f)>{lhs, rhs};
}
template<class T> auto operator*(vector<T> const& lhs, auto const& rhs)
{
    return vector_expr <vector<T>, std::remove_cvref_t<decltype(rhs)>, decltype(mul_f)>{lhs, rhs};
}
template<class T> auto operator/(vector<T> const& lhs, auto const& rhs)
{
    return vector_expr <vector<T>, std::remove_cvref_t<decltype(rhs)>, decltype(div_f)>{lhs, rhs};
}



int main()
{
    auto print = [](const auto& v) {
        std::ranges::copy(v, std::ostream_iterator<std::ranges::range_value_t<decltype(v)>> { std::cout, ", " });
        std::cout << std::endl;
        };
    const vector<double> a{ 1.2764, 1.3536, 1.2806, 1.9124, 1.8871, 1.7455 };
    const vector<double> b{ 2.1258, 2.9679, 2.7635, 2.3796, 2.4820, 2.4195 };
    const vector<double> c{ 3.9064, 3.7327, 3.4760, 3.5705, 3.8394, 3.8993 };
    const vector<double> d{ 4.7337, 4.5371, 4.5517, 4.2110, 4.6760, 4.3139 };
    const vector<double> e{ 5.2126, 5.1452, 5.8678, 5.1879, 5.8816, 5.6282 };

    {
        vector<double> result(6);
        for (std::size_t idx = 0; idx < 6; idx++) {
            result[idx] = a[idx] - b[idx] * c[idx] / d[idx] + e[idx];
        }
        print(result);
    }
    {
        vector<double> result(6);
        result = a - b * c / d + e; // 使用表达式模板计算
        print(result);
    }
    return 0;
}
