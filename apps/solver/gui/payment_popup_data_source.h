#ifndef SOLVER_GUI_PAYMENT_POPUP_DATA_SOURCE_H
#define SOLVER_GUI_PAYMENT_POPUP_DATA_SOURCE_H

#include <escher/list_view_data_source.h>

#include <escher/buffer_text_highlight_cell.h>

namespace Solver {

class PaymentPopupDataSource : public Escher::ListViewDataSource {
public:
  PaymentPopupDataSource() {}
  int numberOfRows() const override { return k_numberOfRows; }
  KDCoordinate rowHeight(int r) override {
    assert(false); /* Not needed because DropdownPopupController takes care of it */
    return 0;
  }
  int reusableCellCount(int type) override { return k_numberOfRows; }
  Escher::BufferTextHighlightCell * reusableCell(int i, int type) override { return &m_cells[i]; }
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override {
    Escher::BufferTextHighlightCell * bufferCell = static_cast<Escher::BufferTextHighlightCell *>(cell);
    bufferCell->setText(index == 0 ? I18n::translate(I18n::Message::FinanceBeginning) : I18n::translate(I18n::Message::FinanceEnd));
  }

private:
  constexpr static int k_numberOfRows = 2;
  Escher::BufferTextHighlightCell m_cells[k_numberOfRows];
};

}  // namespace Solver

#endif /* SOLVER_GUI_PAYMENT_POPUP_DATA_SOURCE_H */
