#include <quiz.h>
#include <ion.h>
#include <assert.h>
#include <apps/shared/global_context.h>
#include "helper.h"

using namespace Poincare;

QUIZ_CASE(poincare_user_variable_simple) {
  // Fill variable
  assert_parsed_expression_simplify_to("1+2>Adadas", "3");
  assert_parsed_expression_simplify_to("Adadas", "3");

  // Fill f1
  assert_parsed_expression_simplify_to("1+x>f1(x)", "x+1");
  assert_parsed_expression_simplify_to("f1(4)", "5");
  assert_parsed_expression_simplify_to("f1(Adadas)", "4");

  // Fill f2
  assert_parsed_expression_simplify_to("x-1>f2(x)", "x-1");
  assert_parsed_expression_simplify_to("f2(4)", "3");
  assert_parsed_expression_simplify_to("f2(Adadas)", "2");

  // Define fBoth with f1 and f2
  assert_parsed_expression_simplify_to("f1(x)+f2(x)>fBoth(x)", "2*x");
  assert_parsed_expression_simplify_to("fBoth(4)", "8");
  assert_parsed_expression_simplify_to("fBoth(Adadas)", "6");

  // Change f2
  assert_parsed_expression_simplify_to("x>f2(x)", "x");
  assert_parsed_expression_simplify_to("f2(4)", "4");
  assert_parsed_expression_simplify_to("f2(Adadas)", "3");

  // Make sure fBoth has changed
  assert_parsed_expression_simplify_to("fBoth(4)", "9");
  assert_parsed_expression_simplify_to("fBoth(Adadas)", "7");

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("Adadas.exp").destroy();
  Ion::Storage::sharedStorage()->recordNamed("f1.func").destroy();
  Ion::Storage::sharedStorage()->recordNamed("f2.func").destroy();
  Ion::Storage::sharedStorage()->recordNamed("funcBoth.func").destroy();
}

QUIZ_CASE(poincare_user_variable_2_circular_variables) {
  assert_simplify("a>b");
  assert_simplify("b>a");
  assert_parsed_expression_evaluates_to<double>("a", Undefined::Name());
  assert_parsed_expression_evaluates_to<double>("b", Undefined::Name());

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("a.exp").destroy();
  Ion::Storage::sharedStorage()->recordNamed("b.exp").destroy();
}

QUIZ_CASE(poincare_user_variable_3_circular_variables) {
  assert_simplify("a>b");
  assert_simplify("b>c");
  assert_simplify("c>a");
  assert_parsed_expression_evaluates_to<double>("a", Undefined::Name());
  assert_parsed_expression_evaluates_to<double>("b", Undefined::Name());
  assert_parsed_expression_evaluates_to<double>("c", Undefined::Name());

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("a.exp").destroy();
  Ion::Storage::sharedStorage()->recordNamed("b.exp").destroy();
  Ion::Storage::sharedStorage()->recordNamed("c.exp").destroy();
}

QUIZ_CASE(poincare_user_variable_1_circular_function) {
  // g: x -> f(x)+1
  assert_simplify("f(x)+1>g(x)");
  assert_parsed_expression_evaluates_to<double>("g(1)", Undefined::Name());
  // f: x -> x+1
  assert_simplify("x+1>f(x)");
  assert_parsed_expression_evaluates_to<double>("g(1)", "3");
  assert_parsed_expression_evaluates_to<double>("f(1)", "2");
  // h: x -> h(x)
  assert_simplify("h(x)>h(x)");
  assert_parsed_expression_evaluates_to<double>("f(1)", "2");
  assert_parsed_expression_evaluates_to<double>("g(1)", "3");
  assert_parsed_expression_evaluates_to<double>("h(1)", Undefined::Name());

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("f.func").destroy();
  Ion::Storage::sharedStorage()->recordNamed("g.func").destroy();
  Ion::Storage::sharedStorage()->recordNamed("h.func").destroy();
}

QUIZ_CASE(poincare_user_variable_2_circular_functions) {
  assert_simplify("f(x)>g(x)");
  assert_simplify("g(x)>f(x)");
  assert_parsed_expression_evaluates_to<double>("f(1)", Undefined::Name());
  assert_parsed_expression_evaluates_to<double>("g(1)", Undefined::Name());

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("f.func").destroy();
  Ion::Storage::sharedStorage()->recordNamed("g.func").destroy();
}

QUIZ_CASE(poincare_user_variable_3_circular_functions) {
  assert_simplify("f(x)>g(x)");
  assert_simplify("g(x)>h(x)");
  assert_simplify("h(x)>f(x)");
  assert_parsed_expression_evaluates_to<double>("f(1)", Undefined::Name());
  assert_parsed_expression_evaluates_to<double>("g(1)", Undefined::Name());
  assert_parsed_expression_evaluates_to<double>("h(1)", Undefined::Name());

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("f.func").destroy();
  Ion::Storage::sharedStorage()->recordNamed("g.func").destroy();
  Ion::Storage::sharedStorage()->recordNamed("h.func").destroy();
}

QUIZ_CASE(poincare_user_variable_circular_variables_and_functions) {
  assert_simplify("a>b");
  assert_simplify("b>a");
  assert_simplify("a>f(x)");
  assert_parsed_expression_evaluates_to<double>("f(1)", Undefined::Name());
  assert_parsed_expression_evaluates_to<double>("a", Undefined::Name());
  assert_parsed_expression_evaluates_to<double>("b", Undefined::Name());

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("f.func").destroy();
  Ion::Storage::sharedStorage()->recordNamed("a.exp").destroy();
  Ion::Storage::sharedStorage()->recordNamed("b.exp").destroy();
}

QUIZ_CASE(poincare_user_variable_composed_functions) {
  // f: x->x^2
  assert_simplify("x^2>f(x)");
  // g: x->f(x-2)
  assert_simplify("f(x-2)>g(x)");
  assert_parsed_expression_evaluates_to<double>("f(2)", "4");
  assert_parsed_expression_evaluates_to<double>("g(3)", "1");
  assert_parsed_expression_evaluates_to<double>("g(5)", "9");

  // g: x->f(x-2)+f(x+1)
  assert_simplify("f(x-2)+f(x+1)>g(x)");
  // Add a matrix to bypass simplification
  assert_parsed_expression_evaluates_to<double>("g(3)+[[1]]", "[[18]]");
  assert_parsed_expression_evaluates_to<double>("g(5)", "45");

  // g: x->x+1
  assert_simplify("x+1>g(x)");
  assert_parsed_expression_evaluates_to<double>("f(g(4))", "25");
  // Add a matrix to bypass simplification
  assert_parsed_expression_evaluates_to<double>("f(g(4))+[[1]]", "[[26]]");

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("f.func").destroy();
  Ion::Storage::sharedStorage()->recordNamed("g.func").destroy();
}

QUIZ_CASE(poincare_user_variable_functions_with_context) {
  // f: x->x^2
  assert_simplify("x^2>f(x)");
  // Approximate f(?-2) with ? = 5
  const char x[] = {Symbol::SpecialSymbols::UnknownX, 0};
  assert_parsed_expression_approximates_with_value_for_symbol(Function("f", 1, Subtraction::Builder(Symbol(Symbol::SpecialSymbols::UnknownX), Rational(2))), x, 5.0, 9.0);
  // Approximate f(?-1)+f(?+1) with ? = 3
  assert_parsed_expression_approximates_with_value_for_symbol(Addition::Builder(Function("f", 1, Subtraction::Builder(Symbol(Symbol::SpecialSymbols::UnknownX), Rational(1))), Function("f", 1, Addition::Builder(Symbol(Symbol::SpecialSymbols::UnknownX), Rational(1)))), x, 3.0, 20.0);

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("f.func").destroy();

  // f: x->R(-1)
  assert_simplify("R(-1)*R(-1)>f(x)");
  // Approximate f(?) with ? = 5
  // Cartesian
  assert_parsed_expression_approximates_with_value_for_symbol(Function("f", 1, Symbol(Symbol::SpecialSymbols::UnknownX)), x, 1.0, -1.0);
    // Real
  assert_parsed_expression_approximates_with_value_for_symbol(Function("f", 1, Symbol(Symbol::SpecialSymbols::UnknownX)), x, 1.0, (double)NAN, Real);

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("f.func").destroy();
}

QUIZ_CASE(poincare_user_variable_properties) {
  Shared::GlobalContext context;

  assert_parsed_expression_evaluates_to<double>("[[1]]>a", "[[1]]");
  quiz_assert(Symbol('a').isApproximate(context));
  quiz_assert(Poincare::Expression::IsMatrix(Symbol('a'), context, true));

  /* [[x]]->f(x) expression contains a matrix, so its simplification is going
   * to be interrupted. We thus rather approximate it instead of simplifying it.
   * TODO: use parse_and_simplify when matrix are simplified. */
  assert_parsed_expression_evaluates_to<double>("[[x]]>f(x)", "[[undef]]");
  quiz_assert(Function("f", 1, Rational(2)).isApproximate(context));
  quiz_assert(Poincare::Expression::IsMatrix(Function("f", 1, Symbol('x')), context, true));

  // Clean the storage for other tests
  Ion::Storage::sharedStorage()->recordNamed("a.exp").destroy();
  Ion::Storage::sharedStorage()->recordNamed("f.func").destroy();
}
