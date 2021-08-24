#include "statistic_graph_controller.h"

#include <escher/clipboard.h>

#include "probability/app.h"
#include "probability/text_helpers.h"

namespace Probability {

StatisticGraphController::StatisticGraphController(StackViewController * stack,
                                                   Statistic * statistic) :
    Page(stack), m_graphView(statistic, &m_range, this), m_statistic(statistic) {
}

ViewController::TitlesDisplay StatisticGraphController::titlesDisplay() {
  bool isCategoricalGraph = App::app()->subapp() == Data::SubApp::Tests &&
                            App::app()->test() == Data::Test::Categorical;
  return isCategoricalGraph ? ViewController::TitlesDisplay::DisplayLastTwoTitles
                            : ViewController::TitlesDisplay::DisplayLastThreeTitles;
}

const char * StatisticGraphController::title() {
  if (App::app()->subapp() == Data::SubApp::Tests) {
    char zBuffer[10];
    char pBuffer[10];
    defaultParseFloat(m_statistic->testCriticalValue(), zBuffer, sizeof(zBuffer));
    defaultParseFloat(m_statistic->pValue(), pBuffer, sizeof(pBuffer));
    const char * format = I18n::translate(I18n::Message::StatisticGraphControllerTestTitleFormat);
    snprintf(m_titleBuffer, sizeof(m_titleBuffer), format, zBuffer, pBuffer);
  } else {
    const char * format = I18n::translate(
        I18n::Message::StatisticGraphControllerIntervalTitleFormat);
    char MEBuffer[30];
    defaultParseFloat(m_statistic->marginOfError(), MEBuffer, sizeof(MEBuffer));
    snprintf(m_titleBuffer, sizeof(m_titleBuffer), format, MEBuffer);
  }
  return m_titleBuffer;
}

void StatisticGraphController::didBecomeFirstResponder() {
  App::app()->setPage(Data::Page::Graph);
  m_graphView.setStatistic(m_statistic);
  m_range.setStatistic(m_statistic);
  if (App::app()->subapp() == Data::SubApp::Intervals) {
    m_graphView.intervalConclusionView()->setInterval(m_statistic->estimate(),
                                                      m_statistic->marginOfError());
  }
  m_graphView.reload();
}

bool StatisticGraphController::handleEvent(Ion::Events::Event event) {
  if (App::app()->subapp() == Data::SubApp::Intervals && event == Ion::Events::Copy) {
    // Copy confidence interval as matrix
    char buffer[40];
    char lowerBound[20];
    char upperBound[20];
    defaultParseFloat(m_statistic->estimate() - m_statistic->marginOfError(),
                      lowerBound,
                      sizeof(lowerBound));
    defaultParseFloat(m_statistic->estimate() + m_statistic->marginOfError(),
                      upperBound,
                      sizeof(upperBound));
    snprintf(buffer, sizeof(buffer), "[[%s,%s]]", lowerBound, upperBound);
    Escher::Clipboard::sharedClipboard()->store(buffer, strlen(buffer));
    return true;
  }
  return false;
}

bool StatisticGraphController::shouldPositionLegendLeft() {
  return m_statistic->hypothesisParams()->op() == HypothesisParams::ComparisonOperator::Lower;
}

}  // namespace Probability
