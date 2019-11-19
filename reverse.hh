#if !defined(REVERSE_HH)
#define REVERSE_HH

#include <generator.hpp>
// #include <sequence.hpp>
#include <attributes.hpp>

struct reverse_generator
{
    template<typename OutputIterator, typename Context>
    bool generate(OutputIterator sink, std::string const& value, Context const& context) const
    {
        for (int i = value.length() - 1; i >= 0; i--)
            *sink++ = value[i];
        return true;
    }
} const reverse;

// Extras tags to mark `reverse_generator` as an actual generator.
namespace efl { namespace eolian { namespace grammar {

    // Means this operator should be evaluated as is and is not replaced by a custom
    // `U as_generator(T g)` call, like we will do belo when dealing with generators
    // with parameters.
    template<>
    struct is_eager_generator<::reverse_generator> : std::true_type {};
    // Tag used inside some operators to make sure T is a generator
    template<>
    struct is_generator<::reverse_generator> : std::true_type {};

    namespace type_traits {
        // This generator will consume 1 attribute from the attribute tuple passed
        // to `generate(sink, params, context)`.
        template<>
        struct attributes_needed<::reverse_generator> : std::integral_constant<int, 1> {};
    }

}}}

#endif // REVERSE_HH