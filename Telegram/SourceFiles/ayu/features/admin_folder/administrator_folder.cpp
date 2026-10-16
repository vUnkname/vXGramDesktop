// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/admin_folder/administrator_folder.h"

#include "ayu/ayu_settings.h"
#include "apiwrap.h"
#include "boxes/premium_limits_box.h"
#include "data/data_channel.h"
#include "data/data_chat.h"
#include "data/data_chat_filters.h"
#include "data/data_folder.h"
#include "data/data_peer.h"
#include "data/data_premium_limits.h"
#include "data/data_session.h"
#include "dialogs/dialogs_indexed_list.h"
#include "dialogs/dialogs_main_list.h"
#include "dialogs/dialogs_row.h"
#include "history/history.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"
#include "mtproto/mtproto_response.h"
#include "ui/filter_icons.h"
#include "ui/layers/generic_box.h"
#include "window/window_session_controller.h"

namespace AyuFeatures::AdminFolder {
namespace {

[[nodiscard]] bool IsAdminPeer(not_null<PeerData*> peer) {
	if (const auto channel = peer->asChannel()) {
		return channel->amCreator() || channel->hasAdminRights();
	} else if (const auto chat = peer->asChat()) {
		return chat->amCreator() || chat->hasAdminRights();
	}
	return false;
}

[[nodiscard]] base::flat_set<not_null<History*>> CollectAdminHistories(
		not_null<Main::Session*> session) {
	auto result = base::flat_set<not_null<History*>>();
	const auto add = [&](not_null<Dialogs::MainList*> list) {
		for (const auto &row : list->indexed()->all()) {
			if (const auto history = row->history()) {
				if (IsAdminPeer(history->peer)) {
					result.emplace(history);
				}
			}
		}
	};
	add(session->data().chatsList());
	if (const auto folder = session->data().folderLoaded(Data::Folder::kId)) {
		add(folder->chatsList());
	}
	return result;
}

[[nodiscard]] int CloudFiltersCount(const std::vector<Data::ChatFilter> &list) {
	return int(ranges::count_if(list, [](const Data::ChatFilter &filter) {
		return filter.id() != FilterId();
	}));
}

} // namespace

bool Exists(not_null<Main::Session*> session) {
	const auto id = AyuSettings::getInstance().administratorFolderId(
		session->userId().bare);
	if (id <= 0) {
		return false;
	}
	const auto &list = session->data().chatsFilters().list();
	return ranges::contains(list, FilterId(id), &Data::ChatFilter::id);
}

rpl::producer<bool> ExistsValue(not_null<Main::Session*> session) {
	return rpl::single(rpl::empty_value()) | rpl::then(
		session->data().chatsFilters().changed()
	) | rpl::map([=] {
		return Exists(session);
	}) | rpl::distinct_until_changed();
}

void Apply(not_null<Window::SessionController*> controller) {
	const auto session = &controller->session();
	auto &filters = session->data().chatsFilters();
	if (!filters.loaded()) {
		controller->showToast(tr::ayu_AdministratorFolderNotReady(tr::now));
		return;
	}

	const auto histories = CollectAdminHistories(session);
	if (histories.empty()) {
		controller->showToast(tr::ayu_AdministratorFolderNoChats(tr::now));
		return;
	}

	const auto limits = Data::PremiumLimits(session);
	if (int(histories.size()) > limits.dialogFiltersChatsCurrent()) {
		controller->show(Box(
			FilterChatsLimitBox,
			session,
			int(histories.size()),
			true));
		return;
	}

	const auto &list = filters.list();
	const auto userId = session->userId().bare;
	const auto savedId = FilterId(
		AyuSettings::getInstance().administratorFolderId(userId));
	const auto existing = (savedId > 0)
		? ranges::find(list, savedId, &Data::ChatFilter::id)
		: list.end();
	const auto updating = (existing != list.end());

	if (!updating) {
		const auto count = CloudFiltersCount(list);
		if (count >= limits.dialogFiltersCurrent()) {
			controller->show(Box(FiltersLimitBox, session, count));
			return;
		}
	}

	auto newId = savedId;
	auto filter = Data::ChatFilter();
	if (updating) {
		auto always = histories;
		auto pinned = std::vector<not_null<History*>>();
		for (const auto &history : existing->pinned()) {
			if (always.contains(history)) {
				pinned.push_back(history);
			}
		}
		filter = Data::ChatFilter(
			existing->id(),
			existing->title(),
			existing->iconEmoji(),
			existing->colorIndex(),
			existing->flags(),
			std::move(always),
			std::move(pinned),
			existing->never());
		if (filter == *existing) {
			controller->showToast(tr::ayu_AdministratorFolderUpdated(
				tr::now,
				lt_count,
				QString::number(histories.size())));
			return;
		}
	} else {
		auto localId = 1;
		while (ranges::contains(list, localId, &Data::ChatFilter::id)) {
			++localId;
		}
		++localId;
		while (ranges::contains(list, localId, &Data::ChatFilter::id)) {
			++localId;
		}
		newId = localId;
		filter = Data::ChatFilter(
			newId,
			Data::ChatFilterTitle{
				.text = TextWithEntities{
					.text = tr::ayu_AdministratorFolderTitle(tr::now),
				},
			},
			Ui::LookupFilterIcon(Ui::FilterIcon::Crown).emoji,
			std::nullopt,
			Data::ChatFilter::Flags(),
			histories,
			std::vector<not_null<History*>>(),
			base::flat_set<not_null<History*>>());
	}

	auto order = std::vector<FilterId>();
	if (!updating) {
		order.reserve(list.size() + 2);
		for (const auto &item : list) {
			order.push_back(item.id());
		}
		if (!ranges::contains(order, FilterId(0))) {
			// "All chats" may be hidden locally, keep it first on the server.
			order.insert(order.begin(), FilterId(0));
		}
		order.push_back(newId);
	}

	const auto tl = filter.tl(newId);
	const auto count = int(histories.size());
	filters.apply(MTP_updateDialogFilter(
		MTP_flags(MTPDupdateDialogFilter::Flag::f_filter),
		MTP_int(newId),
		tl));

	const auto requestId = session->api().request(
		MTPmessages_UpdateDialogFilter(
			MTP_flags(MTPmessages_UpdateDialogFilter::Flag::f_filter),
			MTP_int(newId),
			tl)
	).done(crl::guard(controller, [=] {
		AyuSettings::getInstance().setAdministratorFolderId(userId, newId);
		controller->showToast(updating
			? tr::ayu_AdministratorFolderUpdated(
				tr::now,
				lt_count,
				QString::number(count))
			: tr::ayu_AdministratorFolderCreated(
				tr::now,
				lt_count,
				QString::number(count)));
	})).fail(crl::guard(controller, [=](const MTP::Error &error) {
		session->data().chatsFilters().reload();
		controller->showToast(error.type());
	})).send();

	if (!updating) {
		filters.saveOrder(order, requestId);
	}
}

} // namespace AyuFeatures::AdminFolder
