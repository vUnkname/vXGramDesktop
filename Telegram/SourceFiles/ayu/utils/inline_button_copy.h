// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "data/data_types.h"
#include "ui/click_handler.h"

namespace Ui {
class PopupMenu;
} // namespace Ui

namespace Data {
class Session;
} // namespace Data

namespace AyuInlineCopy {

// Markdown-like representation of a custom emoji,
// e.g. "![🌅](tg://emoji?id=5303400229549135579)".
[[nodiscard]] QString FormatEmojiMarkdown(
	const QString &placeholder,
	DocumentId documentId);

// Returns the visible unicode fallback of a custom emoji (sticker alt),
// or a generic emoji if the document is not loaded yet.
[[nodiscard]] QString EmojiPlaceholder(
	not_null<Data::Session*> owner,
	DocumentId documentId);

// Puts the custom emoji to the clipboard in a way that pasting it into
// a Telegram message field keeps the emoji style.
void CopyCustomEmoji(
	not_null<Data::Session*> owner,
	DocumentId documentId);

// Adds a "Copy" entry with a submenu (Callback Data / Link, Button Text,
// Emoji, Emoji ID, Emoji Markdown) for an inline keyboard button.
// Returns false if the link is not an inline keyboard button
// (nothing is added in that case).
bool AddInlineButtonCopyAction(
	not_null<Ui::PopupMenu*> menu,
	const ClickHandlerPtr &link);

} // namespace AyuInlineCopy
