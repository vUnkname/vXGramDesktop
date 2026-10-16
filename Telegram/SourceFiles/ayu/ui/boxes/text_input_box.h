// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

namespace Ui {
class GenericBox;
} // namespace Ui

namespace AyuUi {

struct TextInputBoxArgs {
	rpl::producer<QString> title;
	rpl::producer<QString> placeholder;
	QString description;
	QString helpLinkText;
	QString helpLinkUrl;
	QString value;
	bool multiline = false;
	bool allowEmpty = true;
	bool showReset = false;
	Fn<bool(const QString &)> validate;
	Fn<void(const QString &)> save;
};

// A simple "enter a value" box used by Ayu settings (API keys, prompts, etc).
void TextInputBox(
	not_null<Ui::GenericBox*> box,
	TextInputBoxArgs &&args);

} // namespace AyuUi
