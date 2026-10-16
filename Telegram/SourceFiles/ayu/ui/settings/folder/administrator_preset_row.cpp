// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/ui/settings/folder/administrator_preset_row.h"

#include "lang/lang_keys.h"
#include "ui/filter_icons.h"
#include "ui/painter.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"

#include "styles/palette.h"
#include "styles/style_ayu_settings.h"
#include "styles/style_boxes.h"
#include "styles/style_settings.h"
#include "styles/style_widgets.h"

namespace Settings {
namespace {

[[nodiscard]] QString SubtitleText(bool created) {
	auto result = tr::ayu_AdministratorFolderSubtitle(tr::now);
	if (created) {
		result += ' ' + tr::ayu_AdministratorFolderRefreshHint(tr::now);
	}
	return result;
}

} // namespace

AdministratorPresetRow::AdministratorPresetRow(not_null<QWidget*> parent)
: RpWidget(parent)
, _title(Ui::CreateChild<Ui::FlatLabel>(
	this,
	tr::ayu_AdministratorFolderTitle(),
	st::ayuAdminPresetTitle))
, _subtitle(Ui::CreateChild<Ui::FlatLabel>(
	this,
	_subtitleText.value(),
	st::ayuAdminPresetSubtitle))
, _add(Ui::CreateChild<Ui::RoundButton>(
	this,
	tr::lng_filters_recommended_add(),
	st::settingsFilterAddRecommended))
, _refresh(Ui::CreateChild<Ui::IconButton>(
	this,
	st::ayuAdminPresetRefresh)) {
	_add->setFullRadius(true);
	_subtitleText = SubtitleText(false);
	_title->setAttribute(Qt::WA_TransparentForMouseEvents);
	_subtitle->setAttribute(Qt::WA_TransparentForMouseEvents);
	updateButtons();
}

void AdministratorPresetRow::setCreated(bool created) {
	if (_created == created) {
		return;
	}
	_created = created;
	_subtitleText = SubtitleText(created);
	updateButtons();
	if (width() > 0) {
		resizeToWidth(width());
	}
}

rpl::producer<> AdministratorPresetRow::addRequests() const {
	return _add->clicks() | rpl::to_empty;
}

rpl::producer<> AdministratorPresetRow::refreshRequests() const {
	return _refresh->clicks() | rpl::to_empty;
}

void AdministratorPresetRow::updateButtons() {
	_add->setVisible(!_created);
	_refresh->setVisible(_created);
	update();
}

int AdministratorPresetRow::resizeGetHeight(int newWidth) {
	const auto left = st::settingsButtonActive.padding.left();
	const auto right = st::contactsPadding.right()
		+ st::contactsCheckPosition.x();
	const auto buttonsWidth = std::max(_add->width(), _refresh->width());
	const auto textWidth = std::max(
		newWidth - left - right - buttonsWidth - right,
		st::ayuAdminPresetTextMinWidth);

	_title->resizeToWidth(textWidth);
	_subtitle->resizeToWidth(textWidth);

	const auto padding = st::ayuAdminPresetPadding;
	const auto textHeight = _title->heightNoMargins()
		+ st::ayuAdminPresetTitleSkip
		+ _subtitle->heightNoMargins();
	const auto height = std::max(
		st::defaultPeerListItem.height,
		padding.top() + textHeight + padding.bottom());

	_title->moveToLeft(left, (height - textHeight) / 2, newWidth);
	_subtitle->moveToLeft(
		left,
		_title->y() + _title->heightNoMargins() + st::ayuAdminPresetTitleSkip,
		newWidth);
	_add->moveToRight(right, (height - _add->height()) / 2, newWidth);
	_refresh->moveToRight(
		right - st::ayuAdminPresetRefreshSkip,
		(height - _refresh->height()) / 2,
		newWidth);
	return height;
}

void AdministratorPresetRow::paintEvent(QPaintEvent *e) {
	auto p = Painter(this);

	const auto icon = Ui::LookupFilterIcon(Ui::FilterIcon::Crown).normal;

	auto hq = PainterHighQualityEnabler(p);
	const auto iconWidth = icon->width() - style::ConvertScale(9);
	const auto scale = st::settingsIconAdd.width() / float64(iconWidth);
	p.translate(
		st::settingsButtonActive.iconLeft,
		(height() - icon->height() * scale) / 2);
	p.translate(-iconWidth / 2, -iconWidth / 2);
	p.scale(scale, scale);
	p.translate(iconWidth / 2, iconWidth / 2);
	icon->paint(p, 0, 0, width(), st::activeButtonBg->c);
}

} // namespace Settings
