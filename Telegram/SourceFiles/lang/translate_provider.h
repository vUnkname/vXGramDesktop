/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <translate_provider.h>

class PeerData;
struct MsgId;

namespace Main {
class Session;
} // namespace Main

namespace Ui {

// `automatic` is true for the auto-translate of whole chats (as opposed to
// a manual translation of one message). `article` is true for messages
// with an Instant View page (uses the separate article provider if enabled).
[[nodiscard]] std::unique_ptr<TranslateProvider> CreateTranslateProvider(
	not_null<Main::Session*> session,
	bool automatic = false,
	bool article = false);

[[nodiscard]] TranslateProviderRequest PrepareTranslateProviderRequest(
	not_null<TranslateProvider*> provider,
	not_null<PeerData*> peer,
	MsgId msgId,
	TextWithEntities text);

} // namespace Ui
