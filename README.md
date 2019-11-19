# C++ Generators

## Introduction

This is a simple introduction on the generator grammar EFL# and EFL C++ bindings use in their
code generators.

## Basic definition

The generators are classes with a templated `bool generate(sink, argument_tuple, context) const`
method tagged with some extra information. These methods are invoked by the API user in a scheme
like `as_generator(generator_expression).generate(sink, parameters, context)`.

* An `OutputIterator` sink to receive the generated data.
* A single value or a tuple with attributes to be consumed by the generator.
* A context instance (More below).

The trait `attributes_needed<>` determines the number of tuple elements a given constructor
will consume.

Below is a sample generator that outputs a reversed string.

```cpp

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
        struct attributes_needed<::reverse_generator> : std::integral_constant<int, 1> {};
    }

}}}
```

## Basic usage

```cpp
auto context = efl::eolian::grammar::context_null(); // More on context below
std::string output;
auto name = "John Doe"

if (!as_generator(reverse).generate(std::back_inserter(output), name , context))
  return false;

// output == "eoD nhoJ"
```

Combining generators can be done with operators like the sequence operator `<<`,
which cause the left side operand to be evaluated before the right side. For example:

```cpp
auto context = efl::eolian::grammar::context_null(); // More on context below
std::string output;
auto first_name = "John"
auto last_name = "Doe"

if (!as_generator(reverse << " " << reverse).generate(std::back_inserter(output)
                                                     , std::make_tuple(first_name, last_name)
                                                     , context))
  return false;

// output == "nhoJ eoD"
```

Later we will other useful operators to do things like repeat and reorder generator expressions.

## Generator receiving a parameter

To create a generator that can be customized with some parameters  you can use a
"constructor generator" that returns the actual generators through either the
`()` operator or an implicit conversion.

In this example, we create a simple 4-space indentation generator.

```cpp
struct scope_tab_generator
{
    constexpr scope_tab_generator(int n) : n(n) {}

    template<typename OutputIterator sink, attributes::unused_type, Context>
    bool as_generator(OutputIterator sink, attributes::unused_type, Context const& context) const
    {
        return true;
    }

    int n;
};

struct scope_tab_terminal
{
    scope_tab_generator operator()(int n=4) const
    {
        return {n};
    }

    operator scope_tab_generator() const
    {
        return {4};
    }
} const scope_tab = {};

namespace efl { namespace eolian { namespace grammar {
    template<>
    struct is_eager_generator<::scope_tab_generator> : std::true_type {};
    template<>
    struct is_generator<::scope_tab_generator> : std::true_type {};

    template<>
    struct is_generator<scope_tab_terminal> : std::true_type {};

    // Replaces `is_eager_generator` meaning `scope_tab_terminal` occurrences will be
    // immediately replaced by this call.
    scope_tab_generator as_generator(scope_tab_terminal)
    {
        return scope_tab_generator(4);
    }

    // No need for `attributes_needed` as it will not consume any attribute
}}}
```

## Context

Context is implemented as a compilation-time linked list of context-holding structs
to pass extra info to the generators. For example, what kind of item we are generating,
like an interface or a class, a map of eolian class names to the library they belong,
etc.

The empty context is defined as `efl::eolian::grammar::context_null()`

### Defining new context tags

```cpp
struct kind_context {
    enum wrapper_kind {
        interface,
        concrete,
        structs,
        enums,
        functions,
        variables,
    };

    wrapper_kind current_wrapper_kind;
    std::string name;

    kind_context(wrapper_kind current_wrapper_kind, std::string const& name)
        : current_wrapper_kind(current_wrapper_kind)
        , name(name)
    {}

};
```

### Adding to context

```cpp
using efl::eolian::grammar::context_add_tag;
auto context = context_add_tag(::kind_context{kind_context::interface, "MyInterface"}
                              , efl::eolian::grammar::context_null()); // The tail of the context list.
/// ...
auto new_context = context_add_tag(::some_context_struct{}
                              , context); // Previous context is the new tail
```

### Checking a context tag

```cpp
using efl::eolian::grammar::context_find_tag;
// Compilation will FAIL if `kind_context` is not found in the context list.
bool is_interface = context_find_tag<::kind_context>(context).current_wrapper_kind == kind_context::interface;
```

## Grammar - Building blocks for more complex generators

### `lit(x)` - Literals

`lit` is a simple wrapper to transform literal strings into generators. They consume
no attributes and are copied directly to the `sink`.

Example:

```cpp
if (!as_generator(lit("abc")).generate(sink, attributes::unused, context))
    return true;
```

They are usually used only if the only or first element of a generator expression is
a literal, as the sequence operator (`<<`) automatically converts following literals

### `string` - Simple strings

### `a << b` - Sequence

### `*(...)` - Kleene operator

### `attribute_reorder<int, int, ...>` - Attribute reorder

### Attribute conditional