// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "settings/settings_builder.h"

namespace Settings::TranslatorUi {

struct TextValueArgs {
	QString id;
	rpl::producer<QString> title;
	QString boxTitle;
	QString placeholder;
	QString description;
	QString helpUrl;
	// Evaluated when the box opens, overrides `helpUrl` if set.
	Fn<QString()> helpUrlGetter;
	bool multiline = false;
	bool secret = false;
	bool showReset = false;
	// Shown instead of "None" for an empty value.
	QString emptyLabel;
	// Current value (reactive, drives the right label) and its snapshot.
	rpl::producer<QString> value;
	Fn<QString()> getter;
	Fn<void(const QString &)> setter;
	Fn<bool(const QString &)> validate;
	rpl::producer<bool> shown;
	QStringList keywords;
};

// Settings button which opens a text input box and shows the current value
// (or "Set" / "None" for secrets) as the right label.
Ui::SettingsButton *AddTextValueButton(
	Builder::SectionBuilder &builder,
	TextValueArgs &&args);

} // namespace Settings::TranslatorUi
