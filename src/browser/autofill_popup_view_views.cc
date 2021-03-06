// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/nw/src/browser/autofill_popup_view_views.h"

#include "content/nw/src/browser/autofill_popup_controller.h"
#include "components/autofill/core/browser/popup_item_ids.h"
#include "grit/ui_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/text_utils.h"
#include "ui/views/border.h"
#include "ui/views/widget/widget.h"

namespace autofill {

AutofillPopupViewViews::AutofillPopupViewViews(
    AutofillPopupController* controller, views::Widget* observing_widget)
    : AutofillPopupBaseView(controller, observing_widget),
      controller_(controller) {}

AutofillPopupViewViews::~AutofillPopupViewViews() {}

void AutofillPopupViewViews::Show() {
  DoShow();
}

void AutofillPopupViewViews::Hide() {
  // The controller is no longer valid after it hides us.
  controller_ = NULL;

  DoHide();
}

void AutofillPopupViewViews::UpdateBoundsAndRedrawPopup() {
  DoUpdateBoundsAndRedrawPopup();
}

void AutofillPopupViewViews::OnPaint(gfx::Canvas* canvas) {
  if (!controller_)
    return;

  canvas->DrawColor(kPopupBackground);
  OnPaintBorder(canvas);

  for (size_t i = 0; i < controller_->names().size(); ++i) {
    gfx::Rect line_rect = controller_->GetRowBounds(i);

    if (controller_->identifiers()[i] == POPUP_ITEM_ID_SEPARATOR) {
      canvas->DrawRect(line_rect, kItemTextColor);
    } else {
      DrawAutofillEntry(canvas, i, line_rect);
    }
  }
}

void AutofillPopupViewViews::InvalidateRow(size_t row) {
  SchedulePaintInRect(controller_->GetRowBounds(row));
}

void AutofillPopupViewViews::DrawAutofillEntry(gfx::Canvas* canvas,
                                               int index,
                                               const gfx::Rect& entry_rect) {
  if (controller_->selected_line() == index)
    canvas->FillRect(entry_rect, kHoveredBackgroundColor);

  const bool is_rtl = controller_->IsRTL();
  const int value_text_width =
      gfx::GetStringWidth(controller_->names()[index],
                          controller_->GetNameFontListForRow(index));
  const int value_content_x = is_rtl ?
      entry_rect.width() - value_text_width - kEndPadding : kEndPadding;

  canvas->DrawStringRectWithFlags(
      controller_->names()[index],
      controller_->GetNameFontListForRow(index),
      controller_->IsWarning(index) ? kWarningTextColor : kValueTextColor,
      gfx::Rect(value_content_x,
                entry_rect.y(),
                value_text_width,
                entry_rect.height()),
      gfx::Canvas::TEXT_ALIGN_CENTER);

  // Use this to figure out where all the other Autofill items should be placed.
  int x_align_left = is_rtl ? kEndPadding : entry_rect.width() - kEndPadding;

  // Draw the Autofill icon, if one exists
  ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
  int row_height = controller_->GetRowBounds(index).height();
  if (!controller_->icons()[index].empty()) {
    int icon = controller_->GetIconResourceID(controller_->icons()[index]);
    DCHECK_NE(-1, icon);
    const gfx::ImageSkia* image = rb.GetImageSkiaNamed(icon);
    int icon_y = entry_rect.y() + (row_height - image->height()) / 2;

    x_align_left += is_rtl ? 0 : -image->width();

    canvas->DrawImageInt(*image, x_align_left, icon_y);

    x_align_left += is_rtl ? image->width() + kIconPadding : -kIconPadding;
  }

  // Draw the name text.
  const int subtext_width =
      gfx::GetStringWidth(controller_->subtexts()[index],
                          controller_->subtext_font_list());
  if (!is_rtl)
    x_align_left -= subtext_width;

  canvas->DrawStringRectWithFlags(
      controller_->subtexts()[index],
      controller_->subtext_font_list(),
      kItemTextColor,
      gfx::Rect(x_align_left,
                entry_rect.y(),
                subtext_width,
                entry_rect.height()),
      gfx::Canvas::TEXT_ALIGN_CENTER);
}

AutofillPopupView* AutofillPopupView::Create(
    AutofillPopupController* controller) {
  views::Widget* observing_widget =
      views::Widget::GetTopLevelWidgetForNativeView(
          controller->container_view());

  // If the top level widget can't be found, cancel the popup since we can't
  // fully set it up.
  if (!observing_widget)
    return NULL;

  return new AutofillPopupViewViews(controller, observing_widget);
}

}  // namespace autofill
