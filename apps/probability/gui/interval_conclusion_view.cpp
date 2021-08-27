#include "interval_conclusion_view.h"

#include <apps/i18n.h>
#include <kandinsky/font.h>

#include "probability/text_helpers.h"

namespace Probability {

IntervalConclusionView::IntervalConclusionView() {
  m_messageView.setMessage(I18n::Message::ConfidenceInterval);
  m_messageView.setAlignment(0.5f, 0.5f);
  m_messageView.setBackgroundColor(Escher::Palette::WallScreen);
  m_intervalView.setFont(KDFont::LargeFont);
  m_intervalView.setAlignment(0.5f, 0.5f);
  m_intervalView.setBackgroundColor(Escher::Palette::WallScreen);
}

void IntervalConclusionView::setInterval(float center, float marginOfError) {
  constexpr static int bufferSize = Constants::k_shortBufferSize;
  char bufferCenter[bufferSize];
  char buffermarginOfError[bufferSize];

  defaultConvertFloatToText(center, bufferCenter, bufferSize);
  defaultConvertFloatToText(marginOfError, buffermarginOfError, bufferSize);

  char buffer[k_intervalBufferSize];
  snprintf(buffer, k_intervalBufferSize, "%s ± %s", bufferCenter, buffermarginOfError);
  m_intervalView.setText(buffer);
}

Escher::View * IntervalConclusionView::subviewAtIndex(int i) {
  assert(i < 2);
  if (i == 0) {
    return &m_messageView;
  }
  return &m_intervalView;
}

}  // namespace Probability
