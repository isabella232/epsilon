#ifndef REGRESSION_POWER_MODEL_H
#define REGRESSION_POWER_MODEL_H

#include "model.h"

namespace Regression {

class PowerModel : public Model {
public:
  using Model::Model;
  Poincare::Layout layout() override;
  I18n::Message formulaMessage() const override { return I18n::Message::PowerRegressionFormula; }
  double evaluate(double * modelCoefficients, double x) const override;
  double levelSet(double * modelCoefficients, double xMin, double xMax, double y, Poincare::Context * context) override;
  double partialDerivate(double * modelCoefficients, int derivateCoefficientIndex, double x) const override;
  void fit(Store * store, int series, double * modelCoefficients, Poincare::Context * context) override;
  int numberOfCoefficients() const override { return 2; }
protected:
  bool dataSuitableForFit(Store * store, int series) const override;
};

}


#endif
