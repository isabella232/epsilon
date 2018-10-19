#include <poincare/sine.h>
#include <poincare/complex.h>
#include <poincare/layout_helper.h>
#include <poincare/serialization_helper.h>
#include <poincare/simplification_helper.h>
#include <cmath>

namespace Poincare {

int SineNode::numberOfChildren() const { return Sine::NumberOfChildren(); }

float SineNode::characteristicXRange(Context & context, Preferences::AngleUnit angleUnit) const {
  return Trigonometry::characteristicXRange(Sine(this), context, angleUnit);
}

template<typename T>
Complex<T> SineNode::computeOnComplex(const std::complex<T> c, Preferences::AngleUnit angleUnit) {
  std::complex<T> angleInput = Trigonometry::ConvertToRadian(c, angleUnit);
  std::complex<T> res = std::sin(angleInput);
  return Complex<T>(Trigonometry::RoundToMeaningfulDigits(res, angleInput));
}

Layout SineNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(Sine(this), floatDisplayMode, numberOfSignificantDigits, Sine::Name());
}

int SineNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, Sine::Name());
}

Expression SineNode::shallowReduce(Context & context, Preferences::AngleUnit angleUnit, bool replaceSymbols) {
  return Sine(this).shallowReduce(context, angleUnit, replaceSymbols);
}

Sine::Sine() : Expression(TreePool::sharedPool()->createTreeNode<SineNode>()) {}

Expression Sine::shallowReduce(Context & context, Preferences::AngleUnit angleUnit, bool replaceSymbols) {
  {
    Expression e = Expression::defaultShallowReduce(context, angleUnit);
    if (e.isUndefined()) {
      return e;
    }
  }
#if MATRIX_EXACT_REDUCING
  Expression op = childAtIndex(0);
  if (op.type() == ExpressionNode::Type::Matrix) {
    return SimplificationHelper::Map(*this, context, angleUnit);
  }
#endif
  return Trigonometry::shallowReduceDirectFunction(*this, context, angleUnit);
}

}
