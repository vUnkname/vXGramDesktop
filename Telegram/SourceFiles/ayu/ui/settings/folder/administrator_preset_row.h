// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "ui/rp_widget.h"

namespace Ui {
class FlatLabel;
class RoundButton;
class IconButton;
} // namespace Ui

namespace Settings {

// "Administrator" preset: crown icon, title, subtitle and a blue "Add"
// button that turns into a "Refresh" icon button once the folder exists.
class AdministratorPresetRow final : public Ui::RpWidget {
public:
	explicit AdministratorPresetRow(not_null<QWidget*> parent);

	void setCreated(bool created);

	[[nodiscard]] rpl::producer<> addRequests() const;
	[[nodiscard]] rpl::producer<> refreshRequests() const;

protected:
	int resizeGetHeight(int newWidth) override;
	void paintEvent(QPaintEvent *e) override;

private:
	void updateButtons();

	const not_null<Ui::FlatLabel*> _title;
	const not_null<Ui::FlatLabel*> _subtitle;
	const not_null<Ui::RoundButton*> _add;
	const not_null<Ui::IconButton*> _refresh;
	rpl::variable<QString> _subtitleText;
	bool _created = false;

};

} // namespace Settings
