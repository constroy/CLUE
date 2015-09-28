#include <clue/formatting.hpp>
#include <vector>
#include <gtest/gtest.h>

using namespace clue;
using std::size_t;

// Auxiliary functions for testing

const char *cfmt_lsym(const fmt::integer_formatter<fmt::dec_t>&) {
    return "%ld";
}

const char *cfmt_lsym(const fmt::integer_formatter<fmt::oct_t>&) {
    return "%lo";
}

const char *cfmt_lsym(const fmt::integer_formatter<fmt::hex_t>&) {
    return "%lx";
}

const char *cfmt_lsym(const fmt::integer_formatter<fmt::Hex_t>&) {
    return "%lX";
}

template<class F>
std::string ref_int_format(const F& f, size_t w, long x) {
    // generate the reference format in a safe but inefficient way

    // format the main digits
    const char *cf = cfmt_lsym(f);
    std::string main = fmt::sprintf(cf, (x >= 0 ? x : -x));
    size_t ml = main.size();

    // get the sign string
    std::string sign = x < 0 ? std::string("-") :
        (f.plus_sign() ? std::string("+") : std::string());
    size_t sl = sign.size() + ml;

    // compose
    if (f.pad_zeros()) {
        std::string ps = w > sl ? std::string(w-sl, '0') : std::string();
        return sign + ps + main;
    } else {
        std::string ps = w > sl ? std::string(w-sl, ' ') : std::string();
        return ps + sign + main;
    }
}

template<typename F, typename X>
::testing::AssertionResult intfmt_assertion_failure(
    const char *title, const F& f, size_t width, long x,
    const char *fexpr, const char *wexpr, const char *xexpr,
    const X& actual, const X& expect) {

    return ::testing::AssertionFailure()
        << "Mismatched " << title << " for "
        << "[" << xexpr << " = " << x << "] "
        << "with " << fexpr << ": \n"
        << "  base: " << f.base() << "\n"
        << "  plus_sign: " << f.plus_sign() << "\n"
        << "  pad_zeros: " << f.pad_zeros() << "\n"
        << "  width: " << wexpr << "=" << width << "\n"
        << "Result:\n"
        << "  ACTUAL = \"" << actual << "\"\n"
        << "  EXPECT = \"" << expect << "\"";
}

template<typename F>
::testing::AssertionResult CheckIntFormat(
    const char *fexpr, const char *wexpr, const char *xexpr,
    const F& f,  size_t width, long x) {

    std::string refstr = ref_int_format(f, width, x);
    std::string r = format(x, f, width);
    if (refstr != r) {
        return intfmt_assertion_failure(
            "formatted string",
            f, width, x, fexpr, wexpr, xexpr,
            r, refstr);
    }
    size_t flen = f.formatted_length(x, width);
    if (refstr.size() != flen) {
        return intfmt_assertion_failure(
            "formatted length",
            f, width, x, fexpr, wexpr, xexpr,
            flen, refstr.size());
    }
    return ::testing::AssertionSuccess();
}


// C-string formatting

TEST(StringFmt, Sprintf) {
    ASSERT_EQ("", fmt::sprintf(""));
    ASSERT_EQ("123", fmt::sprintf("%d", 123));
    ASSERT_EQ("2 + 3 = 5", fmt::sprintf("%d + %d = %d", 2, 3, 5));
    ASSERT_EQ("12.5000", fmt::sprintf("%.4f", 12.5));
}


// Integer formatting

std::vector<long> prepare_test_ints(size_t base, bool show=false) {
    std::vector<long> xs;
    xs.push_back(0);
    xs.push_back(1);
    xs.push_back(base / 2);
    xs.push_back(base - 1);

    size_t m = 0;
    switch (base) {
        case 8:
            m = 10; break;
        case 10:
            m = 10; break;
        case 16:
            m = 7; break;
    }

    long e = 1;
    for (size_t k = 0; k < 9; ++k) {
        long ep = e;
        e *= base;
        xs.push_back(e);
        xs.push_back(2 * e);
        xs.push_back(2 * e + 3 * ep);
        xs.push_back((base / 2) * e + 1);
        xs.push_back(base * e - 1);
    }

    if (show) {
        switch (base) {
            case 8:
                for (long x: xs) std::printf("0o%lo\n", x);
                break;
            case 10:
                for (long x: xs) std::printf("%ld\n", x);
                break;
            case 16:
                for (long x: xs) std::printf("0x%lx\n", x);
                break;
        }
    }

    std::vector<long> xs_aug;
    xs_aug.reserve(xs.size() * 2);
    for (long x: xs) xs_aug.push_back(x);
    for (long x: xs) xs_aug.push_back(-x);
    return xs_aug;
}

template<class Tag>
void batch_test_int_format(const std::vector<fmt::integer_formatter<Tag>>& fmts,
                     const std::vector<size_t>& ws,
                     const std::vector<long>& xs) {

    for (const auto& fmt: fmts) {
        for (size_t w: ws) {
            for (long x: xs) {
                ASSERT_PRED_FORMAT3(CheckIntFormat, fmt, w, x);
            }
        }
    }
}

TEST(IntFmt, Dec) {

    // formatters

    auto f00 = fmt::dec();
    ASSERT_EQ(10,    f00.base());
    ASSERT_EQ(false, f00.pad_zeros());
    ASSERT_EQ(false, f00.plus_sign());

    auto f01 = fmt::dec().plus_sign(true);
    ASSERT_EQ(10,    f01.base());
    ASSERT_EQ(false, f01.pad_zeros());
    ASSERT_EQ(true,  f01.plus_sign());

    auto f10 = fmt::dec().pad_zeros(true);
    ASSERT_EQ(10,    f10.base());
    ASSERT_EQ(true,  f10.pad_zeros());
    ASSERT_EQ(false, f10.plus_sign());

    auto f11 = fmt::dec().plus_sign(true).pad_zeros(true);
    ASSERT_EQ(10,    f11.base());
    ASSERT_EQ(true,  f11.pad_zeros());
    ASSERT_EQ(true,  f11.plus_sign());

    // examples check

    ASSERT_EQ("123",    fmt::format(123, f00));
    ASSERT_EQ("   123", fmt::format(123, f00, 6));
    ASSERT_EQ("+123",   fmt::format(123, f01));
    ASSERT_EQ("  +123", fmt::format(123, f01, 6));
    ASSERT_EQ("123",    fmt::format(123, f10));
    ASSERT_EQ("000123", fmt::format(123, f10, 6));
    ASSERT_EQ("+123",   fmt::format(123, f11));
    ASSERT_EQ("+00123", fmt::format(123, f11, 6));

    ASSERT_EQ("-123",   fmt::format(-123, f00));
    ASSERT_EQ("  -123", fmt::format(-123, f00, 6));
    ASSERT_EQ("-123",   fmt::format(-123, f01));
    ASSERT_EQ("  -123", fmt::format(-123, f01, 6));
    ASSERT_EQ("-123",   fmt::format(-123, f10));
    ASSERT_EQ("-00123", fmt::format(-123, f10, 6));
    ASSERT_EQ("-123",   fmt::format(-123, f11));
    ASSERT_EQ("-00123", fmt::format(-123, f11, 6));

    // combination coverage

    std::vector<fmt::integer_formatter<fmt::dec_t>> fmts {
        f00, f01, f10, f11};
    std::vector<size_t> widths = {0, 5, 12};
    std::vector<long> xs = prepare_test_ints(10);

    batch_test_int_format(fmts, widths, xs);
}


TEST(IntFmt, Oct) {

    // formatters

    auto f00 = fmt::oct();
    ASSERT_EQ(8,     f00.base());
    ASSERT_EQ(false, f00.pad_zeros());
    ASSERT_EQ(false, f00.plus_sign());

    auto f01 = fmt::oct().plus_sign(true);
    ASSERT_EQ(8,     f01.base());
    ASSERT_EQ(false, f01.pad_zeros());
    ASSERT_EQ(true,  f01.plus_sign());

    auto f10 = fmt::oct().pad_zeros(true);
    ASSERT_EQ(8,     f10.base());
    ASSERT_EQ(true,  f10.pad_zeros());
    ASSERT_EQ(false, f10.plus_sign());

    auto f11 = fmt::oct().plus_sign(true).pad_zeros(true);
    ASSERT_EQ(8,     f11.base());
    ASSERT_EQ(true,  f11.pad_zeros());
    ASSERT_EQ(true,  f11.plus_sign());

    // examples check
    // (263)_10 = (407)_8

    ASSERT_EQ("407",    fmt::format(263, f00));
    ASSERT_EQ("   407", fmt::format(263, f00, 6));
    ASSERT_EQ("+407",   fmt::format(263, f01));
    ASSERT_EQ("  +407", fmt::format(263, f01, 6));
    ASSERT_EQ("407",    fmt::format(263, f10));
    ASSERT_EQ("000407", fmt::format(263, f10, 6));
    ASSERT_EQ("+407",   fmt::format(263, f11));
    ASSERT_EQ("+00407", fmt::format(263, f11, 6));

    ASSERT_EQ("-407",   fmt::format(-263, f00));
    ASSERT_EQ("  -407", fmt::format(-263, f00, 6));
    ASSERT_EQ("-407",   fmt::format(-263, f01));
    ASSERT_EQ("  -407", fmt::format(-263, f01, 6));
    ASSERT_EQ("-407",   fmt::format(-263, f10));
    ASSERT_EQ("-00407", fmt::format(-263, f10, 6));
    ASSERT_EQ("-407",   fmt::format(-263, f11));
    ASSERT_EQ("-00407", fmt::format(-263, f11, 6));

    // combination coverage

    std::vector<fmt::integer_formatter<fmt::oct_t>> fmts {
        f00, f01, f10, f11};
    std::vector<size_t> widths = {0, 5, 12};
    std::vector<long> xs = prepare_test_ints(10);

    batch_test_int_format(fmts, widths, xs);
}


TEST(IntFmt, Hex) {

    // formatters

    auto f00 = fmt::hex();
    ASSERT_EQ(16,    f00.base());
    ASSERT_EQ(false, f00.pad_zeros());
    ASSERT_EQ(false, f00.plus_sign());

    auto f01 = fmt::hex().plus_sign(true);
    ASSERT_EQ(16,    f01.base());
    ASSERT_EQ(false, f01.pad_zeros());
    ASSERT_EQ(true,  f01.plus_sign());

    auto f10 = fmt::hex().pad_zeros(true);
    ASSERT_EQ(16,    f10.base());
    ASSERT_EQ(true,  f10.pad_zeros());
    ASSERT_EQ(false, f10.plus_sign());

    auto f11 = fmt::hex().plus_sign(true).pad_zeros(true);
    ASSERT_EQ(16,    f11.base());
    ASSERT_EQ(true,  f11.pad_zeros());
    ASSERT_EQ(true,  f11.plus_sign());

    // examples check
    // (1234)_10 = (4d2)_16

    ASSERT_EQ("4d2",    fmt::format(1234, f00));
    ASSERT_EQ("   4d2", fmt::format(1234, f00, 6));
    ASSERT_EQ("+4d2",   fmt::format(1234, f01));
    ASSERT_EQ("  +4d2", fmt::format(1234, f01, 6));
    ASSERT_EQ("4d2",    fmt::format(1234, f10));
    ASSERT_EQ("0004d2", fmt::format(1234, f10, 6));
    ASSERT_EQ("+4d2",   fmt::format(1234, f11));
    ASSERT_EQ("+004d2", fmt::format(1234, f11, 6));

    ASSERT_EQ("-4d2",   fmt::format(-1234, f00));
    ASSERT_EQ("  -4d2", fmt::format(-1234, f00, 6));
    ASSERT_EQ("-4d2",   fmt::format(-1234, f01));
    ASSERT_EQ("  -4d2", fmt::format(-1234, f01, 6));
    ASSERT_EQ("-4d2",   fmt::format(-1234, f10));
    ASSERT_EQ("-004d2", fmt::format(-1234, f10, 6));
    ASSERT_EQ("-4d2",   fmt::format(-1234, f11));
    ASSERT_EQ("-004d2", fmt::format(-1234, f11, 6));

    // combination coverage

    std::vector<fmt::integer_formatter<fmt::hex_t>> fmts {
        f00, f01, f10, f11};
    std::vector<size_t> widths = {0, 5, 12};
    std::vector<long> xs = prepare_test_ints(10);

    batch_test_int_format(fmts, widths, xs);
}


TEST(IntFmt, UHex) {

    // formatters

    auto f00 = fmt::Hex();
    ASSERT_EQ(16,    f00.base());
    ASSERT_EQ(false, f00.pad_zeros());
    ASSERT_EQ(false, f00.plus_sign());

    auto f01 = fmt::Hex().plus_sign(true);
    ASSERT_EQ(16,    f01.base());
    ASSERT_EQ(false, f01.pad_zeros());
    ASSERT_EQ(true,  f01.plus_sign());

    auto f10 = fmt::Hex().pad_zeros(true);
    ASSERT_EQ(16,    f10.base());
    ASSERT_EQ(true,  f10.pad_zeros());
    ASSERT_EQ(false, f10.plus_sign());

    auto f11 = fmt::Hex().plus_sign(true).pad_zeros(true);
    ASSERT_EQ(16,    f11.base());
    ASSERT_EQ(true,  f11.pad_zeros());
    ASSERT_EQ(true,  f11.plus_sign());

    // examples check
    // (1234)_10 = (4D2)_16

    ASSERT_EQ("4D2",    fmt::format(1234, f00));
    ASSERT_EQ("   4D2", fmt::format(1234, f00, 6));
    ASSERT_EQ("+4D2",   fmt::format(1234, f01));
    ASSERT_EQ("  +4D2", fmt::format(1234, f01, 6));
    ASSERT_EQ("4D2",    fmt::format(1234, f10));
    ASSERT_EQ("0004D2", fmt::format(1234, f10, 6));
    ASSERT_EQ("+4D2",   fmt::format(1234, f11));
    ASSERT_EQ("+004D2", fmt::format(1234, f11, 6));

    ASSERT_EQ("-4D2",   fmt::format(-1234, f00));
    ASSERT_EQ("  -4D2", fmt::format(-1234, f00, 6));
    ASSERT_EQ("-4D2",   fmt::format(-1234, f01));
    ASSERT_EQ("  -4D2", fmt::format(-1234, f01, 6));
    ASSERT_EQ("-4D2",   fmt::format(-1234, f10));
    ASSERT_EQ("-004D2", fmt::format(-1234, f10, 6));
    ASSERT_EQ("-4D2",   fmt::format(-1234, f11));
    ASSERT_EQ("-004D2", fmt::format(-1234, f11, 6));
}



//// Floating-point formatting
//
//TEST(FloatFmt, Fixed) {
//
//    // formatters
//
//    auto
//
//
//}



