// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/utils/inline_button_copy.h"

#include "data/data_document.h"
#include "data/data_session.h"
#include "data/stickers/data_custom_emoji.h"
#include "history/history_item_components.h"
#include "lang_auto.h"
#include "lang/lang_keys.h"
#include "ui/text/text_entity.h"
#include "ui/text/text_utilities.h"
#include "ui/widgets/menu/menu_add_action_callback.h"
#include "ui/widgets/menu/menu_add_action_callback_factory.h"
#include "ui/widgets/popup_menu.h"

#include "styles/style_menu_icons.h"

#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>

namespace AyuInlineCopy {
namespace {

void CopyPlainText(const QString &text) {
	QGuiApplication::clipboard()->setText(text);
}

} // namespace

QString FormatEmojiMarkdown(
		const QString &placeholder,
		DocumentId documentId) {
	return QString("![%1](tg://emoji?id=%2)")
		.arg(placeholder)
		.arg(QString::number(documentId));
}

QString EmojiPlaceholder(
		not_null<Data::Session*> owner,
		DocumentId documentId) {
	if (documentId) {
		const auto document = owner->document(documentId);
		if (const auto sticker = document->sticker()) {
			if (!sticker->alt.isEmpty()) {
				return sticker->alt;
			}
		}
	}
	return QString::fromUcs4(U"\U0001F642", 1);
}

void CopyCustomEmoji(
		not_null<Data::Session*> owner,
		DocumentId documentId) {
	const auto placeholder = EmojiPlaceholder(owner, documentId);
	TextUtilities::SetClipboardText(TextForMimeData{
		.expanded = placeholder,
		.rich = Ui::Text::SingleCustomEmoji(
			Data::SerializeCustomEmojiId(documentId),
			placeholder),
	});
}

bool AddInlineButtonCopyAction(
		not_null<Ui::PopupMenu*> menu,
		const ClickHandlerPtr &link) {
	const auto handler = dynamic_cast<ReplyMarkupClickHandler*>(link.get());
	if (!handler) {
		return false;
	}
	const auto button = handler->getUrlButton();
	if (!button) {
		return false;
	}

	using Type = HistoryMessageMarkupButton::Type;
	const auto isCallback = (button->type == Type::Callback)
		|| (button->type == Type::CallbackWithPassword);
	const auto data = QString::fromUtf8(button->data);
	const auto text = handler->buttonText();
	const auto emojiId = handler->customEmojiId();
	const auto owner = handler->owner();

	const auto callback = Ui::Menu::CreateAddActionCallback(menu);
	callback(Ui::Menu::MenuCallback::Args{
		.text = tr::ayu_CopyMenu(tr::now),
		.handler = nullptr,
		.icon = &st::menuIconCopy,
		.fillSubmenu = [=](not_null<Ui::PopupMenu*> submenu)
		{
			if (!data.isEmpty() || isCallback) {
				submenu->addAction(
					isCallback
						? tr::ayu_CopyCallbackData(tr::now)
						: tr::ayu_CopyButtonLink(tr::now),
					[=] { CopyPlainText(data); },
					isCallback ? &st::menuIconCopy : &st::menuIconLink);
			}
			if (!text.isEmpty()) {
				submenu->addAction(
					tr::ayu_CopyButtonText(tr::now),
					[=] { CopyPlainText(text); },
					&st::menuIconEdit);
			}
			if (emojiId) {
				submenu->addSeparator();
				submenu->addAction(
					tr::ayu_CopyEmoji(tr::now),
					[=] { CopyCustomEmoji(owner, emojiId); },
					&st::menuIconStickers);
				submenu->addAction(
					tr::ayu_CopyEmojiId(tr::now),
					[=] { CopyPlainText(QString::number(emojiId)); },
					&st::menuIconInfo);
				submenu->addAction(
					tr::ayu_CopyEmojiMarkdown(tr::now),
					[=] {
						CopyPlainText(FormatEmojiMarkdown(
							EmojiPlaceholder(owner, emojiId),
							emojiId));
					},
					&st::menuIconCopy);
			}
		},
	});
	return true;
}

} // namespace AyuInlineCopy
